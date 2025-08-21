/*=============================================================================
AvaTerrainLight.cpp: Terrain Radiosity lighting code.
2006-08-10	허 창 민
=============================================================================*/

#include "EnginePrivate.h"
#include "UnTerrain.h"

void ATerrain::CacheRadiosityLighting( const FLightingBuildOptions& BuildOptions, AvaRadiosityAdapter& Adapter )
{
#if !(CONSOLE || FINAL_RELEASE)
	for(INT ComponentIndex = 0;ComponentIndex < TerrainComponents.Num();ComponentIndex++)
	{
		if(TerrainComponents(ComponentIndex))
		{
			TerrainComponents(ComponentIndex)->CacheRadiosityLighting( BuildOptions, Adapter );
		}
	}

	//@TODO : decolayer.....
#endif
}

void UTerrainComponent::CacheRadiosityLighting(const FLightingBuildOptions& BuildOptions, AvaRadiosityAdapter& RadiosityAdapter)
{
#if !(CONSOLE || FINAL_RELEASE)
	// Let the rendering thread 'catch up'
	FlushRenderingCommands();

	// Detach the component from the scene for the duration of this function.
	// Note that we don't have to wait for the detach to complete because InvalidateLightingCache will do that if bShouldRebuildAllLighting.
	// If !bShouldRebuildAllLighting, we'll only be adding new elements to StaticLights, so we won't clobber any of the existing allocations
	// which the rendering thread's FStaticMeshSceneProxy might be referencing.
	FComponentReattachContext ReattachContext(this);

	//@todo: support partial rebuilds
	UBOOL bShouldRebuildAllLighting = !BuildOptions.bOnlyBuildChanged;
	if (bShouldRebuildAllLighting)
	{
		InvalidateLightingCache();
		IrrelevantLights.Empty();
	}
	// Invalidate existing lighting.
	InvalidateLightingCache();
	if(!HasStaticShadowing())
	{
		return;
	}

	TArray<ULightComponent*> RadiosityLights;
	for(TObjectIterator<ULightComponent> LightIt;LightIt;++LightIt)
	{
		const UBOOL bLightIsInWorld = LightIt->GetOwner() && GWorld->ContainsActor(LightIt->GetOwner());
		if (bLightIsInWorld && LightIt->UseDirectLightMap)
		{
			RadiosityLights.AddItem( *LightIt );
		}
	}

	ATerrain *Terrain = GetTerrain();
	const INT LightMapSizeX = TrueSectionSizeX * Terrain->StaticLightingResolution + 1;
	const INT LightMapSizeY = TrueSectionSizeY * Terrain->StaticLightingResolution + 1;
	FLightMapData2D *LightMapData2D	= new FLightMapData2D( LightMapSizeX, LightMapSizeY );
	LightMapData2D->Lights.Append( RadiosityLights );
	
	// get lightmap from vrad output
	INT ExportFaceIndex = FirstExportedFaceNumber;
	const INT QuadLightMapMaxSize = Terrain->StaticLightingResolution * Terrain->MaxTesselationLevel + 1;
	for( INT QuadY = 0; QuadY < SectionSizeY; ++QuadY )
	{
		for( INT QuadX = 0; QuadX < SectionSizeX; ++QuadX )
		{
			const INT TerrainPatchX = SectionBaseX + QuadToPatch(QuadX);
			const int TerrainPatchY = SectionBaseY + QuadToPatch(QuadY);
			FTerrainInfoData* TerrainInfo = Terrain->GetInfoData( TerrainPatchX, TerrainPatchY );
			if( TerrainInfo->IsVisible() )	// IsVisible = quad단위 visible checking이다.
			{
				// get light map
				AvaFaceInfo& Face				= RadiosityAdapter.FaceInfos_(ExportFaceIndex++);
				AvaLightmapInfo& LightmapInfo	= Face.LightmapInfo;
				const INT Width	= LightmapInfo.Width	> QuadLightMapMaxSize ? QuadLightMapMaxSize : LightmapInfo.Width;
				const INT Height= LightmapInfo.Height	> QuadLightMapMaxSize ? QuadLightMapMaxSize : LightmapInfo.Height;
				const INT BaseX	= QuadX * Terrain->StaticLightingResolution * Terrain->MaxTesselationLevel;
				const INT BaseY	= QuadY * Terrain->StaticLightingResolution * Terrain->MaxTesselationLevel;
				for( INT ResY = 0; ResY < Height; ++ResY )
				{
					for( INT ResX = 0; ResX < Width; ++ResX )
					{
						FLightSample& DestSample = (*LightMapData2D)( BaseX + ResX, BaseY + ResY );
						for( INT iBump = 0; iBump < NUM_LIGHTMAP_COEFFICIENTS; ++iBump )
						{
							FLinearColor LightMapColor( 0.0f, 0.0f, 0.0f );
							LightmapInfo.GetSample( ResX, ResY, (iBump+1), &LightMapColor.R, &LightMapColor.G, &LightMapColor.B );
							DestSample.AddLight( LightMapColor, iBump );
						}
						DestSample.bIsMapped = TRUE;
					}
				}
			} // end if terrain is visible
		}	// iter quadx
	}	// for quady

	// create the terrain component's lightmap
	LightMap = FLightMap2D::AllocateLightMap( GetOutermost(), LightMapData2D, LightMapData2D, NULL, Bounds, RadiosityAdapter.bSupportsCascadedShadow_ );

	delete LightMapData2D;
	LightMapData2D = NULL;
#endif
}
