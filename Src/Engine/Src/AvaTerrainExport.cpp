/*=============================================================================
AvaTerrainExport.cpp: Export Unreal Terrain to VRAD
2006-08-10	허 창 민
=============================================================================*/

#include "EnginePrivate.h"
#include "UnTerrain.h"

static FPatchSampler	GCollisionPatchSampler(TERRAIN_MAXTESSELATION);

// patch vertex
// 2--1
// |  |
// 3--0

struct PatchVertexIndex
{
	INT SubX;
	INT SubY;
	PatchVertexIndex( INT X, INT Y ) : SubX( X ), SubY( Y )
	{}
};

static PatchVertexIndex IndexToQuadVertexIndex[4] = 
{
		PatchVertexIndex( 0, 0 ),
		PatchVertexIndex( 0, 1 ),
		PatchVertexIndex( 1, 1 ),
		PatchVertexIndex( 1, 0 )
};

// 2006. 12. 15 changmin
// quad		-> faces
// patch	-> dispinfo
INT UTerrainComponent::PatchToQuad( INT PatchIndex )
{
	return PatchIndex / GetTerrain()->MaxTesselationLevel;
}
INT UTerrainComponent::QuadToPatch( INT QuadIndex )
{
	return QuadIndex * GetTerrain()->MaxTesselationLevel;
}

#if !(CONSOLE || FINAL_RELEASE)
INT UTerrainComponent::ExportMaterial( const INT TerrainPatchX, const INT TerrainPatchY, AvaRadiosityAdapter* RadiosityAdapter )
{
	// get material mask for this patch
	const INT BatchIndex = ( TerrainPatchY - SectionBaseY ) * TrueSectionSizeX + (TerrainPatchX - SectionBaseX );
	const FTerrainMaterialMask& MaterialMask = BatchMaterials( PatchBatches( BatchIndex ) );

	// compute material's diffuse reflectivity
	ATerrain* Terrain = GetTerrain();

	AvaTextureData texData;
	texData.SourceMaterial_ = NULL; 
	if( MaterialMask.Num() == 0 )
		texData.Reflectivity_ = FVector( 0.9f, 0.9f, 0.9f );
	else
		texData.Reflectivity_ = FVector(0.0f, 0.0f, 0.0f);
	texData.SizeX_ = 0;
	texData.SizeY_ = 0;
	texData.Brightness_ = FVector( 0.0f, 0.0f, 0.0f);
	texData.ReflectivityScale_ = 1.0f;	// 계산을 간편하게 하기 위해... ReflectivieScale은 Reflectivity에 곱해서 계산된다.. 따라서 이값은 1만 사용한다.


	for( INT MaterialIndex = 0; MaterialIndex < MaterialMask.Num(); ++MaterialIndex )
	{
		if( MaterialMask.Get(MaterialIndex) )
		{
			FTerrainWeightedMaterial& WeightedMaterial = Terrain->WeightedMaterials(MaterialIndex);

			FLOAT TotalWeight = (FLOAT)WeightedMaterial.Weight(TerrainPatchX + 0,TerrainPatchY + 0) +
				(FLOAT)WeightedMaterial.Weight(TerrainPatchX + 1,TerrainPatchY + 0) +
				(FLOAT)WeightedMaterial.Weight(TerrainPatchX + 0,TerrainPatchY + 1) +
				(FLOAT)WeightedMaterial.Weight(TerrainPatchX + 1,TerrainPatchY + 1);

			TotalWeight /= 255.0f;	// scale to 0..1
			TotalWeight /= 4.0f;	// average four vertex

			// get diffuse texture for 
			UTerrainMaterial*	TerrainMaterial = WeightedMaterial.Material;
			UMaterialInstance*	MaterialInstance = TerrainMaterial ? (TerrainMaterial->Material ? TerrainMaterial->Material : NULL) : NULL;

			INT TexDataIndex = 0;

			if( MaterialInstance )
			{
				UMaterial*	Material = MaterialInstance->GetMaterial();
				TexDataIndex = RadiosityAdapter->FindOrAddTexData( Material );
			}

			
			const AvaTextureData& blendedTexData = RadiosityAdapter->TextureDatas_(TexDataIndex);
			texData.Reflectivity_ += blendedTexData.Reflectivity_ * blendedTexData.ReflectivityScale_ * TotalWeight;
			texData.SizeX_ = Max( texData.SizeX_, blendedTexData.SizeX_ );
			texData.SizeY_ = Max( texData.SizeY_, blendedTexData.SizeY_ );
			texData.Brightness_ += blendedTexData.Brightness_ * TotalWeight;
		}
	}

	// Weight Sum 이 1 로 normalize 되어 있어서, Reflectivity, Brightness 값은 Material 수로 나누지 않아도 됩니다.

	// TexData에 직접 넣습니다... 이 TexData는 동적으로 조합되는 것이어서, 같은 것이 나올 수 없습니다..
	return RadiosityAdapter->TextureDatas_.AddItem(texData);
}

void UTerrainComponent::ExportSurfaces( AvaRadiosityAdapter* RadiosityAdapter, const INT ExportVertexBufferBase, TMap<INT,INT>& PatchIndexToDispInfo )
{
	// store export contexts
	INT CurExportVertexPos	= RadiosityAdapter->VertexBuffer_.Num();
	INT CurExportIndexPos	= RadiosityAdapter->IndexBuffer_.Num();
	FirstExportedFaceNumber = RadiosityAdapter->FaceInfos_.Num();

	ATerrain* Terrain = GetTerrain();
	const FMatrix& LocalToWorld = Terrain->LocalToWorld();

	// export quad to face
	for( INT QuadY = 0; QuadY < SectionSizeX; ++QuadY )
	{
		for( INT QuadX = 0; QuadX < SectionSizeX; ++QuadX )
		{
			const INT TerrainQuadX		= PatchToQuad(SectionBaseX) + QuadX;
			const INT TerrainQuadY		= PatchToQuad(SectionBaseY) + QuadY;
			const INT TerrainQuadIndex	= TerrainQuadY * PatchToQuad(Terrain->NumPatchesX) + TerrainQuadX;
			// do not export invisible quad
			FTerrainInfoData* TerrainInfo = Terrain->GetInfoData( QuadToPatch(TerrainQuadX), QuadToPatch(TerrainQuadY) );
			if( !TerrainInfo->IsVisible() )	// quad 단위 visible check
			{
				continue;
			}
			// export quad
			FVector QuadVertexes[4];
			FVector QuadTangentX[4];
			FVector QuadTangentY[4];
			FVector QuadTangentZ[4];
			for( INT VertexIndex = 0; VertexIndex < 4; ++VertexIndex )
			{
				const UINT PatchX = QuadToPatch(TerrainQuadX + IndexToQuadVertexIndex[VertexIndex].SubX);
				const UINT PatchY = QuadToPatch(TerrainQuadY + IndexToQuadVertexIndex[VertexIndex].SubY);
				const FTerrainPatch& Patch = Terrain->GetPatch( PatchX, PatchY );
				const FVector& TangentX = FVector(1,0,GCollisionPatchSampler.SampleDerivX(Patch,0,0) * TERRAIN_ZSCALE).UnsafeNormal();
				const FVector& TangentY = FVector(0,1,GCollisionPatchSampler.SampleDerivY(Patch,0,0) * TERRAIN_ZSCALE).UnsafeNormal();
				const FVector& TangentZ = (TangentX ^ TangentY).UnsafeNormal();
				const FVector& LocalVertex = FVector( PatchX, PatchY, (-32768.0f + GCollisionPatchSampler.Sample( Patch, 0, 0 ) ) * TERRAIN_ZSCALE)
												+ TangentZ * Terrain->GetCachedDisplacement(SectionBaseX + PatchX, SectionBaseY + PatchY, 0, 0);

				QuadVertexes[VertexIndex] = LocalToWorld.TransformFVector(LocalVertex);
				QuadTangentX[VertexIndex] = LocalToWorld.TransformNormal(TangentX);
				QuadTangentY[VertexIndex] = LocalToWorld.TransformNormal(TangentY);
				QuadTangentZ[VertexIndex] = (QuadTangentX[VertexIndex] ^ QuadTangentY[VertexIndex]).UnsafeNormal();
				// export patch indices
				const INT QuadVertexX = TerrainQuadX + IndexToQuadVertexIndex[VertexIndex].SubX;
				const INT QuadVertexY = TerrainQuadY + IndexToQuadVertexIndex[VertexIndex].SubY;
				const INT QuadVertexIndex = QuadVertexY * ( PatchToQuad(Terrain->NumPatchesX) + 1 ) + QuadVertexX;
				RadiosityAdapter->IndexBuffer_.AddItem( ExportVertexBufferBase + QuadVertexIndex );
			}
			FPlane PatchPlane( QuadVertexes[0], QuadVertexes[2], QuadVertexes[1] );
			const FVector Svec = QuadTangentX[0].UnsafeNormal();
			const FVector Tvec = QuadTangentY[0].UnsafeNormal();
			const FVector WorldX = ((FVector)LocalToWorld.TransformNormal(FVector( 1.0f, 0.0f, 0.0f ))).UnsafeNormal();
			const FVector WorldY = ((FVector)LocalToWorld.TransformNormal(FVector( 0.0f, 1.0f, 0.0f ))).UnsafeNormal();
			// create and export surface map info
			AvaSurfaceMapInfo* SurfaceMap = new(RadiosityAdapter->SurfaceMaps_) AvaSurfaceMapInfo;
			SurfaceMap->TexturemapUAxis = Svec;
			SurfaceMap->TexturemapVAxis = Tvec;
			SurfaceMap->LightmapUAxis	= WorldX;
			SurfaceMap->LightmapVAxis	= WorldY;
			SurfaceMap->Flags = 0;
			FVector Edge = QuadVertexes[1] - QuadVertexes[0];
			SurfaceMap->ShadowMapScale = Edge.Size() / (Terrain->MaxTesselationLevel * Terrain->StaticLightingResolution); // patch can be uniform scaled only, so consider only one edge for calculation.
			// calculate light map offset 
			FVector2D MinUV(WORLD_MAX,WORLD_MAX);
			FVector2D MaxUV(-WORLD_MAX,-WORLD_MAX);
			const FVector UVecsInWorldUnit = WorldX / SurfaceMap->ShadowMapScale;
			const FVector VVecsInWorldUnit = WorldY / SurfaceMap->ShadowMapScale;
			for( INT iVertex = 0; iVertex < 4; ++iVertex )
			{
				const FVector& WorldPos = QuadVertexes[iVertex];
				FLOAT	X = UVecsInWorldUnit | WorldPos, Y = VVecsInWorldUnit | WorldPos;
				MinUV.X = Min(X, MinUV.X);
				MinUV.Y = Min(Y, MinUV.Y);
				MaxUV.X = Max(X, MaxUV.X);
				MaxUV.Y = Max(Y, MaxUV.Y);
			}
			SurfaceMap->LightmapUOffset = -MinUV.X;
			SurfaceMap->LightmapVOffset = -MinUV.Y;
			// create and export displacement info
			// Terrain Quad Index로 Displament info를 indexing한다.
			// 이는 patch 사이의 neighbor 정보 indexing을 terrain patch index로 하기 위해서이다.
			// 다른 terrain과는 neighbor가 없다는 가정을 하고, 분리한다.
			AvaDispInfo* DispInfo		= &RadiosityAdapter->DispInfos_(*(PatchIndexToDispInfo.Find(TerrainQuadIndex)));
			DispInfo->Power				= appCeilLogTwo(Terrain->MaxTesselationLevel);
			DispInfo->StartPosition		= QuadVertexes[0];
			DispInfo->DispVertexStart	= RadiosityAdapter->DispVertexes_.Num();
			//	Terrain quad Index
			//
			//		+--+--+--+
			//		|8 |7 |6 |
			//		+--+--+--+
			//		|5 |4 |3 |		y
			//		+--+--+--+		^
			//		|2 |1 |0 |		|
			//		+--+--+--+		|
			//						|
			//    x <---------------+
			// quad vertex
			// 2--1
			// |  |
			// 3--0
			// ------------------------------------------------------------------------------------------------ //
			// Displacement neighbor rules
			// ------------------------------------------------------------------------------------------------ //
			// Each displacement is considered to be in its own space:
			//
			//               NEIGHBOREDGE_TOP
			//
			//                   1 --- 2
			//                   |     |
			// NEIGHBOREDGE_LEFT |     | NEIGHBOREDGE_RIGHT
			//                   |     |
			//                   0 --- 3
			//
			//   			NEIGHBOREDGE_BOTTOM
			const INT LeftQuadX		= TerrainQuadX - 1;
			const INT TopQuadY		= TerrainQuadY + 1;
			const INT RightQuadX	= TerrainQuadX + 1;
			const INT BottomQuadY	= TerrainQuadY - 1;
			// left
			if( LeftQuadX < 0)
			{
				DispInfo->NeighborIndex[0]			= 0xFFFF;
			}
			else
			{
				FTerrainInfoData* TerrainInfo = Terrain->GetInfoData( QuadToPatch(LeftQuadX), QuadToPatch(TerrainQuadY) );
				if( TerrainInfo->IsVisible() )
				{
					DispInfo->NeighborIndex[0]			= *(PatchIndexToDispInfo.Find(TerrainQuadY * PatchToQuad(Terrain->NumPatchesX) + LeftQuadX));
					DispInfo->NeighborOrientation[0]	= Rad::ORIENTATION_CCW_0;
					DispInfo->NeighborSpan[0]			= Rad::CORNER_TO_CORNER;
					DispInfo->Span[0]					= Rad::CORNER_TO_CORNER;
				}
				else
				{
					DispInfo->NeighborIndex[0]			= 0xFFFF;
				}
				
			}

			// top
			if( TopQuadY < PatchToQuad(Terrain->NumPatchesY) )
			{
				FTerrainInfoData* TerrainInfo = Terrain->GetInfoData( QuadToPatch(TerrainQuadX), QuadToPatch(TopQuadY) );
				if( TerrainInfo->IsVisible() )
				{
					DispInfo->NeighborIndex[1]			= *(PatchIndexToDispInfo.Find(TopQuadY * PatchToQuad(Terrain->NumPatchesX) + TerrainQuadX));
					DispInfo->NeighborOrientation[1]	= Rad::ORIENTATION_CCW_0;
					DispInfo->NeighborSpan[1]			= Rad::CORNER_TO_CORNER;
					DispInfo->Span[1]					= Rad::CORNER_TO_CORNER;
				}
				else
				{
					DispInfo->NeighborIndex[1]			= 0xFFFF;
				}
			}
			else
			{
				DispInfo->NeighborIndex[1]			= 0xFFFF;
			}

			// right
			if( RightQuadX < PatchToQuad(Terrain->NumPatchesX) )
			{
				FTerrainInfoData* TerrainInfo = Terrain->GetInfoData( QuadToPatch(RightQuadX), QuadToPatch(TerrainQuadY) );
				if( TerrainInfo->IsVisible() )
				{
					DispInfo->NeighborIndex[2]				= *(PatchIndexToDispInfo.Find( TerrainQuadY * PatchToQuad(Terrain->NumPatchesX) + RightQuadX));
					DispInfo->NeighborOrientation[2]		= Rad::ORIENTATION_CCW_0;
					DispInfo->NeighborSpan[2]				= Rad::CORNER_TO_CORNER;
					DispInfo->Span[2]						= Rad::CORNER_TO_CORNER;
				}
				else
				{
					DispInfo->NeighborIndex[2]				= 0xFFFF;
				}
			}
			else
			{
				DispInfo->NeighborIndex[2]				= 0xFFFF;
			}

			// bottom
			if( BottomQuadY < 0)
			{
				DispInfo->NeighborIndex[3]				 = 0xFFFF;
			}
			else
			{
				FTerrainInfoData* TerrainInfo = Terrain->GetInfoData( QuadToPatch(TerrainQuadX), QuadToPatch(BottomQuadY) );
				if( TerrainInfo->IsVisible() )
				{
					DispInfo->NeighborIndex[3]				= *(PatchIndexToDispInfo.Find(BottomQuadY * PatchToQuad(Terrain->NumPatchesX) + TerrainQuadX));
					DispInfo->NeighborOrientation[3]		= Rad::ORIENTATION_CCW_0;
					DispInfo->NeighborSpan[3]				= Rad::CORNER_TO_CORNER;
					DispInfo->Span[3]						= Rad::CORNER_TO_CORNER;
				}
				else
				{
					DispInfo->NeighborIndex[3]				= 0xFFFF;
				}
			}
			
			// create spacing vector in world space
			FVector EdgeInterval[2];
			EdgeInterval[0] = ( QuadVertexes[1] - QuadVertexes[0] ) / Terrain->MaxTesselationLevel;
			EdgeInterval[1] = ( QuadVertexes[2] - QuadVertexes[3] ) / Terrain->MaxTesselationLevel;
			// create and displacement vertex
			for( INT SubY = 0; SubY < (Terrain->MaxTesselationLevel + 1); ++SubY )
			{
				FVector EndPoint[2];
				EndPoint[0] = QuadVertexes[0] + EdgeInterval[0] * SubY;
				EndPoint[1] = QuadVertexes[3] + EdgeInterval[1] * SubY;
				FVector Segment = (EndPoint[1] - EndPoint[0]) / Terrain->MaxTesselationLevel;
				for( INT SubX = 0; SubX < (Terrain->MaxTesselationLevel + 1); ++SubX )
				{
					FVector WorldVertex = Terrain->GetWorldVertex( QuadToPatch(TerrainQuadX) + SubX, QuadToPatch(TerrainQuadY) + SubY );
					FVector QuadVertex = EndPoint[0] + (Segment * SubX);
					FVector WorldVectorField = WorldVertex - QuadVertex;
					AvaDispVertex DispVertex;
					DispVertex.DispVector = WorldVectorField.SafeNormal();
					DispVertex.DispDistance = WorldVectorField.Size();
					DispVertex.Alpha = 1.0f;	//@TODO : in order to fill this field, need to know how to use alpha...
					RadiosityAdapter->DispVertexes_.AddItem( DispVertex );
				}
			}
			// create and export face info
			AvaFaceInfo* FaceInfo = new(RadiosityAdapter->FaceInfos_) AvaFaceInfo;
			FaceInfo->IndexBufferOffset	= CurExportIndexPos;
			FaceInfo->NumVertices		= 4;
			FaceInfo->iPlane			= RadiosityAdapter->PlaneBuffer_.AddItem( PatchPlane );
			FaceInfo->iSurfaceMapInfo	= RadiosityAdapter->SurfaceMaps_.Num() - 1;
			FaceInfo->iTextureData		= ExportMaterial(QuadToPatch(TerrainQuadX), QuadToPatch(TerrainQuadY), RadiosityAdapter);
			FaceInfo->Emission			= FVector( 0.0f, 0.0f, 0.0f );
			FaceInfo->iDispInfo			= *(PatchIndexToDispInfo.Find(TerrainQuadIndex));
			// update buffer pos info for export
			CurExportVertexPos += 4;
			CurExportIndexPos += 4;
		}
	}
}
#else
void UTerrainComponent::ExportSurfaces( AvaRadiosityAdapter* RadiosityAdapter, const INT ExportVertexBufferBase, TMap<INT,INT>& PatchIndexToDispInfo )
{}
#endif