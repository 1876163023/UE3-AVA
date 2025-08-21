/*=============================================================================
	StaticLightingTextureMapping.cpp: Static lighting texture mapping implementation.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "StaticLightingPrivate.h"
#include "UnRaster.h"

// Don't compile the static lighting system on consoles.
#if !CONSOLE

/** A map from light-map texels to the world-space surface points which map the texels. */
class FTexelToVertexMap
{
public:

	/** A map from a texel to the world-space surface point which maps the texel. */
	struct FTexelToVertex
	{
		FStaticLightingVertex Vertex;
		FVector SurfaceNormal;
		FLOAT TotalSampleWeight;
	};

	/** The mapping data. */
	TArray<FTexelToVertex> Data;

	/** Initialization constructor. */
	FTexelToVertexMap(INT InSizeX,INT InSizeY)
	{
		Data.Empty(InSizeX * InSizeY);
		Data.AddZeroed(InSizeX * InSizeY);
	}
};

/** Used to map static lighting texels to vertices. */
class FStaticLightingRasterPolicy
{
public:

	typedef FStaticLightingVertex InterpolantType;

    /** Initialization constructor. */
	FStaticLightingRasterPolicy(
		const FStaticLightingTextureMapping* InMapping,
		FTexelToVertexMap& InTexelToVertexMap,
		const FVector& InNormal,
		FLOAT InSampleWeight
		):
		Mapping(InMapping),
		TexelToVertexMap(InTexelToVertexMap),
		Normal(InNormal),
		SampleWeight(InSampleWeight)
	{
	}

protected:

	// FTriangleRasterizer policy interface.

	INT GetMinX() const { return 0; }
	INT GetMaxX() const { return Mapping->SizeX - 1; }
	INT GetMinY() const { return 0; }
	INT GetMaxY() const { return Mapping->SizeY - 1; }

	void ProcessPixel(INT X,INT Y,const InterpolantType& Interpolant,UBOOL BackFacing);

private:

	/** The mapping which is being rasterized. */
    const FStaticLightingTextureMapping* Mapping;
	
	/** The texel to vertex map which is being rasterized to. */
	FTexelToVertexMap& TexelToVertexMap;

	/** The normal of the triangle being rasterized. */
	const FVector Normal;

	/** The weight of the current sample. */
	const FLOAT SampleWeight;
};

void FStaticLightingRasterPolicy::ProcessPixel(INT X,INT Y,const InterpolantType& Vertex,UBOOL BackFacing)
{
	FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap.Data(Y * Mapping->SizeX + X);

	// Add this sample to the mapping.
	TexelToVertex.Vertex = TexelToVertex.Vertex + Vertex * SampleWeight;
	TexelToVertex.SurfaceNormal += Normal * SampleWeight;
	TexelToVertex.TotalSampleWeight += SampleWeight;
}

void FStaticLightingSystem::ProcessTextureMapping(FStaticLightingTextureMapping* TextureMapping)
{
	TMap<ULightComponent*,FShadowMapData2D*> ShadowMaps;
	FCoherentRayCache CoherentRayCache;

	// Allocate light-map data.
	FLightMapData2D* LightMapData = new FLightMapData2D(TextureMapping->SizeX,TextureMapping->SizeY);

	// Allocate the texel to vertex map.
	FTexelToVertexMap TexelToVertexMap(TextureMapping->SizeX,TextureMapping->SizeY);

	// Rasterize the triangles into the texel to vertex map.
	// Use multiple samples to approximate convolving the triangle by the bilinear filter used to sample the light-map texture.
	const UINT NumBilinearFilterSamples = 16;
	FRandomStream SampleGenerator(0);
	for(INT SampleIndex = 0;SampleIndex < NumBilinearFilterSamples;SampleIndex++)
	{
		// Randomly sample the bilinear filter function.
		const FLOAT SampleX = -1.0f + 2.0f * SampleGenerator.GetFraction();
		const FLOAT SampleY = -1.0f + 2.0f * SampleGenerator.GetFraction();
		const FLOAT SampleWeight = (1.0f - Abs(SampleX)) * (1.0f - Abs(SampleY));

		// Rasterize the triangles offset by the random sample location.
		for(INT TriangleIndex = 0;TriangleIndex < TextureMapping->Mesh->NumTriangles;TriangleIndex++)
		{
			// Query the mesh for the triangle's vertices.
			FStaticLightingVertex V0;
			FStaticLightingVertex V1;
			FStaticLightingVertex V2;
			TextureMapping->Mesh->GetTriangle(TriangleIndex,V0,V1,V2);

			// Rasterize the triangle using its the mapping's texture coordinate channel.
			FTriangleRasterizer<FStaticLightingRasterPolicy> TexelMappingRasterizer(FStaticLightingRasterPolicy(
					TextureMapping,
					TexelToVertexMap,
					((V2.WorldPosition - V0.WorldPosition) ^ (V1.WorldPosition - V0.WorldPosition)).SafeNormal(),
					SampleWeight
					));
			TexelMappingRasterizer.DrawTriangle(
				V0,
				V1,
				V2,
				V0.TextureCoordinates[TextureMapping->TextureCoordinateIndex] * FVector2D(TextureMapping->SizeX,TextureMapping->SizeY) + FVector2D(-0.5f + SampleX,-0.5f + SampleY),
				V1.TextureCoordinates[TextureMapping->TextureCoordinateIndex] * FVector2D(TextureMapping->SizeX,TextureMapping->SizeY) + FVector2D(-0.5f + SampleX,-0.5f + SampleY),
				V2.TextureCoordinates[TextureMapping->TextureCoordinateIndex] * FVector2D(TextureMapping->SizeX,TextureMapping->SizeY) + FVector2D(-0.5f + SampleX,-0.5f + SampleY),
				FALSE
				);
		}
	}

	for(INT LightIndex = 0;LightIndex < TextureMapping->Mesh->RelevantLights.Num();LightIndex++)
	{
		ULightComponent* Light = TextureMapping->Mesh->RelevantLights(LightIndex);
		const FVector4 LightPosition = Light->GetPosition();

		// Raytrace the texels of the shadow-map that map to vertices on a world-space surface.
		FShadowMapData2D ShadowMapData(TextureMapping->SizeX,TextureMapping->SizeY);
		for(INT Y = 0;Y < TextureMapping->SizeY;Y++)
		{
			for(INT X = 0;X < TextureMapping->SizeX;X++)
			{
				const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap.Data(Y * TextureMapping->SizeX + X);
				if(TexelToVertex.TotalSampleWeight > DELTA)
				{
					FShadowSample& ShadowSample = ShadowMapData(X,Y);
					ShadowSample.IsMapped = TRUE;

					// Average the texel's vertex samples.
					const FStaticLightingVertex Vertex = TexelToVertex.Vertex / TexelToVertex.TotalSampleWeight;
					const FVector SurfaceNormal = TexelToVertex.SurfaceNormal / TexelToVertex.TotalSampleWeight;

					// Calculate the direction from the vertex to the light.
					const FVector WorldLightVector = (FVector)LightPosition - Vertex.WorldPosition * LightPosition.W;

					// Check if the light is in front of the surface.
					const UBOOL bLightIsInFrontOfTriangle = (WorldLightVector | SurfaceNormal) > 0.0f;
					if(bLightIsInFrontOfTriangle)
					{
						// Compute the shadow factors for this sample from the shadow-mapped lights.
						ShadowSample.Visibility = CalculatePointShadowing(TextureMapping,Vertex.WorldPosition,Light,CoherentRayCache) ? 0.0f : 1.0f;
					}
				}
			}
		}

		// Filter the shadow-map, and detect completely occluded lights.
		FShadowMapData2D* FilteredShadowMapData = new FShadowMapData2D(TextureMapping->SizeX,TextureMapping->SizeY);;
		UBOOL bIsCompletelyOccluded = TRUE;
		for(INT Y = 0;Y < TextureMapping->SizeY;Y++)
		{
			for(INT X = 0;X < TextureMapping->SizeX;X++)
			{
				if(ShadowMapData(X,Y).IsMapped)
				{
					// The shadow-map filter.
					static const UINT FilterSizeX = 5;
					static const UINT FilterSizeY = 5;
					static const UINT FilterMiddleX = (FilterSizeX - 1) / 2;
					static const UINT FilterMiddleY = (FilterSizeY - 1) / 2;
					static const UINT Filter[5][5] =
					{
						{ 58,  85,  96,  85, 58 },
						{ 85, 123, 140, 123, 85 },
						{ 96, 140, 159, 140, 96 },
						{ 85, 123, 140, 123, 85 },
						{ 58,  85,  96,  85, 58 }
					};

					// Gather the filtered samples for this texel.
					UINT Visibility = 0;
					UINT Coverage = 0;
					for(UINT FilterY = 0;FilterY < FilterSizeX;FilterY++)
					{
						for(UINT FilterX = 0;FilterX < FilterSizeY;FilterX++)
						{
							INT	SubX = (INT)X - FilterMiddleX + FilterX,
								SubY = (INT)Y - FilterMiddleY + FilterY;
							if(SubX >= 0 && SubX < (INT)TextureMapping->SizeX && SubY >= 0 && SubY < (INT)TextureMapping->SizeY)
							{
								if(ShadowMapData(SubX,SubY).IsMapped)
								{
									Visibility += Filter[FilterX][FilterY] * ShadowMapData(SubX,SubY).Visibility;
									Coverage += Filter[FilterX][FilterY];
								}
							}
						}
					}

					// Keep track of whether any texels have an unoccluded view of the light.
					if(Visibility > 0)
					{
						bIsCompletelyOccluded = FALSE;
					}

					// Write the filtered shadow-map texel.
					(*FilteredShadowMapData)(X,Y).Visibility = (FLOAT)Visibility / (FLOAT)Coverage;
					(*FilteredShadowMapData)(X,Y).IsMapped = TRUE;
				}
				else
				{
					(*FilteredShadowMapData)(X,Y).IsMapped = FALSE;
				}
			}
		}

		if(bIsCompletelyOccluded)
		{
			// If the light is completely occluded, discard the shadow-map.
			delete FilteredShadowMapData;
			FilteredShadowMapData = NULL;
		}
		else
		{
			// Check whether the light should use a light-map or shadow-map.
			const UBOOL bUseStaticLighting = Light->UseStaticLighting(TextureMapping->bForceDirectLightMap);
			if(bUseStaticLighting)
			{
				// Convert the shadow-map into a light-map.
				for(INT Y = 0;Y < TextureMapping->SizeY;Y++)
				{
					for(INT X = 0;X < TextureMapping->SizeX;X++)
					{
						if((*FilteredShadowMapData)(X,Y).IsMapped)
						{
							(*LightMapData)(X,Y).bIsMapped = TRUE;

							// Compute the light sample for this texel based on the corresponding vertex and its shadow factor.
							FLOAT ShadowFactor = (*FilteredShadowMapData)(X,Y).Visibility;
							if(ShadowFactor > 0.0f)
							{
								const FTexelToVertexMap::FTexelToVertex& TexelToVertex = TexelToVertexMap.Data(Y * TextureMapping->SizeX + X);
								check(TexelToVertex.TotalSampleWeight > DELTA);

								// Average the texel's vertex samples.
								const FStaticLightingVertex TexelVertex = TexelToVertex.Vertex / TexelToVertex.TotalSampleWeight;

								(*LightMapData)(X,Y).AddWeighted(CalculatePointLighting(TextureMapping,TexelVertex,Light),ShadowFactor);
							}
						}
					}
				}

				// Add the light to the light-map's light list.
				LightMapData->Lights.AddItem(Light);

				// Free the shadow-map.
				delete FilteredShadowMapData;
			}
			else
			{
				ShadowMaps.Set(Light,FilteredShadowMapData);
			}
		}
	}

	// Enqueue the static lighting for application in the main thread.
	TList<FTextureMappingStaticLightingData>* StaticLightingLink = new TList<FTextureMappingStaticLightingData>(FTextureMappingStaticLightingData(),NULL);
	StaticLightingLink->Element.Mapping = TextureMapping;
	StaticLightingLink->Element.LightMapData = LightMapData;
	StaticLightingLink->Element.ShadowMaps = ShadowMaps;
	CompleteTextureMappingList.AddElement(StaticLightingLink);
}

#endif