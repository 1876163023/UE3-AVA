/*=============================================================================
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineDecalClasses.h"
#include "UnDecalRenderData.h"
#include "LevelUtils.h"
#include "HModel.h"

/*
 * ScenePrivate.h is needed by callbacks from the rendering thread:
 *   UModelComponent::BuildShadowVolume()
 *   UModelSceneProxy::DrawShadowVolumes()
 */
#include "ScenePrivate.h"
#include "BSPDynamicBatch.h"
#include "DecalRendering.h"
#include "BatchedElements.h"

/*-----------------------------------------------------------------------------
FModelVertexBuffer
-----------------------------------------------------------------------------*/

FModelVertexBuffer::FModelVertexBuffer(UModel* InModel):
	Model(InModel)
{}

void FModelVertexBuffer::InitRHI()
{
	// Calculate the buffer size.
	UINT Size = Vertices.GetResourceDataSize();
	if( Size > 0 )
	{
		// Create the buffer.
		VertexBufferRHI = RHICreateVertexBuffer(Size,&Vertices,FALSE);
	}
}

/**
* Serializer for this class
* @param Ar - archive to serialize to
* @param B - data to serialize
*/
FArchive& operator<<(FArchive& Ar,FModelVertexBuffer& B)
{
   B.Vertices.BulkSerialize(Ar);
   return Ar;
}

/*-----------------------------------------------------------------------------
FModelShadowVertexBuffer
-----------------------------------------------------------------------------*/

FModelShadowVertexBuffer::FModelShadowVertexBuffer(UModel* InModel)
:	FShadowVertexBuffer(0)
,	Model(InModel)
{
	check( Model );
}

void FModelShadowVertexBuffer::InitRHI()
{
	INT NumVertices = Model->Points.Num();
	if( NumVertices > 0)
	{
		Setup( NumVertices );
		FShadowVertexBuffer::InitRHI();
		UpdateVertices( &Model->Points(0), NumVertices, sizeof(Model->Points(0)) );
	}
}

/*-----------------------------------------------------------------------------
UModelComponent
-----------------------------------------------------------------------------*/

/**
 * Used to sort the model elements by material.
 */
void UModelComponent::BuildRenderData()
{
	UModel* Model = GetModel();

	// Initialize the component's light-map resources.
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		FModelElement& Element = Elements(ElementIndex);
		if(Element.LightMap != NULL)
		{
			Element.LightMap->InitResources();
		}
	}

	// Find the edges in UModel::Edges which are adjacent to this component's nodes.
	Edges.Empty();
	for(INT EdgeIndex = 0;EdgeIndex < Model->Edges.Num();EdgeIndex++)
	{
		FMeshEdge& Edge = Model->Edges(EdgeIndex);
		if(Model->Nodes(Edge.Faces[0]).ComponentIndex == ComponentIndex || (Edge.Faces[1] != INDEX_NONE && Model->Nodes(Edge.Faces[1]).ComponentIndex == ComponentIndex))
		{
			Edges.AddItem(EdgeIndex);
		}
	}

	// Build the component's index buffer and compute each element's bounding box.
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		FModelElement& Element = Elements(ElementIndex);

		// Find the index buffer for the element's material.
		FRawIndexBuffer32* IndexBuffer = Model->MaterialIndexBuffers.Find(Element.Material);
		if(!IndexBuffer)
		{
			IndexBuffer = &Model->MaterialIndexBuffers.Set(Element.Material,FRawIndexBuffer32());
		}
		Element.IndexBuffer = IndexBuffer;
		Element.FirstIndex = IndexBuffer->Indices.Num();
		Element.NumTriangles = 0;
		Element.MinVertexIndex = 0xffffffff;
		Element.MaxVertexIndex = 0;
		Element.BoundingBox.Init();
		Element.NodeRenderData.Empty();
		Element.SunLitNodeRenderData.Empty();

		for(INT NodeIndex = 0;NodeIndex < Element.Nodes.Num();NodeIndex++)
		{
			FBspNode& Node = Model->Nodes(Element.Nodes(NodeIndex));
			FBspSurf& Surf = Model->Surfs(Node.iSurf);
			// Don't put portal polygons in the static index buffer.
			if(Surf.PolyFlags & PF_Portal)
				continue;
			//<@ ava specific ; 2006. 11. 27 changmin
			if(Surf.PolyFlags & PF_Hint )
				continue;
			//>@ ava
			FBspNodeRenderData* RenderData = new(Element.NodeRenderData) FBspNodeRenderData;
			RenderData->bIsTwoSided = (Surf.PolyFlags & PF_TwoSided) ? 1 : 0;
			RenderData->NumVertices = Node.NumVertices;
			RenderData->VertexStart = Node.iVertexIndex;

			//<@ ava specific ; 2007. 11. 14 changmin
			extern UBOOL GUseCascadedShadow;
			if( GUseCascadedShadow )
			{
				if( Node.NodeFlags & NF_SunVisible )
				{
					FBspNodeRenderData* RenderData = new(Element.SunLitNodeRenderData) FBspNodeRenderData;
					RenderData->bIsTwoSided = (Surf.PolyFlags & PF_TwoSided) ? 1 : 0;
					RenderData->NumVertices = Node.NumVertices;
					RenderData->VertexStart = Node.iVertexIndex;
				}
			}
			//>@ ava

			for(UINT BackFace = 0;BackFace < (UINT)((Surf.PolyFlags & PF_TwoSided) ? 2 : 1);BackFace++)
			{
				if(Node.iZone[1-BackFace] == GetZoneIndex() || GetZoneIndex() == INDEX_NONE)
				{
					for(INT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
					{
						Element.BoundingBox += Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex);
					}
					for(INT VertexIndex = 2;VertexIndex < Node.NumVertices;VertexIndex++)
					{
						IndexBuffer->Indices.AddItem(Node.iVertexIndex + Node.NumVertices * BackFace);
						IndexBuffer->Indices.AddItem(Node.iVertexIndex + Node.NumVertices * BackFace + VertexIndex);
						IndexBuffer->Indices.AddItem(Node.iVertexIndex + Node.NumVertices * BackFace + VertexIndex - 1);
						Element.NumTriangles++;
					}
					Element.MinVertexIndex = Min(Node.iVertexIndex + Node.NumVertices * BackFace,Element.MinVertexIndex);
					Element.MaxVertexIndex = Max(Node.iVertexIndex + Node.NumVertices * BackFace + Node.NumVertices - 1,Element.MaxVertexIndex);
				}
			}
		}
		IndexBuffer->Indices.Shrink();
	}

	// 없어진 기능인데, 아직도 data를 만들고 있었다니요... 2008. 2. 25 changmin
	//CreateDepthMeshElement();
}

//<@ ava specific ; 2006. 10. 9 changmin
void UModelComponent::CreateDepthMeshElement()
{
	DepthElements.Empty();
	FModelElement& DepthElement = *(new(DepthElements) FModelElement(this, NULL));
	FRawIndexBuffer32* OpaqueIndexBuffer = Model->ComponentIndexBuffers.Find(ComponentIndex);
	if( !OpaqueIndexBuffer )
	{
		OpaqueIndexBuffer = &Model->ComponentIndexBuffers.Set(ComponentIndex, FRawIndexBuffer32());
	}
	DepthElement.IndexBuffer	= OpaqueIndexBuffer;
	DepthElement.FirstIndex		= OpaqueIndexBuffer->Indices.Num();
	DepthElement.NumTriangles	= 0;
	DepthElement.MinVertexIndex = 0xffffffff;
	DepthElement.MaxVertexIndex = 0;
	DepthElement.BoundingBox.Init();

	// Build the component's index buffer and compute each element's bounding box.
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		FModelElement& Element = Elements(ElementIndex);
		FMaterialInstance* MatInstance = NULL;
		if( Element.Material )
		{
			MatInstance = Element.Material->GetMaterial()->GetInstanceInterface(FALSE);
		}
		const UBOOL bIsOpaque = MatInstance ? (MatInstance->GetMaterial()->GetBlendMode() == BLEND_Opaque)  : TRUE;
		for(INT NodeIndex = 0;NodeIndex < Element.Nodes.Num();NodeIndex++)
		{
			FBspNode& Node = Model->Nodes(Element.Nodes(NodeIndex));
			FBspSurf& Surf = Model->Surfs(Node.iSurf);

			// Don't put portal polygons in the static index buffer.
			if(Surf.PolyFlags & PF_Portal)
				continue;
			//<@ ava specific ; 2006. 11. 27 changmin
			if( Surf.PolyFlags & PF_Hint)
				continue;
			//>@ ava
			if( bIsOpaque )
			{
				for(UINT BackFace = 0;BackFace < (UINT)((Surf.PolyFlags & PF_TwoSided) ? 2 : 1);BackFace++)
				{
					if(Node.iZone[1-BackFace] == GetZoneIndex() || GetZoneIndex() == INDEX_NONE)
					{
						for(INT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
						{
							DepthElement.BoundingBox += Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex);
						}
						for(INT VertexIndex = 2;VertexIndex < Node.NumVertices;VertexIndex++)
						{
							OpaqueIndexBuffer->Indices.AddItem(Node.iVertexIndex + Node.NumVertices * BackFace);
							OpaqueIndexBuffer->Indices.AddItem(Node.iVertexIndex + Node.NumVertices * BackFace + VertexIndex);
							OpaqueIndexBuffer->Indices.AddItem(Node.iVertexIndex + Node.NumVertices * BackFace + VertexIndex - 1);
							DepthElement.NumTriangles++;
						}
						DepthElement.MinVertexIndex = Min(Node.iVertexIndex + Node.NumVertices * BackFace,Element.MinVertexIndex);
						DepthElement.MaxVertexIndex = Max(Node.iVertexIndex + Node.NumVertices * BackFace + Node.NumVertices - 1,Element.MaxVertexIndex);
					}
				}
			}
		}
	}
	OpaqueIndexBuffer->Indices.Shrink();
}
//>@ ava


/**
 * Called from the rendering thread to generate shadow volume indices for a particular light.
 * @param IndexBuffer	Index buffer to store the generated shadow volume indices
 * @param Light			The light to generate the shadow volume from
 */
void UModelComponent::BuildShadowVolume( FShadowIndexBuffer &IndexBuffer, const FLightSceneInfo* Light ) const
{
	FVector4 LightPosition = LocalToWorld.Inverse().TransformFVector4(Light->GetPosition());
	WORD FirstExtrudedVertex = Model->Points.Num();
	FLOAT* PlaneDots = new FLOAT[Nodes.Num()];

	IndexBuffer.Indices.Empty();

	for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
		PlaneDots[NodeIndex] = Model->Nodes(Nodes(NodeIndex)).Plane | FPlane(LightPosition.X,LightPosition.Y,LightPosition.Z,-LightPosition.W);

	for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
	{
		FBspNode&	Node = Model->Nodes(Nodes(NodeIndex));
		FBspSurf&	Surf = Model->Surfs(Node.iSurf);
		if(!(Surf.PolyFlags & PF_TwoSided))
		{
			if(IsNegativeFloat(PlaneDots[NodeIndex]))
			{
				for(UINT VertexIndex = 2;VertexIndex < Node.NumVertices;VertexIndex++)
				{
					IndexBuffer.AddFace(
						Model->Verts(Node.iVertPool).pVertex,
						Model->Verts(Node.iVertPool + VertexIndex - 1).pVertex,
						Model->Verts(Node.iVertPool + VertexIndex).pVertex
						);
					IndexBuffer.AddFace(
						FirstExtrudedVertex + Model->Verts(Node.iVertPool).pVertex,
						FirstExtrudedVertex + Model->Verts(Node.iVertPool + VertexIndex).pVertex,
						FirstExtrudedVertex + Model->Verts(Node.iVertPool + VertexIndex - 1).pVertex
						);
				}
			}
		}
	}

	for(INT EdgeIndex = 0;EdgeIndex < Edges.Num();EdgeIndex++)
	{
		FMeshEdge& Edge = Model->Edges(Edges(EdgeIndex));
		FBspNode& Node0 = Model->Nodes(Edge.Faces[0]);
		if(Node0.ComponentIndex == ComponentIndex)
		{
			if(Edge.Faces[1] != INDEX_NONE && Model->Nodes(Edge.Faces[1]).ComponentIndex == ComponentIndex)
			{
				if(IsNegativeFloat(PlaneDots[Node0.ComponentNodeIndex]) == IsNegativeFloat(PlaneDots[Model->Nodes(Edge.Faces[1]).ComponentNodeIndex]))
					continue;
			}
			else if(!IsNegativeFloat(PlaneDots[Node0.ComponentNodeIndex]))
				continue;

			IndexBuffer.AddEdge(
				IsNegativeFloat(PlaneDots[Node0.ComponentNodeIndex]) ? Edge.Vertices[0] : Edge.Vertices[1],
				IsNegativeFloat(PlaneDots[Node0.ComponentNodeIndex]) ? Edge.Vertices[1] : Edge.Vertices[0],
				FirstExtrudedVertex
				);
		}
		else if(Edge.Faces[1] != INDEX_NONE)
		{
			FBspNode& Node1 = Model->Nodes(Edge.Faces[1]);
			if(Node1.ComponentIndex == ComponentIndex)
			{
				if(!IsNegativeFloat(PlaneDots[Node1.ComponentNodeIndex]))
					continue;

				IndexBuffer.AddEdge(
					IsNegativeFloat(PlaneDots[Node1.ComponentNodeIndex]) ? Edge.Vertices[1] : Edge.Vertices[0],
					IsNegativeFloat(PlaneDots[Node1.ComponentNodeIndex]) ? Edge.Vertices[0] : Edge.Vertices[1],
					FirstExtrudedVertex
					);
			}
		}
	}

	delete [] PlaneDots;

    IndexBuffer.Indices.Shrink();
}

/**
 * A dynamic model index buffer.
 */
class FModelDynamicIndexBuffer : public FIndexBuffer
{
public:

	FModelDynamicIndexBuffer(UINT InTotalIndices):
		FirstIndex(0),
		NextIndex(0),
		TotalIndices(InTotalIndices)
	{
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(UINT),TotalIndices * sizeof(UINT),NULL,FALSE);
		Lock();
	}

	~FModelDynamicIndexBuffer()
	{
		IndexBufferRHI.Release();
	}

	void AddNode(const UModel* Model,UINT NodeIndex,INT ZoneIndex)
	{
		const FBspNode& Node = Model->Nodes(NodeIndex);
		const FBspSurf& Surf = Model->Surfs(Node.iSurf);

		for(UINT BackFace = 0;BackFace < (UINT)((Surf.PolyFlags & PF_TwoSided) ? 2 : 1);BackFace++)
		{
			if(Node.iZone[1-BackFace] == ZoneIndex || ZoneIndex == INDEX_NONE)
			{
				for(INT VertexIndex = 2;VertexIndex < Node.NumVertices;VertexIndex++)
				{
					*Indices++ = Node.iVertexIndex + Node.NumVertices * BackFace;
					*Indices++ = Node.iVertexIndex + Node.NumVertices * BackFace + VertexIndex;
					*Indices++ = Node.iVertexIndex + Node.NumVertices * BackFace + VertexIndex - 1;
					NextIndex += 3;
				}
				MinVertexIndex = Min(Node.iVertexIndex + Node.NumVertices * BackFace,MinVertexIndex);
				MaxVertexIndex = Max(Node.iVertexIndex + Node.NumVertices * BackFace + Node.NumVertices - 1,MaxVertexIndex);
			}
		}
	}

	void Draw(
		const UModelComponent* Component,
		BYTE DepthPriorityGroup,
		const FMaterialInstance* MaterialInstance,
		const FLightCacheInterface* LCI,
		FPrimitiveDrawInterface* PDI,
		class FPrimitiveSceneInfo *PrimitiveSceneInfo,
		const FLinearColor& LevelColor,
		const FLinearColor& PropertyColor
		)
	{
		if(NextIndex > FirstIndex)
		{
			RHIUnlockIndexBuffer(IndexBufferRHI);

			FMeshElement MeshElement;
			MeshElement.IndexBuffer = this;
			MeshElement.VertexFactory = &Component->GetModel()->VertexFactory;
			MeshElement.MaterialInstance = MaterialInstance;
			MeshElement.LCI = LCI;
			MeshElement.LocalToWorld = Component->LocalToWorld;
			MeshElement.WorldToLocal = Component->LocalToWorld.Inverse();			
			MeshElement.FirstIndex = FirstIndex;
			MeshElement.NumPrimitives = (NextIndex - FirstIndex) / 3;
			MeshElement.MinVertexIndex = MinVertexIndex;
			MeshElement.MaxVertexIndex = MaxVertexIndex;
			MeshElement.Type = PT_TriangleList;
			MeshElement.DepthPriorityGroup = DepthPriorityGroup;
			DrawRichMesh(PDI,MeshElement,FLinearColor::White, LevelColor, PropertyColor, PrimitiveSceneInfo,FALSE);

			FirstIndex = NextIndex;
			Lock();
		}
	}

private:
	UINT FirstIndex;
	UINT NextIndex;
	UINT MinVertexIndex;
	UINT MaxVertexIndex;
	UINT TotalIndices;
	UINT* Indices;
	
	void Lock()
	{
		if(NextIndex < TotalIndices)
		{
			Indices = (UINT*)RHILockIndexBuffer(IndexBufferRHI,FirstIndex * sizeof(UINT),TotalIndices * sizeof(UINT) - FirstIndex * sizeof(UINT));
			MaxVertexIndex = 0;
			MinVertexIndex = MAXINT;
		}
	}
};

IMPLEMENT_COMPARE_CONSTPOINTER( FDecalInteraction, UnModelRender,
{
	if ((A->DecalState.SortOrder == B->DecalState.SortOrder))
	{
		return (INT)A->DecalState.DecalMaterial - (INT)B->DecalState.DecalMaterial;
	}

	return (A->DecalState.SortOrder < B->DecalState.SortOrder) ? -1 : 1;
} );

//<@ ava specific ; 2006. 12. 06 changmin
UBOOL GUsingBSPDynamicBatch = FALSE;
//>@ ava

/**
 * A model component scene proxy.
 */
class FModelSceneProxy : public FPrimitiveSceneProxy
{
public:

	FModelSceneProxy(const UModelComponent* InComponent):
		FPrimitiveSceneProxy(InComponent),
		Component(InComponent),
		LevelColor(255,255,255),
		PropertyColor(255,255,255),
		bHasTranslucency(InComponent->HasTranslucency()),
		bHasDistortion(InComponent->HasUnlitDistortion()),
		bUsesSceneColor(bHasTranslucency && Component->UsesSceneColor()),
		bUsesLightMapping(FALSE)
	{
		Elements.Empty(Component->GetElements().Num());
		for(INT ElementIndex = 0;ElementIndex < Component->GetElements().Num();ElementIndex++)
		{
			const FElementInfo* NewElementInfo = new(Elements) FElementInfo(Component->GetElements()(ElementIndex));
			if ( !bUsesLightMapping && NewElementInfo->GetLightMap() )
			{
				bUsesLightMapping = TRUE;
			}
		}

		//<@ ava specific ; 2007. 2. 28 changmin
		BspElements.Empty(Component->GetElements().Num());
		for(INT ElementIndex = 0;ElementIndex < Component->GetElements().Num();ElementIndex++)
		{
			if( Component->GetElements()(ElementIndex).NumTriangles > 0 )
			{
				new(BspElements) AvaElementInfo(Component->GetElements()(ElementIndex));
			}
		}
		BspElements.Shrink();
		//>@ ava

		// Try to find a color for level coloration.
		UObject* ModelOuter = InComponent->GetModel()->GetOuter();
		ULevel* Level = Cast<ULevel>( ModelOuter );
		if ( Level )
		{
			ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel( Level );
			if ( LevelStreaming )
			{
				LevelColor = LevelStreaming->DrawColor;
			}
		}

		// Get a color for property coloration.
		GEngine->GetPropertyColorationColor( (UObject*)InComponent, PropertyColor );
	}

	UBOOL IsCacheable() const
	{
		return TRUE;
	}

	UBOOL IsStillValid( const UModelComponent* InComponent ) const
	{
		const UModelComponent* Component = static_cast<const UModelComponent*>(InComponent);

		return GIsGame && FPrimitiveSceneProxy::IsStillValid( InComponent );		
	}

	void Touch( const FPrimitiveSceneProxyCacheInfo& PRI ) 
	{
		FPrimitiveSceneProxy::Touch( PRI );

		// The scene proxy simply copies decal interaction data.
		// Interaction->RenderData is owned by the decal.
		Decals.Empty( Component->DecalList.Num() );
		for ( INT DecalIndex = 0 ; DecalIndex < Component->DecalList.Num() ; ++DecalIndex )
		{
			const FDecalInteraction* DecalInteraction = Component->DecalList(DecalIndex);
			if ( DecalInteraction )
			{
				const FDecalInteraction* NewDecalInteraction = new(Decals) FDecalInteraction( *DecalInteraction );
			}
		}
	}

	virtual HHitProxy* CreateHitProxies(const UPrimitiveComponent*,TArray<TRefCountPtr<HHitProxy> >& OutHitProxies)
	{
		HHitProxy* ModelHitProxy = new HModel(Component->GetModel());
		OutHitProxies.AddItem(ModelHitProxy);
		return ModelHitProxy;
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

		// Determine the DPG the primitive should be drawn in for this view.
		const BYTE PrimitiveDPG = GetDepthPriorityGroup(View);
		
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
		
		// Sort and render all decals.
		Sort<USE_COMPARE_CONSTPOINTER(FDecalInteraction,UnModelRender)>( DPGDecals.GetTypedData(), DPGDecals.Num() );		
		for ( INT DecalIndex = 0 ; DecalIndex < DPGDecals.Num() ; ++DecalIndex )
		{
			FDecalInteraction* Decal = DPGDecals(DecalIndex);

			const FDecalState& DecalState = Decal->DecalState;
			FDecalRenderData* RenderData = Decal->RenderData;

			FMeshElement MeshElement;				
			MeshElement.MaterialInstance = DecalState.DecalMaterial->GetInstanceInterface(FALSE);				
			MeshElement.LCI = RenderData->LCI;
			MeshElement.LocalToWorld = LocalToWorld;
			MeshElement.WorldToLocal = LocalToWorld.Inverse();
			MeshElement.DepthBias = DecalState.DepthBias;
			MeshElement.SlopeScaleDepthBias = DecalState.SlopeScaleDepthBias;
			MeshElement.DepthPriorityGroup = PrimitiveDPG;

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
					MeshElement.MaterialInstance != Candidate->DecalState.DecalMaterial->GetInstanceInterface(FALSE) ||
					Decal->RenderData->LCI != Candidate->RenderData->LCI)
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

		// Determine the DPG the primitive should be drawn in for this view.
		const BYTE PrimitiveDPG = GetDepthPriorityGroup(View);

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
		// Sort and render all decals.
		Sort<USE_COMPARE_CONSTPOINTER(FDecalInteraction,UnModelRender)>( DPGDecals.GetTypedData(), DPGDecals.Num() );		
		for ( INT DecalIndex = 0 ; DecalIndex < DPGDecals.Num() ; ++DecalIndex )
		{
			FDecalInteraction* Decal = DPGDecals(DecalIndex);

			const FDecalState& DecalState = Decal->DecalState;
			FDecalRenderData* RenderData = Decal->RenderData;

			FMeshElement MeshElement;				
			MeshElement.MaterialInstance = DecalState.DecalMaterial->GetInstanceInterface(FALSE);				
			MeshElement.LCI = &Elements(RenderData->Data);
			MeshElement.LocalToWorld = LocalToWorld;
			MeshElement.WorldToLocal = LocalToWorld.Inverse();
			MeshElement.DepthBias = DecalState.DepthBias;
			MeshElement.SlopeScaleDepthBias = DecalState.SlopeScaleDepthBias;
			MeshElement.DepthPriorityGroup = PrimitiveDPG;			

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
					MeshElement.MaterialInstance != Candidate->DecalState.DecalMaterial->GetInstanceInterface(FALSE) ||
					Decal->RenderData->Data != Candidate->RenderData->Data )
					//Decal->RenderData->LCI != Candidate->RenderData->LCI)
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
	
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DepthPriorityGroup)
	{
		const UBOOL bDynamicBSPTriangles = GUsingBSPDynamicBatch || (View->Family->ShowFlags & SHOW_Selection) || IsRichView(View) || IsCollisionView(View);
		
		//<@ ava specific ; 2007. 2. 28 changmin
		// Determine the DPG the primitive should be drawn in for this view.
		BYTE PrimitiveDPG = GetDepthPriorityGroup(View);

		// 이것은 ViewOwner의 DepthPriorityGroup 기능이 필요한 Component에 필요한 함수입니다.
		// Model은 그것이 필요 없습니다...
		//BYTE PrimitiveDPG = DepthPriorityGroup;
		//> ava

		// Draw the BSP triangles.
		if((View->Family->ShowFlags & SHOW_BSPTriangles) && (View->Family->ShowFlags & SHOW_BSP) && bDynamicBSPTriangles)
		{
			if(DepthPriorityGroup == PrimitiveDPG)
			{
				if (GUsingBSPDynamicBatch)
				{
					extern UBOOL GDrawingShadowDepth;

					if (BspElements.Num() > 0)
					{
						// BspElement는 Trinagle > 0 인 ModelElement만의 모임입니다.
						const INT ElementsCount = BspElements.Num() -1;
						for(INT ElementIndex = 0;ElementIndex < ElementsCount;ElementIndex++)
						{
							const AvaElementInfo *BspElement = &BspElements(ElementIndex);
							PREFETCH( &BspElements(ElementIndex+1) );
							if (GDrawingShadowDepth || View->ViewFrustum.IntersectBox( BspElement->BoundingBox.GetCenter(), BspElement->BoundingBox.GetExtent() ) )
							{
								BspRendering_Add( *(BspElement->ModelElement) );
							}													
						}

						// last one
						const AvaElementInfo *BspElement = &BspElements(ElementsCount);
						if(GDrawingShadowDepth || View->ViewFrustum.IntersectBox( BspElement->BoundingBox.GetCenter(), BspElement->BoundingBox.GetExtent() ) )
						{
							BspRendering_Add( *(BspElement->ModelElement) );
						}
					}

					return;
				}
				
				FLinearColor UtilColor = LevelColor;
				if( IsCollisionView(View) )
				{
					UtilColor = GEngine->C_BSPCollision;
				}
				// <@ ava specific ; 2007. 1. 8 changmin
				// leaf coloration
				// Draw the mesh with Leaf Coloration
				UModel *Bsp = GWorld->PersistentLevel->StructuralModel;
				const UBOOL bHasPvsInfo = Bsp->LeafBytes > 0;
				const UBOOL bHasLeafInfo = Bsp->LeafInfos.Num() > 0 ;

				if(	(View->Family->ShowFlags & SHOW_LeafColoration)	&& !(View->Family->ShowFlags & SHOW_Wireframe)	&& bHasLeafInfo )
				{
					//<@ ava specific ; 2008. 2. 26 changmin
					// hit proxy rendering 시, model 의 surface 가 invalid 하다. 이를 막기 위한 코드
					if( !Component->GetModel()->IsReadyToRender() )
						return;
					//>@ ava .. check..

					UBOOL bSelected = FALSE;
					for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
					{
						const FModelElement& ModelElement = Component->GetElements()(ElementIndex);
						if(ModelElement.NumTriangles > 0)
						{
							const FElementInfo& ProxyElementInfo = Elements(ElementIndex);
							for(INT NodeIndex = 0;NodeIndex < ModelElement.Nodes.Num();NodeIndex++)
							{
								FBspNode& Node = Component->GetModel()->Nodes(ModelElement.Nodes(NodeIndex));
								FBspSurf& Surf = Component->GetModel()->Surfs(Node.iSurf);

								// Don't draw portal polygons.
								if(Surf.PolyFlags & PF_Portal)
									continue;

								//<@ ava specific ; 2006. 11. 27 changmin
								if( Surf.PolyFlags & PF_Hint)
									continue;
								//>@ ava

								bSelected = (Surf.PolyFlags & PF_Selected) ? TRUE : FALSE;
								if( bSelected )
									break;
							}
						}
						if( bSelected )
							break;
					}

					const UBOOL bHasLeafColors = Bsp->LeafColors.Num() > 0;
					for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
					{
						const FModelElement& ModelElement = Component->GetElements()(ElementIndex);
						if(ModelElement.NumTriangles > 0)
						{
							FMeshElement MeshElement;
							MeshElement.IndexBuffer = ModelElement.IndexBuffer;
							MeshElement.VertexFactory = &Component->GetModel()->VertexFactory;
							MeshElement.MaterialInstance = Elements(ElementIndex).GetMaterial()->GetInstanceInterface(FALSE);
							MeshElement.LCI = &Elements(ElementIndex);
							MeshElement.LocalToWorld = Component->LocalToWorld;
							MeshElement.WorldToLocal = Component->LocalToWorld.Inverse();								
							MeshElement.FirstIndex = ModelElement.FirstIndex;
							MeshElement.NumPrimitives = ModelElement.NumTriangles;
							MeshElement.MinVertexIndex = ModelElement.MinVertexIndex;
							MeshElement.MaxVertexIndex = ModelElement.MaxVertexIndex;
							MeshElement.Type = PT_TriangleList;
							MeshElement.DepthPriorityGroup = PrimitiveDPG;
							const UMaterial			*LevelColorationMaterial = (View->Family->ShowFlags & SHOW_ViewMode_Lit) ? GEngine->LevelColorationLitMaterial : GEngine->LevelColorationUnlitMaterial;
							const FMaterialInstance *LCMaterialInstance = LevelColorationMaterial->GetInstanceInterface(FALSE);

							UBOOL bColoration = FALSE;
							FColor LeafColor( 255, 255, 255 );
							if( bHasLeafColors )
							{
								INT ClusterNumber = Component->GetClusterNumber();	// 이전 component data는 초기화되지 않은 값을 가지고 있을 수 있다.
								if( ClusterNumber != INDEX_NONE
								&& (ClusterNumber >= 0)
								&& (ClusterNumber < Bsp->LeafColors.Num()) )
								{
									LeafColor = Bsp->LeafColors(ClusterNumber);
									FSceneViewState *ViewState = (FSceneViewState*) View->State;

									if( ViewState )
									{
										if( GEngine->LockPvs )
										{
											if( bHasPvsInfo )
											{
												const BYTE *Vis = ViewState->LockedVis;
												if( Vis && Vis[ClusterNumber>>3] & (1<<(ClusterNumber&7)) )
												{
													bColoration = TRUE;
												}
												else
												{
													bColoration = FALSE;
												}
											}
										}
										else
										{
											if( bSelected )
											{
												bColoration = TRUE;
											}
											//if( ViewState->CurrentLeaf == ClusterNumber )
											//{
											//	bColoration = TRUE;
											//}
											//else
											//{
											//	bColoration = FALSE;
											//}
										}
									}
								}
								if( ClusterNumber == INDEX_NONE )
								{
									LeafColor= FColor( 255, 255, 255 );
									bColoration = TRUE;
								}
							}
							if( bColoration )
							{
								const FColoredMaterialInstance LeafColorationMaterialInstance( LCMaterialInstance, LeafColor );
								MeshElement.MaterialInstance = &LeafColorationMaterialInstance;	// change material
								PDI->DrawMesh(MeshElement);
							}
							else
							{
								PDI->DrawMesh(MeshElement);
							}
						}
					}

					
					// view pvs mode 일 때는 locked leaf의 portal과 선택된 surface의 leaf의 portal을 그린다.
					// view pvs mode 가 아닐 때에는, select 된 surface의 leaf의 portal 만 그린다.
					if( GEngine->LockPvs )
					{
						const INT LeafIndex = Component->GetClusterNumber();
						FSceneViewState *ViewState = (FSceneViewState*) View->State;
						if( !bSelected && ViewState && ViewState->LockedLeaf != LeafIndex )
						{
							return;
						}
					}
					else
					{
						if( !bSelected )
						{
							return;
						}
					}

					//<@ ava specific ; 2008. 2. 25 changmin
					// draw leaf portal
					const INT LeafIndex = Component->GetClusterNumber();
					const UBOOL bValidLeaf = LeafIndex >= 0;
					if( bValidLeaf && bHasLeafColors && bHasLeafInfo )
					{
						UBOOL bDrawLeafPortal = FALSE;
						FSceneViewState *ViewState = (FSceneViewState*) View->State;
						BYTE *Vis = GEngine->LockPvs ? ViewState->LockedVis : ViewState->Vis;
						if( GEngine->LockPvs )
						{
							if( bHasPvsInfo )
							{
								if( Vis && Vis[LeafIndex>>3] & (1<<(LeafIndex&7)) )
								{
									bDrawLeafPortal = TRUE;
								}
							}
						}
						else
						{
							//if( LeafIndex == ViewState->CurrentLeaf )
							{
								bDrawLeafPortal = TRUE;
							}
						}

						if( bDrawLeafPortal )
						{
							FColor LeafColor(255, 255, 255);
							if( LeafIndex < Bsp->LeafColors.Num() )
							{
								LeafColor = Bsp->LeafColors(LeafIndex);
							}
							
							AvaLeafInfo& LeafInfo = Bsp->LeafInfos(LeafIndex);

							// build mesh of portals
							BYTE Alpha = (View->Family->ShowFlags & SHOW_ViewMode_Lit) ? 150 : 100;
							if( DepthPriorityGroup == SDPG_World )
							{
								for( INT PortalIndex = 0; PortalIndex < LeafInfo.NumPortals; ++PortalIndex )
								{
									AvaPortal& Portal = Bsp->Portals( LeafInfo.iFirstPortal + PortalIndex );

									check( Portal.Vertices.Num() > 2);

									// draw portal edge
									LeafColor.A = 255;
									for( INT VertexIndex = 0; VertexIndex < Portal.Vertices.Num(); ++VertexIndex )
									{
										const FVector& V0 = Portal.Vertices(VertexIndex);
										const FVector& V1 = Portal.Vertices((VertexIndex + 1) % Portal.Vertices.Num());
										PDI->DrawLine( V0, V1, LeafColor, SDPG_World );
									}

									// draw portal surface
									LeafColor.A = Alpha;
									INT BaseVertexIndex = PDI->Ava_AddVertex(Portal.Vertices(0), FVector2D(0.0f,0.0f), LeafColor, SDPG_World );
									if( BaseVertexIndex == INDEX_NONE )
									{
										continue;
									}
									for( INT VertexIndex = 1; VertexIndex < Portal.Vertices.Num(); ++VertexIndex )
									{
										PDI->Ava_AddVertex( Portal.Vertices(VertexIndex), FVector2D(0.0f,0.0f), LeafColor, SDPG_World );
									}

									for( INT PrimitiveIndex = 0; PrimitiveIndex < Portal.Vertices.Num()-2; ++PrimitiveIndex )
									{
										PDI->Ava_AddTriangle( BaseVertexIndex, BaseVertexIndex + PrimitiveIndex + 1, BaseVertexIndex + PrimitiveIndex + 2,
															  GWhiteTexture, BLEND_Translucent, SDPG_World );
									}
								}
							}
						}
					}
					//>@ ava
				}
				else
				// >@ ava
				if(View->Family->ShowFlags & SHOW_Selection)
				{
					UINT TotalIndices = 0;
					for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
					{
						const FModelElement& ModelElement = Component->GetElements()(ElementIndex);
						TotalIndices += ModelElement.NumTriangles * 3;
					}

					if(TotalIndices > 0)
					{
						FModelDynamicIndexBuffer IndexBuffer(TotalIndices);
						for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
						{
							const FModelElement& ModelElement = Component->GetElements()(ElementIndex);
							if(ModelElement.NumTriangles > 0)
							{
								const FElementInfo& ProxyElementInfo = Elements(ElementIndex);
								for(UINT bSelected = 0;bSelected < 2;bSelected++)
								{
									for(INT NodeIndex = 0;NodeIndex < ModelElement.Nodes.Num();NodeIndex++)
									{
										FBspNode& Node = Component->GetModel()->Nodes(ModelElement.Nodes(NodeIndex));
										FBspSurf& Surf = Component->GetModel()->Surfs(Node.iSurf);

										// Don't draw portal polygons.
										if(Surf.PolyFlags & PF_Portal)
											continue;

										//<@ ava specific ; 2006. 11. 27 changmin
										if( Surf.PolyFlags & PF_Hint)
											continue;
										//>@ ava

										if(((Surf.PolyFlags & PF_Selected) ? TRUE : FALSE) == bSelected)
										{
											IndexBuffer.AddNode(Component->GetModel(),ModelElement.Nodes(NodeIndex),Component->GetZoneIndex());
										}
									}

									IndexBuffer.Draw(
										Component,
										PrimitiveDPG,
										ProxyElementInfo.GetMaterial()->GetInstanceInterface(bSelected),
										&ProxyElementInfo,
										PDI,
										PrimitiveSceneInfo,
										UtilColor,
										PropertyColor
										);
								}
							}
						}
					}
				}
				else
				{
					//<@ ava specific ; 2008. 2. 26 changmin
					// hit proxy rendering 시, model 의 surface 가 invalid 하다. 이를 막기 위한 코드
					if( !Component->GetModel()->IsReadyToRender() )
						return;
					//>@ ava .. check..

					for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
					{
						const FModelElement& ModelElement = Component->GetElements()(ElementIndex);
						if(ModelElement.NumTriangles > 0)
						{
							FMeshElement MeshElement;
							MeshElement.IndexBuffer = ModelElement.IndexBuffer;
							MeshElement.VertexFactory = &Component->GetModel()->VertexFactory;
							MeshElement.MaterialInstance = Elements(ElementIndex).GetMaterial()->GetInstanceInterface(FALSE);
							MeshElement.LCI = &Elements(ElementIndex);
							MeshElement.LocalToWorld = Component->LocalToWorld;
							MeshElement.WorldToLocal = Component->LocalToWorld.Inverse();							
							MeshElement.FirstIndex = ModelElement.FirstIndex;
							MeshElement.NumPrimitives = ModelElement.NumTriangles;
							MeshElement.MinVertexIndex = ModelElement.MinVertexIndex;
							MeshElement.MaxVertexIndex = ModelElement.MaxVertexIndex;
							MeshElement.Type = PT_TriangleList;
							MeshElement.DepthPriorityGroup = PrimitiveDPG;
							DrawRichMesh(PDI,MeshElement,FLinearColor::White,UtilColor,FLinearColor::White,PrimitiveSceneInfo,FALSE);														
						}
					}
				}
			}
		}
	}

	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI)
	{
		//<@ ava speicifc ; 206. 12. 04 changmin.
		if( GIsGame && GUsingBSPDynamicBatch )
		{
			return;
		}
		//>@ ava
		if(!HasViewDependentDPG())
		{
			// Determine the DPG the primitive should be drawn in.
			BYTE PrimitiveDPG = GetStaticDepthPriorityGroup();

			for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
			{
				const FModelElement& ModelElement = Component->GetElements()(ElementIndex);
				if(ModelElement.NumTriangles > 0)
				{
					FMeshElement MeshElement;
					MeshElement.IndexBuffer = ModelElement.IndexBuffer;
					MeshElement.VertexFactory = &Component->GetModel()->VertexFactory;
					MeshElement.MaterialInstance = Elements(ElementIndex).GetMaterial()->GetInstanceInterface(FALSE);
					MeshElement.LCI = &Elements(ElementIndex);
					MeshElement.LocalToWorld = Component->LocalToWorld;
					MeshElement.WorldToLocal = Component->LocalToWorld.Inverse();					
					MeshElement.FirstIndex = ModelElement.FirstIndex;
					MeshElement.NumPrimitives = ModelElement.NumTriangles;
					MeshElement.MinVertexIndex = ModelElement.MinVertexIndex;
					MeshElement.MaxVertexIndex = ModelElement.MaxVertexIndex;
					MeshElement.Type = PT_TriangleList;
					MeshElement.DepthPriorityGroup = PrimitiveDPG;

					//<@ ava specific ; 2006. 9. 28 changmin
					MeshElement.bRenderMesh	= TRUE;
					MeshElement.bDepthMesh	= TRUE;
					MeshElement.BoundingBox = ModelElement.BoundingBox;
					//>@ ava

					PDI->DrawMesh(MeshElement,0,WORLD_MAX);
				}
			}

			if (0)
			//<@ ava specific ; 2006. 9. 27 changmin
			for( INT DepthElementIndex = 0; DepthElementIndex < Component->GetDepthElements().Num(); ++DepthElementIndex )
			{
				const FModelElement& DepthModelElement = Component->GetDepthElements()(DepthElementIndex);

				if( DepthModelElement.NumTriangles > 0  )
				{
					FMeshElement DepthMeshElement;

					// important.. this flag make difference from other rendering mesh elements
					DepthMeshElement.bRenderMesh= FALSE;
					DepthMeshElement.bDepthMesh	= TRUE;

					DepthMeshElement.IndexBuffer = DepthModelElement.IndexBuffer;
					DepthMeshElement.VertexFactory = &Component->GetModel()->VertexFactory;
					DepthMeshElement.MaterialInstance = GEngine->DefaultMaterial->GetInstanceInterface(FALSE);
					DepthMeshElement.LCI = NULL;
					DepthMeshElement.LocalToWorld = Component->LocalToWorld;
					DepthMeshElement.WorldToLocal = Component->LocalToWorld.Inverse();					
					DepthMeshElement.FirstIndex = DepthModelElement.FirstIndex;
					DepthMeshElement.NumPrimitives = DepthModelElement.NumTriangles;
					DepthMeshElement.MinVertexIndex = DepthModelElement.MinVertexIndex;
					DepthMeshElement.MaxVertexIndex = DepthModelElement.MaxVertexIndex;
					DepthMeshElement.Type = PT_TriangleList;
					DepthMeshElement.DepthPriorityGroup = PrimitiveDPG;
					DepthMeshElement.BoundingBox = DepthModelElement.BoundingBox;

					PDI->DrawMesh(DepthMeshElement,0,WORLD_MAX);
				}
			}
			//>@ ava
		}
	}

	/**
	 * Draws the primitive's shadow volumes.  This is called from the rendering thread,
	 * in the FSceneRenderer::RenderLights phase.
	 * @param SVDI - The interface which performs the actual rendering of a shadow volume.
	 * @param View - The view which is being rendered.
	 * @param Light - The light for which shadows should be drawn.
	 * @param DPGIndex - The depth priority group the light is being drawn for.
	 */
	virtual void DrawShadowVolumes(FShadowVolumeDrawInterface* SVDI, const FSceneView* View, const FLightSceneInfo* Light,UINT DPGIndex)
	{
		checkSlow(UEngine::ShadowVolumesAllowed());

		// Determine the DPG the primitive should be drawn in for this view.
		BYTE PrimitiveDPG = GetDepthPriorityGroup(View);

		if( PrimitiveDPG != DPGIndex )
		{
			return;
		}

		SCOPE_CYCLE_COUNTER(STAT_ShadowVolumeRenderTime);

		// Check for the shadow volume in the cache.
		const FShadowVolumeCache::FCachedShadowVolume* CachedShadowVolume = CachedShadowVolumes.GetShadowVolume(Light);
		if (!CachedShadowVolume)
		{
			SCOPE_CYCLE_COUNTER(STAT_ShadowExtrusionTime);

			FShadowIndexBuffer IndexBuffer;
			Component->BuildShadowVolume( IndexBuffer, Light );

			// Add the new shadow volume to the cache.
			CachedShadowVolume = CachedShadowVolumes.AddShadowVolume(Light,IndexBuffer);
		}

		// Draw the cached shadow volume.
		if(CachedShadowVolume->NumTriangles)
		{
			INC_DWORD_STAT_BY(STAT_ShadowVolumeTriangles, CachedShadowVolume->NumTriangles);
			INT NumVertices = Component->GetModel()->Points.Num();
			const FLocalShadowVertexFactory &ShadowVertexFactory = Component->GetModel()->ShadowVertexBuffer.GetVertexFactory();
			SVDI->DrawShadowVolume( CachedShadowVolume->IndexBufferRHI, ShadowVertexFactory, Component->LocalToWorld, 0, CachedShadowVolume->NumTriangles, 0, NumVertices * 2 - 1 );
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{
		FPrimitiveViewRelevance Result;
		if(IsShown(View))
		{
			//<@ ava specific ; 2006. 12. 01 changmin
			// bsp rendering
			if( GIsGame && GUsingBSPDynamicBatch)
			{
				Result.bDynamicRelevance	= TRUE;
				Result.bStaticRelevance		= FALSE;
				Result.bTranslucentRelevance	= bHasTranslucency;
				Result.bDistortionRelevance		= bHasDistortion;
				Result.bUsesSceneColor			= bUsesSceneColor;
				Result.SetDPG(GetDepthPriorityGroup(View),TRUE);

				if(View->Family->ShowFlags & SHOW_Decals)
				{
					const UBOOL bHasDecals = Decals.Num() > 0;
					Result.bDecalRelevance = bHasDecals;
				}
				return Result;
			}
			//>@ ava

			if((View->Family->ShowFlags & SHOW_BSPTriangles) && (View->Family->ShowFlags & SHOW_BSP))
			{
				if((View->Family->ShowFlags & SHOW_Selection) || IsRichView(View) || IsCollisionView(View) || HasViewDependentDPG())
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
			Result.bDecalRelevance = HasRelevantDecals(View);
		}
		if (IsShadowCast(View))
		{
			Result.bShadowRelevance = TRUE;
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

		if (Elements.Num() > 0)
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
			for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
			{
				FElementInfo* LCI = &Elements(ElementIndex);
				if (LCI)
				{
					ELightInteractionType InteractionType = LCI->GetInteraction(LightSceneInfo).GetType();
					if(InteractionType != LIT_CachedIrrelevant)
					{
						bRelevant = TRUE;
						if(InteractionType != LIT_CachedLightMap)
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
		}
		else
		{
			bRelevant = TRUE;
			bLightMapped = FALSE;
		}
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
	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const 
	{ 
		DWORD AdditionalSize = FPrimitiveSceneProxy::GetAllocatedSize();

		AdditionalSize += Elements.GetAllocatedSize();		

		return( AdditionalSize ); 
	}

private:
	/** Cached shadow volumes. */
	FShadowVolumeCache CachedShadowVolumes;
	const UModelComponent* Component;

	class FElementInfo: public FLightCacheInterface
	{
	public:

		/** Initialization constructor. */
		FElementInfo(const FModelElement& ModelElement):
			Bounds(ModelElement.BoundingBox)
		{
			// Determine the material applied to the model element.
			Material = ModelElement.Material;
			if(!Material)
			{
				Material = GEngine->DefaultMaterial;
			}

			// Build the static light interaction map.
			for(INT LightIndex = 0;LightIndex < ModelElement.IrrelevantLights.Num();LightIndex++)
			{
				StaticLightInteractionMap.Set(ModelElement.IrrelevantLights(LightIndex),FLightInteraction::Irrelevant());
			}
			LightMap = ModelElement.LightMap;
			if(LightMap)
			{
				for(INT LightIndex = 0;LightIndex < LightMap->LightGuids.Num();LightIndex++)
				{
					StaticLightInteractionMap.Set(LightMap->LightGuids(LightIndex),FLightInteraction::LightMap());
				}
			}
			for(INT LightIndex = 0;LightIndex < ModelElement.ShadowMaps.Num();LightIndex++)
			{
				UShadowMap2D* ShadowMap = ModelElement.ShadowMaps(LightIndex);
				if(ShadowMap && ShadowMap->IsValid())
				{
					StaticLightInteractionMap.Set(
						ShadowMap->GetLightGuid(),
						FLightInteraction::ShadowMap2D(
							ShadowMap->GetTexture(),
							ShadowMap->GetCoordinateScale(),
							ShadowMap->GetCoordinateBias()
							)
						);
				}
			}
		}

		// FLightCacheInterface.
		virtual FLightInteraction GetInteraction(const FLightSceneInfo* LightSceneInfo) const
		{
			//<@ ava specific ; 2007. 10. 31 changmin
			// add cascaded shadow
			extern UBOOL GUseCascadedShadow;
			if( LightSceneInfo->bUseCascadedShadowmap && GUseCascadedShadow )
				return FLightInteraction::Uncached();
			//>@ ava

			// Check for a static light Interaction->
			const FLightInteraction* Interaction = StaticLightInteractionMap.Find(LightSceneInfo->LightmapGuid);
			if(!Interaction)
			{
				Interaction = StaticLightInteractionMap.Find(LightSceneInfo->LightGuid);
			}
			if(Interaction)
			{
				return *Interaction;
			}
			
			// Cull the uncached light against the bounding box of the element.
			if( LightSceneInfo->AffectsBounds(Bounds) )
			{
				return FLightInteraction::Uncached();
			}
			else
			{
				return FLightInteraction::Irrelevant();
			}
		}
		virtual const FLightMap* GetLightMap() const
		{
			return LightMap;
		}

		// Accessors.
		const UMaterialInstance* GetMaterial() const { return Material; }

	private:

		/** The element's material. */
		const UMaterialInstance* Material;

		/** A map from persistent light IDs to information about the light's interaction with the model element. */
		TMap<FGuid,FLightInteraction> StaticLightInteractionMap;

		/** The light-map used by the element. */
		const FLightMap* LightMap;

		/** The element's bounding volume. */
		FBoxSphereBounds Bounds;
	};

	TArray<FElementInfo> Elements;	

	//<@ ava specific ; 2007. 2. 28 changmin
	struct AvaElementInfo
	{
		AvaElementInfo( const FModelElement& InModelElement )
		{
			BoundingBox = InModelElement.BoundingBox;
			ModelElement = &InModelElement;
		}
		const FModelElement *ModelElement;
		FBox BoundingBox;
	};
	TArray<AvaElementInfo> BspElements;
	//>@ ava

	FColor LevelColor;
	FColor PropertyColor;

	const BITFIELD bHasTranslucency : 1;
	const BITFIELD bHasDistortion : 1;
	const BITFIELD bUsesSceneColor : 1;
	BITFIELD bUsesLightMapping : 1;
};




FPrimitiveSceneProxy* UModelComponent::CreateSceneProxy()
{
	return ::new FModelSceneProxy(this);
}

UBOOL UModelComponent::ShouldRecreateProxyOnUpdateTransform() const
{
	return TRUE;
}

void UModelComponent::UpdateBounds()
{
	if(Model)
	{
		FBox	BoundingBox(0);
		for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
		{
			FBspNode& Node = Model->Nodes(Nodes(NodeIndex));
			for(INT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
			{
				BoundingBox += Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex);
			}
		}

		Bounds = FBoxSphereBounds(BoundingBox.TransformBy(LocalToWorld));

		//<@ ava specific ; 2008. 2. 27 changmin
		// compute leaf bounds for component
		// editor 에서만 leafbounds를 적용합니다... 차후에 게임에서 leaf 에 대한 occlusion query를 하지 않도록 하면, GIsEditor는 지원도 됩니다...
		if( GIsEditor && GWorld && GWorld->PersistentLevel && ClusterNumber != INDEX_NONE )
		{
			if( GWorld->PersistentLevel->StructuralModel->LeafInfos.Num() > 0 )
			{
				UModel *Bsp = GWorld->PersistentLevel->StructuralModel;
				if( Bsp->LeafInfos(ClusterNumber).PortalBounds.IsValid )
				{
					Bounds = Bounds.GetBox() + Bsp->LeafInfos(ClusterNumber).PortalBounds;
				}
			}
		}
		//>@ ava
	}
	else
	{
		Super::UpdateBounds();
	}
}

UBOOL UModelComponent::HasUnlitDistortion() const
{
	UBOOL bHasUnlitDistortion = FALSE;
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		UMaterialInstance* MaterialInstance = Elements(ElementIndex).Material;
		if(MaterialInstance)
		{
			UMaterial* Material = MaterialInstance->GetMaterial();
			if( Material && 
				Material->LightingModel == MLM_Unlit &&
				Material->HasDistortion() )
			{
				bHasUnlitDistortion = TRUE;
			}
		}
	}
	return bHasUnlitDistortion;
}

UBOOL UModelComponent::HasUnlitTranslucency() const
{
	UBOOL bHasUnlitTranslucency = FALSE;
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		UMaterialInstance* MaterialInstance = Elements(ElementIndex).Material;
		if(MaterialInstance)
		{
			UMaterial* Material = MaterialInstance->GetMaterial();
			if( Material &&
				Material->LightingModel == MLM_Unlit &&
				IsTranslucentBlendMode((EBlendMode)Material->BlendMode) )
			{
				bHasUnlitTranslucency = TRUE;
				break;
			}
		}
	}
	return bHasUnlitTranslucency;
}

UBOOL UModelComponent::HasTranslucency() const
{
	UBOOL bHasTranslucency = FALSE;
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		UMaterialInstance* MaterialInstance = Elements(ElementIndex).Material;
		if(MaterialInstance)
		{
			UMaterial* Material = MaterialInstance->GetMaterial();
			if( Material &&
				(Material->LightingModel == MLM_Unlit || Material->LightingModel == MLM_Phong) &&
				IsTranslucentBlendMode((EBlendMode)Material->BlendMode) )
			{
				bHasTranslucency = TRUE;
				break;
			}
		}
	}
	return bHasTranslucency;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Decals on BSP.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////



FDecalRenderData* UModelComponent::GenerateDecalRenderData(FDecalState* Decal) const
{
	// Do nothing if the specified decal doesn't project on BSP.
	if ( !Decal->bProjectOnBSP )
	{
		return NULL;
	}

	FDecalRenderData* DecalRenderData = NULL;

	// Transform decal properties into local space.
	const FDecalLocalSpaceInfo DecalInfo( *Decal, LocalToWorld.Inverse() );
	const FMatrix LocalToDecal( LocalToWorld * Decal->WorldToDecal );

	// Don't perform software clipping for dynamic decals or when requested.
	const UBOOL bNoClip = /*!Decal->DecalComponent->bStaticDecal || */Decal->bNoClip;

	// Temporaries.
	FDecalPoly Poly;
	FVector2D TempTexCoords;

	const FBox& DecalBoundingBox = Decal->DecalComponent->Bounds.GetBox();

	//UBOOL bIssuedWarning = FALSE;
	for( INT ElementIndex = 0 ; ElementIndex < Elements.Num() ; ++ElementIndex )
	{
		const FModelElement& Element = Elements(ElementIndex);

		if (!Element.BoundingBox.Intersect( DecalBoundingBox ))
			continue;		

		for ( INT NodeIndex = 0 ; NodeIndex < Element.Nodes.Num() ; ++NodeIndex )
		{
			const WORD ElementNodeIndex = Element.Nodes(NodeIndex);
			// If the hit node was specified on the decal and this is not that node, skip the element.
			if ( Decal->HitNodeIndex != -1 && Decal->HitNodeIndex != ElementNodeIndex )
			{
				continue;
			}

			const FBspNode& Node = Model->Nodes( ElementNodeIndex );			
			const FBspSurf& Surf = Model->Surfs( Node.iSurf );
			if ( (Surf.PolyFlags & PF_Portal) || (Surf.PolyFlags & PF_Invisible) )
			{
				continue;
			}

			//<@ ava specific ; 2006. 11. 27 changmin
			if( Surf.PolyFlags & PF_Hint )
			{
				continue;
			}
			//>@ ava			

			//const UBOOL bIsBackVisible = (Surf.PolyFlags & PF_TwoSided) ? TRUE : FALSE;

			// Discard if the polygon faces away from the decal.
			// The dot product is inverted because BSP polygon winding is clockwise.
			const FLOAT Dot = -(DecalInfo.LocalLookVector | Node.Plane);

			// Even if backface culling is disabled, reject triangles that view the decal at grazing angles.
			//<@ ava specific; 2007. 5. 17 changmin
			// 좀 더 엄격하게... 늘어나는 decal 생성을 막기 위해...
			if ( Dot > 0.5f || ( Decal->bProjectOnBackfaces && Abs( Dot ) > 0.5f ) )
			//>@ ava
			//if ( Dot > 1.e-3 || ( Decal->bProjectOnBackfaces && Abs( Dot ) > 1.e-3 ) )
			{
				if (Node.iCollisionBound != INDEX_NONE)
				{
					// Get nodes on this leaf's collision hull.
					const INT *HullNodes = &Model->LeafHulls( Node.iCollisionBound );

					// let for-loops do its role!
					for( ;*HullNodes!=INDEX_NONE;++HullNodes)
					{}				

					// Get precomputed maxima.
					const FLOAT* Temp = (FLOAT*)(++HullNodes);
					FBox NodeBoundingBox;
					NodeBoundingBox.Min.X = Temp[0]; NodeBoundingBox.Min.Y = Temp[1]; NodeBoundingBox.Min.Z = Temp[2];
					NodeBoundingBox.Max.X = Temp[3]; NodeBoundingBox.Max.Y = Temp[4]; NodeBoundingBox.Max.Z = Temp[5];

					if (!NodeBoundingBox.Intersect( DecalBoundingBox ))
						continue;		
				}			

				// Copy off the vertices into a temporary poly for clipping.
				Poly.Init();
				const INT FirstVertexIndex = Node.iVertPool;
				for ( INT VertexIndex = 0 ; VertexIndex < Node.NumVertices ; ++VertexIndex )
				{
					const FVert& ModelVert = Model->Verts( FirstVertexIndex + VertexIndex );
					new(Poly.Vertices) FVector( Model->Points( ModelVert.pVertex ) );
					new (Poly.LightmapCoords) FVector2D(ModelVert.ShadowTexCoord);
					Poly.Indices.AddItem( FirstVertexIndex + VertexIndex );					
				}

				// Clip against the decal.
				const UBOOL bClipPassed = bNoClip ? Poly.PartOfPolyInside( DecalInfo.Convex ) : Poly.ClipAgainstConvex( DecalInfo.Convex );
				if ( bClipPassed )
				{
					// Allocate a FDecalRenderData object if we haven't already.
					if ( !DecalRenderData )
					{
						DecalRenderData = new FDecalRenderData( NULL, TRUE, TRUE );
						// Store the model element index.
						DecalRenderData->Data = ElementIndex;
					}

					/*
					if ( !bIssuedWarning && DecalRenderData->Data != ElementIndex )
					{
					bIssuedWarning = TRUE;
					debugf(TEXT("UModelComponent::GenerateDecalRenderData - decal spans multiple elements"));
					}
					*/

					const FVector TextureX( Model->Vectors(Surf.vTextureU).SafeNormal() );
					const FVector TextureY( Model->Vectors(Surf.vTextureV).SafeNormal() );

					/* 잘 모르겠음~~~ 대략 맞을 듯 
					원본에서의 space는 decal space였으나 static mesh와 동일하게 local space로 이동시켰음. 이유는, batching하기 위해서... -_- */

					const FVector TangentZ = Node.Plane;
					FVector TangentX;
					FVector TangentY;

					// The tangent is nearly parallel to the normal, so use the binormal instead.
					TangentY = ( DecalInfo.LocalBinormal  - ((DecalInfo.LocalBinormal|TangentZ)*TangentZ) ).SafeNormal();
					TangentX = ( TangentY ^ TangentZ ).SafeNormal();

					// Generate transform from mesh tangent basis to decal tangent basis.
					// m is the upper 2x2 of the decal basis projected onto the mesh basis.
					// m[0],m[1] first column, m[2],m[3] second column.
					const FPlane m( TextureX | TangentX, TextureX | TangentY, TextureY | TangentX, TextureY | TangentY );

					const DWORD FirstVertexIndex = DecalRenderData->GetNumVertices();

					checkSlow( Poly.Vertices.Num() == Poly.LightmapCoords.Num() );
					for ( INT i = 0 ; i < Poly.Vertices.Num() ; ++i )
					{
						// Generate texture coordinates for the vertex using the decal tangents.
						DecalInfo.ComputeTextureCoordinates( LocalToWorld.TransformFVector( Poly.Vertices(i) ), TempTexCoords );

						// Store the decal vertex.
						DecalRenderData->AddVertex( FDecalVertex( Poly.Vertices( i ),
							TextureX,
							TextureY,
							Node.Plane,
							TempTexCoords,
							FVector2D( m.X, m.Z ),
							FVector2D( m.Y, m.W ),
							Poly.LightmapCoords( i ) ) );
					}

					// Triangulate the polygon and add indices to the index buffer
					for ( INT i = 0 ; i < Poly.Vertices.Num() - 2 ; ++i )
					{
						DecalRenderData->AddIndex( FirstVertexIndex+0 );
						DecalRenderData->AddIndex( FirstVertexIndex+i+2 );
						DecalRenderData->AddIndex( FirstVertexIndex+i+1 );
					}
				}
			} // if ( Dot )
		} // for ( Nodes )
	}

	// Finalize the data.
	if ( DecalRenderData )
	{
		DecalRenderData->NumTriangles = 0;// DecalRenderData->GetNumIndices()/3;		
	}

	return DecalRenderData;
}