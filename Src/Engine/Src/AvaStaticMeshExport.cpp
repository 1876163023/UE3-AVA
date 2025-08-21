/*=============================================================================
AvaStaticMeshExport.cpp: Export Static Mesh to VRAD.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnStaticMeshLegacy.h"

#pragma optimize("", off)

void UStaticMeshComponent::ExportSurfaces( AvaRadiosityAdapter* RadiosityAdapter )
{
#if !(CONSOLE || FINAL_RELEASE)
	if( !( StaticMesh && HasStaticShadowing() && bAcceptsLights ) )
	{
		return;
	}	

	if( StaticMesh->LODModels.Num() == 0 )
	{
		return;
	}

	FStaticMeshRenderData& RenderData = StaticMesh->LODModels(0);

	const INT MAX_SUBDIVISION_STEP_SIZE = 1024;
	const INT MIN_SUBDIVISION_STEP_SIZE = 1;

	INT StepSize = Clamp<INT>(SubDivisionStepSize, MIN_SUBDIVISION_STEP_SIZE, MAX_SUBDIVISION_STEP_SIZE );
	INT							LightMapWidth		= 0;
	INT							LightMapHeight		= 0;
	GetLightMapResolution( LightMapWidth, LightMapHeight );

	RenderData.ExportSurfaces(
		RadiosityAdapter,
		LocalToWorld,
		IsRadiosityTextureMapLighting(RenderData),
		StepSize,
		StaticMesh->LightMapCoordinateIndex,
		LightMapWidth,
		*this);
#endif
}


namespace
{
	static UBOOL FloatEquals(const FLOAT F1, const FLOAT F2, FLOAT Tolerance=KINDA_SMALL_NUMBER)
	{
		return Abs(F1-F2) < Tolerance;
	}
};


FLOAT CalculateScaleFactor( const FMatrix& LocalToWorld )
{
	// static mesh ���� scale�� ���´�.
	// For each row, find magnitude, and if its non-zero re-scale so its unit length.
	FLOAT ScaleFactor[3];
	for(INT i=0; i<3; i++)
	{
		FVector Axis = LocalToWorld.GetAxis(i);
		ScaleFactor[i] = Axis.Size();
	}

	if( FloatEquals(ScaleFactor[0], ScaleFactor[1]) && FloatEquals(ScaleFactor[0], ScaleFactor[2]) && FloatEquals(ScaleFactor[1], ScaleFactor[2]) )
	{
		//warnf(NAME_Log, TEXT("There is Uniform Scaled(%f) StaticMeshComponent : %s"), StaticMeshScaleFactor, *StaticMeshComponent.GetName() );
		return (ScaleFactor[0] + ScaleFactor[1] + ScaleFactor[2]) / 3.0f;
	}
	else
	{
		//warnf(NAME_Warning, TEXT("There is Non Uniform Scaled StaticMeshComponent : %s"), *StaticMeshComponent.GetName() );
		return 1.0f;
	}
}

void FStaticMeshRenderData::ExportElementAsRadiosityFaces( AvaRadiosityAdapter* RadiosityAdapter,
										   const FMatrix& LocalToWorld,
										   const INT ElementIndex,
										   UStaticMeshComponent& StaticMeshComponent,
										   const INT ExportVertexBufferBase )
{
#if !(CONSOLE || FINAL_RELEASE)
	//<@ 2006. 10. 20 changmin
	// �� �Լ��� emissive face�� ���� �Լ��Դϴ�.
	// emissive face�� ava rad���� simulation�� �ǰ�, ue3 �ʿ����� ����� face�� �ƴϹǷ�, �Ʒ��� �������� �ʽ��ϴ�.
	//StaticMeshComponent.FirstExportedFaceNumber = RadiosityAdapter->FaceInfos_.Num();
	//>@ 2006. 10. 20

	// Radiosity Adapter �󿡼� ���� export �Ǵ� mesh�� ���۵Ǵ� ��ġ
	const INT IndexBufferBase	= RadiosityAdapter->IndexBuffer_.Num();

	const AvaStaticMeshExportElement	*RawExportElements	= (AvaStaticMeshExportElement*)BulkExportElements.Lock( LOCK_READ_ONLY );
	const AvaConvexPolygon				*RawConvexPolygons	= (AvaConvexPolygon*)BulkConvexPolygons.Lock( LOCK_READ_ONLY );
	const FVector						*RawTangentXs		= (FVector*)BulkConvexTangentXs.Lock( LOCK_READ_ONLY );
	const FVector						*RawTangentYs		= (FVector*)BulkConvexTangentYs.Lock( LOCK_READ_ONLY );
	const FLegacyStaticMeshVertex		*RawVertexData		= (FLegacyStaticMeshVertex*)BulkConvexPolygonVertexBuffer.Lock( LOCK_READ_ONLY );
	const INT							*RawIndex			= (INT*) BulkConvexPolygonIndexBuffer.Lock( LOCK_READ_ONLY );

	const FLOAT StaticMeshScaleFactor = CalculateScaleFactor( LocalToWorld );	

	// create TexData
	UMaterialInstance* MaterialInstance	= StaticMeshComponent.GetMaterial(ElementIndex);
	INT			TextureDataIndex	= RadiosityAdapter->FindOrAddTexData(MaterialInstance);

	const AvaStaticMeshExportElement& ExportElement = RawExportElements[ElementIndex];
	INT IndexOffset = 0;
	for( INT iFace = 0; iFace < ExportElement.NumPolygons_; ++iFace )
	{
		const AvaConvexPolygon& poly = RawConvexPolygons[ExportElement.PolygonStart_ + iFace];

		FVector LocalMapX;
		FVector LocalMapY;

		LocalMapX = poly.LightMapXInWorldUnit_;
		LocalMapY = poly.LightMapYInWorldUnit_;

		FVector TransformedWorldMapX = LocalToWorld.TransformNormal( LocalMapX );
		FVector TransformedWorldMapY = LocalToWorld.TransformNormal( LocalMapY );

		// �ּ� 3���� �ְ���~
		FVector WorldPosition[3];
		for( INT iVertex = 0; iVertex < 3; ++iVertex )
		{
			const FLegacyStaticMeshVertex& Vertex = RawVertexData[ RawIndex[poly.IndexStart_ + iVertex] ];
			WorldPosition[iVertex]			= LocalToWorld.TransformFVector( Vertex.Position );
		}

		// colinear�� vertex ������ ������ ���� �� �ִ�......
		FPlane Plane(WorldPosition[0], WorldPosition[1], WorldPosition[2]);	//�̰� ��Ȯ.. ��� ����...

		// ����ȭ
		FVector WorldNormal = (FVector) Plane;
		FVector WorldMapX	= ( TransformedWorldMapX - ( WorldNormal | TransformedWorldMapX ) * WorldNormal ).SafeNormal();
		FVector WorldMapY	= ( TransformedWorldMapY - ( WorldNormal | TransformedWorldMapY ) * WorldNormal ).SafeNormal();

		// Create Surface Map Info
		AvaSurfaceMapInfo* SurfaceMap = new( RadiosityAdapter->SurfaceMaps_ ) AvaSurfaceMapInfo;

		// ��¦ ����Ȯ�ص�... �ٽ� ����ϱ� ����� �̰� ������...
		SurfaceMap->TexturemapUAxis = LocalToWorld.TransformNormal( RawTangentXs[poly.TangentXIndex_] );
		SurfaceMap->TexturemapVAxis = LocalToWorld.TransformNormal( RawTangentYs[poly.TangentYIndex_] );
		SurfaceMap->LightmapUAxis = WorldMapX.SafeNormal();
		SurfaceMap->LightmapVAxis = WorldMapY.SafeNormal();
		SurfaceMap->ShadowMapScale = StaticMeshComponent.StaticMesh->LuxelSize * StaticMeshScaleFactor;
		SurfaceMap->Flags = SURF_SKIPCOLLISION;	//2008. 1. 10 changmin ; emissive part �� collsion�� ������ �ʴ´�. black mesh�� collision ����. �ߺ� collision ����

		// Create Face Info
		AvaFaceInfo* FaceInfo		= new( RadiosityAdapter->FaceInfos_ ) AvaFaceInfo;
		FaceInfo->IndexBufferOffset	= IndexBufferBase + IndexOffset;
		FaceInfo->NumVertices		= poly.NumIndex_;
		FaceInfo->iPlane			= RadiosityAdapter->PlaneBuffer_.AddItem( Plane );
		FaceInfo->iSurfaceMapInfo	= RadiosityAdapter->SurfaceMaps_.Num() - 1;
		FaceInfo->iTextureData		= TextureDataIndex;
		FaceInfo->Emission			= FVector( 0.0f, 0.0f, 0.0f );
		FaceInfo->iDispInfo			= -1;

		// export index buffer
		for( INT iIndex = 0; iIndex < poly.NumIndex_; ++iIndex )
		{
			RadiosityAdapter->IndexBuffer_.AddItem( ExportVertexBufferBase + RawIndex[ poly.IndexStart_ + iIndex ] );
		}

		// update offset
		IndexOffset += poly.NumIndex_;

		// calculate light map offset 
		FVector2D MinUV(WORLD_MAX,WORLD_MAX);
		FVector2D MaxUV(-WORLD_MAX,-WORLD_MAX);

		FVector UVecsInWorldUnit = WorldMapX / SurfaceMap->ShadowMapScale;
		FVector VVecsInWorldUnit = WorldMapY / SurfaceMap->ShadowMapScale;

		for( INT iVertex = 0; iVertex < poly.NumIndex_; ++iVertex )
		{
			const FLegacyStaticMeshVertex& Vertex = RawVertexData[ RawIndex[poly.IndexStart_ + iVertex] ];
			FVector WorldPos = LocalToWorld.TransformFVector( Vertex.Position );

			FLOAT	X = UVecsInWorldUnit | WorldPos, Y = VVecsInWorldUnit | WorldPos;

			MinUV.X = Min(X, MinUV.X);
			MinUV.Y = Min(Y, MinUV.Y);
			MaxUV.X = Max(X, MaxUV.X);
			MaxUV.Y = Max(Y, MaxUV.Y);
		}

		SurfaceMap->LightmapUOffset = -MinUV.X;
		SurfaceMap->LightmapVOffset = -MinUV.Y;
	}

	BulkConvexPolygonVertexBuffer.Unlock();
	BulkConvexPolygonIndexBuffer.Unlock();
	BulkExportElements.Unlock();
	BulkConvexPolygons.Unlock();
	BulkConvexTangentXs.Unlock();
	BulkConvexTangentYs.Unlock();
#endif
}

void FStaticMeshRenderData::ExportSurfaces( AvaRadiosityAdapter* RadiosityAdapter,
										   const FMatrix& LocalToWorld,
										   UBOOL bIsTextureMapLighting,
										   INT SubDivisionStepSize,
										   INT LightMapCoordinateIndex,
										   INT LightMapWidth,
										   UStaticMeshComponent& StaticMeshComponent
										   )
{
#if !(CONSOLE || FINAL_RELEASE)
	if( !bIsTextureMapLighting )
	{
		// ����!!! : vertex lighting �ÿ���, ������ geometry�� ����ؾ� �Ѵ�.

		// Radiosity Adapter �󿡼� ���� export �Ǵ� black mesh�� ���۵Ǵ� ��ġ
		const INT BlackMeshVertexBufferBase  = RadiosityAdapter->BlackMeshVertexBuffer_.Num();
		
		//<@ 2006. 10. 20 changmin
		// emissive element�� radiosity face�� export�� ���Դϴ�. ����, vertex lit object index���� �����ؼ�, �浹ó������ �������� �մϴ�.
		UBOOL bHasEmissiveFace = FALSE;
		const INT RadiosityFaceVertexBufferBase  = RadiosityAdapter->VertexBuffer_.Num();

		INT BlackMeshTriangleCount = 0;
		for( INT iElement = 0; iElement < Elements.Num(); ++iElement )
		{
			FStaticMeshElement* StaticMeshElement = &Elements(iElement);

			// create TexData
			UMaterialInstance* MaterialInstance		= StaticMeshComponent.GetMaterial(iElement);
			INT TextureDataIndex	= RadiosityAdapter->FindOrAddTexData(MaterialInstance);

			//check for emissive element
			const AvaTextureData& TextureData = RadiosityAdapter->TextureDatas_(TextureDataIndex);

			// export black mesh index
			{
				const INT FirstIndex	= StaticMeshElement->FirstIndex;
				const INT NumTris		= StaticMeshElement->NumTriangles;

				BlackMeshTriangleCount += NumTris;
				// copy index buffer
				// ccw -> cw
				// unreal engine 3�� render index order�� ccw�̸鼭 normal �� cw �������� �����~~
				for( int TriIndex = 0; TriIndex < NumTris; ++TriIndex )
				{
					RadiosityAdapter->BlackMeshIndexBuffer_.AddItem( BlackMeshVertexBufferBase + IndexBuffer.Indices( FirstIndex + TriIndex * 3 + 2 ) );
					RadiosityAdapter->BlackMeshIndexBuffer_.AddItem( BlackMeshVertexBufferBase + IndexBuffer.Indices( FirstIndex + TriIndex * 3 + 1 ) );
					RadiosityAdapter->BlackMeshIndexBuffer_.AddItem( BlackMeshVertexBufferBase + IndexBuffer.Indices( FirstIndex + TriIndex * 3 + 0 ) );

					RadiosityAdapter->BlackMeshTexDataIndex_.AddItem( TextureDataIndex );
				}
			}

			// export emissive element as radiosity face
			if( TextureData.Brightness_.Size() > 0.1f && BulkConvexPolygons.GetElementCount() > 0 )
			{
				// �� �Լ��� emissive face�� export �� ���� ����ؾ� �Ѵ�. ; 2008. 1. 10 changmin
				ExportElementAsRadiosityFaces( RadiosityAdapter, LocalToWorld, iElement, StaticMeshComponent, RadiosityFaceVertexBufferBase );
				const INT NumVertices					= BulkConvexPolygonVertexBuffer.GetElementCount();
				const FLegacyStaticMeshVertex* RawVertexData	= (FLegacyStaticMeshVertex*)BulkConvexPolygonVertexBuffer.Lock( LOCK_READ_ONLY );
				for( INT VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex )
				{
					FVector VertexPosition = LocalToWorld.TransformFVector( RawVertexData[VertexIndex].Position );
					RadiosityAdapter->VertexBuffer_.AddItem(  VertexPosition );
				}
				BulkConvexPolygonVertexBuffer.Unlock();
				bHasEmissiveFace = TRUE;
			}
		}		

		// triangle count �� �����ؼ� �߰��Ѵ�.
		// per static mesh data
		INT BlackMeshId = -1;
		check( BlackMeshTriangleCount > 0 );	// mesh�� ������ �ȵ�����~
		if( BlackMeshTriangleCount )
		{
			// copy vertex buffer
			const UINT NumVertices = this->NumVertices;
			FMatrix LocalToWorldInverseTranpose = LocalToWorld.Inverse().Transpose();
			for( UINT iVert = 0; iVert < NumVertices; ++iVert )
			{
				RadiosityAdapter->BlackMeshVertexBuffer_.AddItem( LocalToWorld.TransformFVector( PositionVertexBuffer.VertexPosition( iVert ) ) );
				RadiosityAdapter->BlackMeshVertexTangentXBuffer_.AddItem( LocalToWorld.TransformNormal( (FVector) VertexBuffer.VertexTangentX(iVert) ).SafeNormal() );
				RadiosityAdapter->BlackMeshVertexTangentYBuffer_.AddItem( LocalToWorld.TransformNormal( (FVector) VertexBuffer.VertexTangentY(iVert) ).SafeNormal() );
				RadiosityAdapter->BlackMeshVertexTangentZBuffer_.AddItem( LocalToWorldInverseTranpose.TransformNormal( (FVector) VertexBuffer.VertexTangentZ(iVert) ).SafeNormal() );
			}

			// vertex counts�� �����ؼ� �߰��Ѵ�.
			// per static mesh data
			RadiosityAdapter->BlackMeshVertexCounts_.AddItem(BlackMeshVertexBufferBase + NumVertices );

			// lighting info
			RadiosityAdapter->BlackMeshSampleToAreaRatios_.AddItem( 1.0f / SubDivisionStepSize );
			INT StaticMeshFlags = 0;
			if( StaticMeshComponent.bUseSubDivisions )
			{
				StaticMeshFlags |= (1<<0);
			}
			if( bHasEmissiveFace )
			{
				StaticMeshFlags |= (1<<1);
			}
			if( StaticMeshComponent.bSelfShadowing )
			{
				StaticMeshFlags |= (1<<2);
			}
			if( StaticMeshComponent.bBlockLight )
			{
				StaticMeshFlags |= (1<<3);
			}
			RadiosityAdapter->BlackMeshSampleVerticesFlags_.AddItem( StaticMeshFlags );

			const INT NumExportedBlackMesh = RadiosityAdapter->BlackMeshTriangleCounts_.Num();
			BlackMeshId = NumExportedBlackMesh;
			if( NumExportedBlackMesh )
			{
				const INT NumAccumulatedTriangles = RadiosityAdapter->BlackMeshTriangleCounts_(NumExportedBlackMesh-1);
				RadiosityAdapter->BlackMeshTriangleCounts_.AddItem( NumAccumulatedTriangles + BlackMeshTriangleCount );
			}
			else
			{
				RadiosityAdapter->BlackMeshTriangleCounts_.AddItem( BlackMeshTriangleCount );
			}
		}
		//!{ 2006-05-12	�� â ��
		StaticMeshComponent.FirstExportedFaceNumber = -1;
		StaticMeshComponent.FirstExportedVertexNumber = BlackMeshVertexBufferBase;
		//!} 2006-05-12	�� â ��
		StaticMeshComponent.ExportId = BlackMeshId;	// 2007. 11. 13 changmin
	}
	else
	{
		StaticMeshComponent.FirstExportedVertexNumber = -1;
		StaticMeshComponent.ExportId = -1;

		// radiosity face export
		if( IndexBuffer.Indices.Num() == 0 )
		{
			StaticMeshComponent.FirstExportedFaceNumber = -1;
			return;
		}

		StaticMeshComponent.FirstExportedFaceNumber = RadiosityAdapter->FaceInfos_.Num();

		// Radiosity Adapter �󿡼� ���� export �Ǵ� mesh�� ���۵Ǵ� ��ġ
		const INT VertexBufferBase  = RadiosityAdapter->VertexBuffer_.Num();
		const INT IndexBufferBase	= RadiosityAdapter->IndexBuffer_.Num();

		// copy vertex buffer
		//const INT NumVertices = ConvexPolygonVertexBuffer.Num();
		const INT NumVertices = BulkConvexPolygonVertexBuffer.GetElementCount();
		const FLegacyStaticMeshVertex* RawVertexData = (FLegacyStaticMeshVertex*)BulkConvexPolygonVertexBuffer.Lock( LOCK_READ_ONLY );
		for( INT VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex )
		{
			RadiosityAdapter->VertexBuffer_.AddItem( LocalToWorld.TransformFVector( RawVertexData[VertexIndex].Position ) );
		}
		

		// copy index buffer
		// ccw to  cw winding
		const INT NumIndices = BulkConvexPolygonIndexBuffer.GetElementCount();
		const INT* RawIndex = (INT*) BulkConvexPolygonIndexBuffer.Lock( LOCK_READ_ONLY );
		for( INT iIndex = 0; iIndex < NumIndices; ++iIndex )
		{
			RadiosityAdapter->IndexBuffer_.AddItem( VertexBufferBase + RawIndex[iIndex] );
		}
		
		// export radiosity face, 
		const AvaStaticMeshExportElement* RawExportElements = (AvaStaticMeshExportElement*)BulkExportElements.Lock( LOCK_READ_ONLY );
		const AvaConvexPolygon* RawConvexPolygons = (AvaConvexPolygon*)BulkConvexPolygons.Lock( LOCK_READ_ONLY );
		const FVector* RawTangentXs = (FVector*)BulkConvexTangentXs.Lock( LOCK_READ_ONLY );
		const FVector* RawTangentYs = (FVector*)BulkConvexTangentYs.Lock( LOCK_READ_ONLY );

		// static mesh ���� scale�� ���´�.
		// For each row, find magnitude, and if its non-zero re-scale so its unit length.
		FLOAT ScaleFactor[3];
		for(INT i=0; i<3; i++)
		{
			FVector Axis = LocalToWorld.GetAxis(i);
			ScaleFactor[i] = Axis.Size();
		}

		FLOAT StaticMeshScaleFactor = 1.0f;
		if( FloatEquals(ScaleFactor[0], ScaleFactor[1]) && FloatEquals(ScaleFactor[0], ScaleFactor[2]) && FloatEquals(ScaleFactor[1], ScaleFactor[2]) )
		{
			StaticMeshScaleFactor = (ScaleFactor[0] + ScaleFactor[1] + ScaleFactor[2]) / 3.0f;
			//warnf(NAME_Log, TEXT("There is Uniform Scaled(%f) StaticMeshComponent : %s"), StaticMeshScaleFactor, *StaticMeshComponent.GetName() );
		}
		else
		{
			//warnf(NAME_Warning, TEXT("There is Non Uniform Scaled StaticMeshComponent : %s"), *StaticMeshComponent.GetName() );
		}

		for( INT iElement = 0; iElement < Elements.Num(); ++iElement )
		{
			FStaticMeshElement* StaticMeshElement = &Elements(iElement);

			// create TexData
			UMaterialInstance* MaterialInstance = StaticMeshComponent.GetMaterial(iElement);
			INT TextureDataIndex = RadiosityAdapter->FindOrAddTexData(MaterialInstance);

			//const AvaStaticMeshExportElement& ExportElement = ExportElements(iElement);
			const AvaStaticMeshExportElement& ExportElement = RawExportElements[iElement];
			for( INT iFace = 0; iFace < ExportElement.NumPolygons_; ++iFace )
			{
				const AvaConvexPolygon& poly = RawConvexPolygons[ExportElement.PolygonStart_ + iFace];

				FVector LocalMapX;
				FVector LocalMapY;

				LocalMapX = poly.LightMapXInWorldUnit_;
				LocalMapY = poly.LightMapYInWorldUnit_;

				FVector TransformedWorldMapX = LocalToWorld.TransformNormal( LocalMapX );
				FVector TransformedWorldMapY = LocalToWorld.TransformNormal( LocalMapY );

				// �ּ� 3���� �ְ���~
				FVector WorldPosition[3];
				for( INT iVertex = 0; iVertex < 3; ++iVertex )
				{
					const FLegacyStaticMeshVertex& Vertex = RawVertexData[ RawIndex[poly.IndexStart_ + iVertex] ];
					WorldPosition[iVertex] = LocalToWorld.TransformFVector( Vertex.Position );
				}

				// colinear�� vertex ������ ������ ���� �� �ִ�......
				FPlane Plane(WorldPosition[0], WorldPosition[1], WorldPosition[2]);	//�̰� ��Ȯ.. ��� ����...
				
				// ����ȭ
				FVector WorldNormal = (FVector) Plane;
				FVector WorldMapX = ( TransformedWorldMapX - ( WorldNormal | TransformedWorldMapX ) * WorldNormal ).SafeNormal();
				FVector WorldMapY = ( TransformedWorldMapY - ( WorldNormal | TransformedWorldMapY ) * WorldNormal ).SafeNormal();
				

				// Create Surface Map Info
				AvaSurfaceMapInfo* SurfaceMap = new( RadiosityAdapter->SurfaceMaps_ ) AvaSurfaceMapInfo;

				// ��¦ ����Ȯ�ص�... �ٽ� ����ϱ� ����� �̰� ������...
				SurfaceMap->TexturemapUAxis = LocalToWorld.TransformNormal( RawTangentXs[poly.TangentXIndex_] );
				SurfaceMap->TexturemapVAxis = LocalToWorld.TransformNormal( RawTangentYs[poly.TangentYIndex_] );
				SurfaceMap->LightmapUAxis = WorldMapX.SafeNormal();
				SurfaceMap->LightmapVAxis = WorldMapY.SafeNormal();
				SurfaceMap->Flags = 0;
				SurfaceMap->ShadowMapScale = StaticMeshComponent.StaticMesh->LuxelSize * StaticMeshScaleFactor;

				// Create Face Info
				AvaFaceInfo* FaceInfo = new( RadiosityAdapter->FaceInfos_ ) AvaFaceInfo;
				FaceInfo->IndexBufferOffset	= IndexBufferBase + poly.IndexStart_;
				FaceInfo->NumVertices		= poly.NumIndex_;
				FaceInfo->iPlane			= RadiosityAdapter->PlaneBuffer_.AddItem( Plane );
				FaceInfo->iSurfaceMapInfo	= RadiosityAdapter->SurfaceMaps_.Num() - 1;
				FaceInfo->iTextureData		= TextureDataIndex;
				FaceInfo->Emission			= FVector( 0.0f, 0.0f, 0.0f );
				FaceInfo->iDispInfo			= -1;

				// calculate light map offset 
				FVector2D MinUV(WORLD_MAX,WORLD_MAX);
				FVector2D MaxUV(-WORLD_MAX,-WORLD_MAX);

				FVector UVecsInWorldUnit = WorldMapX / SurfaceMap->ShadowMapScale;
				FVector VVecsInWorldUnit = WorldMapY / SurfaceMap->ShadowMapScale;

				for( INT iVertex = 0; iVertex < poly.NumIndex_; ++iVertex )
				{
					const FLegacyStaticMeshVertex& Vertex = RawVertexData[ RawIndex[poly.IndexStart_ + iVertex] ];
					FVector WorldPos = LocalToWorld.TransformFVector( Vertex.Position );

					FLOAT	X = UVecsInWorldUnit | WorldPos, Y = VVecsInWorldUnit | WorldPos;

					MinUV.X = Min(X, MinUV.X);
					MinUV.Y = Min(Y, MinUV.Y);
					MaxUV.X = Max(X, MaxUV.X);
					MaxUV.Y = Max(Y, MaxUV.Y);
				}

				SurfaceMap->LightmapUOffset = -MinUV.X;
				SurfaceMap->LightmapVOffset = -MinUV.Y;
			}
		}
		
		BulkConvexPolygonVertexBuffer.Unlock();
		BulkConvexPolygonIndexBuffer.Unlock();
		BulkExportElements.Unlock();
		BulkConvexPolygons.Unlock();
		BulkConvexTangentXs.Unlock();
		BulkConvexTangentYs.Unlock();
	}
#endif
}


#pragma optimize("", on)