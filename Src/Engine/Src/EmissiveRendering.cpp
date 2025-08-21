/*=============================================================================
	EmissiveRendering.cpp: Emissive rendering implementation.
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

//<@ ava specific ; 2007. 1. 18  changmin -> licensee version up~ 
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TEmissiveVertexShader<FNoLightMapPolicy>,TEXT("EmissiveVertexShader"),TEXT("Main"),SF_Vertex,0,VER_AVA_SHADER_TRANSFORM_OPTIMIZED);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TEmissiveVertexShader<FVertexLightMapPolicy>,TEXT("EmissiveVertexShader"),TEXT("Main"),SF_Vertex,VER_VERTEX_LIGHTMAP_GAMMACORRECTION_FIX, VER_AVA_SHADER_TRANSFORM_OPTIMIZED);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TEmissiveVertexShader<FLightMapTexturePolicy>,TEXT("EmissiveVertexShader"),TEXT("Main"),SF_Vertex,0,VER_AVA_SHADER_TRANSFORM_OPTIMIZED);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TEmissivePixelShader<FNoLightMapPolicy>,TEXT("EmissivePixelShader"),TEXT("Main"),SF_Pixel,VER_SKYLIGHT_LOWERHEMISPHERE_SHADER_RECOMPILE,VER_AVA_SHADER_TRANSFORM_OPTIMIZED);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TEmissivePixelShader<FVertexLightMapPolicy>,TEXT("EmissivePixelShader"),TEXT("Main"),SF_Pixel,VER_SKYLIGHT_LOWERHEMISPHERE_SHADER_RECOMPILE,VER_AVA_SHADER_EMISSIVE_PATH_OPTIMIZED);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TEmissivePixelShader<FLightMapTexturePolicy>,TEXT("EmissivePixelShader"),TEXT("Main"),SF_Pixel,VER_SKYLIGHT_LOWERHEMISPHERE_SHADER_RECOMPILE,VER_AVA_SHADER_EMISSIVE_PATH_OPTIMIZED);
IMPLEMENT_MATERIAL_SHADER_TYPE(,TUnlitPixelShader,TEXT("UnlitPixelShader"),TEXT("Main"),SF_Pixel,VER_SKYLIGHT_LOWERHEMISPHERE_SHADER_RECOMPILE,VER_AVA_SHADER_TRANSFORM_OPTIMIZED);

void FEmissiveDrawingPolicyFactory::AddStaticMesh(FScene* Scene,FStaticMesh* StaticMesh,ContextType )
{
#if !FINAL_RELEASE
	FUnlitDrawingPolicyFactory::AddStaticMesh( Scene, StaticMesh );	
#endif

	if(!StaticMesh->IsTranslucent() && !StaticMesh->IsDistortion())
	{
		//<@ ava specific ; 2006. 9. 27 changmin
		if( StaticMesh->bRenderMesh )
		//<@ ava
		{
			// Check for a cached light-map.
			const FLightMap* LightMap = StaticMesh->LCI ? StaticMesh->LCI->GetLightMap() : NULL;
			if(LightMap && (!LightMap->IsValid() || StaticMesh->MaterialInstance->GetMaterial()->GetLightingModel() == MLM_Unlit))
			{
				LightMap = NULL;
			}

			if(LightMap)
			{
				const FLightMap1D* LightMap1D = LightMap->GetLightMap1D();
				const FLightMap2D* LightMap2D = LightMap->GetLightMap2D();
				if(LightMap1D)
				{
					// Add the static mesh to the vertex light-map emissive draw list.
					Scene->DPGs[StaticMesh->DepthPriorityGroup].EmissiveVertexLightMapDrawList.AddMesh(
						StaticMesh,
						TEmissiveDrawingPolicy<FVertexLightMapPolicy>::ElementDataType(LightMap1D),
						TEmissiveDrawingPolicy<FVertexLightMapPolicy>(
						StaticMesh->VertexFactory,
						StaticMesh->MaterialInstance,
						FVertexLightMapPolicy()
						)
						);
				}
				else if(LightMap2D)
				{
					// Add the static mesh to the light-map texture emissive draw list.
					Scene->DPGs[StaticMesh->DepthPriorityGroup].EmissiveLightMapTextureDrawList.AddMesh(
						StaticMesh,
						TEmissiveDrawingPolicy<FLightMapTexturePolicy>::ElementDataType(LightMap2D),
						TEmissiveDrawingPolicy<FLightMapTexturePolicy>(
						StaticMesh->VertexFactory,
						StaticMesh->MaterialInstance,
						FLightMapTexturePolicy(LightMap2D)
						)
						);
				}
			}
			else
			{
				// Add the static mesh to the no light-map emissive draw list.
				Scene->DPGs[StaticMesh->DepthPriorityGroup].EmissiveNoLightMapDrawList.AddMesh(
					StaticMesh,
					TEmissiveDrawingPolicy<FNoLightMapPolicy>::ElementDataType(),
					TEmissiveDrawingPolicy<FNoLightMapPolicy>(
					StaticMesh->VertexFactory,
					StaticMesh->MaterialInstance,
					FNoLightMapPolicy()
					)
					);
			}
		}
	}
}

UBOOL FEmissiveDrawingPolicyFactory::DrawDynamicMesh(
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
#if !FINAL_RELEASE
	if ((View->Family->ShowFlags & SHOW_Lighting) == 0)
	{
		return FUnlitDrawingPolicyFactory::DrawDynamicMesh( Context, View, FUnlitDrawingPolicyFactory::ContextType(), Mesh, bBackFace, bPreFog, PrimitiveSceneInfo, HitProxyId );
	}
#endif

	if(!Mesh.IsTranslucent() && !Mesh.IsDistortion())
	{
		const UBOOL bUnlit =	Mesh.MaterialInstance->GetMaterial()->GetLightingModel() == MLM_Unlit ||
								!(View->Family->ShowFlags & SHOW_Lighting);

		// Check for a cached light-map.
		const FLightMap* LightMap = !bUnlit && Mesh.LCI ? Mesh.LCI->GetLightMap() : NULL;
		if(LightMap && !LightMap->IsValid())
		{
			LightMap = NULL;
		}

		if(LightMap)
		{
			const FLightMap1D* LightMap1D = LightMap->GetLightMap1D();
			const FLightMap2D* LightMap2D = LightMap->GetLightMap2D();
			if(LightMap1D)
			{
				// Draw the mesh using the vertex light-map.
				TEmissiveDrawingPolicy<FVertexLightMapPolicy> DrawingPolicy(
					Mesh.VertexFactory,
					Mesh.MaterialInstance,
					FVertexLightMapPolicy()
					);

				if (!Mesh.bDrawnShared)
				{
					DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
				}
				DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,(FLightMap1D*)LightMap);
				DrawingPolicy.DrawMesh(Context,Mesh);
			}
			else if(LightMap2D)
			{
				// Draw the mesh using the light-map texture.
				const FLightMap2D* LightMap2D = (FLightMap2D*)LightMap;
				TEmissiveDrawingPolicy<FLightMapTexturePolicy> DrawingPolicy(
					Mesh.VertexFactory,
					Mesh.MaterialInstance,
					FLightMapTexturePolicy(LightMap2D)
					);

				if (!Mesh.bDrawnShared)
				{
					DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
				}
				DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,LightMap2D);
				DrawingPolicy.DrawMesh(Context,Mesh);
			}
		}
		else
		{
			// Draw the mesh using no light-map.
			TEmissiveDrawingPolicy<FNoLightMapPolicy> DrawingPolicy(
				Mesh.VertexFactory,
				Mesh.MaterialInstance,
				FNoLightMapPolicy()
				);

			if (!Mesh.bDrawnShared)
			{
				DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
			}
			DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FNoLightMapPolicy::ElementDataType());
			DrawingPolicy.DrawMesh(Context,Mesh);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

#if !FINAL_RELEASE
void FUnlitDrawingPolicyFactory::AddStaticMesh(FScene* Scene,FStaticMesh* StaticMesh,ContextType)
{
	if(!StaticMesh->IsTranslucent() && !StaticMesh->IsDistortion())
	{		
		if( StaticMesh->bRenderMesh )			
		{			
			// Add the static mesh to the no light-map Unlit draw list.
			Scene->DPGs[StaticMesh->DepthPriorityGroup].UnlitDrawList.AddMesh(
				StaticMesh,
				TUnlitDrawingPolicy::ElementDataType(),
				TUnlitDrawingPolicy(
				StaticMesh->VertexFactory,
				StaticMesh->MaterialInstance,
				FNoLightMapPolicy()
				)
				);			
		}
	}
}

UBOOL FUnlitDrawingPolicyFactory::DrawDynamicMesh(
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
	if(!Mesh.IsTranslucent() && !Mesh.IsDistortion())
	{		
		// Draw the mesh using no light-map.
		TUnlitDrawingPolicy DrawingPolicy(
			Mesh.VertexFactory,
			Mesh.MaterialInstance,
			FNoLightMapPolicy()
			);
		
		if (!Mesh.bDrawnShared)
		{
			DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
		}
		DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FNoLightMapPolicy::ElementDataType());
		DrawingPolicy.DrawMesh(Context,Mesh);
		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
#endif