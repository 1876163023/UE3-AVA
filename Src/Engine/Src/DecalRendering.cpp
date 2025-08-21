/*=============================================================================
	DecalRendering.cpp: High-level decal rendering implementation.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "DecalRendering.h"
#include "EngineDecalClasses.h"
#include "UnDecalRenderData.h"

/** 
 * Iterates over the sorted list of sorted decal prims and draws them.
 *
 * @param	Context							Command context.
 * @param	View							The Current view used to draw items.
 * @param	DPGIndex						The current DPG used to draw items.
 * @param	bTranslucentReceiverPass		TRUE during the decal pass for translucent receivers, FALSE for opaque receivers.
 * @param	bPreFog							TRUE if the draw call is occurring before fog has been rendered.
 * @return									TRUE if anything was drawn, FALSE otherwise.
 */
UBOOL FDecalPrimSet::Draw(FCommandContextRHI* Context,
						  const FViewInfo* View,
						  UINT DPGIndex,
						  UBOOL bTranslucentReceiverPass,
						  UBOOL bPreFog)
{
	UBOOL bDirty = FALSE;
	if( Prims.Num() )
	{
		TDynamicPrimitiveDrawer<FEmissiveDrawingPolicyFactory> EmissiveDrawer(
			Context,
			View,
			DPGIndex,
			FEmissiveDrawingPolicyFactory::ContextType(),
			bPreFog
			);

		TDynamicPrimitiveDrawer<FTranslucencyDrawingPolicyFactory> TranslucencyDrawer(
			Context,
			View,
			DPGIndex,
			FTranslucencyDrawingPolicyFactory::ContextType(),
			bPreFog
			);

		// Draw sorted scene prims
		for( INT PrimIdx = 0 ; PrimIdx < Prims.Num() ; ++PrimIdx )
		{
			const FPrimitiveSceneInfo* PrimitiveSceneInfo = Prims(PrimIdx);

			// Check that the primitive hasn't been occluded.
			if( View->PrimitiveVisibilityMap( PrimitiveSceneInfo->Id ) )
			{
				const UBOOL bHasTranslucentRelevance = View->PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id).bTranslucentRelevance;
				if((bTranslucentReceiverPass && bHasTranslucentRelevance) || (!bTranslucentReceiverPass && !bHasTranslucentRelevance))
				{
					EmissiveDrawer.SetPrimitive( PrimitiveSceneInfo );
					TranslucencyDrawer.SetPrimitive( PrimitiveSceneInfo );

					PrimitiveSceneInfo->Proxy->DrawDecalElements(
						Context,
						&EmissiveDrawer,
						&TranslucencyDrawer,
						View,
						DPGIndex,
						bTranslucentReceiverPass
						);
				}
			}
		}

		// Mark dirty if something was rendered.
		bDirty |= EmissiveDrawer.IsDirty();
		bDirty |= TranslucencyDrawer.IsDirty();
	}
	return bDirty;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FSceneRenderer::RenderTranslucentDecals
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** 
 * Renders the scene's decals.
 *
 * @param	DPGIndex					Current DPG used to draw items.
 * @param	bTranslucentReceiverPass	TRUE during the decal pass for translucent receivers, FALSE for opaque receivers.
 * @param	bPreFog						TRUE if the draw call is occurring before fog has been rendered.
 * @return								TRUE if anything was drawn.
 */
UBOOL FSceneRenderer::RenderDecals(UINT DPGIndex, UBOOL bTranslucentReceiverPass, UBOOL bPreFog)
{
	SCOPED_DRAW_EVENT(EventDecals)(DEC_SCENE_ITEMS,TEXT("RenderDecals"));

	UBOOL bRenderDecals = FALSE;
	for( INT ViewIndex = 0 ; ViewIndex < Views.Num() ; ++ViewIndex )
	{
		const FViewInfo& View = Views(ViewIndex);
		if( View.DecalPrimSet[DPGIndex].NumPrims() > 0 )
		{
			bRenderDecals = TRUE;
			break;
		}
	}

	UBOOL bDirty = FALSE;
	if( bRenderDecals )
	{
		RHISetDepthState( GlobalContext, TStaticDepthState<FALSE,CF_LessEqual>::GetRHI() );

		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);		

			// Set the viewport to match the view size.
			FViewInfo& View = Views(ViewIndex);
			RHISetViewport(GlobalContext,View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
			RHISetViewParameters( GlobalContext, &View, View.ViewProjectionMatrix, View.ViewOrigin );

			// Render the decals.
			bDirty |= View.DecalPrimSet[DPGIndex].Draw( GlobalContext, &View, DPGIndex, bTranslucentReceiverPass, bPreFog );
		}
	}
	return bDirty;
}

static struct FDecalVertexFactory : FLocalVertexFactory 
{
	FDecalVertexFactory() 
	{
		DataType Data;

		Data.PositionComponent			= STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FDecalVertex, Position, VET_Float3 );
		Data.TangentBasisComponents[0]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FDecalVertex, TangentX, VET_PackedNormal );
		Data.TangentBasisComponents[1]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FDecalVertex, TangentY, VET_PackedNormal );
		Data.TangentBasisComponents[2]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FDecalVertex, TangentZ, VET_PackedNormal );

		// Texture coordinates.
		Data.TextureCoordinates.Empty();
		Data.TextureCoordinates.AddItem( STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FDecalVertex, UV, VET_Float2 ) );

		// @todo DB: At the moment, FLocalVertexFactory asserts on non VET_Float2 texture coordinate channels,
		// @todo DB: so the normal transform information has to be split across two texture coordinates.

		Data.TextureCoordinates.AddItem( STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FDecalVertex, NormalTransform0, VET_Float2 ) );
		Data.TextureCoordinates.AddItem( STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FDecalVertex, NormalTransform1, VET_Float2 ) );

		Data.ShadowMapCoordinateComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FDecalVertex, LightMapCoordinate, VET_Float2 );

		SetData( Data );
	}
} GDecalVertexFactory;

UBOOL IsAffordableForDecalBatcher( INT NumVertices, INT NumIndices )
{
	return IsAffordableForDynamicMesh( NumVertices * sizeof(FDecalVertex), NumIndices * sizeof(WORD), NumVertices * sizeof(FQuantizedLightSample) );
}

void DrawBatchedDecals(
					   FDrawBatchedDecalContext* Context,		
					   INT NumVerticesToRender,
					   INT NumIndicesToRender,
					   const TArray<FDecalInteraction*>& DPGDecals,
					   INT& DecalIndex,
					   INT Batch,
					   UBOOL bUsingVertexLightmapping
					   )
{
	const INT BaseDecalIndex = DecalIndex;
	DecalIndex += Batch - 1;

	INT NumAvailableVertices = 0;
	INT NumAvailableIndices = 0;

	FDynamicMeshContext DynamicMeshContext = {0};			

	FDecalVertex* Vertex = NULL;
	WORD* Index = NULL;
	FQuantizedLightSample* VertexColor = NULL;

	for (INT BatchIndex = 0; BatchIndex < Batch; ++BatchIndex)
	{
		FDecalInteraction* DecalInteraction = DPGDecals(BaseDecalIndex+BatchIndex);			

		INT NumVertices = DecalInteraction->RenderData->Vertices.Num();

		const TArray<WORD>& Indices = DecalInteraction->RenderData->IndexBuffer.Indices;
		INT NumIndices = Indices.Num();

		if (NumAvailableVertices < NumVertices || NumAvailableIndices < NumIndices)
		{
			// Commit first :)
			if (BatchIndex > 0)
			{
				CommitDynamicMesh( DynamicMeshContext );				
				DynamicMeshContext.GenerateMeshElement( *Context->MeshElement, GDecalVertexFactory );			

				INC_DWORD_STAT_BY(STAT_DecalTriangles,Context->MeshElement->NumPrimitives);
				DrawRichMesh(Context->PDI,*Context->MeshElement,FLinearColor(0.5f,1.0f,0.5f),Context->LevelColor,Context->PropertyColor,Context->PrimitiveSceneInfo,FALSE);
			}			

			AllocDynamicMesh( DynamicMeshContext, sizeof(FDecalVertex), NumVertices, sizeof(WORD), NumIndices, bUsingVertexLightmapping );
			Vertex = (FDecalVertex*)DynamicMeshContext.VertexBuffer;
			Index = (WORD*)DynamicMeshContext.IndexBuffer;
			VertexColor = (FQuantizedLightSample*)DynamicMeshContext.VertexLightmapBuffer;
			NumAvailableVertices = DynamicMeshContext.MaxVertices;
			NumAvailableIndices = DynamicMeshContext.MaxIndices;

			// 뭔가 잘못된 경우 alloc되지 않았다.
			if (NumAvailableVertices < NumVertices || NumAvailableIndices < NumIndices)
			{				
				return;
			}
		}

		check( NumAvailableVertices >= NumVertices );
		check( NumAvailableIndices >= NumIndices );

		NumAvailableVertices -= NumVertices;
		NumAvailableIndices -= NumIndices;				

		NumVerticesToRender -= NumVertices;
		NumIndicesToRender -= NumIndices;

		/* generate indices */
		INT VertexOffset = DynamicMeshContext.NumVertices;

		for (INT i=0; i<NumIndices; ++i)
		{
			*Index++ = VertexOffset + Indices(i);						
		}

		/* advance indices */
		DynamicMeshContext.NumIndices += NumIndices;

		/* copy indices */
		appMemcpy( Vertex, &DecalInteraction->RenderData->Vertices(0), sizeof(FDecalVertex) * NumVertices );
		Vertex += NumVertices;

		if (bUsingVertexLightmapping)
		{					
			/// 이 상황이 발생하면 안되는데 -_-;
			if ( DecalInteraction->RenderData != NULL && DecalInteraction->RenderData->LightMap1D != NULL )
			{
				if (!static_cast<const FDynamicLightMap1D*>((FLightMap*)DecalInteraction->RenderData->LightMap1D)->Copy( VertexColor ))
				{
					appMemzero( VertexColor, sizeof(FQuantizedLightSample) * NumVertices );
				}
			}					
			else
			{
				appMemzero( VertexColor, sizeof(FQuantizedLightSample) * NumVertices );
			}

			VertexColor += NumVertices;
		}

		/* advance vertices */
		DynamicMeshContext.NumVertices += NumVertices;					
	}

	CommitDynamicMesh( DynamicMeshContext );
	DynamicMeshContext.GenerateMeshElement( *Context->MeshElement, GDecalVertexFactory );

	INC_DWORD_STAT_BY(STAT_DecalTriangles,Context->MeshElement->NumPrimitives);
	DrawRichMesh(Context->PDI,*Context->MeshElement,FLinearColor(0.5f,1.0f,0.5f),Context->LevelColor,Context->PropertyColor,Context->PrimitiveSceneInfo,FALSE);	
}
