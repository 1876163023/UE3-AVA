/*=============================================================================
AvaModelLight.cpp: Unreal model Radisity lighting.
Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnRaster.h"
#include "AvaStaticLighting.h"
#include "UnTextureLayout.h"

#pragma DISABLE_OPTIMIZATION


#define SHADOWMAP_MAX_WIDTH			2048
#define SHADOWMAP_MAX_HEIGHT		2048

//!{ 2006-05-02	허 창 민
// AvaCode를 Unreal Code에서 분리하기 위해 복사해옴.

//
//	Definitions.
//



struct FSurfaceShadowMapInfo2
{
	ULightComponent			*Light;
	UPointLightComponent	*PointLight;
	FPlane LightPosition;
	FLOAT LightRadiusSquared;

	UBOOL Irrelevant;
	UBOOL IsShadowCast;
	TArray<BYTE> Visibility;

	/**
	* Minimal initialization constructor.
	*/
	FSurfaceShadowMapInfo2(ULightComponent* InLight): Light(InLight), Irrelevant(1)
	{
		LightPosition = Light->GetPosition();
		PointLight = Cast<UPointLightComponent>(Light);
		LightRadiusSquared = PointLight ? Square(PointLight->Radius) : 0.0f;
		IsShadowCast = Light->CastShadows && Light->CastStaticShadows;
	}
};

//!} 2006-05-02	허 창 민


struct FNodeMapInfo
{
	FNodeMapInfo* NextNode;

	INT NodeIndex;
	
	FVector2D Displacement, MaxLuxel;
	
	UINT SizeX;
	UINT SizeY;

	void ComputeBaseXY( UINT& BaseX, UINT& BaseY, INT SurfaceX, INT SurfaceY ) const
	{
#if GROUP_NODES_ON_SURFACE
		BaseX = SurfaceX + (INT)Displacement.X;
		BaseY = SurfaceY + (INT)Displacement.Y;
#else
		BaseX = SurfaceX;
		BaseY = SurfaceY;
#endif
	}

	UBOOL bCastSunShadow;

	UBOOL bHasLightmap;

	void ComputeSizeXY( UModel* Model, FBspNode& Node, const FBspSurf& Surf )
	{
		FMatrix WorldToMap, MapToWorld;

		FVector2D MinUV(WORLD_MAX,WORLD_MAX);
		FVector2D MaxUV(-WORLD_MAX,-WORLD_MAX);
		FVector MapX, MapY;
		for( INT VertexIndex = 0; VertexIndex < Node.NumVertices; ++VertexIndex )
		{
			FVector	Position = Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex);
			FLOAT	X = MapX | Position,
				Y = MapY | Position;
			MinUV.X = Min(X, MinUV.X);
			MinUV.Y = Min(Y, MinUV.Y);
			MaxUV.X = Max(X, MaxUV.X);
			MaxUV.Y = Max(Y, MaxUV.Y);
		}

		// Create a Node Mapping.		
		WorldToMap = FMatrix(
			FPlane(MapX.X / (MaxUV.X - MinUV.X),	MapY.X / (MaxUV.Y - MinUV.Y),	Surf.Plane.X,	0),
			FPlane(MapX.Y / (MaxUV.X - MinUV.X),	MapY.Y / (MaxUV.Y - MinUV.Y),	Surf.Plane.Y,	0),
			FPlane(MapX.Z / (MaxUV.X - MinUV.X),	MapY.Z / (MaxUV.Y - MinUV.Y),	Surf.Plane.Z,	0),
			FPlane(-MinUV.X / (MaxUV.X - MinUV.X),	-MinUV.Y / (MaxUV.Y - MinUV.Y),	-Surf.Plane.W,	1)
			);
		MapToWorld = WorldToMap.Inverse();

		SizeX = Clamp(appCeil((MaxUV.X - MinUV.X) / Surf.ShadowMapScale),2,SHADOWMAP_MAX_WIDTH);
		SizeY = Clamp(appCeil((MaxUV.Y - MinUV.Y) / Surf.ShadowMapScale),2,SHADOWMAP_MAX_HEIGHT);
	}

	FNodeMapInfo( UModel* Model, FBspNode& Node, const FBspSurf& Surf, INT InNodeIndex, FNodeMapInfo* InNextNode, AvaRadiosityAdapter& RadiosityAdapter )
		: SizeX(0), SizeY(0), bCastSunShadow(FALSE), NextNode(InNextNode)
	{
		NodeIndex = InNodeIndex;

		// Find a plane parallel to the surface.
		FVector MapX;
		FVector MapY;
		Surf.Plane.FindBestAxisVectors(MapX,MapY);

		bHasLightmap = (Surf.PolyFlags & PF_AcceptsLights) && !(Surf.PolyFlags & PF_Hint);

		if (!bHasLightmap)
		{
			SizeX = 1;
			SizeY = 1;

			//<@ 2007. 11. 13 changmin
			// gi 계산속도 향상을 위해, 이 face에 대한 shadow 정보는 없습니다.
			// 그래서 기본값으로 세팅합니다.
			bCastSunShadow = TRUE;
			Node.NodeFlags |= NF_SunVisible;
			//>@ 
		}
		else
		{		
			AvaFaceInfo* Face = &RadiosityAdapter.FaceInfos_(Model->NodeToExportedFace(NodeIndex));
			if( !Face )
			{
				ComputeSizeXY( Model, Node, Surf );
			}
			else
			{
				AvaLightmapInfo& LightmapInfo = Face->LightmapInfo;
				SizeX = LightmapInfo.Width;
				SizeY = LightmapInfo.Height;

				//<@ ava specific ; 2008. 1. 24 changmin
				// add cascaded shadow
				//// node 는 backface culling 만 합니다.
				//if( SunLight )
				//{
				//	UDirectionalLightComponent* DirectionalLight = Cast<UDirectionalLightComponent>(SunLight);
				//	UBOOL BackFacing = (FVector(Node.Plane) | DirectionalLight->GetDirection()) > 0.0f;
				//	if( !BackFacing )
				//	{
				//		Node.NodeFlags |= NF_SunVisible;
				//	}
				//	else
				//	{
				//		Node.NodeFlags &= ~NF_SunVisible;
				//	}
				//}
				////>@ 

				//<@ 2007. 11. 13 changmin
				// 노드 하나라도 sun을 보면, cast sun shadow가 켜진다.
				if( Face->bSunVisible == 1 || (Surf.PolyFlags & PF_ForceCastSunShadow) )
				{
					bCastSunShadow = TRUE;
					Node.NodeFlags |= NF_SunVisible;
				}
				else
				{
					// clear flags
					Node.NodeFlags &= ~NF_SunVisible;
				}
				//>@ 
			}
		}

		if (bHasLightmap)
		{
			AvaFaceInfo* Face = &RadiosityAdapter.FaceInfos_(Model->NodeToExportedFace(NodeIndex));

			Displacement = RadiosityAdapter.Displacements( Face->iSurfaceMapInfo );
			MaxLuxel = RadiosityAdapter.MaxLuxels( Face->iSurfaceMapInfo );
		}		
	}
};

#if !(CONSOLE || FINAL_RELEASE)

struct FAvaModelSurfaceStaticLighting : FAvaSurfaceStaticLighting
{
	INT SurfIndex;		

	FNodeMapInfo* FirstNode;

	FAvaModelSurfaceStaticLighting( FAvaComponentStaticLighting* InComponent, UModel* Model, INT InSurfIndex, const FBspSurf& Surf )
		: FAvaSurfaceStaticLighting(
			InComponent,
			Surf.Material,
			Model->Vectors(Surf.vTextureU).SafeNormal(),
			Model->Vectors(Surf.vTextureV).SafeNormal(),
			Model->Vectors(Surf.vNormal).SafeNormal()
			), 
		SurfIndex( InSurfIndex ),
		FirstNode( NULL )
	{		
	}
};

struct FAvaModelComponentStaticLighting : FAvaComponentStaticLighting
{
	virtual FAvaSurfaceStaticLighting* GetSurface( INT SurfaceIndex ) 
	{
		return &Surfaces(SurfaceIndex);
	}

	TArray<FAvaModelSurfaceStaticLighting>	Surfaces;

	UModelComponent*				Component;	
	AvaRadiosityAdapter&			RadiosityAdapter;

	TArray<ULightComponent*>		RelevantLights;
	TArray<FNodeMapInfo>			NodeMaps;		

	FComponentReattachContext		ReattachContext;	

	FAvaModelComponentStaticLighting( UModelComponent* InComponent, AvaRadiosityAdapter& InRadiosityAdapter )
		: ReattachContext(InComponent), Component(InComponent), RadiosityAdapter(InRadiosityAdapter), FAvaComponentStaticLighting( InComponent->GetOutermost(), InComponent->Bounds )
	{
	}

	void EnqueueSurfaces() 
	{		
		UModel* Model = Component->GetModel();

#if GROUP_NODES_ON_SURFACE
		TArray<INT> SurfaceIndices;

		SurfaceIndices.Reserve( Model->Surfs.Num() );

		// component를 구성하는 node들에 대해 mapping 정보를 생성한다.
		for( INT NodeIndex = 0; NodeIndex < Component->Nodes.Num(); ++NodeIndex )
		{
			FBspNode& Node = Model->Nodes( Component->Nodes(NodeIndex) );			

			SurfaceIndices.AddUniqueItem( Node.iSurf );
		}

		NumSurfaces = SurfaceIndices.Num();

		// To prevent memory fragmentation
		Surfaces.Reserve( NumSurfaces );
		NodeMaps.Reserve( Component->Nodes.Num() );		

		// 같은 surface에 있는 node끼리 묶어야!
		for(INT SurfIndex = 0; SurfIndex < NumSurfaces; ++SurfIndex)
		{
			new(Surfaces) FAvaModelSurfaceStaticLighting( this, Model, SurfaceIndices(SurfIndex), Model->Surfs(SurfaceIndices(SurfIndex)) );			
		}		

		// Surface group 정보 생성
		for(INT SurfaceGroupIndex= 0; SurfaceGroupIndex < NumSurfaces; ++SurfaceGroupIndex)
		{		
			FAvaModelSurfaceStaticLighting* Surface = &Surfaces(SurfaceGroupIndex);
			INT SurfIndex = Surface->SurfIndex;
			FBspSurf& Surf = Model->Surfs(SurfIndex);		

			Surface->SizeX = 0;
			Surface->SizeY = 0;

			// component를 구성하는 node들에 대해 mapping 정보를 생성한다.
			for( INT NodeIndex = 0; NodeIndex < Component->Nodes.Num(); ++NodeIndex )
			{
				FBspNode& Node = Model->Nodes( Component->Nodes(NodeIndex) );			

				if (Node.iSurf == SurfIndex)
				{
					// Create a Node Mapping.
					FNodeMapInfo* NodeMap = new(NodeMaps) FNodeMapInfo( Model, Node, Surf, Component->Nodes(NodeIndex), Surface->FirstNode, RadiosityAdapter );				
					Surface->FirstNode = NodeMap;

					Component->bCastSunShadow |= NodeMap->bCastSunShadow;

					if (NodeMap->bHasLightmap)
					{						
						Surface->SizeX = Max<INT>( Surface->SizeX, NodeMap->Displacement.X + NodeMap->MaxLuxel.X + 1 );
						Surface->SizeY = Max<INT>( Surface->SizeY, NodeMap->Displacement.Y + NodeMap->MaxLuxel.Y + 1 );											

						Surface->bHasLightmap = TRUE;
					}				
				}
			}	

			check( Surface->FirstNode != NULL );
		}		
#else
		NumSurfaces = Component->Nodes.Num();

		// To prevent memory fragmentation
		Surfaces.Reserve( NumSurfaces );
		NodeMaps.Reserve( Component->Nodes.Num() );		

		// 같은 surface에 있는 node끼리 묶어야!
		for(INT NodeIndex = 0; NodeIndex < NumSurfaces; ++NodeIndex)
		{
			FBspNode& Node = Model->Nodes( Component->Nodes(NodeIndex) );						
			FBspSurf& Surf = Model->Surfs(Node.iSurf);		

			FAvaModelSurfaceStaticLighting* Surface = new(Surfaces) FAvaModelSurfaceStaticLighting( this, Model, Node.iSurf, Surf );			

			// Create a Node Mapping.
			FNodeMapInfo* NodeMap = new(NodeMaps) FNodeMapInfo( Model, Node, Surf, Component->Nodes(NodeIndex), Surface->FirstNode, RadiosityAdapter );				
			Surface->FirstNode = NodeMap;

			Component->bCastSunShadow |= NodeMap->bCastSunShadow;

			if (NodeMap->bHasLightmap)
			{					
				AvaFaceInfo *Face = &RadiosityAdapter.FaceInfos_(Model->NodeToExportedFace(NodeMap->NodeIndex));
				if( Face )
				{
					AvaLightmapInfo &LightmapInfo = Face->LightmapInfo;

					Surface->SizeX = LightmapInfo.Width;
					Surface->SizeY = LightmapInfo.Height;

					Surface->bHasLightmap = TRUE;
				}
			}				
		}		
#endif

		// Find the static lights relevant to the model.		
		GWorld->FindRelevantLights(Component,FALSE,RelevantLights);
		for(INT LightIndex = 0;LightIndex < RelevantLights.Num();LightIndex++)
		{
			ULightComponent* Light = RelevantLights(LightIndex);
			if( ! ((Light->UseDirectLightMap || Light->IsShadowCast(Component)) && Light->HasStaticShadowing()) )
			{
				RelevantLights.Remove(LightIndex--);
			}
		}			
	}

	virtual void GenerateShadowCoordinates( FAvaSurfaceStaticLighting* InSurface ) 
	{
		FAvaModelSurfaceStaticLighting* Surface = (FAvaModelSurfaceStaticLighting*)InSurface;

		UModel* Model = Component->Model;
		
		INT SurfIndex = Surface->SurfIndex;
		FBspSurf& Surf = Model->Surfs(SurfIndex);		

		if (!Surface->bHasLightmap)
		{
			return;
		}

		FAvaStaticLightmapPage* LightmapPage = Surface->LightmapPage;

		for(FNodeMapInfo* NodeMap = Surface->FirstNode; NodeMap; NodeMap = NodeMap->NextNode)
		{			
			FBspNode& Node = Model->Nodes( NodeMap->NodeIndex );

			check(Node.iSurf == SurfIndex);

			check(NodeMap->bHasLightmap);											
		}		

		check(Surface->bHasLightmap);

		for(FNodeMapInfo* NodeMap = Surface->FirstNode; NodeMap; NodeMap = NodeMap->NextNode)
		{			
			FBspNode& Node = Model->Nodes( NodeMap->NodeIndex );

			check(Node.iSurf == SurfIndex);

			// accept light가 꺼져있거나, hint인 경우, 위에서.. 계산했습니다.. 0으로..
			if (!NodeMap->bHasLightmap)
			{
				for(INT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
				{
					FVert& Vert = Model->Verts(Node.iVertPool + VertexIndex);					
					Vert.ShadowTexCoord = FVector2D(0,0);
				}
				continue;
			}
			UBOOL IsFrontVisible = !(Surf.PolyFlags & PF_TwoSided) || Node.iZone[1] == Component->ZoneIndex || Component->ZoneIndex == INDEX_NONE;
			UBOOL IsBackVisible = (Surf.PolyFlags & PF_TwoSided) && (Node.iZone[0] == Component->ZoneIndex || Component->ZoneIndex == INDEX_NONE);
			AvaFaceInfo* Face = &RadiosityAdapter.FaceInfos_(Model->NodeToExportedFace(NodeMap->NodeIndex));

			UINT BaseX, BaseY;
			NodeMap->ComputeBaseXY(BaseX,BaseY,Surface->BaseX,Surface->BaseY);

			if(IsFrontVisible)
			{
				for(INT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
				{
					FVert& Vert = Model->Verts(Node.iVertPool + VertexIndex);
					FVector2D LightMapCoords = Face->RadiosityLightmapCoords(VertexIndex);
					LightMapCoords += FVector2D( BaseX, BaseY );
					LightMapCoords /= FVector2D( LightmapPage->TextureLayout->GetSizeX(), LightmapPage->TextureLayout->GetSizeY() );
					Vert.ShadowTexCoord = LightMapCoords;
				}
			}
			if(IsBackVisible)
			{
				for(INT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
				{
					FVert& Vert = Model->Verts(Node.iVertPool + VertexIndex);
					FVector2D LightMapCoords = Face->RadiosityLightmapCoords(VertexIndex);
					LightMapCoords += FVector2D( BaseX, BaseY );
					LightMapCoords /= FVector2D( LightmapPage->TextureLayout->GetSizeX(), LightmapPage->TextureLayout->GetSizeY() );
					Vert.ShadowTexCoord = LightMapCoords;
				}
			}
		}
	}

	virtual void FillLightmap( FAvaSurfaceStaticLighting* InSurface, FLightMapData2D* LightMapData, FLightMapData2D *LightMapData_2 )
	{
		FAvaModelSurfaceStaticLighting* Surface = (FAvaModelSurfaceStaticLighting*)InSurface;				

		FAvaStaticLightmapPage* LightmapPage = Surface->LightmapPage;

		UModel* Model = Component->Model;

		INT SurfIndex = Surface->SurfIndex;
		FBspSurf& Surf = Model->Surfs(SurfIndex);		

		check(Surface->bHasLightmap);

		for(FNodeMapInfo* NodeMap = Surface->FirstNode; NodeMap; NodeMap = NodeMap->NextNode)
		{			
			FBspNode& Node = Model->Nodes( NodeMap->NodeIndex );

			check(Node.iSurf == SurfIndex);

			// Copy Lightmap Data~~~~
			check( NUM_AVA_LIGHTMAPS == 4 );			

			// accept light가 꺼져있거나, hint인 경우, radiosity lightmap은 없습니다.
			if (!NodeMap->bHasLightmap)
			{
				continue;
			}

			UINT BaseX, BaseY;
			NodeMap->ComputeBaseXY(BaseX,BaseY,Surface->BaseX,Surface->BaseY);

			UBOOL IsFrontVisible	= !(Surf.PolyFlags & PF_TwoSided) || Node.iZone[1] == Component->ZoneIndex || Component->ZoneIndex == INDEX_NONE;
			UBOOL IsBackVisible		= (Surf.PolyFlags & PF_TwoSided) && (Node.iZone[0] == Component->ZoneIndex || Component->ZoneIndex == INDEX_NONE);
			AvaFaceInfo *Face = &RadiosityAdapter.FaceInfos_(Model->NodeToExportedFace(NodeMap->NodeIndex));
			if( Face )
			{
				AvaLightmapInfo &LightmapInfo = Face->LightmapInfo;
				unsigned char *RadiosityLightMap = LightmapInfo.Lightmap;
				unsigned char *RadiosityLightMap_2 = LightmapInfo.LightmapWithoutSun;

				INT NumLuxels	= LightmapInfo.Width * LightmapInfo.Height;
				INT Width		= LightmapInfo.Width;
				INT Height		= LightmapInfo.Height;
				// boundary check
				Width	= (BaseX + Width > LightMapData->GetSizeX())	? (LightMapData->GetSizeX() - BaseX) : Width;
				Height	= (BaseY + Height > LightMapData->GetSizeY())	? (LightMapData->GetSizeY() - BaseY) : Height;
				if (RadiosityLightMap && RadiosityLightMap_2)
				{
					const INT LightmapCounts = (RadiosityAdapter.SurfaceMaps_(Face->iSurfaceMapInfo).Flags & SURF_BUMPLIGHT) ? NUM_AVA_LIGHTMAPS : 1;
					for( INT Y = 0; Y < Height; ++Y )
					{
						for( INT X = 0; X < Width; ++X )
						{
							FLightSample& DestSample = (*LightMapData)( BaseX + X, BaseY + Y);
							FLightSample& DestSample_2 = (*LightMapData_2)( BaseX + X, BaseY + Y);
							for( INT iBump = 0; iBump < LightmapCounts; ++iBump )
							{
								FLinearColor LightMapColor;
								LightmapInfo.GetSample( X, Y, iBump, &LightMapColor.R, &LightMapColor.G, &LightMapColor.B );
								DestSample.AddLight( LightMapColor, iBump );
								LightmapInfo.GetSampleWithoutSun(X, Y, iBump, &LightMapColor.R, &LightMapColor.G, &LightMapColor.B);
								DestSample_2.AddLight( LightMapColor, iBump );
							}
							(*LightMapData)( BaseX + X, BaseY + Y).bIsMapped = TRUE;
							(*LightMapData_2)( BaseX + X, BaseY + Y).bIsMapped = TRUE;
						}
					}
				}
			}
		}
	}
	
	void RebuildElements( FAvaStaticLighting* StaticLighting )
	{
		UModel* Model = Component->Model;

		// Rebuild the model elements with the new shadow maps.
		//TODO:: acceptlight가 꺼져 있는 element만 모아봅시다.. editor에서 필요할테니..
		Component->Elements.Empty();
		for(INT LightGroupIndex = 0;LightGroupIndex < StaticLighting->Lightmaps.Num();LightGroupIndex++)
		{
			FAvaStaticLightmapPage* LightmapPage = StaticLighting->Lightmaps(LightGroupIndex);

			// Build the set of irrelevant lights for this group.
			TArray<FGuid> IrrelevantLights;
			for(INT LightIndex = 0;LightIndex < RelevantLights.Num();LightIndex++)
			{
				ULightComponent* Light = RelevantLights(LightIndex);
				if(!(LightmapPage->LightMap && LightmapPage->LightMap->ContainsLight(Light->LightmapGuid)))
				{
					IrrelevantLights.AddItem(RelevantLights(LightIndex)->LightGuid);
				}
			}

			for(INT SurfaceGroupIndex= 0; SurfaceGroupIndex < NumSurfaces; ++SurfaceGroupIndex)
			{		
				FAvaModelSurfaceStaticLighting* Surface = &Surfaces(SurfaceGroupIndex);

				if (Surface->LightmapPage != LightmapPage)
					continue;

				INT SurfIndex = Surface->SurfIndex;
				FBspSurf& Surf = Model->Surfs(SurfIndex);		

				check(Surface->bHasLightmap);

				UMaterialInstance	*Material	= Surface->Material;

				for(FNodeMapInfo* NodeMap = Surface->FirstNode; NodeMap; NodeMap = NodeMap->NextNode)
				{					
					FBspNode& Node = Model->Nodes( NodeMap->NodeIndex );

					check(Node.iSurf == SurfIndex);

					// Find an existing element which matches this surface's material and relevant light set.
					FModelElement* Element = NULL;
					for(INT ElementIndex = 0;ElementIndex < Component->Elements.Num();ElementIndex++)
					{
						FModelElement& ExistingElement = Component->Elements(ElementIndex);
						if(	ExistingElement.Material			== Material
							&&	ExistingElement.LightMap			== LightmapPage->LightMap
							&&	ExistingElement.IrrelevantLights	== IrrelevantLights )
						{
							Element = &ExistingElement;
							break;
						}
					}
					if(!Element)
					{
						// There's no existing element which matches this surface's material and relevant light set.  Create a new element.
						Element = new(Component->Elements) FModelElement(Component,Material);
						Element->IrrelevantLights	= IrrelevantLights;
						Element->LightMap			= LightmapPage->LightMap;
					}

					// Add  node to the matching element.
					Element->Nodes.AddItem(NodeMap->NodeIndex);
				}
			}
		}		

		// Build the render data for the new elements.
		Component->BuildRenderData();
	}
};

#endif

void UModelComponent::CacheRadiosityLighting( const FLightingBuildOptions& BuildOptions, AvaRadiosityAdapter& RadiosityAdapter )
{
#if !(CONSOLE || FINAL_RELEASE)
	// Invalidate existing lighting.
	InvalidateLightingCache();	

	if( !HasStaticShadowing() )
	{
		FComponentReattachContext ReattachContext(this);	

		return;
	}

	//<@ 2007. 11. 13 changmin
	// add cascaded shadow
	bCastSunShadow = FALSE;
	//>@ ava		

	FAvaModelComponentStaticLighting* Component = new FAvaModelComponentStaticLighting( this, RadiosityAdapter );

	Component->EnqueueSurfaces();

	GAvaStaticLighting->AddComponent( Component );	
#endif
}

