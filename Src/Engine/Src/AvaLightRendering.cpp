/*======================================================
	AvaLightRendering.cpp: Light rendering implementation.
	Copyright 2007 Redduck, Inc. All Rights Reserved.
============================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "BspDynamicBatch.h"
#include "EnvCubePrivate.h"

UBOOL FSceneRenderer::Ava_RenderLightsUseCascadedShadow( UINT DPGIndex )
{
	UBOOL bSceneColorDirty = FALSE;

	// Draw each light.
	for( TSparseArray<FLightSceneInfo*>::TConstIterator LightIt(Scene->Lights); LightIt ; ++LightIt)
	{
		const FLightSceneInfo *LightSceneInfo = *LightIt;
		if( LightSceneInfo->bUseCascadedShadowmap )
		{
			UBOOL bUseAttenuationBuffer = FALSE;
			UBOOL bUseStencilBuffer = FALSE;
			UBOOL bSavedRawSceneColor = FALSE;

			SCOPED_DRAW_EVENT(EventLightPass)(DEC_SCENE_ITEMS, TEXT("Light Pass %s"), LightSceneInfo->GetLightName());

			const UBOOL bDrawShadows = (ViewFamily.ShowFlags & SHOW_DynamicShadows)
										&& !GIsLowEndHW
										&& (DPGIndex == SDPG_World || DPGIndex == SDPG_Foreground );
			if( bDrawShadows)
			{
				bUseAttenuationBuffer = TRUE;
				bUseStencilBuffer = FALSE;
			}

			if( bUseAttenuationBuffer )
			{
				if( bSceneColorDirty )
				{
					GSceneRenderTargets.SaveSceneColorRaw();
					bSavedRawSceneColor = TRUE;
				}
				// Clear the light attenuation surface to white.
				GSceneRenderTargets.BeginRenderingLightAttenuation();
				RHIClear( GlobalContext, TRUE, FLinearColor::White, FALSE, 0, FALSE, 0 );

				if( bDrawShadows )
				{
					// 현재 임시 방편임... Cascaded light 는 하나라는 가정이 있다..
					Ava_RenderCascadedShadows( LightSceneInfo, (DPGIndex == SDPG_World) );
				}
			}

			// Resove light attenuation buffer
			GSceneRenderTargets.FinishRenderingLightAttenuation();
			GSceneRenderTargets.SetLightAttenuationMode(bUseAttenuationBuffer);

			// Rendering the light pass to the scene color buffer.

			if( bSavedRawSceneColor )
			{
				GSceneRenderTargets.RestoreSceneColorRaw();
				GSceneRenderTargets.BeginRenderingSceneColor(FALSE);
			}
			else
			{
				GSceneRenderTargets.BeginRenderingSceneColor(bUseAttenuationBuffer);
			}

			{
				SCOPE_CYCLE_COUNTER(STAT_AVA_CascadedShadow_Lighting);
				// Render the light to the scene color buffer, conditionally using the attenuation buffer or a 1x1 white texture as input 
				bSceneColorDirty |= RenderLight( LightSceneInfo, DPGIndex );
			}

			//<@ ava specific ; 2007. 5. 13 changmin
			// DepthdrawingLight는 Blend가 필요없고, 따라서 Resolve가 필요 없습니다.
			extern UBOOL GDepthDrawingLightOnly;
			const UBOOL bResolveSceneColor = !(GSupportsFPBlending || GIsLowEndHW ) && !GDepthDrawingLightOnly && bSceneColorDirty;
			GSceneRenderTargets.FinishRenderingSceneColor( bResolveSceneColor );
			//>@ ava
		}
	}

	// Restore the default mode
	GSceneRenderTargets.SetLightAttenuationMode(TRUE);
	return bSceneColorDirty;
}


UBOOL FSceneRenderer::AVA_RenderLitDecals(UINT DPGIndex)
{
	for( INT ViewIndex =0; ViewIndex < Views.Num(); ++ViewIndex )
	{
		FViewInfo& View = Views(ViewIndex);

		// Opaque blending, depth tests and writes.
		// Additive blending, depth tests, no depth writes.
		RHISetBlendState(GlobalContext,TStaticBlendState<BO_Add,BF_One,BF_One,BO_Add,BF_Zero,BF_One>::GetRHI());
		RHISetDepthState(GlobalContext,TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
		
		// render emissive lit decals
		//TDynamicPrimitiveDrawer<FEmissiveDrawingPolicyFactory> Drawer(GlobalContext,&View,DPGIndex,FEmissiveDrawingPolicyFactory::ContextType(),TRUE);
		//for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleLitDecalPrimitives.Num();PrimitiveIndex++)
		//{
		//	const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleLitDecalPrimitives(PrimitiveIndex);
		//	const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

		//	const UBOOL bVisible = View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id);
		//	const UBOOL bRelevantDPG = PrimitiveViewRelevance.GetDPG(DPGIndex) != 0;

		//	// TRUE if rendering decals modified the depth state.
		//	UBOOL bDecalDepthStateIsSet = FALSE;
		//	// Only draw decals if the primitive if it's visible and relevant in the current DPG
		//	if(bVisible && bRelevantDPG)
		//	{
		//		Drawer.SetPrimitive(PrimitiveSceneInfo);
		//		PrimitiveSceneInfo->Proxy->DrawLitDecalElements(
		//			GlobalContext,
		//			&Drawer,
		//			&View,
		//			DPGIndex,
		//			FALSE
		//			);
		//	}
		//}

		// render dynamic lit decals
		// Draw each light.

		for(TSparseArray<FLightSceneInfo*>::TConstIterator LightIt(Scene->Lights);LightIt;++LightIt)
		{
			const FLightSceneInfo* LightSceneInfo = *LightIt;
			const INT LightId = LightIt.GetIndex();

			// shadowing이 필요해서, lighting때 그린다. 다른 light의 decal의 shadowing이 없다는 가정이 있다.
			if ( LightSceneInfo->bUseCascadedShadowmap )
			{
				continue;
			}

			const FVisibleLightInfo& VisibleLightInfo = View.VisibleLightInfos(LightSceneInfo->Id);

			// Check if the light is visible in any of the views.
			UBOOL bDecalIsVisibleInAnyView = FALSE;
			for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
			{
				FViewInfo& View = Views(ViewIndex);
				if( View.VisibleLightInfos(LightId).DPGInfo[DPGIndex].VisibleLitDecalPrimitives.Num() > 0 )
				{
					bDecalIsVisibleInAnyView = TRUE;
					break;
				}
			}

			if(bDecalIsVisibleInAnyView)
			{
				UBOOL SceneColorDirty = FALSE;
				GSceneRenderTargets.BeginRenderingSceneColor(TRUE);

				TDynamicPrimitiveDrawer<FMeshLightingDrawingPolicyFactory> Drawer(GlobalContext,&View,DPGIndex,LightSceneInfo,TRUE);

				if( DPGIndex == SDPG_World )
				{
					// Render decals for visible primitives affected by this light.
					for(INT PrimitiveIndex = 0;PrimitiveIndex < VisibleLightInfo.DPGInfo[DPGIndex].VisibleLitDecalPrimitives.Num();PrimitiveIndex++)
					{
						const FPrimitiveSceneInfo* PrimitiveSceneInfo = VisibleLightInfo.DPGInfo[DPGIndex].VisibleLitDecalPrimitives(PrimitiveIndex);
						if(View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id))
						{
							Drawer.SetPrimitive(PrimitiveSceneInfo);
							PrimitiveSceneInfo->Proxy->DrawLitDecalElements(GlobalContext,&Drawer,&View,DPGIndex,TRUE);
						}
					}
					SceneColorDirty |= Drawer.IsDirty();

				}
				GSceneRenderTargets.FinishRenderingSceneColor( SceneColorDirty );
			}
		}

		// restore depth / blend state
		RHISetBlendState(GlobalContext,TStaticBlendState<>::GetRHI());
		RHISetDepthState(GlobalContext,TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());
	}

	return TRUE;

}