/*=========================================================================================
	CheckNormalmapRenering.cpp : Implementatino for rendering lighting normal map only
	Copyrighr 2007 Redduck, inc. All Rights Reserved.
==========================================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "BSPDynamicBatch.h"

INT GBumpState = 0;

// Emissive Rendering
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TBumpOnlyVertexShader<FNoLightMapPolicy>,		TEXT("CheckNormalmapShader"), TEXT("MainVertexShader"), SF_Vertex, 0, VER_AVA_ADD_BUMPONLY_VIEWMODE );
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TBumpOnlyVertexShader<FVertexLightMapPolicy>,	TEXT("CheckNormalmapShader"), TEXT("MainVertexShader"), SF_Vertex, 0, VER_AVA_ADD_BUMPONLY_VIEWMODE );
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TBumpOnlyVertexShader<FLightMapTexturePolicy>,TEXT("CheckNormalmapShader"), TEXT("MainVertexShader"), SF_Vertex, 0, VER_AVA_ADD_BUMPONLY_VIEWMODE );
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TBumpOnlyPixelShader<FNoLightMapPolicy>,			TEXT("CheckNormalmapShader"), TEXT("MainPixelShader"), SF_Pixel, 0, VER_AVA_ADD_BUMPONLY_VIEWMODE);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TBumpOnlyPixelShader<FVertexLightMapPolicy>,		TEXT("CheckNormalmapShader"), TEXT("MainPixelShader"), SF_Pixel, 0, VER_AVA_ADD_BUMPONLY_VIEWMODE);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TBumpOnlyPixelShader<FLightMapTexturePolicy>,	TEXT("CheckNormalmapShader"), TEXT("MainPixelShader"), SF_Pixel, 0, VER_AVA_ADD_BUMPONLY_VIEWMODE);

UBOOL FBumpOnlyDrawingPolicyFactory::DrawDynamicMesh(
	FCommandContextRHI *Context,
	const FViewInfo *View,
	ContextType DrawingContext,
	const FMeshElement &Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo *PrimitiveSceneInfo,
	FHitProxyId HitProxyId)
{
	if( !Mesh.IsTranslucent()
		&& !Mesh.IsDistortion()
		&& Mesh.MaterialInstance->GetMaterial()->GetLightingModel() != MLM_Unlit )
	{
		// Check for a cached light-map
		const FLightMap *LightMap = Mesh.LCI ? Mesh.LCI->GetLightMap() : NULL;
		if( LightMap && !LightMap->IsValid() )
		{
			LightMap = NULL;
		}
		if( LightMap )
		{
			const FLightMap1D *LightMap1D = LightMap->GetLightMap1D();
			const FLightMap2D *LightMap2D = LightMap->GetLightMap2D();
			if( LightMap1D )
			{
				// Draw the mesh using the vertex lightmap.
				TBumpOnlyDrawingPolicy<FVertexLightMapPolicy> DrawingPolicy(
					Mesh.VertexFactory,
					Mesh.MaterialInstance,
					FVertexLightMapPolicy());
				DrawingPolicy.DrawShared(Context, View, DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
				DrawingPolicy.SetMeshRenderState( Context, PrimitiveSceneInfo, Mesh, bBackFace, (FLightMap1D*)LightMap);
				DrawingPolicy.DrawMesh(Context, Mesh);
			}
			else if( LightMap2D )
			{
				// Draw the mesh using the lightmap texture.
				const FLightMap2D *LightMap2D = (FLightMap2D*) LightMap;
				TBumpOnlyDrawingPolicy<FLightMapTexturePolicy> DrawingPolicy(
					Mesh.VertexFactory,
					Mesh.MaterialInstance,
					FLightMapTexturePolicy(LightMap2D));
				DrawingPolicy.DrawShared(Context, View, DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
				DrawingPolicy.SetMeshRenderState( Context, PrimitiveSceneInfo, Mesh, bBackFace, LightMap2D );
				DrawingPolicy.DrawMesh(Context, Mesh);
			}
		}
		else
		{
			TBumpOnlyDrawingPolicy<FNoLightMapPolicy> DrawingPolicy(
				Mesh.VertexFactory,
				Mesh.MaterialInstance,
				FNoLightMapPolicy());
			DrawingPolicy.DrawShared( Context, View, DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
			DrawingPolicy.SetMeshRenderState( Context, PrimitiveSceneInfo, Mesh, bBackFace, FNoLightMapPolicy::ElementDataType());
			DrawingPolicy.DrawMesh(Context, Mesh);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

UBOOL FBumpOnlyDrawingPolicyFactory::DrawStaticMesh(
	FCommandContextRHI *Context,
	const FSceneView *View,
	const FStaticMesh *StaticMesh,
	UBOOL bDrawShared )
{
	UBOOL bBackFace = FALSE;
	FMeshElement Mesh( *StaticMesh );

	if( !Mesh.IsTranslucent()
		&& !Mesh.IsDistortion()
		&& Mesh.MaterialInstance->GetMaterial()->GetLightingModel() != MLM_Unlit )
	{
		// Check for a cached light-map
		const FLightMap *LightMap = Mesh.LCI ? Mesh.LCI->GetLightMap() : NULL;
		if( LightMap && !LightMap->IsValid() )
		{
			LightMap = NULL;
		}
		if( LightMap )
		{
			const FLightMap1D *LightMap1D = LightMap->GetLightMap1D();
			const FLightMap2D *LightMap2D = LightMap->GetLightMap2D();
			if( LightMap1D )
			{
				// Draw the mesh using the vertex lightmap.
				TBumpOnlyDrawingPolicy<FVertexLightMapPolicy> DrawingPolicy(
					Mesh.VertexFactory,
					Mesh.MaterialInstance,
					FVertexLightMapPolicy());
				DrawingPolicy.DrawShared(Context, View, DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
				DrawingPolicy.SetMeshRenderState( Context, StaticMesh->PrimitiveSceneInfo, Mesh, bBackFace, (FLightMap1D*)LightMap);
				DrawingPolicy.DrawMesh(Context, Mesh);
			}
			else if( LightMap2D )
			{
				// Draw the mesh using the lightmap texture.
				const FLightMap2D *LightMap2D = (FLightMap2D*) LightMap;
				TBumpOnlyDrawingPolicy<FLightMapTexturePolicy> DrawingPolicy(
					Mesh.VertexFactory,
					Mesh.MaterialInstance,
					FLightMapTexturePolicy(LightMap2D));
				DrawingPolicy.DrawShared(Context, View, DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
				DrawingPolicy.SetMeshRenderState( Context, StaticMesh->PrimitiveSceneInfo, Mesh, bBackFace, LightMap2D );
				DrawingPolicy.DrawMesh(Context, Mesh);
			}
		}
		else
		{
			TBumpOnlyDrawingPolicy<FNoLightMapPolicy> DrawingPolicy(
				Mesh.VertexFactory,
				Mesh.MaterialInstance,
				FNoLightMapPolicy());
			DrawingPolicy.DrawShared( Context, View, DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
			DrawingPolicy.SetMeshRenderState( Context, StaticMesh->PrimitiveSceneInfo, Mesh, bBackFace, FNoLightMapPolicy::ElementDataType());
			DrawingPolicy.DrawMesh(Context, Mesh);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}

// Renders bump lighting
UBOOL FSceneRenderer::RenderBumpLighting(UINT DPGIndex)
{
	SCOPED_DRAW_EVENT(EventRenderBumpLighting)(DEC_SCENE_ITEMS, TEXT("RenderBumpLighting"));

	UBOOL bWorldDpg = (DPGIndex == SDPG_World);
	UBOOL bDirty = FALSE;

	// Opaque blending, enable depth tests and writes.
	RHISetBlendState( GlobalContext, TStaticBlendState<>::GetRHI());
	RHISetDepthState( GlobalContext, TStaticDepthState<TRUE, CF_LessEqual>::GetRHI());

	FBumpOnlyDrawingPolicyFactory PolicyFactory;
	for( INT ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex )
	{
		SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS, TEXT("View%d"), ViewIndex);

		FViewInfo &View = Views(ViewIndex);
		const FSceneViewState *ViewState = (FSceneViewState*)View.State;

		// Set the device viewport for the view.
		RHISetViewport(GlobalContext,
			View.RenderTargetX,								View.RenderTargetY,								0.0f,
			View.RenderTargetX + View.RenderTargetSizeX,	View.RenderTargetY + View.RenderTargetSizeY,	1.0f );

		if(GUseTilingCode && bWorldDpg)
		{
			RHIMSAAFixViewport();
		}
		// draw bump lighting for static meshes.
		bDirty |= Scene->DPGs[DPGIndex].EmissiveNoLightMapDrawList.DrawVisible(GlobalContext, &View, View.StaticMeshVisibilityMap, PolicyFactory );
		bDirty |= Scene->DPGs[DPGIndex].EmissiveVertexLightMapDrawList.DrawVisible(GlobalContext, &View, View.StaticMeshVisibilityMap, PolicyFactory );
		bDirty |= Scene->DPGs[DPGIndex].EmissiveLightMapTextureDrawList.DrawVisible(GlobalContext, &View, View.StaticMeshVisibilityMap, PolicyFactory );

		extern void ParticleRendering_StartBatch( UBOOL bTranslucentPass );
		extern void ParticleRendering_EndBatch( FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex );

		BspRendering_StartBatch(BSP_EMISSIVE);
		ParticleRendering_StartBatch(FALSE);

		// Draw bump lighting for dynamic meshes.
		TDynamicPrimitiveDrawer<FBumpOnlyDrawingPolicyFactory> Drawer(GlobalContext,&View,DPGIndex,FBumpOnlyDrawingPolicyFactory::ContextType(),TRUE);
		for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
		{
			const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);
			const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

			const UBOOL bVisible = View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id);
			if( bVisible && PrimitiveViewRelevance.GetDPG(DPGIndex) )
			{
				Drawer.SetPrimitive(PrimitiveSceneInfo);
				PrimitiveSceneInfo->Proxy->DrawDynamicElements( &Drawer, &View, DPGIndex );
			}
		}

		BspRendering_EndBatch( &Drawer );
		ParticleRendering_EndBatch(&Drawer, &View, DPGIndex);

		bDirty |= Drawer.IsDirty();
	}
	return bDirty;
}