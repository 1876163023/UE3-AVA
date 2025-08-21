#include "EnginePrivate.h"
#include "ScenePrivate.h"

DWORD GSilhouetteGroupFlags = 0;
DWORD GSeeThroughGroupMask = 0;
UBOOL GIsSubtracting;

static void TouchPrimitiveSceneInfo( const FPrimitiveSceneInfo* PrimitiveSceneInfo, FLOAT CurrentWorldTime )
{
	PrimitiveSceneInfo->Component->LastRenderTime = CurrentWorldTime;

	if(PrimitiveSceneInfo->Owner)
	{									
		PrimitiveSceneInfo->Owner->LastRenderTime = CurrentWorldTime;
	}
}

void SetSeeThroughGroup(FCommandContextRHI* Context, INT GroupIndex)
{
	if (GIsSubtracting)
	{
		if (GroupIndex & 1)
		{
			RHISetStencilState(Context,TStaticStencilState<
				TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,
				FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
				0xff,0xff,4
			>::GetRHI());

			if (GroupIndex != 3)
				GSilhouetteGroupFlags |= (1 << 1);
		}	
		else 
		{
			RHISetStencilState(Context,TStaticStencilState<
				TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,
				FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
				0xff,0xff,8
			>::GetRHI());

			if (GroupIndex != 4)
				GSilhouetteGroupFlags |= (1 << 2);
		}			
	}	
}	

class FSeeThroughPositionOnlyDrawingPolicy : public FPositionOnlyDepthDrawingPolicy 
{
public:
	FSeeThroughPositionOnlyDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialInstance* InMaterialInstance
		)
		: FPositionOnlyDepthDrawingPolicy(InVertexFactory,InMaterialInstance)
	{
	}

	void SetMeshRenderState(
		FCommandContextRHI* Context,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		const ElementDataType& ElementData
		) const
	{
		FPositionOnlyDepthDrawingPolicy::SetMeshRenderState( Context, PrimitiveSceneInfo, Mesh, bBackFace, ElementData );
		
		SetSeeThroughGroup( Context, PrimitiveSceneInfo->SeeThroughGroupIndex );
	}
};

/**
* Outputs no color, but can be used to write the mesh's depth values to the depth buffer.
*/
class FSeeThroughDrawingPolicy : public FDepthDrawingPolicy
{
public:
	FSeeThroughDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialInstance* InMaterialInstance
		)
		: FDepthDrawingPolicy(InVertexFactory,InMaterialInstance)
	{
	}
	
	void SetMeshRenderState(
		FCommandContextRHI* Context,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		const ElementDataType& ElementData
		) const
	{
		FDepthDrawingPolicy::SetMeshRenderState( Context, PrimitiveSceneInfo, Mesh, bBackFace, ElementData );

		SetSeeThroughGroup( Context, PrimitiveSceneInfo->SeeThroughGroupIndex );
	}
};

/**
* A drawing policy factory for the emissive drawing policy.
*/
class FSeeThroughDrawingPolicyFactory
{
public:

	enum { bAllowSimpleElements = FALSE };
	struct ContextType {};

	static void AddStaticMesh(FScene* Scene,FStaticMesh* StaticMesh,ContextType DrawingContext = ContextType());
	static UBOOL DrawDynamicMesh(
		FCommandContextRHI* Context,
		const FSceneView* View,
		ContextType DrawingContext,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		);

	static UBOOL IsMaterialIgnored(const FMaterialInstance* MaterialInstance);
	static UBOOL DrawStaticMesh( FCommandContextRHI* Context, const FSceneView* View, const FStaticMesh* Mesh, UBOOL bDrawnShared );
};

void FSeeThroughDrawingPolicyFactory::AddStaticMesh(FScene* Scene,FStaticMesh* StaticMesh,ContextType)
{	
}

UBOOL FSeeThroughDrawingPolicyFactory::DrawDynamicMesh(
	FCommandContextRHI* Context,
	const FSceneView* View,
	ContextType DrawingContext,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	// Only draw opaque materials in the depth pass.
	if(Mesh.MaterialInstance->GetMaterial()->GetBlendMode() == BLEND_Opaque)
	{
		if (!(PrimitiveSceneInfo && (GSeeThroughGroupMask & (1 << PrimitiveSceneInfo->SeeThroughGroupIndex ))))
			return FALSE;

		TouchPrimitiveSceneInfo( PrimitiveSceneInfo, View->Family->CurrentWorldTime );

		if (Mesh.VertexFactory->SupportsPositionOnlyStream())
		{
			FSeeThroughPositionOnlyDrawingPolicy DrawingPolicy(Mesh.VertexFactory,GEngine->DefaultMaterial->GetInstanceInterface(FALSE));

			if (!Mesh.bDrawnShared)
			{
				DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
			}
			DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FPositionOnlyDepthDrawingPolicy::ElementDataType());
			DrawingPolicy.DrawMesh(Context,Mesh);
		}
		else 
		{
			FSeeThroughDrawingPolicy DrawingPolicy(Mesh.VertexFactory,GEngine->DefaultMaterial->GetInstanceInterface(FALSE));

			if (!Mesh.bDrawnShared)
			{
				DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
			}
			DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FMeshDrawingPolicy::ElementDataType());
			DrawingPolicy.DrawMesh(Context,Mesh);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

UBOOL FSeeThroughDrawingPolicyFactory::IsMaterialIgnored(const FMaterialInstance* MaterialInstance)
{
	return IsTranslucentBlendMode(MaterialInstance->GetMaterial()->GetBlendMode());
}

UBOOL FSeeThroughDrawingPolicyFactory::DrawStaticMesh( FCommandContextRHI* Context, const FSceneView* View, const FStaticMesh* Mesh, UBOOL bDrawnShared )
{
	// Only draw opaque materials in the depth pass.
	if(Mesh->MaterialInstance->GetMaterial()->GetBlendMode() == BLEND_Opaque)
	{
		if (!(Mesh->PrimitiveSceneInfo && (GSeeThroughGroupMask & (1 << Mesh->PrimitiveSceneInfo->SeeThroughGroupIndex ))))
			return FALSE;

		UBOOL bBackFace = FALSE;
		FMeshElement MeshElement( *Mesh );

		TouchPrimitiveSceneInfo( Mesh->PrimitiveSceneInfo, View->Family->CurrentWorldTime );

		if (Mesh->VertexFactory->SupportsPositionOnlyStream())
		{
			FPositionOnlyDepthDrawingPolicy DrawingPolicy(Mesh->VertexFactory,GEngine->DefaultMaterial->GetInstanceInterface(FALSE));

			if (!Mesh->bDrawnShared)
			{
				DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh->GetDynamicVertexStride()));
			}
			DrawingPolicy.SetMeshRenderState(Context,Mesh->PrimitiveSceneInfo, MeshElement,bBackFace,FMeshDrawingPolicy::ElementDataType());
			DrawingPolicy.DrawMesh(Context,MeshElement);
		}
		else 
		{
			FSeeThroughDrawingPolicy DrawingPolicy(Mesh->VertexFactory,GEngine->DefaultMaterial->GetInstanceInterface(FALSE));

			if (!Mesh->bDrawnShared)
			{
				DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh->GetDynamicVertexStride()));
			}
			DrawingPolicy.SetMeshRenderState(Context,Mesh->PrimitiveSceneInfo, MeshElement,bBackFace,FMeshDrawingPolicy::ElementDataType());
			DrawingPolicy.DrawMesh(Context,MeshElement);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

UBOOL FSceneRenderer::RenderSeeThrough(UINT DPGIndex)
{
	if (DPGIndex != SDPG_World)
		return FALSE;	

	UBOOL bDirty=0;	

	// Draw the scene's emissive and light-map color.
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);
		SCOPE_CYCLE_COUNTER(STAT_EmissiveDrawTime);

		FViewInfo& View = Views(ViewIndex);

		if (View.SeeThroughGroupMask == 0)
			continue;

		// 없으면 안한다
		if (View.NumVisibleSeeThroughDynamicPrimitives == 0 &&
			View.NumVisibleSeeThroughDynamicPrimitives == 0)
			continue;

		UBOOL bViewDirty=0;	

		// 늘 Z pass, Z write 하지 ㅇ낳음
		RHISetDepthState(GlobalContext,TStaticDepthState<FALSE,CF_Always>::GetRHI());

		// Color write 하지 않음
		RHISetColorWriteEnable(GlobalContext,FALSE);

		// Stencil은 +1하도록
		RHISetStencilState(GlobalContext,TStaticStencilState<
			TRUE,CF_Always,SO_Keep,SO_Keep,SO_Increment,
			FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
			0xff,0xff,1
		>::GetRHI());

		const FLOAT DeltaXYs[][2] = {{-1,0},{+1,0},{0,-1},{0,+1},{0,0}};
		static FLOAT PixelSize = 2.0f;

		RHISetViewport(
			GlobalContext,
			View.RenderTargetX,
			View.RenderTargetY,
			0.0f,
			View.RenderTargetX + View.RenderTargetSizeX,
			View.RenderTargetY + View.RenderTargetSizeY,
			1.0f);

		GSeeThroughGroupMask = View.SeeThroughGroupMask;
		GSilhouetteGroupFlags = 0;
		GIsSubtracting = FALSE;

		for (INT DeltaIndex=0;DeltaIndex<ARRAY_COUNT(DeltaXYs);++DeltaIndex)
		{		
			FLOAT OffsetX = DeltaXYs[DeltaIndex][0] * PixelSize;
			FLOAT OffsetY = DeltaXYs[DeltaIndex][1] * PixelSize;

			// 마지막에 가운데를 파냄
			if (DeltaIndex == ARRAY_COUNT(DeltaXYs) - 1)
			{
				GIsSubtracting = TRUE;
				RHISetDepthState(GlobalContext,TStaticDepthState<FALSE,CF_GreaterEqual>::GetRHI());				
			}

			FTranslationMatrix OffsetMatrix(
				FVector( 
				OffsetX * 2.0f / View.RenderTargetSizeX,
				OffsetY * 2.0f / View.RenderTargetSizeY,
				0 ) );

			FMatrix SavedProjectionMatrix = View.ProjectionMatrix;
			FMatrix SavedViewProjectionMatrix = View.ViewProjectionMatrix;

			View.ViewProjectionMatrix *= OffsetMatrix;
			View.ProjectionMatrix *= OffsetMatrix;

			RHISetViewParameters( GlobalContext, &View, View.ViewProjectionMatrix, View.ViewOrigin );

			if (GUseTilingCode)
			{
				RHIMSAAFixViewport();
			}

			FSeeThroughDrawingPolicyFactory PolicyFactory;
			bViewDirty |= Scene->DPGs[DPGIndex].EmissiveNoLightMapDrawList.DrawVisible(GlobalContext,&View,View.SeeThroughStaticMeshVisibilityMap,PolicyFactory);			

			// Draw the dynamic non-occluded primitives using an emissive drawing policy.
			TDynamicPrimitiveDrawer<FSeeThroughDrawingPolicyFactory> Drawer(GlobalContext,&View,DPGIndex,FSeeThroughDrawingPolicyFactory::ContextType(),TRUE);
			for(INT PrimitiveIndex = 0;PrimitiveIndex < View.SeeThroughVisibleDynamicPrimitives.Num();PrimitiveIndex++)
			{
				const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.SeeThroughVisibleDynamicPrimitives(PrimitiveIndex);
				const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

				const UBOOL bRelevantDPG = PrimitiveViewRelevance.GetDPG(DPGIndex) != 0;

				// Only draw the primitive if it's visible and relevant in the current DPG
				if(bRelevantDPG)
				{
					Drawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawDynamicElements(
						&Drawer,
						&View,
						DPGIndex
						);
				}
			}

			View.ProjectionMatrix = SavedProjectionMatrix;
			View.ViewProjectionMatrix = SavedViewProjectionMatrix;


			bViewDirty |= Drawer.IsDirty();
		}

		extern void DrawSilhouette( FCommandContextRHI*, const FSceneView&, const FLinearColor& );

		if (bViewDirty)
		{
			bDirty = TRUE;

			RHISetRasterizerState(GlobalContext,TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
			RHISetDepthState(GlobalContext,TStaticDepthState<FALSE,CF_Always>::GetRHI());	
			RHISetColorWriteEnable(GlobalContext,TRUE);

			//RHISetBlendState(GlobalContext, TStaticBlendState<>::GetRHI());
			RHISetBlendState(GlobalContext, TStaticBlendState<BO_Add,BF_SourceAlpha,BF_One>::GetRHI());	
			RHISetStencilState( GlobalContext, TStaticStencilState<TRUE,CF_NotEqual,SO_Keep,SO_Keep,SO_Keep,FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,0x3,0xff,0>::GetRHI());
			DrawSilhouette( GlobalContext, View, View.SeeThroughSilhouetteColor );

			//RHISetBlendState(GlobalContext, TStaticBlendState<BO_Add,BF_SourceAlpha,BF_One>::GetRHI());	

			if (GSilhouetteGroupFlags & (1 << 1))
			{			
				if (View.SeeThroughGroupColors.Num() >= 1 && View.SeeThroughGroupColors(0).A >= 0)
				{
					RHISetStencilState(GlobalContext, TStaticStencilState<TRUE,CF_Equal,SO_Keep,SO_Keep,SO_Keep,FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,0xff,0xff,4>::GetRHI());
					DrawSilhouette( GlobalContext, View, View.SeeThroughGroupColors(0) );
				}				
			}

			if (GSilhouetteGroupFlags & (1 << 2))
			{
				if (View.SeeThroughGroupColors.Num() >= 2 && View.SeeThroughGroupColors(1).A >= 0 )
				{
					RHISetStencilState(GlobalContext, TStaticStencilState<TRUE,CF_Equal,SO_Keep,SO_Keep,SO_Keep,FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,0xff,0xff,8>::GetRHI());
					DrawSilhouette( GlobalContext, View, View.SeeThroughGroupColors(1) );
				}
			}			

			RHISetRasterizerState(GlobalContext,TStaticRasterizerState<FM_Solid,CM_CW>::GetRHI());

			// Clear the stencil buffer to 0.
			RHIClear(GlobalContext,FALSE,FColor(0,0,0),FALSE,0,TRUE,0);	
		}	
	}		

	// Reset the stencil state.
	RHISetStencilState(GlobalContext,TStaticStencilState<>::GetRHI());	

	RHISetDepthState(GlobalContext,TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
	RHISetColorWriteEnable(GlobalContext,TRUE);		

	return bDirty;
}