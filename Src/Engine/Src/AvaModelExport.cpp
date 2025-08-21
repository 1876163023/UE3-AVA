/*=============================================================================
AvaModelExport.cpp: Export Unreal Model to VRAD
2006-05-02	�� â ��
=============================================================================*/

#include "EnginePrivate.h"

#if !(CONSOLE || FINAL_RELEASE)
void UModelComponent::ExportSurfaces( AvaRadiosityAdapter* RadiosityAdapter )
{
	for(INT SurfaceIndex = 0;SurfaceIndex < Model->Surfs.Num();SurfaceIndex++)
	{
		FBspSurf& Surf = Model->Surfs(SurfaceIndex);
		if( Surf.PolyFlags & PF_Hint ) continue;

		const UBOOL NoLight = !(Surf.PolyFlags & PF_AcceptsLights);
		// create TexData
		UMaterialInstance* Material = Surf.Material;
		UBOOL NeedsBumpedLightmap = TRUE;
		if( Material )
		{
			// ���� : 2007. 2. 20
			// �÷��� ���� �Ʒ��� �÷��װ� ���� �� ������, Lighting�� �ְ� �÷������� �ؾ��Ѵ�.
			FMaterialInstance* MaterialInstance = Material->GetMaterial()->GetInstanceInterface(FALSE);
			const FMaterial* RenderMaterial = MaterialInstance->GetMaterial();
			if( !RenderMaterial->NeedsBumpedLightmap() )
			{
				NeedsBumpedLightmap = FALSE;
			}
		}
		INT TexDataIndex = RadiosityAdapter->FindOrAddTexData( Material);

		// Find a plane parallel to the surface.
		FVector MapX;
		FVector MapY;
		Surf.Plane.FindBestAxisVectors(MapX,MapY);

		FVector BackMapX;
		FVector BackMapY;
		if(Surf.PolyFlags & PF_TwoSided )
		{
			FPlane BackPlane = Surf.Plane * (-1);
			BackPlane.FindBestAxisVectors(BackMapX, BackMapY);
		}

		// Find the surface's nodes and the part of the plane they map to.
		TArray<WORD> SurfaceNodes;

		FVector2D MinUV(WORLD_MAX,WORLD_MAX);
		FVector2D MaxUV(-WORLD_MAX,-WORLD_MAX);

		for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
		{
			FBspNode& Node = Model->Nodes(Nodes(NodeIndex));

			if(Node.iSurf == SurfaceIndex)
			{
				// Add the node to the list of nodes in this surface.
				SurfaceNodes.AddItem((WORD)Nodes(NodeIndex));
			}
		}

		// Ignore the surface if it has no nodes in this component's zone.
		if(!SurfaceNodes.Num())
		{
			continue;
		}

		// FaceInfo / plane buffer / surface map / vertex buffer / Index buffer ä��� : Node�� Vertex ������� ä���.

		FVector UVecsInWorldUnits = MapX / Surf.ShadowMapScale;
		FVector VVecsInWorldUnits = MapY / Surf.ShadowMapScale;


		FVector2D MinSurfaceUV(WORLD_MAX,WORLD_MAX);
		FVector2D MaxSurfaceUV(-WORLD_MAX,-WORLD_MAX);

		for(INT NodeIndex = 0; NodeIndex < SurfaceNodes.Num(); ++NodeIndex )
		{
			INT NodeIndexInModel = SurfaceNodes(NodeIndex);

			//!{ 2006-05-16	�� â ��
			// export �� ���� ������, �ǳʶٱ�
			if( Model->NodeToExportedFace( NodeIndexInModel ) != -1 )
			{
				continue;
			}
			//!} 2006-05-16	�� â ��

			FBspNode& Node = Model->Nodes(SurfaceNodes(NodeIndex));

			// Node UV Min / Max�� ����Ѵ�.

			for(UINT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
			{
				FVector	Position = Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex);

				FLOAT	X = UVecsInWorldUnits | Position, Y = VVecsInWorldUnits | Position;
				MinSurfaceUV.X = Min(X,MinSurfaceUV.X);
				MinSurfaceUV.Y = Min(Y,MinSurfaceUV.Y);
				MaxSurfaceUV.X = Max(X,MaxSurfaceUV.X);
				MaxSurfaceUV.Y = Max(Y,MaxSurfaceUV.Y);
			}			
		}		

		for(INT NodeIndex = 0; NodeIndex < SurfaceNodes.Num(); ++NodeIndex )
		{
			INT NodeIndexInModel = SurfaceNodes(NodeIndex);

			//!{ 2006-05-16	�� â ��
			// export �� ���� ������, �ǳʶٱ�
			if( Model->NodeToExportedFace( NodeIndexInModel ) != -1 )
			{
				continue;
			}
			//!} 2006-05-16	�� â ��

			FBspNode& Node = Model->Nodes(SurfaceNodes(NodeIndex));

			FVector2D MinNodeUV(WORLD_MAX,WORLD_MAX);
			FVector2D MaxNodeUV(-WORLD_MAX,-WORLD_MAX);

			// Node UV Min / Max�� ����Ѵ�.

			for(UINT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
			{
				FVector	Position = Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex);

				FLOAT	X = UVecsInWorldUnits | Position, Y = VVecsInWorldUnits | Position;
				MinNodeUV.X = Min(X,MinNodeUV.X);
				MinNodeUV.Y = Min(Y,MinNodeUV.Y);
				MaxNodeUV.X = Max(X,MaxNodeUV.X);
				MaxNodeUV.Y = Max(Y,MaxNodeUV.Y);
			}

			// sse�� ����ؼ�, vrad�� �����ϰ� ���ǵ��� �ִ��� �����ؾ� �Ѵ�.
			FMatrix WorldToLuxel = FMatrix(
				FPlane(UVecsInWorldUnits.X,		VVecsInWorldUnits.X,		Surf.Plane.X,	0),
				FPlane(UVecsInWorldUnits.Y,		VVecsInWorldUnits.Y,		Surf.Plane.Y,	0),
				FPlane(UVecsInWorldUnits.Z,		VVecsInWorldUnits.Z,		Surf.Plane.Z,	0),
				FPlane(-MinNodeUV.X,			-MinNodeUV.Y,				-Surf.Plane.W,	1)
				);

			FVector2D MinLuxel(WORLD_MAX, WORLD_MAX);
			FVector2D MaxLuxel(-WORLD_MAX, -WORLD_MAX);
			for(UINT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
			{
				FVector	Position = Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex);
				FVector Luxel = WorldToLuxel.TransformFVector( Position );
				MinLuxel.X = Min(Luxel.X, MinLuxel.X);
				MinLuxel.Y = Min(Luxel.Y, MinLuxel.Y);
				MaxLuxel.X = Max(Luxel.X, MaxLuxel.X);
				MaxLuxel.Y = Max(Luxel.Y, MaxLuxel.Y);
			}			

			MinLuxel.X = (FLOAT) appFloor( MinLuxel.X );
			MinLuxel.Y = (FLOAT) appFloor( MinLuxel.Y );
			MaxLuxel.X = (FLOAT) appCeil( MaxLuxel.X );
			MaxLuxel.Y = (FLOAT) appCeil( MaxLuxel.Y );

			INT LightmapWidth	= ((INT)(MaxLuxel.X - MinLuxel.X)) + 1;
			INT LightmapHeight	= ((INT)(MaxLuxel.Y - MinLuxel.Y)) + 1;

			//!{ 2006-04-13	�� â ��
			Model->NodeToExportedFace(SurfaceNodes(NodeIndex)) = RadiosityAdapter->FaceInfos_.Num();
			//!} 2006-04-13	�� â ��

			// 2006/8/28 changmin... PF_TwoSided�� �ո鸸 �����Ѵ�.. ���߿� ��� �������� Ȯ���Ұ�
			//for( UINT BackFace = 0; BackFace < (UINT) ((Surf.PolyFlags & PF_TwoSided ) ? 2 : 1 ); ++BackFace )
			for( UINT BackFace = 0; BackFace < 1 ; ++BackFace )
			{
				// SurfaceMapInfo�� �߰��Ѵ�.
				AvaSurfaceMapInfo* SurfaceMap = new(RadiosityAdapter->SurfaceMaps_) AvaSurfaceMapInfo;

				RadiosityAdapter->Displacements.AddItem(MinNodeUV - MinSurfaceUV);
				RadiosityAdapter->MaxLuxels.AddItem(MaxLuxel);

				SurfaceMap->TexturemapUAxis = Model->Vectors( Model->Surfs( Node.iSurf ).vTextureU ).SafeNormal();
				SurfaceMap->TexturemapVAxis = Model->Vectors( Model->Surfs( Node.iSurf ).vTextureV ).SafeNormal();
				SurfaceMap->LightmapUAxis	= BackFace ? BackMapX : MapX;
				SurfaceMap->LightmapVAxis	= BackFace ? BackMapY : MapY;
				SurfaceMap->ShadowMapScale = Surf.ShadowMapScale;
				SurfaceMap->LightmapUOffset = -MinNodeUV.X;
				SurfaceMap->LightmapVOffset = -MinNodeUV.Y;
				SurfaceMap->Flags = 0;
				if( NoLight )
				{
					SurfaceMap->Flags |= SURF_NOLIGHT;
				}
				if( NeedsBumpedLightmap )
				{
					SurfaceMap->Flags |= SURF_BUMPLIGHT;
				}

				//<@ 2007. 8. 14 changmin
				// Secondary Light Source
				if( Surf.PolyFlags & PF_SecondaryLightSource )
				{
					SurfaceMap->Flags |= SURF_SECONDARYLIGHTSOURCE;
					debugf( NAME_Log, TEXT("add secondary light source flags to face %d"), RadiosityAdapter->FaceInfos_.Num() );
				}
				//>@ changmin

				// Face Info�� �����.
				AvaFaceInfo* Face		= new(RadiosityAdapter->FaceInfos_) AvaFaceInfo;
				Face->IndexBufferOffset	= RadiosityAdapter->IndexBuffer_.Num();
				Face->NumVertices		= Node.NumVertices;

				// for debug complex node polygon
				if( Face->NumVertices == FBspNode::MAX_NODE_VERTICES )
				{
					for(UINT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
					{
						const FVector& Position0 = Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex);
						const FVector& Position1 = Model->Points(Model->Verts(Node.iVertPool + (VertexIndex + 1)%Node.NumVertices).pVertex);
						GWorld->PersistentLineBatcher->DrawLine( Position0, Position1, FLinearColor(1.0f, 0.0f, 0.0f), SDPG_Foreground );
					}
				}
				Face->iPlane = RadiosityAdapter->PlaneBuffer_.AddUniqueItem( BackFace ? FPlane( -1 * Surf.Plane, -Surf.Plane.W ) : Surf.Plane );	// Plane Buffer�� ä���.
				Face->iSurfaceMapInfo = RadiosityAdapter->SurfaceMaps_.Num() - 1;
				Face->iTextureData = TexDataIndex;
				Face->Emission = FVector( 0.0f, 0.0f, 0.0f );
				Face->LightmapInfo.Width = LightmapWidth;
				Face->LightmapInfo.Height = LightmapHeight;
				Face->iDispInfo = -1;

				// radiosity light map ��ǥ�� / index buffer �� �����.
				for( INT VertexIndex = 0; VertexIndex < Node.NumVertices; ++VertexIndex )
				{
					INT VertexIndexInModel = Model->Verts(Node.iVertPool + VertexIndex).pVertex;
					FVector Position = Model->Points( VertexIndexInModel );

					// �� vertex �� radiosity lightmap �� ���� ��ǥ�� ����Ѵ�.
					FVector UVForRadiosity = WorldToLuxel.TransformFVector(Position);

					if( UVForRadiosity.X < 0.0f || UVForRadiosity.Y < 0.0f )
					{
						UVForRadiosity.X = Max<FLOAT>( UVForRadiosity.X, 0.0f );
						UVForRadiosity.Y = Max<FLOAT>( UVForRadiosity.Y, 0.0f );
					}
					Face->RadiosityLightmapCoords.AddItem( FVector2D( UVForRadiosity.X, UVForRadiosity.Y ) );
					RadiosityAdapter->IndexBuffer_.AddItem( Model->ExportedVertexBase + VertexIndexInModel );
				}
			}
		}	
	}
}
#else
void UModelComponent::ExportSurfaces( AvaRadiosityAdapter* RadiosityAdapter )
{}
#endif