/*=============================================================================
	UnStaticMeshRender.cpp: Static mesh rendering code.
	Copyright 1997-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EnginePhysicsClasses.h"
#include "EngineDecalClasses.h"
#include "UnDecalRenderData.h"
#include "LevelUtils.h"

// Needed for FStaticMeshSceneProxy::DrawShadowVolumes().
#include "ScenePrivate.h"

#if XBOX
// Contains the optimized plane dots function
#include "UnStaticMeshRenderXe.h"
#endif

#include "DecalRendering.h"
#include "UnStaticMeshLegacy.h"

FDecalRenderData* UStaticMeshComponent::GenerateDecalRenderData(FDecalState* Decal) const
{
	// Do nothing if the specified decal doesn't project on static meshes.
	if ( !Decal->bProjectOnStaticMeshes )
	{
		return NULL;
	}

	// Perform a kDOP query
	const FStaticMeshCollisionDataProvider MeshData( this );
	TArray<DWORD> Leaves;
	TkDOPFrustumQuery<FStaticMeshCollisionDataProvider> kDOPQuery( static_cast<FPlane*>( Decal->Planes.GetData() ),
																	Decal->Planes.Num(),
																	Leaves,
																	MeshData );

	const UStaticMesh::kDOPTreeType& kDOPTree = StaticMesh->kDOPTree;
	const UBOOL bHit = kDOPTree.FrustumQuery( kDOPQuery );
	if ( !bHit )
	{
		return NULL;
	}

	FDecalRenderData* DecalRenderData = NULL;

	// Transform decal properties into local space.
	const FDecalLocalSpaceInfo DecalInfo( *Decal, LocalToWorld.Inverse() );
	const FStaticMeshRenderData& StaticMeshRenderData = StaticMesh->LODModels(0);	

	// Is the decal lit?
	const UMaterial* DecalMaterial = Decal->DecalMaterial->GetMaterial();
	const UBOOL bLitDecal = DecalMaterial->LightingModel != MLM_Unlit;

	// Static mesh light map info.
	INT LightMapWidth	= 0;
	INT LightMapHeight	= 0;
	if ( bLitDecal )
	{
		GetLightMapResolution( LightMapWidth, LightMapHeight );
	}

	// Should the decal use texture lightmapping?  FALSE if the decal is unlit.
	const UBOOL bHasLightMap =
		bLitDecal
		&& LODData.Num() > 0
		&& LODData(0).LightMap;

	// Should the decal use texture lightmapping?  FALSE if the decal has no lightmap.
	const UBOOL bUsingTextureLightmapping =
		bHasLightMap
		&& (LightMapWidth > 0)
		&& (LightMapHeight > 0) 
		&& (StaticMesh->LightMapCoordinateIndex >= 0) 
		&& ((UINT)StaticMesh->LightMapCoordinateIndex < StaticMeshRenderData.VertexBuffer.GetNumTexCoords())
		&& (LODData(0).LightMap->GetLightMap2D() != NULL);	// 조건 추가.. 위 조건을 다 만족하더라고. VertexLighting이 되는 경우가 있다...preopenbeta를 위해.. 일단 이렇게 처리... 2007. 6. 26 changmin

	// Should the decal use vertex lightmapping?  FALSE if the decal has no lightmap.
	const UBOOL bUsingVertexLightmapping =
		bHasLightMap
		&& !bUsingTextureLightmapping;

	const UBOOL bNoSoftwareClipping = Decal->bNoClip;	

	// vertex lightmapping data.
	FLightMapData1D* LightMapData1D = NULL;
	TArray<INT> SampleRemapping;

#if 0
	if ( bUsingTextureLightmapping )
	{
		debugf( NAME_DevDecals, TEXT("UStaticMeshComponent::GenerateDecalRenderData -- lit by textures") );
	}
	if ( bUsingVertexLightmapping )
	{
		debugf( NAME_DevDecals, TEXT("UStaticMeshComponent::GenerateDecalRenderData -- lit by vertex buffers") );
	}
	if ( !bLitDecal )
	{
		debugf( NAME_DevDecals, TEXT("UStaticMeshComponent::GenerateDecalRenderData -- unlit") );
	}
#endif

	// Create temporary structures.
	FDecalPoly Poly;
	FVector2D TempTexCoords;

	TArray<FQuantizedLightSample> LightSamples;
	const FQuantizedLightSample* LightMapData = NULL;	

	if( bUsingVertexLightmapping && !bNoSoftwareClipping )
	{
		//<@ ava specific ; 2006. 10. 16 changmin
		// LODData가 없는 경우도 있다. 과거 리소스의 잔재인가.. T T
		FLightMap* LightMap = NULL;
		if( LODData.Num() )
		{
			const FStaticMeshComponentLODInfo& LODInstanceData = LODData(0);
			LightMap = LODInstanceData.LightMap;
		}

		if (LightMap != NULL)
		{
			const FLightMap1D* LightMap1D = LightMap->GetLightMap1D();

			if (LightMap1D != NULL)
			{
				LightMapData = (const FQuantizedLightSample*)( LightMap1D->GetCachedSampleData() );
			}			
		}
	}	

	// Iterate over intersected kDOP nodes.
	for ( INT LeafIndex = 0 ; LeafIndex < Leaves.Num() ; ++LeafIndex )
	{
		const UStaticMesh::kDOPTreeType::NodeType& Node = kDOPQuery.Nodes(Leaves(LeafIndex));

		const DWORD FirstTriangle = Node.t.StartIndex;
		const DWORD LastTriangle = FirstTriangle + Node.t.NumTriangles;

		for ( DWORD TriangleIndex = FirstTriangle ; TriangleIndex < LastTriangle ; ++TriangleIndex )
		{
			const FkDOPCollisionTriangle& Triangle = kDOPTree.Triangles(TriangleIndex);

			// Construct a FDecalPoly with the triangle so we can clip it.
			Poly.Init();
			new(Poly.Vertices) FVector(StaticMeshRenderData.PositionVertexBuffer.VertexPosition(Triangle.v1));
			new(Poly.Vertices) FVector(StaticMeshRenderData.PositionVertexBuffer.VertexPosition(Triangle.v2));
			new(Poly.Vertices) FVector(StaticMeshRenderData.PositionVertexBuffer.VertexPosition(Triangle.v3));			

			// Skip over zero-area triangles.
			if ( Poly.CalcNormal() != 0 )
			{
				continue;
			}

			// Discard if the polygon faces away from the decal.
			const FLOAT Dot = DecalInfo.LocalLookVector | Poly.FaceNormal;

			// Even if backface culling is disabled, reject triangles that view the decal at grazing angles.
			if ( Dot > 1.e-3 || ( Decal->bProjectOnBackfaces && Abs( Dot ) > 1.e-3 ) )
			{
				Poly.Indices.AddItem(Triangle.v1);
				Poly.Indices.AddItem(Triangle.v2);
				Poly.Indices.AddItem(Triangle.v3);

				//<@ ava specific ; 2006. 10. 16 changmin
				if( bUsingTextureLightmapping )
				{				
					new(Poly.LightmapCoords) FVector2D( StaticMeshRenderData.VertexBuffer.VertexUV(Triangle.v1, StaticMesh->LightMapCoordinateIndex) );
					new(Poly.LightmapCoords) FVector2D( StaticMeshRenderData.VertexBuffer.VertexUV(Triangle.v2, StaticMesh->LightMapCoordinateIndex) );
					new(Poly.LightmapCoords) FVector2D( StaticMeshRenderData.VertexBuffer.VertexUV(Triangle.v3, StaticMesh->LightMapCoordinateIndex) );
				}
				else if( bUsingVertexLightmapping )
				{
					if ( bNoSoftwareClipping )
					{
						SampleRemapping.AddItem(Triangle.v1);
						SampleRemapping.AddItem(Triangle.v2);
						SampleRemapping.AddItem(Triangle.v3);
					}
					else
					{						
						if( LightMapData != NULL )
						{							
							new(Poly.VertexLights) FQuantizedLightSample( *(LightMapData + Triangle.v1) );
							new(Poly.VertexLights) FQuantizedLightSample( *(LightMapData + Triangle.v2) );
							new(Poly.VertexLights) FQuantizedLightSample( *(LightMapData + Triangle.v3) );
						}
					}					
				}
				//>@ ava

				if ( (bNoSoftwareClipping && Poly.PartOfPolyInside( DecalInfo.Convex )) || Poly.ClipAgainstConvex( DecalInfo.Convex ) )
				{
					///////////////////////////////////////
					// Add the polygon's verts to the decal geometry buffer.

					// Allocate a FDecalRenderData object if we haven't already.
					if ( !DecalRenderData )
					{
#if GEMINI_TODO
#else
						FLightCacheInterface* LCI = NULL;
#endif
						DecalRenderData = new FDecalRenderData( LCI, TRUE, TRUE );
					}					

					const INT FirstVertexIndex = DecalRenderData->GetNumVertices();
					for ( INT i = 0 ; i < Poly.Vertices.Num() ; ++i )
					{
						// Create decal vertex tangent basis by projecting decal tangents onto the plane defined by the vertex plane.
						const INT VertexIndex = Poly.Indices(i);
						const FPackedNormal& PackedPolyTangentX = StaticMeshRenderData.VertexBuffer.VertexTangentX(VertexIndex);
						const FPackedNormal& PackedPolyTangentY = StaticMeshRenderData.VertexBuffer.VertexTangentY(VertexIndex);
						const FPackedNormal& PackedPolyTangentZ = StaticMeshRenderData.VertexBuffer.VertexTangentZ(VertexIndex);

						const FVector PolyTangentX( FVector( PackedPolyTangentX ).UnsafeNormal() );
						const FVector PolyTangentY( FVector( PackedPolyTangentY ).UnsafeNormal() );
						const FVector PolyTangentZ( FVector( PackedPolyTangentZ ).UnsafeNormal() );

						const FVector TangentZ = PolyTangentZ;
						FVector TangentX;
						FVector TangentY;

						/*
						const FLOAT Dot = -DecalInfo.LocalTangent | TangentZ;
						if ( Abs( Dot ) < 0.9f )
						{
							TangentX = ( -DecalInfo.LocalTangent  - (Dot*TangentZ) ).SafeNormal();
							TangentY = ( TangentZ ^ TangentX ).SafeNormal();
						}
						else
						*/
						{
							// The tangent is nearly parallel to the normal, so use the binormal instead.
							TangentY = ( DecalInfo.LocalBinormal  - ((DecalInfo.LocalBinormal|TangentZ)*TangentZ) ).SafeNormal();
							TangentX = ( TangentY ^ TangentZ ).SafeNormal();
						}

						// Generate transform from mesh tangent basis to decal tangent basis.
						// m is the upper 2x2 of the decal basis projected onto the mesh basis.
						// m[0],m[1] first column, m[2],m[3] second column.
						const FPlane m( PolyTangentX | TangentX, PolyTangentX | TangentY, PolyTangentY | TangentX, PolyTangentY | TangentY );

						// NormalTransformBuffer0 is the top row,
						// NormalTransformBuffer1 is the second row.

						// Generate texture coordinates for the vertex using the decal tangents.
						DecalInfo.ComputeTextureCoordinates( LocalToWorld.TransformFVector( Poly.Vertices(i) ), TempTexCoords );

						// Store the decal vertex.
						//<@ ava specific ; 2006. 10. 16 changmin
						if( bUsingTextureLightmapping )
						{
							DecalRenderData->AddVertex(
								FDecalVertex( Poly.Vertices( i ),
									PackedPolyTangentX,
									PackedPolyTangentY,
									PackedPolyTangentZ,
									TempTexCoords,
									FVector2D( -m.X, -m.Y ),
									FVector2D( -m.Z, -m.W ),
									Poly.LightmapCoords(i))
								);
						}
						else
						//>@ ava
						{
							DecalRenderData->AddVertex(
								FDecalVertex( Poly.Vertices( i ),
									PackedPolyTangentX,
									PackedPolyTangentY,
									PackedPolyTangentZ,
									TempTexCoords,
									FVector2D( -m.X, -m.Y ),
									FVector2D( -m.Z, -m.W ))
								);
						}						
					}

					// Triangulate the polygon and add indices to the index buffer
					const INT FirstIndex = DecalRenderData->GetNumIndices();
					for ( INT i = 0 ; i < Poly.Vertices.Num() - 2 ; ++i )
					{
						DecalRenderData->AddIndex( FirstVertexIndex+0 );
						DecalRenderData->AddIndex( FirstVertexIndex+i+1 );
						DecalRenderData->AddIndex( FirstVertexIndex+i+2 );
					}

					//<@ ava specific ; 2006. 10 .16 changmin
					if( bUsingVertexLightmapping && !bNoSoftwareClipping )
					{						
						if ( LightMapData != NULL )
						{
							LightSamples.Append( Poly.VertexLights );
						}						
						else
						{
							FQuantizedLightSample WhiteSample;

							for( INT CoeffIndex = 0; CoeffIndex < NUM_LIGHTMAP_COEFFICIENTS; ++CoeffIndex )
							{
								WhiteSample.Coefficients[CoeffIndex] = FColor( 255, 255, 255 );
							}

							for( INT VertexIndex = 0; VertexIndex < Poly.Vertices.Num(); ++VertexIndex)
							{
								LightSamples.AddItem( WhiteSample );	
							}
						}
					}
					//>@ ava
				}
			}
		}
	}

	// Finalize the data.
	if ( DecalRenderData )
	{
		DecalRenderData->NumTriangles = 0;//DecalRenderData->GetNumIndices()/3;

		// Vertex light mapping?
		if ( bUsingVertexLightmapping )
		{
			const FLightMapRef& SourceLightMap = LODData(0).LightMap;
			checkSlow( SourceLightMap );
			FLightMap1D* SourceLightMap1D = const_cast<FLightMap1D*>(SourceLightMap->GetLightMap1D());
			if( SourceLightMap1D )
			{
				if (bNoSoftwareClipping)
				{
					// Create the vertex light map data.
					DecalRenderData->LightMap1D = SourceLightMap1D->DuplicateWithRemappedVerts( SampleRemapping );
				}				
				else
				{
					DecalRenderData->LightMap1D = SourceLightMap1D->DuplicateWithNewSamples( LightSamples );										
				}
			}
			else
			{
				debugf( NAME_Warning, TEXT("Static mesh uses vertex light map but has no lightmap1D data") );
			}
		}
	}
	
	return DecalRenderData;
}

IMPLEMENT_COMPARE_CONSTPOINTER( FDecalInteraction, UnStaticMeshRender,
{
	if ((A->DecalState.SortOrder == B->DecalState.SortOrder))
	{
		return (INT)A->DecalState.DecalMaterial - (INT)B->DecalState.DecalMaterial;
	}

	return (A->DecalState.SortOrder <= B->DecalState.SortOrder) ? -1 : 1;
} );


/**
 * A static mesh component scene proxy.
 */
class FStaticMeshSceneProxy : public FPrimitiveSceneProxy
{
protected:
	/** Creates a light cache for the decal if it has a lit material. */
	void CreateDecalLightCache(const FDecalInteraction& DecalInteraction)
	{
		if ( DecalInteraction.DecalState.bIsLit )
		{
			new(DecalLightCaches) FDecalLightCache( DecalInteraction, *this );
		}
	}

public:
	virtual UBOOL IsCacheable() const
	{
		return TRUE;
	}

	virtual UBOOL IsStillValid( const UPrimitiveComponent* InComponent ) const
	{ 		
		const UStaticMeshComponent* Component = static_cast<const UStaticMeshComponent*>(InComponent);

		return 
			FPrimitiveSceneProxy::IsStillValid( InComponent ) &&
			Owner == Component->GetOwner() &&
			StaticMesh == Component->StaticMesh &&
			ForcedLodModel == Component->ForcedLodModel &&
			/*LocalToWorld == Component->LocalToWorld &&
			LocalToWorldDeterminant == Component->LocalToWorldDeterminant &&			
			Bounds == Component->Bounds &&*/
			bCastShadow == Component->CastShadow &&
			bSelected == Component->IsOwnerSelected() &&
			bShouldCollide == Component->ShouldCollide() &&
			bBlockZeroExtent == Component->BlockZeroExtent &&
			bBlockNonZeroExtent == Component->BlockNonZeroExtent &&
			bBlockRigidBody == Component->BlockRigidBody &&
			bHasTranslucency == Component->HasTranslucency() &&
			bHasDistortion == Component->HasUnlitDistortion() &&
			WireframeColor == Component->WireframeColor;
	}	

	/** Initialization constructor. */
	FStaticMeshSceneProxy(const UStaticMeshComponent* Component):
		FPrimitiveSceneProxy(Component),
		Owner(Component->GetOwner()),
		StaticMesh(Component->StaticMesh),
		StaticMeshComponent(Component),
		ForcedLodModel(Component->ForcedLodModel),		
		LevelColor(255,255,255),
		PropertyColor(255,255,255),
		bCastShadow(Component->CastShadow),
		bSelected(Component->IsOwnerSelected()),
		bShouldCollide(Component->ShouldCollide()),
		bBlockZeroExtent(Component->BlockZeroExtent),
		bBlockNonZeroExtent(Component->BlockNonZeroExtent),
		bBlockRigidBody(Component->BlockRigidBody),
		bHasTranslucency(Component->HasTranslucency()),
		bHasDistortion(Component->HasUnlitDistortion()),
		bUsesSceneColor(Component->HasUnlitTranslucency() && Component->UsesSceneColor()),
		WireframeColor(Component->WireframeColor)
	{
		// Build the proxy's LOD data.
		for(INT LODIndex = 0;LODIndex < StaticMesh->LODModels.Num();LODIndex++)
		{
			new(LODs) FLODInfo(Component,LODIndex);
		}

		// If the static mesh can accept decals, copy off statically irrelevant lights and light map guids.
		if ( Component->bAcceptsDecals )
		{
			// Create light cache information for any decals that were attached in
			// the FPrimitiveSceneProxy ctor.
			for( INT DecalIndex = 0 ; DecalIndex < Decals.Num() ; ++DecalIndex )
			{
				CreateDecalLightCache( *Decals(DecalIndex) );
			}
		}

#if !FINAL_RELEASE
		if( GIsEditor )
		{
			// Try to find a color for level coloration.
			if ( Owner )
			{
				ULevel* Level = Owner->GetLevel();
				ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel( Level );
				if ( LevelStreaming )
				{
					LevelColor = LevelStreaming->DrawColor;
				}
			}

			// Get a color for property coloration.
			GEngine->GetPropertyColorationColor( (UObject*)Component, PropertyColor );
		}
#endif

		LastLOD = -1;
	}

	/** Sets up a FMeshElement for a specific LOD and element. */
	UBOOL GetMeshElement(INT LODIndex,INT ElementIndex,BYTE DepthPriorityGroup,FMeshElement& OutMeshElement) const
	{
		const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(LODIndex);
		const FStaticMeshElement& Element = LODModel.Elements(ElementIndex);

		if(Element.NumTriangles > 0)
		{
			OutMeshElement.IndexBuffer = &LODModel.IndexBuffer;
			OutMeshElement.VertexFactory = &LODModel.VertexFactory;
			OutMeshElement.DynamicVertexData = NULL;
			OutMeshElement.MaterialInstance = LODs(LODIndex).Elements(ElementIndex).Material->GetInstanceInterface(bSelected);
			OutMeshElement.LCI = &LODs(LODIndex);
			OutMeshElement.LocalToWorld = LocalToWorld;
			OutMeshElement.WorldToLocal = LocalToWorld.Inverse();			
			OutMeshElement.FirstIndex = Element.FirstIndex;
			OutMeshElement.NumPrimitives = Element.NumTriangles;
			OutMeshElement.MinVertexIndex = Element.MinVertexIndex;
			OutMeshElement.MaxVertexIndex = Element.MaxVertexIndex;
			OutMeshElement.UseDynamicData = FALSE;
			OutMeshElement.ReverseCulling = LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;
			OutMeshElement.CastShadow = bCastShadow;
			OutMeshElement.Type = PT_TriangleList;
			OutMeshElement.DepthPriorityGroup = (ESceneDepthPriorityGroup)DepthPriorityGroup;
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}


	// FPrimitiveSceneProxy interface.
	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI)
	{
		if(!HasViewDependentDPG() && !IsMovable())
		{
			// Determine the DPG the primitive should be drawn in.
			BYTE PrimitiveDPG = GetStaticDepthPriorityGroup();
			INT NumLODs = StaticMesh->LODModels.Num();

			//check if a LOD is being forced
			if (ForcedLodModel > 0) 
			{
				INT LODIndex = ForcedLodModel - 1;
				check(LODIndex < NumLODs);
				const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(LODIndex);
				// Draw the static mesh elements.
				for(INT ElementIndex = 0;ElementIndex < LODModel.Elements.Num();ElementIndex++)
				{
					FMeshElement MeshElement;
					if(GetMeshElement(LODIndex,ElementIndex,PrimitiveDPG,MeshElement))
					{
						PDI->DrawMesh(MeshElement, 0, WORLD_MAX);
					}
				}
			} 
			else //no LOD is being forced, submit them all with appropriate cull distances
			{
				for(INT LODIndex = 0;LODIndex < NumLODs;LODIndex++)
				{
					const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(LODIndex);

					FLOAT MinDist = GetMinLODDist(LODIndex);
					FLOAT MaxDist = GetMaxLODDist(LODIndex);

					// Draw the static mesh elements.
					for(INT ElementIndex = 0;ElementIndex < LODModel.Elements.Num();ElementIndex++)
					{
						FMeshElement MeshElement;
						if(GetMeshElement(LODIndex,ElementIndex,PrimitiveDPG,MeshElement))
						{
							PDI->DrawMesh(MeshElement, MinDist, MaxDist);
						}
					}
				}
			}
		}
	}

	/** Determines if any collision should be drawn for this mesh. */
	UBOOL ShouldDrawCollision(const FSceneView* View)
	{
		if((View->Family->ShowFlags & SHOW_CollisionNonZeroExtent) && bBlockNonZeroExtent && bShouldCollide)
		{
			return TRUE;
		}

		if((View->Family->ShowFlags & SHOW_CollisionZeroExtent) && bBlockZeroExtent && bShouldCollide)
		{
			return TRUE;
		}	

		if((View->Family->ShowFlags & SHOW_CollisionRigidBody) && bBlockRigidBody)
		{
			return TRUE;
		}

		return FALSE;
	}

	/** Determines if the simple or complex collision should be drawn for a particular static mesh. */
	UBOOL ShouldDrawSimpleCollision(const FSceneView* View, const UStaticMesh* Mesh)
	{
		if(Mesh->UseSimpleBoxCollision && (View->Family->ShowFlags & SHOW_CollisionNonZeroExtent))
		{
			return TRUE;
		}	

		if(Mesh->UseSimpleLineCollision && (View->Family->ShowFlags & SHOW_CollisionZeroExtent))
		{
			return TRUE;
		}	

		if(Mesh->UseSimpleRigidBodyCollision && (View->Family->ShowFlags & SHOW_CollisionRigidBody))
		{
			return TRUE;
		}

		return FALSE;
	}

	/**
	* Draws the primitive's decal elements.  This is called from the rendering thread for each frame of each view.
	* The dynamic elements will only be rendered if GetViewRelevance declares decal relevance.
	* Called in the rendering thread.
	*
	* @param	Context							The RHI command context to which the primitives are being rendered.
	* @param	OpaquePDI						The interface which receives the opaque primitive elements.
	* @param	TranslucentPDI					The interface which receives the translucent primitive elements.
	* @param	View							The view which is being rendered.
	* @param	DepthPriorityGroup				The DPG which is being rendered.
	* @param	bTranslucentReceiverPass		TRUE during the decal pass for translucent receivers, FALSE for opaque receivers.
	*/
	virtual void DrawDecalElements(
		FCommandContextRHI* Context, 
		FPrimitiveDrawInterface* OpaquePDI, 
		FPrimitiveDrawInterface* TranslucentPDI, 
		const FSceneView* View, 
		UINT DepthPriorityGroup, 
		UBOOL bTranslucentReceiverPass
		)
	{
		SCOPE_CYCLE_COUNTER(STAT_DecalRenderTime);
		checkSlow( View->Family->ShowFlags & SHOW_Decals );

		if ( (!bTranslucentReceiverPass && bHasTranslucency) || (bTranslucentReceiverPass && !bHasTranslucency) )
		{
			return;
		}		
		
		// Compute the set of decals in this DPG.
		TArray<FDecalInteraction*> DPGDecals;
		for ( INT DecalIndex = 0 ; DecalIndex < Decals.Num() ; ++DecalIndex )
		{
			FDecalInteraction* Interaction = Decals(DecalIndex);
			if ( DepthPriorityGroup == Interaction->DecalState.DepthPriorityGroup )
			{
				if ( !Interaction->DecalState.bIsLit )
				{
					DPGDecals.AddItem( Interaction );
				}
			}
		}

		//@todo epic의 코드에 보면 scissor rect계산 부분이 있는데 이를 갖고 올까?
		// Sort and render all decals.
		Sort<USE_COMPARE_CONSTPOINTER(FDecalInteraction,UnStaticMeshRender)>( DPGDecals.GetTypedData(), DPGDecals.Num() );		
		for ( INT DecalIndex = 0 ; DecalIndex < DPGDecals.Num() ; ++DecalIndex )
		{
			const FDecalInteraction* Decal	= DPGDecals(DecalIndex);
			const FDecalState& DecalState	= Decal->DecalState;
			
			const FDecalRenderData* RenderData = Decal->RenderData;

			FMeshElement MeshElement;				
			MeshElement.MaterialInstance = DecalState.DecalMaterial->GetInstanceInterface(FALSE);							
			MeshElement.LCI = RenderData->LCI;						
			MeshElement.LocalToWorld = LocalToWorld;
			MeshElement.WorldToLocal = LocalToWorld.Inverse();
			MeshElement.DepthBias = DecalState.DepthBias;
			MeshElement.SlopeScaleDepthBias = DecalState.SlopeScaleDepthBias;
			MeshElement.DepthPriorityGroup = GetDepthPriorityGroup(View);

			FPrimitiveDrawInterface* PDI;
			if ( Decal->DecalState.bHasUnlitTranslucency )
			{
				PDI = TranslucentPDI;
			}
			else
			{
				RHISetBlendState( Context, TStaticBlendState<>::GetRHI() );
				PDI = OpaquePDI;
			}				

			const TArray<WORD>& Indices = RenderData->IndexBuffer.Indices;
			INT NumIndices = Indices.Num();
			INT NumVertices = RenderData->Vertices.Num();				

			// 너무 큰 거 pass
			if (!IsAffordableForDecalBatcher( NumVertices, NumIndices ))
				continue;

			INT Batch = 1;

			for (; DecalIndex + Batch < DPGDecals.Num(); ++Batch)
			{
				FDecalInteraction* Candidate = DPGDecals(DecalIndex+Batch);

				if (DecalState.bHasUnlitTranslucency ^ Candidate->DecalState.bHasUnlitTranslucency ||
					MeshElement.MaterialInstance != Candidate->DecalState.DecalMaterial->GetInstanceInterface(FALSE)
					)
					break;

				const TArray<WORD>& Indices = Candidate->RenderData->IndexBuffer.Indices;					

				const INT NumThisVertices = Candidate->RenderData->Vertices.Num();
				const INT NumThisIndices = Indices.Num();

				if (!IsAffordableForDecalBatcher( NumVertices + NumThisVertices, NumIndices + NumThisIndices ))
				{
					break;
				}

				NumVertices += Candidate->RenderData->Vertices.Num();
				NumIndices += Indices.Num();
			}						

			FDrawBatchedDecalContext DecalContext;
			DecalContext.Context = Context;
			DecalContext.PDI = PDI;
			DecalContext.MeshElement = &MeshElement;
			DecalContext.PropertyColor = PropertyColor;
			DecalContext.LevelColor = LevelColor;
			DecalContext.PrimitiveSceneInfo = PrimitiveSceneInfo;

			DrawBatchedDecals( &DecalContext, NumVertices, NumIndices, DPGDecals, DecalIndex, Batch );						
		}								
	}

protected:
	/**
	* @return		The index into DecalLightCaches of the specified component, or INDEX_NONE if not found.
	*/
	INT FindDecalLightCacheIndex(const UDecalComponent* DecalComponent) const
	{
		for( INT DecalIndex = 0 ; DecalIndex < DecalLightCaches.Num() ; ++DecalIndex )
		{
			const FDecalLightCache& DecalLightCache = DecalLightCaches(DecalIndex);
			if( DecalLightCache.GetDecalComponent() == DecalComponent )
			{
				return DecalIndex;
			}
		}
		return INDEX_NONE;
	}

public:
	virtual void InitLitDecalFlags(UINT DepthPriorityGroup)
	{
		// When drawing the first set of decals for this light, the blend state needs to be "set" rather
		// than "add."  Subsequent calls use "add" to accumulate color.
		for( INT DecalIndex = 0 ; DecalIndex < DecalLightCaches.Num() ; ++DecalIndex )
		{
			FDecalLightCache& DecalLightCache = DecalLightCaches(DecalIndex);
			DecalLightCache.ClearFlags();
		}
	}

	static UBOOL IsMergeable( const FDecalInteraction* A, const FDecalInteraction* B )
	{
		check( A->DecalState.DecalMaterial != NULL && B->DecalState.DecalMaterial != NULL );

		const UBOOL bHasLightMapA = A->RenderData->LightMap1D != NULL;
		const UBOOL bHasLightMapB = B->RenderData->LightMap1D != NULL;

		return !(	A->DecalState.bHasUnlitTranslucency ^ B->DecalState.bHasUnlitTranslucency ||
			A->DecalState.DecalMaterial->GetInstanceInterface(FALSE) != B->DecalState.DecalMaterial->GetInstanceInterface(FALSE) ||
			bHasLightMapA ^ bHasLightMapB			
			);					
	}

	/**
	* Draws the primitive's lit decal elements.  This is called from the rendering thread for each frame of each view.
	* The dynamic elements will only be rendered if GetViewRelevance declares dynamic relevance.
	* Called in the rendering thread.
	*
	* @param	Context					The RHI command context to which the primitives are being rendered.
	* @param	PDI						The interface which receives the primitive elements.
	* @param	View					The view which is being rendered.
	* @param	DepthPriorityGroup		The DPG which is being rendered.
	* @param	bDrawingDynamicLights	TRUE if drawing dynamic lights, FALSE if drawing static lights.
	*/
	virtual void DrawLitDecalElements(
		FCommandContextRHI* Context,
		FPrimitiveDrawInterface* PDI,
		const FSceneView* View,
		UINT DepthPriorityGroup,
		UBOOL bDrawingDynamicLights
		)
	{
		SCOPE_CYCLE_COUNTER(STAT_DecalRenderTime);
		checkSlow( View->Family->ShowFlags & SHOW_Decals );

		// Compute the set of decals in this DPG.
		TArray<FDecalInteraction*> DPGDecals;
		for ( INT DecalIndex = 0 ; DecalIndex < Decals.Num() ; ++DecalIndex )
		{
			FDecalInteraction* Interaction = Decals(DecalIndex);
			if ( DepthPriorityGroup == Interaction->DecalState.DepthPriorityGroup )
			{
				if ( Interaction->DecalState.bIsLit )
				{
					DPGDecals.AddItem( Interaction );
				}
			}
		}

		if (Decals.Num() == 0)
			return;		

		//@todo epic의 코드에 보면 scissor rect계산 부분이 있는데 이를 갖고 올까?
		// Sort and render all decals.
		Sort<USE_COMPARE_CONSTPOINTER(FDecalInteraction,UnStaticMeshRender)>( DPGDecals.GetTypedData(), DPGDecals.Num() );		
		for ( INT DecalIndex = 0 ; DecalIndex < DPGDecals.Num() ; ++DecalIndex )
		{
			const FDecalInteraction* Decal	= DPGDecals(DecalIndex);
			const FDecalState& DecalState	= Decal->DecalState;

			const UDecalComponent* DecalComponent = Decal->Decal;
			const INT DecalLightCacheIndex = FindDecalLightCacheIndex( DecalComponent );
			FDecalLightCache* DecalLCI = ( DecalLightCacheIndex != INDEX_NONE ) ? &DecalLightCaches(DecalLightCacheIndex) : NULL;
			// The decal is lit and so should have an entry in the DecalLightCaches list.
			if ( !DecalLCI )
			{
				debugf( NAME_Warning, TEXT("DecalLightCacheIndex == INDEX_NONE!!") );
				continue;
			}			

			const FDecalRenderData* RenderData = Decal->RenderData;

			// 1D Lightmap이 걸려 있다면 vertex lightmapping을 해야겠지요!!
			const UBOOL bUsingVertexLightmapping = RenderData->LightMap1D != NULL;			

			FMeshElement MeshElement;				
			MeshElement.MaterialInstance = DecalState.DecalMaterial->GetInstanceInterface(FALSE);							
			MeshElement.LCI = DecalLCI;			
			MeshElement.LocalToWorld = LocalToWorld;
			MeshElement.WorldToLocal = LocalToWorld.Inverse();
			MeshElement.DepthBias = DecalState.DepthBias;
			MeshElement.SlopeScaleDepthBias = DecalState.SlopeScaleDepthBias;
			MeshElement.DepthPriorityGroup = GetDepthPriorityGroup(View);			

			const TArray<WORD>& Indices = RenderData->IndexBuffer.Indices;
			INT NumIndices = Indices.Num();
			INT NumVertices = RenderData->Vertices.Num();				

			// 너무 큰 거 pass
			if (!IsAffordableForDecalBatcher( NumVertices, NumIndices ))
				continue;

			INT Batch = 1;

			for (; DecalIndex + Batch < DPGDecals.Num(); ++Batch)
			{
				FDecalInteraction* Candidate = DPGDecals(DecalIndex+Batch);

				if (!IsMergeable( Decal, Candidate ))
					break;
				
				const TArray<WORD>& Indices = Candidate->RenderData->IndexBuffer.Indices;					

				const INT NumThisVertices = Candidate->RenderData->Vertices.Num();
				const INT NumThisIndices = Indices.Num();

				if (!IsAffordableForDecalBatcher( NumVertices + NumThisVertices, NumIndices + NumThisIndices ))
				{
					break;
				}

				NumVertices += Candidate->RenderData->Vertices.Num();
				NumIndices += Indices.Num();
			}			
			
			FDrawBatchedDecalContext DecalContext;
			DecalContext.Context = Context;
			DecalContext.PDI = PDI;
			DecalContext.MeshElement = &MeshElement;
			DecalContext.PropertyColor = PropertyColor;
			DecalContext.LevelColor = LevelColor;
			DecalContext.PrimitiveSceneInfo = PrimitiveSceneInfo;			

			DrawBatchedDecals( &DecalContext, NumVertices, NumIndices, DPGDecals, DecalIndex, Batch, bUsingVertexLightmapping );						

			// Unlink dynamic vertex buffer
			if (bUsingVertexLightmapping && RenderData->LightMap1D)
			{					
				FLightMap1D* Lightmap1D = const_cast<FLightMap1D*>( RenderData->LightMap1D->GetLightMap1D() );

				check( Lightmap1D != NULL );

				Lightmap1D->VertexBufferRHI = NULL;
			}			
		}										

		// Restore the scissor rect.
		RHISetScissorRect( Context, FALSE, 0, 0, 0, 0 );
	}

	/**
	* Adds a decal interaction to the primitive.  This is called in the rendering thread by AddDecalInteraction_GameThread.
	*/
	virtual void AddDecalInteraction_RenderingThread(const FDecalInteraction& DecalInteraction)
	{
		FPrimitiveSceneProxy::AddDecalInteraction_RenderingThread( DecalInteraction );
		CreateDecalLightCache( DecalInteraction );
	}

	/**
	* Removes a decal interaction from the primitive.  This is called in the rendering thread by RemoveDecalInteraction_GameThread.
	*/
	virtual void RemoveDecalInteraction_RenderingThread(UDecalComponent* DecalComponent)
	{
		FPrimitiveSceneProxy::RemoveDecalInteraction_RenderingThread( DecalComponent );

		// Find the decal interaction representing the given decal component, and remove it from the interaction list.
		const INT DecalLightCacheIndex = FindDecalLightCacheIndex( DecalComponent);
		if ( DecalLightCacheIndex != INDEX_NONE )
		{
			DecalLightCaches.Remove( DecalLightCacheIndex );
		}
	}

	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex)
	{
		// Bool that defines how we should draw the collision for this mesh.
		const UBOOL bIsCollisionView = IsCollisionView(View);
		const UBOOL bDrawCollision = bIsCollisionView && ShouldDrawCollision(View);
		const UBOOL bDrawSimple = ShouldDrawSimpleCollision(View, StaticMesh);
		const UBOOL bDrawComplexCollision = (bDrawCollision && !bDrawSimple);
		const UBOOL bDrawSimpleCollision = (bDrawCollision && bDrawSimple);
		const UBOOL bDrawMesh = bIsCollisionView ?
									bDrawComplexCollision :
									IsRichView(View) || HasViewDependentDPG() || IsMovable();

		// Determine the DPG the primitive should be drawn in for this view.
		BYTE PrimitiveDPG = GetDepthPriorityGroup(View);

		//<@ ava specific ; 2006. 12 .18 changmin.
		// draw the mesh with leaf coloration
		if( DPGIndex == PrimitiveDPG
		&&	View->Family->ShowFlags & SHOW_LeafColoration
		&& !(View->Family->ShowFlags & SHOW_Wireframe )
		&& (View->Family->ShowFlags & SHOW_StaticMeshes)
		&& bDrawMesh)
		{
			UModel *Bsp = GWorld->PersistentLevel->StructuralModel;
			const UBOOL bHasLeafColors	= Bsp->LeafColors.Num() > 0;
			const UBOOL bHasPvsInfo		= Bsp->LeafBytes > 0 ;
			UStaticMeshComponent *StaticMeshComp = Cast<UStaticMeshComponent>(PrimitiveSceneInfo->Component);
			if( bHasPvsInfo && bHasLeafColors && StaticMeshComp )
			{
				FColor LeafColor(255,255,255);
				UBOOL bColoration = FALSE;
				for( INT LeafIndex = 0; LeafIndex < StaticMeshComp->LeafCount; ++LeafIndex )
				{
					const INT ClusterNumber = Bsp->StaticMeshLeaves(StaticMeshComp->FirstLeaf + LeafIndex);
					if( ClusterNumber != INDEX_NONE )
					{
						LeafColor = Bsp->LeafColors(ClusterNumber);
						bColoration = TRUE;
						break;
					}
				}
				// 보일때만 coloration한다.
				bColoration = FALSE;
				FSceneViewState *ViewState = (FSceneViewState*) View->State;
				const INT CurrentLeaf	= GEngine->LockPvs ? ViewState->LockedLeaf : ViewState->CurrentLeaf;
				const BYTE *Vis			= GEngine->LockPvs ? ViewState->LockedVis : ViewState->Vis;
				if( CurrentLeaf != INDEX_NONE )
				{
					for( INT LeafIndex = 0; LeafIndex < StaticMeshComp->LeafCount; ++LeafIndex )
					{
						const INT ClusterNumber = Bsp->StaticMeshLeaves(StaticMeshComp->FirstLeaf + LeafIndex);
						if( ClusterNumber != INDEX_NONE && Vis[ClusterNumber>>3] & ( 1 << (ClusterNumber&7) ) )
						{
							bColoration = TRUE;
							break;
						}
					}
				}
				// coloration~
				const UMaterial* LevelColorationMaterial = (View->Family->ShowFlags & SHOW_ViewMode_Lit) ? GEngine->LevelColorationLitMaterial : GEngine->LevelColorationUnlitMaterial;
				const FColoredMaterialInstance LeafColorationMaterialInstance( LevelColorationMaterial->GetInstanceInterface(FALSE), LeafColor );
				// Determine the LOD to use.
				FLOAT Distance = (FVector(View->ViewOrigin)-PrimitiveSceneInfo->Bounds.Origin).Size();
				INT LODIndex = GetLOD(Distance);
				const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(LODIndex);
				for(INT ElementIndex = 0;ElementIndex < LODModel.Elements.Num();ElementIndex++)
				{
					FMeshElement MeshElement;
					if( GetMeshElement(LODIndex, ElementIndex, PrimitiveDPG, MeshElement) )
					{
						if( bColoration )
						{
							MeshElement.MaterialInstance = &LeafColorationMaterialInstance;
						}
						PDI->DrawMesh(MeshElement);
					}
				}
			}
			return;
		}
		//>@ ava

		// Draw polygon mesh if we are either not in a collision view, or are drawing it as collision.
		if( DPGIndex == PrimitiveDPG && (View->Family->ShowFlags & SHOW_StaticMeshes) && bDrawMesh )
		{
			const UBOOL bLevelColorationEnabled = (View->Family->ShowFlags & SHOW_LevelColoration) ? TRUE : FALSE;
			const UBOOL bPropertyColorationEnabled = (View->Family->ShowFlags & SHOW_PropertyColoration) ? TRUE : FALSE;

			// Determine the LOD to use.
			FLOAT Distance = (FVector(View->ViewOrigin)-PrimitiveSceneInfo->Bounds.Origin).Size();
			INT LODIndex = GetLOD(Distance);
			const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(LODIndex);

			if( (View->Family->ShowFlags & SHOW_Wireframe) && !(View->Family->ShowFlags & SHOW_Materials) )
			{
				FLinearColor ViewWireframeColor( bLevelColorationEnabled ? FLinearColor(LevelColor) : WireframeColor );
				if ( bPropertyColorationEnabled )
				{
					ViewWireframeColor = PropertyColor;
				}
				// Use collision color if we are drawing this as collision
				else if(bDrawComplexCollision)
				{
					ViewWireframeColor = FLinearColor(GEngine->C_ScaleBoxHi);
				}

				FColoredMaterialInstance WireframeMaterialInstance(
					GEngine->WireframeMaterial->GetInstanceInterface(FALSE),
					GetSelectionColor(ViewWireframeColor,!(View->Family->ShowFlags & SHOW_Selection)||bSelected)
					);

				FMeshElement Mesh;
				Mesh.VertexFactory = &LODModel.VertexFactory;
				Mesh.MaterialInstance = &WireframeMaterialInstance;
				Mesh.LocalToWorld = LocalToWorld;
				Mesh.WorldToLocal = LocalToWorld.Inverse();				
				Mesh.FirstIndex = 0;
				Mesh.MinVertexIndex = 0;
				Mesh.MaxVertexIndex = LODModel.NumVertices - 1;
				Mesh.ReverseCulling = LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;
				Mesh.CastShadow = bCastShadow;
				Mesh.Type = PT_LineList;
				Mesh.DepthPriorityGroup = (ESceneDepthPriorityGroup)PrimitiveDPG;
				
				if( LODModel.WireframeIndexBuffer.IsInitialized() )
				{
					Mesh.IndexBuffer = &LODModel.WireframeIndexBuffer;
					Mesh.NumPrimitives = LODModel.WireframeIndexBuffer.Indices.Num() / 2;
				}
				else
				{
					Mesh.IndexBuffer = &LODModel.IndexBuffer;
					Mesh.Type = PT_TriangleList;
					Mesh.bWireframe = TRUE;
					Mesh.NumPrimitives = LODModel.IndexBuffer.Indices.Num() / 3;
				}

				PDI->DrawMesh(Mesh);
			}
			else
			{
				const FLinearColor UtilColor( IsCollisionView(View) ? FLinearColor(GEngine->C_ScaleBoxHi) : FLinearColor(LevelColor) );

				// Draw the static mesh elements.
				for(INT ElementIndex = 0;ElementIndex < LODModel.Elements.Num();ElementIndex++)
				{
					FMeshElement MeshElement;
					if(GetMeshElement(LODIndex,ElementIndex,PrimitiveDPG,MeshElement))
					{
						DrawRichMesh(
							PDI,
							MeshElement,
							WireframeColor,
							UtilColor,
							PropertyColor,							
							PrimitiveSceneInfo,
							bSelected
							);
					}
				}
			}
		}

		if(DPGIndex == SDPG_World && ((View->Family->ShowFlags & SHOW_Collision) && bShouldCollide) || (bDrawSimpleCollision))
		{
			if(StaticMesh->BodySetup)
			{
				// Make a material for drawing solid collision stuff
				const UMaterial* LevelColorationMaterial = (View->Family->ShowFlags & SHOW_ViewMode_Lit) ? GEngine->LevelColorationLitMaterial : GEngine->LevelColorationUnlitMaterial;
				const FColoredMaterialInstance CollisionMaterialInstance(
					LevelColorationMaterial->GetInstanceInterface(bSelected),
					FLinearColor(GEngine->C_ScaleBoxHi)
					);

				// Draw the static mesh's body setup.

				// Get transform without scaling.
				FMatrix GeomMatrix = LocalToWorld;
				FVector RecipScale( 1.f/TotalScale3D.X, 1.f/TotalScale3D.Y, 1.f/TotalScale3D.Z );

				GeomMatrix.M[0][0] *= RecipScale.X;
				GeomMatrix.M[0][1] *= RecipScale.X;
				GeomMatrix.M[0][2] *= RecipScale.X;

				GeomMatrix.M[1][0] *= RecipScale.Y;
				GeomMatrix.M[1][1] *= RecipScale.Y;
				GeomMatrix.M[1][2] *= RecipScale.Y;

				GeomMatrix.M[2][0] *= RecipScale.Z;
				GeomMatrix.M[2][1] *= RecipScale.Z;
				GeomMatrix.M[2][2] *= RecipScale.Z;

				// Slight hack here - draw each hull in a different color if no Owner (usually in a tool like StaticMeshEditor).
				UBOOL bDrawSimpleSolid = (bDrawSimpleCollision && !(View->Family->ShowFlags & SHOW_Wireframe));

				// In old wireframe collision mode, always draw the wireframe highlighted (selected or not).
				UBOOL bDrawWireSelected = bSelected;
				if(View->Family->ShowFlags & SHOW_Collision)
				{
					bDrawWireSelected = TRUE;
				}

				StaticMesh->BodySetup->AggGeom.DrawAggGeom(PDI, GeomMatrix, TotalScale3D, GetSelectionColor(GEngine->C_ScaleBoxHi, bDrawWireSelected), &CollisionMaterialInstance, (Owner == NULL), bDrawSimpleSolid);
			}
		}

		if(DPGIndex == SDPG_Foreground && (View->Family->ShowFlags & SHOW_Bounds) && (View->Family->ShowFlags & SHOW_StaticMeshes) && (GIsGame || !Owner || bSelected))
		{
			// Draw the static mesh's bounding box and sphere.
			DrawWireBox(PDI,PrimitiveSceneInfo->Bounds.GetBox(), FColor(72,72,255),SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(1,0,0),FVector(0,1,0),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(1,0,0),FVector(0,0,1),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(0,1,0),FVector(0,0,1),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
		}

		//<@ ava specific ; 2006. 8. 23 chnagmin
		if( DPGIndex == SDPG_Foreground && View->Family->ShowFlags & SHOW_RadiosityGeometry )
		{
			// Determine the LOD to use.
			FLOAT Distance = (FVector(View->ViewOrigin)-Bounds.Origin).Size();
			INT LODIndex = GetLOD(Distance);
			FStaticMeshRenderData& LODModel = const_cast<FStaticMeshRenderData&>(StaticMesh->LODModels(LODIndex));



			FMatrix ViewToWorld = View->ViewMatrix.Inverse();
			FVector WorldCameraZ = ViewToWorld.TransformNormal(FVector( 0, 0, 1));

			if( LODModel.BulkConvexPolygons.GetElementCount() )
			{
				//
				const AvaConvexPolygon* RawPolygons = (AvaConvexPolygon*)LODModel.BulkConvexPolygons.Lock( LOCK_READ_ONLY );
				const FVector* RawNormals = (FVector*)LODModel.BulkConvexPlanes.Lock( LOCK_READ_ONLY );
				const FVector* RawTanXs = (FVector*)LODModel.BulkConvexTangentXs.Lock( LOCK_READ_ONLY );
				const FVector* RawTanYs = (FVector*)LODModel.BulkConvexTangentYs.Lock( LOCK_READ_ONLY );
				const FLegacyStaticMeshVertex* RawVertices = (FLegacyStaticMeshVertex*)LODModel.BulkConvexPolygonVertexBuffer.Lock( LOCK_READ_ONLY );
				const INT* RawIndices = (INT*)LODModel.BulkConvexPolygonIndexBuffer.Lock( LOCK_READ_ONLY );

				for( INT PolygonIndex = 0; PolygonIndex < LODModel.BulkConvexPolygons.GetElementCount(); ++PolygonIndex )
				{
					//const AvaConvexPolygon& Poly = LODModel.ConvexPolygons(PolygonIndex);
					const AvaConvexPolygon& Poly = RawPolygons[PolygonIndex];

					FVector Normal = LocalToWorld.TransformNormal( RawNormals[Poly.PlainIndex_] );

					// back face culling
					if( (WorldCameraZ | Normal) > 0.0f )
					{
						continue;
					}

					// draw tangent space
					FVector s_axis = LocalToWorld.TransformNormal( RawTanXs[Poly.TangentXIndex_] );
					FVector t_axis = LocalToWorld.TransformNormal( RawTanYs[Poly.TangentYIndex_] );

					for( INT PolygonPoint = 0; PolygonPoint < Poly.NumIndex_; ++PolygonPoint )
					{
						INT LineStartIndex = RawIndices[Poly.IndexStart_ + PolygonPoint];
						INT LineEndIndex = RawIndices[Poly.IndexStart_ + ( PolygonPoint + 1) % Poly.NumIndex_];

						const FVector& LineStart = LocalToWorld.TransformFVector( RawVertices[LineStartIndex].Position );
						const FVector& LineEnd = LocalToWorld.TransformFVector( RawVertices[LineEndIndex].Position );

						// draw winding
						FVector Inside = Normal ^ (LineEnd-LineStart).SafeNormal();
						PDI->DrawLine( LineStart + Inside, LineEnd + Inside, Poly.Color_, SDPG_Foreground );

						// draw tangent space of vertex
						PDI->DrawLine( LineStart, LineStart + Normal * 8.0f, FColor(0, 255, 0), SDPG_Foreground );
						PDI->DrawLine( LineStart, LineStart + s_axis * 8.0f, FColor(255, 0, 0), SDPG_Foreground );
						PDI->DrawLine( LineStart, LineStart + t_axis * 8.0f, FColor(0, 0, 255), SDPG_Foreground );
					}
				}

				LODModel.BulkConvexPolygons.Unlock();
				LODModel.BulkConvexPlanes.Unlock();
				LODModel.BulkConvexTangentXs.Unlock();
				LODModel.BulkConvexTangentYs.Unlock();
				LODModel.BulkConvexPolygonVertexBuffer.Unlock();
				LODModel.BulkConvexPolygonIndexBuffer.Unlock();
			}
		}
		//>@ ava
	}

	/**
	 * Called by the rendering thread to notify the proxy when a light is no longer
	 * associated with the proxy, so that it can clean up any cached resources.
	 * @param Light - The light to be removed.
	 */
	virtual void OnDetachLight(const FLightSceneInfo* Light)
	{
		CachedShadowVolumes.RemoveShadowVolume(Light);
	}

	virtual void OnTransformChanged()
	{
		// Update the cached scaling.
		TotalScale3D.X = FVector(LocalToWorld.TransformNormal(FVector(1,0,0))).Size();
		TotalScale3D.Y = FVector(LocalToWorld.TransformNormal(FVector(0,1,0))).Size();
		TotalScale3D.Z = FVector(LocalToWorld.TransformNormal(FVector(0,0,1))).Size();
	}

	/**
	 * Removes potentially cached shadow volume data for the passed in light.
	 *
	 * @param Light		The light for which cached shadow volume data will be removed.
	 */
	virtual void RemoveCachedShadowVolumeData( const FLightSceneInfo* Light )
	{
		CachedShadowVolumes.RemoveShadowVolume(Light);
	}

	/**
	 * Called from the rendering thread, in the FSceneRenderer::RenderLights phase.
	 */
	virtual void DrawShadowVolumes(FShadowVolumeDrawInterface* SVDI,const FSceneView* View,const FLightSceneInfo* Light,UINT DPGIndex)
	{
		checkSlow(UEngine::ShadowVolumesAllowed());
		
		// Determine the DPG the primitive should be drawn in for this view.
		BYTE PrimitiveDPG = GetDepthPriorityGroup(View);

		if( PrimitiveDPG != DPGIndex ||
			// Don't draw shadows if we are in a collision geometry drawing mode.
			IsCollisionView(View) )
		{
			return;
		}

		SCOPE_CYCLE_COUNTER(STAT_ShadowVolumeRenderTime);

		//@todo: Use MemMark/MemPop for PlaneDots
		//@todo: Use a (not-yet-existing) dynamic IB API for IndexBuffer
		FLOAT Distance = (FVector(View->ViewOrigin)-PrimitiveSceneInfo->Bounds.Origin).Size();
		INT LODIndex = GetLOD(Distance);
		const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(LODIndex);

		// Check for the shadow volume in the cache.
		const FShadowVolumeCache::FCachedShadowVolume* CachedShadowVolume = CachedShadowVolumes.GetShadowVolume(Light);
		
		//check if the LOD has changed since last cache
		if (CachedShadowVolume && LastLOD != LODIndex) 
		{
			//remove the cache
			CachedShadowVolumes.RemoveShadowVolume(Light);

			//trigger a cache rebuild
			CachedShadowVolume = NULL;

			//update the lastLOD that the cache was updated at
			LastLOD = LODIndex;
		}
		
		if (!CachedShadowVolume)
		{
			SCOPE_CYCLE_COUNTER(STAT_ShadowExtrusionTime);

			FVector4	LightPosition	= LocalToWorld.Inverse().TransformFVector4(Light->GetPosition());
			UINT		NumTriangles	= (UINT)LODModel.IndexBuffer.Indices.Num() / 3;
			FLOAT*		PlaneDots		= new FLOAT[NumTriangles];
			const WORD*	Indices			= &LODModel.IndexBuffer.Indices(0);
			WORD		FirstExtrudedVertex = (WORD)LODModel.NumVertices;
			FShadowIndexBuffer IndexBuffer;

	#if XBOX
			// Use Xbox optimized function to get plane dots
			GetPlaneDotsXbox(NumTriangles,LightPosition, LODModel,(WORD*)Indices,PlaneDots);
	#else
			for(UINT TriangleIndex = 0;TriangleIndex < NumTriangles;TriangleIndex++)
			{
  				const FVector&	V1 = LODModel.PositionVertexBuffer.VertexPosition(Indices[TriangleIndex * 3 + 0]),
  								V2 = LODModel.PositionVertexBuffer.VertexPosition(Indices[TriangleIndex * 3 + 1]),
  								V3 = LODModel.PositionVertexBuffer.VertexPosition(Indices[TriangleIndex * 3 + 2]);
  				PlaneDots[TriangleIndex] = (((V2-V3) ^ (V1-V3)) | (FVector(LightPosition) - V1 * LightPosition.W));
  			}
	#endif
			IndexBuffer.Indices.Empty(NumTriangles * 3);

			for(UINT TriangleIndex = 0;TriangleIndex < NumTriangles;TriangleIndex++)
			{
				if(IsNegativeFloat(PlaneDots[TriangleIndex]))
				{
					IndexBuffer.AddFace(
						FirstExtrudedVertex + Indices[TriangleIndex * 3 + 0],
						FirstExtrudedVertex + Indices[TriangleIndex * 3 + 1],
						FirstExtrudedVertex + Indices[TriangleIndex * 3 + 2]
						);

					IndexBuffer.AddFace(
						Indices[TriangleIndex * 3 + 2],
						Indices[TriangleIndex * 3 + 1],
						Indices[TriangleIndex * 3 + 0]
						);
				}
			}

			for(UINT EdgeIndex = 0;EdgeIndex < (UINT)LODModel.Edges.Num();EdgeIndex++)
			{
				const FMeshEdge& Edge = LODModel.Edges(EdgeIndex);
				if(	(Edge.Faces[1] == INDEX_NONE && IsNegativeFloat(PlaneDots[Edge.Faces[0]])) ||
					(Edge.Faces[1] != INDEX_NONE && IsNegativeFloat(PlaneDots[Edge.Faces[0]]) != IsNegativeFloat(PlaneDots[Edge.Faces[1]])))
				{
					IndexBuffer.AddEdge(
						Edge.Vertices[IsNegativeFloat(PlaneDots[Edge.Faces[0]]) ? 1 : 0],
						Edge.Vertices[IsNegativeFloat(PlaneDots[Edge.Faces[0]]) ? 0 : 1],
						FirstExtrudedVertex
						);
				}
			}

			delete [] PlaneDots;

			// Add the new shadow volume to the cache.
			CachedShadowVolume = CachedShadowVolumes.AddShadowVolume(Light,IndexBuffer);
		}

		// Draw the cached shadow volume.
		if(CachedShadowVolume->NumTriangles)
		{
			INC_DWORD_STAT_BY(STAT_ShadowVolumeTriangles, CachedShadowVolume->NumTriangles);
			SVDI->DrawShadowVolume( CachedShadowVolume->IndexBufferRHI, LODModel.ShadowVertexFactory, LocalToWorld, 0, CachedShadowVolume->NumTriangles, 0, LODModel.NumVertices * 2 - 1 );
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{   
		FPrimitiveViewRelevance Result;
		if(IsShown(View))
		{
			if(View->Family->ShowFlags & SHOW_StaticMeshes)
			{
#if !FINAL_RELEASE
				if(IsCollisionView(View))
				{
					Result.bDynamicRelevance = TRUE;
					Result.bForceDirectionalLightsDynamic = TRUE;
				}
				else 
#endif
				if(
#if !FINAL_RELEASE
					IsRichView(View) || 
#endif
					HasViewDependentDPG() ||
					IsMovable()	)
				{
					Result.bDynamicRelevance = TRUE;
				}
				else
				{
					Result.bStaticRelevance = TRUE;
				}
				Result.bTranslucentRelevance = bHasTranslucency;
				Result.bDistortionRelevance = bHasDistortion;
				Result.bUsesSceneColor = bUsesSceneColor;
				Result.SetDPG(GetDepthPriorityGroup(View),TRUE);
			}
#if !FINAL_RELEASE
			if(View->Family->ShowFlags & (SHOW_Bounds|SHOW_Collision))
			{
				Result.bDynamicRelevance = TRUE;
				Result.SetDPG(SDPG_Foreground,TRUE);
			}
#endif
			Result.bDecalRelevance = HasRelevantDecals(View);

			if (IsShadowCast(View))
			{
				Result.bShadowRelevance = TRUE;
			}

			if (IsSeenThrough(View))
			{
				Result.bSeeThroughRelevance = TRUE;
			}
		}
		return Result;
	}


	/**
	 *	Determines the relevance of this primitive's elements to the given light.
	 *	@param	LightSceneInfo			The light to determine relevance for
	 *	@param	bDynamic (output)		The light is dynamic for this primitive
	 *	@param	bRelevant (output)		The light is relevant for this primitive
	 *	@param	bLightMapped (output)	The light is light mapped for this primitive
	 */
	virtual void GetLightRelevance(FLightSceneInfo* LightSceneInfo, UBOOL& bDynamic, UBOOL& bRelevant, UBOOL& bLightMapped)
	{
		// Attach the light to the primitive's static meshes.
		bDynamic = TRUE;
		bRelevant = FALSE;
		bLightMapped = TRUE;

		if (LODs.Num() > 0)
		{
			//<@ ava specific ; 2007. 10. 2 changmin
			extern UBOOL GUseCascadedShadow;
			if( LightSceneInfo->bUseCascadedShadowmap && GUseCascadedShadow )
			{
				if( bCastSunShadow )
				{
					bRelevant = TRUE;
					bLightMapped = FALSE;
					bDynamic = TRUE;
				}
				else
				{
					bRelevant = FALSE;
					bDynamic = TRUE;
					bLightMapped = TRUE;
				}
			}
			else
			//>@ ava
			for(INT LODIndex = 0; LODIndex < LODs.Num(); LODIndex++)
			{
				FLODInfo* LCI = &LODs(LODIndex);
				if (LCI)
				{
					ELightInteractionType InteractionType = LCI->GetInteraction(LightSceneInfo).GetType();
					if(InteractionType != LIT_CachedIrrelevant)
					{
						bRelevant = TRUE;
					}
					if(InteractionType != LIT_CachedLightMap && InteractionType != LIT_CachedIrrelevant)
					{
						bLightMapped = FALSE;
					}
					if(InteractionType != LIT_Uncached)
					{
						bDynamic = FALSE;
					}
				}
			}
		}
		else
		{
			bRelevant = TRUE;
			bLightMapped = FALSE;
		}
	}
	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocStMSP ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() + LODs.GetAllocatedSize() ); }

private:

	/** Information used by the proxy about a single LOD of the mesh. */
	class FLODInfo : public FLightCacheInterface
	{
	public:

		/** Information about an element of a LOD. */
		struct FElementInfo
		{
			UMaterialInstance* Material;			
		};
		TArray<FElementInfo> Elements;

		/** Initialization constructor. */
		FLODInfo(const UStaticMeshComponent* InComponent,INT InLODIndex):
		Component(InComponent),
			LODIndex(InLODIndex)
		{
			// Gather the materials applied to the LOD.
			Elements.Empty(Component->StaticMesh->LODModels(LODIndex).Elements.Num());
			for(INT MaterialIndex = 0;MaterialIndex < Component->StaticMesh->LODModels(LODIndex).Elements.Num();MaterialIndex++)
			{
				FElementInfo ElementInfo;

				// Determine the material applied to this element of the LOD.
				ElementInfo.Material = Component->GetMaterial(MaterialIndex,LODIndex);
				if(!ElementInfo.Material)
				{
					ElementInfo.Material = GEngine->DefaultMaterial;
				}

				// Store the element info.
				Elements.AddItem(ElementInfo);
			}
		}

		// Accessors.
		const FLightMap* GetLightMap() const
		{
			return LODIndex < Component->LODData.Num() ?
				Component->LODData(LODIndex).LightMap :
			NULL;
		}

		// FLightCacheInterface.
		virtual FLightInteraction GetInteraction(const FLightSceneInfo* LightSceneInfo) const
		{
			//<@ ava specific ; 2007. 10. 5 changmin
			// add cascaded shadow
			extern UBOOL GUseCascadedShadow;
			if( LightSceneInfo->bUseCascadedShadowmap && GUseCascadedShadow )
			{
				return FLightInteraction::Uncached();
			}
			else
			//>@ ava
			// Check if the light has static lighting or shadowing.
			// This directly accesses the component's static lighting with the assumption that it won't be changed without synchronizing with the rendering thread.
			if(LightSceneInfo->bStaticShadowing)
			{
				if(LODIndex < Component->LODData.Num())
				{
					const FStaticMeshComponentLODInfo& LODInstanceData = Component->LODData(LODIndex);
					if(LODInstanceData.LightMap)
					{
						if(LODInstanceData.LightMap->ContainsLight(LightSceneInfo->LightmapGuid))
						{
							return FLightInteraction::LightMap();
						}
					}
					for(INT LightIndex = 0;LightIndex < LODInstanceData.ShadowVertexBuffers.Num();LightIndex++)
					{
						const UShadowMap1D* const ShadowVertexBuffer = LODInstanceData.ShadowVertexBuffers(LightIndex);
						if(ShadowVertexBuffer && ShadowVertexBuffer->GetLightGuid() == LightSceneInfo->LightGuid)
						{
							return FLightInteraction::ShadowMap1D(ShadowVertexBuffer);
						}
					}
					for(INT LightIndex = 0;LightIndex < LODInstanceData.ShadowMaps.Num();LightIndex++)
					{
						const UShadowMap2D* const ShadowMap = LODInstanceData.ShadowMaps(LightIndex);
						if(ShadowMap && ShadowMap->IsValid() && ShadowMap->GetLightGuid() == LightSceneInfo->LightGuid)
						{
							return FLightInteraction::ShadowMap2D(
								LODInstanceData.ShadowMaps(LightIndex)->GetTexture(),
								LODInstanceData.ShadowMaps(LightIndex)->GetCoordinateScale(),
								LODInstanceData.ShadowMaps(LightIndex)->GetCoordinateBias()
								);
						}
					}
				}

				if(Component->IrrelevantLights.ContainsItem(LightSceneInfo->LightGuid))
				{
					return FLightInteraction::Irrelevant();
				}
			}

			// Use dynamic lighting if the light doesn't have static lighting.
			return FLightInteraction::Uncached();
		}		

	private:

		/** The static mesh component. */
		const UStaticMeshComponent* const Component;

		/** The LOD index. */
		const INT LODIndex;
	};

	/** Information about lights affecting a decal. */
	class FDecalLightCache : public FLightCacheInterface
	{
	public:
		FDecalLightCache()
			: DecalComponent( NULL )
			, LightMap( NULL )
		{
			ClearFlags();
		}

		FDecalLightCache(const FDecalInteraction& DecalInteraction, const FStaticMeshSceneProxy& Proxy)
			: DecalComponent( DecalInteraction.Decal )
		{
			ClearFlags();

			// Build the static light interaction map.
			for( INT LightIndex = 0 ; LightIndex < Proxy.StaticMeshComponent->IrrelevantLights.Num() ; ++LightIndex )
			{
				StaticLightInteractionMap.Set( Proxy.StaticMeshComponent->IrrelevantLights(LightIndex), FLightInteraction::Irrelevant() );
			}

			// If a custom vertex lightmap was specified with the decal, use it.
			// Otherwise, use the mesh's lightmap texture.
			if ( DecalInteraction.RenderData->LightMap1D )
			{
				// Decal vertex lightmap.
				LightMap = DecalInteraction.RenderData->LightMap1D;
			}
			else
			{
				// Lightmap texture from the underlying mesh.
				LightMap = Proxy.LODs.Num() > 0 ? Proxy.LODs(0).GetLightMap() : NULL;
			}
			if(LightMap)
			{
				for( INT LightIndex = 0 ; LightIndex < LightMap->LightGuids.Num() ; ++LightIndex )
				{
					StaticLightInteractionMap.Set( LightMap->LightGuids(LightIndex), FLightInteraction::LightMap() );
				}
			}
		}

		/**
		* Clears flags used to track whether or not this decal has already been drawn for dynamic lighting.
		* When drawing the first set of decals for this light, the blend state needs to be "set" rather
		* than "add."  Subsequent calls use "add" to accumulate color.
		*/
		void ClearFlags()
		{
			for ( INT DPGIndex = 0 ; DPGIndex < SDPG_MAX_SceneRender ; ++DPGIndex )
			{
				Flags[DPGIndex] = FALSE;
			}
		}

		/**
		* @return		The decal component associated with the decal interaction that uses this lighting information.
		*/
		const UDecalComponent* GetDecalComponent() const
		{
			return DecalComponent;
		}

		// FLightCacheInterface.
		virtual FLightInteraction GetInteraction(const FLightSceneInfo* LightSceneInfo) const
		{
			//<@ ava specific ; 2008. 1. 3 changmin
			// add cascaded shadow
			extern UBOOL GUseCascadedShadow;
			if( LightSceneInfo->bUseCascadedShadowmap && GUseCascadedShadow )
				return FLightInteraction::Uncached();
			//>@ ava
			// Check for a static light interaction.
			const FLightInteraction* Interaction = StaticLightInteractionMap.Find(LightSceneInfo->LightmapGuid);
			if(!Interaction)
			{
				Interaction = StaticLightInteractionMap.Find(LightSceneInfo->LightGuid);
			}
			return Interaction ? *Interaction : FLightInteraction::Uncached();
		}
		virtual const FLightMap* GetLightMap() const
		{
			return LightMap;
		}

		/** Tracks whether or not this decal has already been rendered for dynamic lighting. */
		// @todo: make a bitfield.
		UBOOL Flags[SDPG_MAX_SceneRender];

	private:
		/** The decal component associated with the decal interaction that uses this lighting information. */
		const UDecalComponent* DecalComponent;

		/** A map from persistent light IDs to information about the light's interaction with the primitive. */
		TMap<FGuid,FLightInteraction> StaticLightInteractionMap;

		/** The light-map used by the decal. */
		const FLightMap* LightMap;
	};

	TIndirectArray<FDecalLightCache> DecalLightCaches;	

	AActor* Owner;
	const UStaticMesh* StaticMesh;
	const UStaticMeshComponent* StaticMeshComponent;

	TIndirectArray<FLODInfo> LODs;

	/**
	* Used to update the shadow volume cache
	* LastLOD is initialized to an invalid LOD
	* so that the cache will be updated on the first DrawVolumeShadows()
	*/

	INT LastLOD;

	/**
	* The forcedLOD set in the static mesh editor, copied from the mesh component
	*/

	INT ForcedLodModel;

	FVector TotalScale3D;

	FColor LevelColor;
	FColor PropertyColor;

	const BITFIELD bCastShadow : 1;
	const BITFIELD bSelected : 1;
	const BITFIELD bShouldCollide : 1;
	const BITFIELD bBlockZeroExtent : 1;
	const BITFIELD bBlockNonZeroExtent : 1;
	const BITFIELD bBlockRigidBody : 1;
	const BITFIELD bHasTranslucency : 1;
	const BITFIELD bHasDistortion : 1;
	const BITFIELD bUsesSceneColor : 1;	

	const FLinearColor WireframeColor;

	/** Cached shadow volumes. */
	FShadowVolumeCache CachedShadowVolumes;

	/**
	* Returns the minimum distance that the given LOD should be displayed at
	*
	* @param CurrentLevel - the LOD to find the min distance for
	*/

	FLOAT GetMinLODDist(INT CurrentLevel) const {
		//Scale LODMaxRange by LODDistanceRatio and then split this range up by the number of LOD's
		FLOAT MinDist = CurrentLevel * StaticMesh->LODMaxRange * StaticMesh->LODDistanceRatio / StaticMesh->LODModels.Num();
		return MinDist;
	}

	/**
	* Returns the maximum distance that the given LOD should be displayed at
	* If the given LOD is the lowest detail LOD, then its maxDist will be WORLD_MAX
	*
	* @param CurrentLevel - the LOD to find the max distance for
	*/

	FLOAT GetMaxLODDist(INT CurrentLevel) const {

		//This level's MaxDist is the next level's MinDist
		FLOAT MaxDist = GetMinLODDist(CurrentLevel + 1);

		//If the lowest detail LOD was passed in, set MaxDist to WORLD_MAX so that it doesn't get culled
		if (CurrentLevel + 1 == StaticMesh->LODModels.Num()) 
		{
			MaxDist = WORLD_MAX;
		} 
		return MaxDist;
	}

	/**
	* Returns the LOD that should be used at the given distance
	*
	* @param Distance - distance from the current view to the component's bound origin
	*/

	INT GetLOD(FLOAT Distance) const {

		//If an LOD is being forced, use that one
		if (ForcedLodModel > 0)
		{
			return ForcedLodModel - 1;
		}

		//Calculate the maximum distance for the lowest detail LOD
		FLOAT EndDistance = StaticMesh->LODDistanceRatio * StaticMesh->LODMaxRange;

		//Get the percentage of the EndDistance that Distance is, then use this to choose the appropriate LOD
		INT NewLOD = appTrunc(FLOAT(StaticMesh->LODModels.Num()) * Distance / EndDistance);

		//make sure the result is valid
		NewLOD = Clamp(NewLOD, 0, StaticMesh->LODModels.Num() - 1);

		return NewLOD;
	}
};

FPrimitiveSceneProxy* UStaticMeshComponent::CreateSceneProxy()
{
	//@todo: figure out why i need a ::new (gcc3-specific)
	return ::new FStaticMeshSceneProxy(this);
}

UBOOL UStaticMeshComponent::ShouldRecreateProxyOnUpdateTransform() const
{
	// If the primitive is movable during gameplay, it won't use static mesh elements, and the proxy doesn't need to be recreated on UpdateTransform.
	// If the primitive isn't movable during gameplay, it will use static mesh elements, and the proxy must be recreated on UpdateTransform.
	UBOOL	bMovable = FALSE;
	if( GetOwner() )
	{
		bMovable = !GetOwner()->bStatic && GetOwner()->bMovable;
	}

	return bMovable==FALSE;
}
