/*=============================================================================
	StaticLightingVertexMapping.cpp: Static lighting vertex mapping implementation.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "StaticLightingPrivate.h"

// Don't compile the static lighting system on consoles.
#if !CONSOLE

/**
 * Caches the vertices of a mesh.
 * @param Mesh - The mesh to cache vertices from.
 * @param OutVertices - Upon return, contains the meshes vertices.
 */
static void CacheVertices(const FStaticLightingMesh* Mesh,TArray<FStaticLightingVertex>& OutVertices)
{
	OutVertices.Empty(Mesh->NumVertices);
	OutVertices.Add(Mesh->NumVertices);

	for(INT TriangleIndex = 0;TriangleIndex < Mesh->NumTriangles;TriangleIndex++)
	{
		// Query the mesh for the triangle's vertices.
		FStaticLightingVertex V0;
		FStaticLightingVertex V1;
		FStaticLightingVertex V2;
		Mesh->GetTriangle(TriangleIndex,V0,V1,V2);
		INT I0 = 0;
		INT I1 = 0;
		INT I2 = 0;
		Mesh->GetTriangleIndices(TriangleIndex,I0,I1,I2);

		// Cache the vertices by vertex index.
		OutVertices(I0) = V0;
		OutVertices(I1) = V1;
		OutVertices(I2) = V2;
	}
}

/**
 * Interpolates a triangle's vertices to find the attributes of a point in the triangle.
 * @param V0 - The triangle's first vertex.
 * @param V1 - The triangle's second vertex.
 * @param V2 - The triangle's third vertex.
 * @param S - The barycentric coordinates of the point on the triangle to derive.
 * @param T - The barycentric coordinates of the point on the triangle to derive.
 * @param U - The barycentric coordinates of the point on the triangle to derive.
 * @return A vertex representing the specified point on the triangle/
 */
static FStaticLightingVertex InterpolateTrianglePoint(const FStaticLightingVertex& V0,const FStaticLightingVertex& V1,const FStaticLightingVertex& V2,FLOAT S,FLOAT T,FLOAT U)
{
	FStaticLightingVertex Result;
	Result.WorldPosition =	S * V0.WorldPosition +		T * V1.WorldPosition +	U * V2.WorldPosition;
	Result.WorldTangentX =	S * V0.WorldTangentX +		T * V1.WorldTangentX +	U * V2.WorldTangentX;
	Result.WorldTangentY =	S * V0.WorldTangentY +		T * V1.WorldTangentY +	U * V2.WorldTangentY;
	Result.WorldTangentZ =	S * V0.WorldTangentZ +		T * V1.WorldTangentZ +	U * V2.WorldTangentZ;
	return Result;
}

void FStaticLightingSystem::ProcessVertexMapping(FStaticLightingVertexMapping* VertexMapping)
{
	FCoherentRayCache CoherentRayCache;

	const FStaticLightingMesh* const Mesh = VertexMapping->Mesh;

	// Cache the mesh's vertex data, and build a map from position to indices of vertices at that position.
	TArray<FStaticLightingVertex> Vertices;
	CacheVertices(Mesh,Vertices);
	
	// Sort the mapping's relevant lights by whether they can be added to the light-map, or need a separate shadow-map.
	TArray<ULightComponent*> LightMappedLights;
	TArray<ULightComponent*> ShadowMappedLights;
	SortMappingLightsByLightingType(VertexMapping,LightMappedLights,ShadowMappedLights);

	// Allocate shadow-map data for each light relevant to the mapping.
	TArray<FShadowMapData1D> ShadowMapData;
	ShadowMapData.Empty(ShadowMappedLights.Num());
	for(INT LightIndex = 0;LightIndex < ShadowMappedLights.Num();LightIndex++)
	{
		new(ShadowMapData) FShadowMapData1D(Mesh->NumVertices);
	}
	
	// Allocate light-map data.
	FLightMapData1D LightMapData(Mesh->NumVertices);
	for(INT LightIndex = 0;LightIndex < LightMappedLights.Num();LightIndex++)
	{
		LightMapData.Lights.AddItem(LightMappedLights(LightIndex));
	}

	// Allocate the vertex sample weight total map.
	TArray<FLOAT> VertexSampleWeightTotals;
	VertexSampleWeightTotals.Empty(Mesh->NumVertices);
	VertexSampleWeightTotals.AddZeroed(Mesh->NumVertices);

	// Setup a thread-safe random stream with a fixed seed, so the sample points are deterministic.
	FRandomStream RandomStream(0);

	if(!VertexMapping->bSampleVertices)
	{
		// Calculate light visibility for each triangle.
		for(INT TriangleIndex = 0;TriangleIndex < Mesh->NumTriangles;TriangleIndex++)
		{
			// Query the mesh for the triangle's vertex indices.
			INT I0 = 0;
			INT I1 = 0;
			INT I2 = 0;
			Mesh->GetTriangleIndices(TriangleIndex,I0,I1,I2);

			// Lookup the triangle's vertices.
			const FStaticLightingVertex TriangleVertices[3] =
			{
				Vertices(I0),
				Vertices(I1),
				Vertices(I2)
			};

			// Compute the triangle's normal.
			const FVector TriangleNormal = (TriangleVertices[2].WorldPosition - TriangleVertices[0].WorldPosition) ^ (TriangleVertices[1].WorldPosition - TriangleVertices[0].WorldPosition);

			// Find the lights which are in front of this triangle.
			const FBitArray TriangleLightMappedLightsMask = CullBackfacingLights(TriangleVertices[0].WorldPosition,TriangleNormal,LightMappedLights);
			const FBitArray TriangleShadowMappedLightsMask = CullBackfacingLights(TriangleVertices[0].WorldPosition,TriangleNormal,ShadowMappedLights);

			// Compute the triangle's area.
			const FLOAT TriangleArea = 0.5f * TriangleNormal.Size();

			// Compute the number of samples to use for the triangle, proportional to the triangle area.
			const INT NumSamples = Clamp(appTrunc(TriangleArea * VertexMapping->SampleToAreaRatio),0,MAX_SHADOW_SAMPLES_PER_TRIANGLE);

			// Sample the triangle's lighting.
			for(INT SampleIndex = 0;SampleIndex < NumSamples;SampleIndex++)
			{
				// Choose a uniformly distributed random point on the triangle.
				const FLOAT S = 1.0f - appSqrt(RandomStream.GetFraction());
				const FLOAT T = RandomStream.GetFraction() * (1.0f - S);
				const FLOAT U = 1 - S - T;

				// Interpolate the triangle's vertex attributes at the sample point.
				const FStaticLightingVertex SampleVertex = 
					TriangleVertices[0] * S +
					TriangleVertices[1] * T +
					TriangleVertices[2] * U;

				// Accumulate the light-mapped lighting for this sample.
				FLightSample LightMapSample;
				for(INT LightIndex = 0;LightIndex < LightMappedLights.Num();LightIndex++)
				{
					if(TriangleLightMappedLightsMask(LightIndex))
					{
						ULightComponent* Light = LightMappedLights(LightIndex);
						const UBOOL bIsShadowed = CalculatePointShadowing(VertexMapping,SampleVertex.WorldPosition,Light,CoherentRayCache);
						if(!bIsShadowed)
						{
							LightMapSample.AddWeighted(CalculatePointLighting(VertexMapping,SampleVertex,Light),1.0f);
						}
					}
				}

				// Compute the shadow factors for this sample from the shadow-mapped lights.
				FBitArray ShadowMapSamples(FALSE,ShadowMappedLights.Num());
				for(INT LightIndex = 0;LightIndex < ShadowMappedLights.Num();LightIndex++)
				{
					if(TriangleShadowMappedLightsMask(LightIndex))
					{
						ULightComponent* Light = ShadowMappedLights(LightIndex);
						ShadowMapSamples(LightIndex) = !CalculatePointShadowing(VertexMapping,SampleVertex.WorldPosition,Light,CoherentRayCache);
					}
				}

				// Accumulate the sample lighting and shadowing at the adjacent vertices.
				const FLOAT TriangleVertexWeights[3] = { S, T, U };
				const INT TriangleVertexIndices[3] = { I0, I1, I2 };
				for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
				{
					// Accumulate the sample lighting.
					LightMapData(TriangleVertexIndices[VertexIndex]).AddWeighted(LightMapSample,TriangleVertexWeights[VertexIndex]);

					// Accumulate the sample shadowing.
					for(INT LightIndex = 0;LightIndex < ShadowMappedLights.Num();LightIndex++)
					{
						if(ShadowMapSamples(LightIndex))
						{
							ShadowMapData(LightIndex)(TriangleVertexIndices[VertexIndex]) += TriangleVertexWeights[VertexIndex];
						}
					}

					// Accumulate the sample weight.
					VertexSampleWeightTotals(TriangleVertexIndices[VertexIndex]) += TriangleVertexWeights[VertexIndex];
				}
			}
		}
	}

	// Calculate light visibility for vertices which had no samples (if bSampleVertices, or they are from small triangles).
	for(INT VertexIndex = 0;VertexIndex < Mesh->NumVertices;VertexIndex++)
	{
		if(VertexSampleWeightTotals(VertexIndex) == 0.0f)
		{
			const FStaticLightingVertex& SampleVertex = Vertices(VertexIndex);

			// Clear the light-map sample for this vertex.
			LightMapData(VertexIndex) = FLightSample();
			LightMapData(VertexIndex).bIsMapped = TRUE;

			// Accumulate the light-mapped lighting for this vertex.
			for(INT LightIndex = 0;LightIndex < LightMappedLights.Num();LightIndex++)
			{
				ULightComponent* Light = LightMappedLights(LightIndex);
				if(!IsLightBehindSurface(SampleVertex.WorldPosition,SampleVertex.WorldTangentZ,Light))
				{
					const UBOOL bIsShadowed = CalculatePointShadowing(VertexMapping,SampleVertex.WorldPosition,Light,CoherentRayCache);
					if(!bIsShadowed)
					{
						LightMapData(VertexIndex).AddWeighted(CalculatePointLighting(VertexMapping,SampleVertex,Light),1.0f);
					}
				}
			}

			// Compute the shadow factors for this vertex from the shadow-mapped lights.
			for(INT LightIndex = 0;LightIndex < ShadowMappedLights.Num();LightIndex++)
			{
				ULightComponent* Light = ShadowMappedLights(LightIndex);
				FLOAT LightVisibility = 0.0f;
				if(!IsLightBehindSurface(SampleVertex.WorldPosition,SampleVertex.WorldTangentZ,Light))
				{
					LightVisibility = CalculatePointShadowing(VertexMapping,SampleVertex.WorldPosition,Light,CoherentRayCache) ? 0.0f : 1.0f;
				}
				ShadowMapData(LightIndex)(VertexIndex) = LightVisibility;
			}

			// Set the vertex sample weight.
			VertexSampleWeightTotals(VertexIndex) = 1.0f;
		}
	}

	// Renormalize the super-sampled vertex lighting and shadowing.
	for(INT VertexIndex = 0;VertexIndex < Mesh->NumVertices;VertexIndex++)
	{
		const FLOAT TotalWeight = VertexSampleWeightTotals(VertexIndex);
		if(TotalWeight > DELTA)
		{
			const FLOAT InvTotalWeight = 1.0f / VertexSampleWeightTotals(VertexIndex);

			// Renormalize the vertex's light-map sample.
			FLightSample RenormalizedLightSample;
			RenormalizedLightSample.AddWeighted(LightMapData(VertexIndex),InvTotalWeight);
			LightMapData(VertexIndex) = RenormalizedLightSample;

			// Renormalize the vertex's shadow-map samples.
			for(INT LightIndex = 0;LightIndex < ShadowMappedLights.Num();LightIndex++)
			{
				ShadowMapData(LightIndex)(VertexIndex) *= InvTotalWeight;
			}
		}
	}

	// Create shadow-maps from the shadow-map data.
	TMap<ULightComponent*,FShadowMapData1D*> FinalShadowMapData;
	for(INT LightIndex = 0;LightIndex < ShadowMappedLights.Num();LightIndex++)
	{
		FinalShadowMapData.Set(ShadowMappedLights(LightIndex),new FShadowMapData1D(ShadowMapData(LightIndex)));
	}

	// Enqueue the static lighting for application in the main thread.
	TList<FVertexMappingStaticLightingData>* StaticLightingLink = new TList<FVertexMappingStaticLightingData>(FVertexMappingStaticLightingData(),NULL);
	StaticLightingLink->Element.Mapping = VertexMapping;
	StaticLightingLink->Element.LightMapData = new FLightMapData1D(LightMapData);
	StaticLightingLink->Element.ShadowMaps = FinalShadowMapData;
	CompleteVertexMappingList.AddElement(StaticLightingLink);
}

#endif
