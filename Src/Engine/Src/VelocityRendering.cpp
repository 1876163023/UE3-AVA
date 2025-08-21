/*=============================================================================
	VelocityRendering.cpp: Velocity rendering implementation.
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "BSPDynamicBatch.h"

//=============================================================================
/** Encapsulates the Velocity vertex shader. */
class FVelocityVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FVelocityVertexShader,MeshMaterial);
public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only compile the velocity shaders for the default material or if it's masked.  Also exclude decals.
		return (Material->IsSpecialEngineMaterial() || Material->IsMasked() || Material->IsTwoSided()) && !Material->IsDecalMaterial();
	}

	FVelocityVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer),
		VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{		
		PrevViewProjectionMatrixParameter.Bind(Initializer.ParameterMap,TEXT("PrevViewProjectionMatrix"));
		PreviousLocalToWorldParameter.Bind(Initializer.ParameterMap,TEXT("PreviousLocalToWorld"),TRUE);
	}
	FVelocityVertexShader() {}
	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		Ar << VertexFactoryParameters << PrevViewProjectionMatrixParameter << PreviousLocalToWorldParameter;
	}
	void SetParameters(FCommandContextRHI* Context,const FVertexFactory* VertexFactory,const FMaterialInstance* MaterialInstance,const FViewInfo* View)
	{
		FSceneViewState* ViewState = (FSceneViewState*) View->State;

		VertexFactoryParameters.Set( Context, this, VertexFactory, View );		

		// Set the view-projection matrix from the previous frame.
		SetVertexShaderValue( Context, GetVertexShader(), PrevViewProjectionMatrixParameter, View->PrevViewProjMatrix );
	}

	void SetLocalTransforms(FCommandContextRHI* Context,const FMatrix& LocalToWorld,const FMatrix& WorldToLocal,const FMatrix& PreviousLocalToWorld)
	{
		VertexFactoryParameters.SetLocalTransforms(Context,this,LocalToWorld,WorldToLocal);
		SetVertexShaderValue(Context, GetVertexShader(), PreviousLocalToWorldParameter, PreviousLocalToWorld);
	}

	UBOOL SupportsVelocity( ) const
	{
		return PreviousLocalToWorldParameter.IsBound();
	}

private:
	FVertexFactoryParameterRef	VertexFactoryParameters;	
	FShaderParameter			PrevViewProjectionMatrixParameter;
	FShaderParameter			PreviousLocalToWorldParameter;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FVelocityVertexShader,TEXT("VelocityShader"),TEXT("MainVertexShader"),SF_Vertex,VER_VELOCITYSHADER_RECOMPILE,0);


//=============================================================================
/** Encapsulates the Velocity pixel shader. */
class FVelocityPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FVelocityPixelShader,MeshMaterial);
public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only compile the velocity shaders for the default material or if it's masked.  Also exclude decals.
		return (Material->IsSpecialEngineMaterial() || Material->IsMasked() || Material->IsTwoSided()) && !Material->IsDecalMaterial();
	}

	FVelocityPixelShader()	: View(NULL)	{}

	FVelocityPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FShader(Initializer)
		, View(NULL)
	{
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
		VelocityScaleOffset.Bind( Initializer.ParameterMap, TEXT("VelocityScaleOffset"), TRUE );
		IndividualVelocityScale.Bind( Initializer.ParameterMap, TEXT("IndividualVelocityScale"), TRUE );
	}

	void SetParameters(FCommandContextRHI* Context,const FVertexFactory* VertexFactory,const FMaterialInstance* MaterialInstance,const FViewInfo* ViewInfo)
	{
		View = ViewInfo;

		FMaterialRenderContext MaterialRenderContext(MaterialInstance, View->Family->CurrentWorldTime, View->Family->CurrentRealTime, View);
		MaterialParameters.Set( Context, this, MaterialRenderContext );

//		SetPixelShaderValue( Context, GetPixelShader(), VelocityScaleOffset, FVector4( ScaleX*0.5f, ScaleY*0.5f, 0.5f, 0.5f ) );
	}

	void SetLocalTransforms(FCommandContextRHI* Context,const FMaterialInstance* MaterialInstance,const FMatrix& LocalToWorld,UBOOL bBackFace,FLOAT VelocityScale)
	{
		MaterialParameters.SetLocalTransforms( Context, this, MaterialInstance, LocalToWorld, bBackFace );

		// Calculate the maximum velocity (MAX_PIXELVELOCITY is per 30 fps frame).
		FSceneViewState* ViewState = (FSceneViewState*) View->State;
		const FLOAT SizeX	= View->SizeX;
		const FLOAT SizeY	= View->SizeY;
		VelocityScale		*= ViewState->MotionBlurTimeScale * View->MotionBlurParameters.VelocityScale;
		FLOAT MaxVelocity	= MAX_PIXELVELOCITY * View->MotionBlurParameters.MaxVelocity;
		FLOAT ScaleX		= ( MaxVelocity > 0.0001f ) ? 0.5f*VelocityScale/MaxVelocity : 0.0f;
		FLOAT ScaleY		= ( MaxVelocity > 0.0001f ) ? 0.5f*VelocityScale/(MaxVelocity*SizeY/SizeX) : 0.0f;

		static FLOAT MinimumValue = 0.0002f;
		static FLOAT MaximumValue = 0.9998f;

		// xy = scale values
		// zw = clamp values
		FVector4 VelocityScaleClamp( ScaleX, ScaleY, MinimumValue, MaximumValue );

		SetPixelShaderValue( Context, GetPixelShader(), IndividualVelocityScale, VelocityScaleClamp );
	}

	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		Ar << MaterialParameters << VelocityScaleOffset << IndividualVelocityScale;
	}

private:
	FMaterialPixelShaderParameters	MaterialParameters;
	FShaderParameter				VelocityScaleOffset;
	FShaderParameter				IndividualVelocityScale;

	// Temporary variable to keep between calls to SetParameters and SetLocalTransforms.
	mutable const FViewInfo *		View;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FVelocityPixelShader,TEXT("VelocityShader"),TEXT("MainPixelShader"),SF_Pixel,VER_VELOCITYSHADER_RECOMPILE,0);


//=============================================================================
/** FVelocityDrawingPolicy - Policy to wrap FMeshDrawingPolicy with new shaders. */

FVelocityDrawingPolicy::FVelocityDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialInstance* InMaterialInstance
	)
	:	FMeshDrawingPolicy(InVertexFactory,InMaterialInstance)
{
	const FMaterialShaderMap* MaterialShaderIndex = InMaterialInstance->GetMaterial()->GetShaderMap();
	const FMeshMaterialShaderMap* MeshShaderIndex = MaterialShaderIndex->GetMeshShaderMap(InVertexFactory->GetType());
	UBOOL HasVertexShader = MeshShaderIndex->HasShader(&FVelocityVertexShader::StaticType);
	UBOOL HasPixelShader = MeshShaderIndex->HasShader(&FVelocityPixelShader::StaticType);
	VertexShader = HasVertexShader ? MeshShaderIndex->GetShader<FVelocityVertexShader>() : NULL;
	PixelShader = HasPixelShader ? MeshShaderIndex->GetShader<FVelocityPixelShader>() : NULL;
}

UBOOL FVelocityDrawingPolicy::SupportsVelocity() const
{
	return (VertexShader && PixelShader) ? VertexShader->SupportsVelocity() : FALSE;
}

void FVelocityDrawingPolicy::DrawShared( FCommandContextRHI* Context,const FSceneView* SceneView, FBoundShaderStateRHIRef ShaderState ) const
{
	SCOPE_CYCLE_COUNTER(STAT_DepthDrawSharedTime);

	// NOTE: Assuming this cast is always safe!
	FViewInfo* View = (FViewInfo*) SceneView;

	// Set the depth-only shader parameters for the material.
	RHISetBoundShaderState( Context, ShaderState );
	VertexShader->SetParameters( Context, VertexFactory, MaterialInstance, View );
	PixelShader->SetParameters( Context, VertexFactory, MaterialInstance, View );

	// Set the shared mesh resources.
	FMeshDrawingPolicy::DrawShared( Context, View );
}

void FVelocityDrawingPolicy::SetMeshRenderState(
	FCommandContextRHI* Context,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const ElementDataType& ElementData
	) const
{
	const FMotionBlurInfo* MotionBlurInfo;
	if (PrimitiveSceneInfo->Scene->GetPrimitiveMotionBlurInfo(PrimitiveSceneInfo, MotionBlurInfo) == TRUE)
	{
		VertexShader->SetLocalTransforms(Context,Mesh.LocalToWorld,Mesh.WorldToLocal,MotionBlurInfo->PreviousLocalToWorld);
	}
	else
	{
		VertexShader->SetLocalTransforms(Context,Mesh.LocalToWorld,Mesh.WorldToLocal,Mesh.LocalToWorld);
	}
	PixelShader->SetLocalTransforms(Context,Mesh.MaterialInstance,Mesh.LocalToWorld,bBackFace,PrimitiveSceneInfo->Component->MotionBlurScale);
	FMeshDrawingPolicy::SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,ElementData);
}

/** 
 * Create bound shader state using the vertex decl from the mesh draw policy
 * as well as the shaders needed to draw the mesh
 * @param DynamicStride - optional stride for dynamic vertex data
 * @return new bound shader state object
 */
FBoundShaderStateRHIRef FVelocityDrawingPolicy::CreateBoundShaderState(DWORD DynamicStride)
{
	FVertexDeclarationRHIParamRef VertexDeclaration;
	DWORD StreamStrides[MaxVertexElementCount];

	FMeshDrawingPolicy::GetVertexDeclarationInfo(VertexDeclaration, StreamStrides);
	if (DynamicStride)
	{
		StreamStrides[0] = DynamicStride;
	}

	return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, VertexShader->GetVertexShader(), PixelShader->GetPixelShader());	
}

INT Compare(const FVelocityDrawingPolicy& A,const FVelocityDrawingPolicy& B)
{
	COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
	COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
	COMPAREDRAWINGPOLICYMEMBERS(MaterialInstance);
	return 0;
}


//=============================================================================
/** Policy to wrap FMeshDrawingPolicy with new shaders. */

void FVelocityDrawingPolicyFactory::AddStaticMesh(FScene* Scene,FStaticMesh* StaticMesh,ContextType)
{
	//<@ ava specific ; 2006. 9. 27 changmin
	if( StaticMesh->bRenderMesh )
	//>@ ava
	{
		const FMaterialInstance* MaterialInstance = StaticMesh->MaterialInstance;
		const FMaterial* Material = MaterialInstance->GetMaterial();
		EBlendMode BlendMode = Material->GetBlendMode();
		if ( (BlendMode == BLEND_Opaque || BlendMode == BLEND_Masked) && !Material->IsDecalMaterial() )
		{
			if ( !Material->IsMasked() && !Material->IsTwoSided() )
			{
				// Default material doesn't handle masked, and doesn't have the correct bIsTwoSided setting.
				MaterialInstance = GEngine->DefaultMaterial->GetInstanceInterface(FALSE);
			}
			// Add the static mesh to the depth-only draw list.
			Scene->DPGs[StaticMesh->DepthPriorityGroup].VelocityDrawList.AddMesh(
				StaticMesh,
				FVelocityDrawingPolicy::ElementDataType(),
				FVelocityDrawingPolicy(StaticMesh->VertexFactory,MaterialInstance)
				);
		}
	}
}

UBOOL FVelocityDrawingPolicyFactory::DrawDynamicMesh(
	FCommandContextRHI* Context,
	const FViewInfo* View,
	ContextType DrawingContext,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	// Only draw opaque materials in the depth pass.
	const FMaterialInstance* MaterialInstance = Mesh.MaterialInstance;
	const FMaterial* Material = MaterialInstance->GetMaterial();
	EBlendMode BlendMode = Material->GetBlendMode();
	if ( BlendMode == BLEND_Opaque || BlendMode == BLEND_Masked && !Material->IsDecalMaterial() )
	{
		const FMatrix* CheckMatrix = NULL;
		const FMotionBlurInfo* MBInfo;
		if (PrimitiveSceneInfo->Scene->GetPrimitiveMotionBlurInfo(PrimitiveSceneInfo, MBInfo) == TRUE)
		{
			CheckMatrix = &(MBInfo->PreviousLocalToWorld);
		}
		// Only render velocity if it has moved since last frame.
		if ( (Abs(PrimitiveSceneInfo->Component->MotionBlurScale - 1.0f) > 0.0001f) ||
			(CheckMatrix && !Mesh.LocalToWorld.Equals(*CheckMatrix, 0.0001f))
			)
		{
			if ( !Material->IsMasked() && !Material->IsTwoSided() )
			{
				// Default material doesn't handle masked, and doesn't have the correct bIsTwoSided setting.
				MaterialInstance = GEngine->DefaultMaterial->GetInstanceInterface(FALSE);
			}
			FVelocityDrawingPolicy DrawingPolicy( Mesh.VertexFactory, MaterialInstance );
			if ( DrawingPolicy.SupportsVelocity() )
			{
				if (!Mesh.bDrawnShared)
				{
					DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
				}
				DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FMeshDrawingPolicy::ElementDataType());
				DrawingPolicy.DrawMesh(Context,Mesh);
				return TRUE;
			}
		}
		return FALSE;
	}
	else
	{
		return FALSE;
	}
}

/** Renders the velocities of movable objects for the motion blur effect. */
void FSceneRenderer::RenderVelocities(UINT DPGIndex)
{
	SCOPED_DRAW_EVENT(EventVelocities)(DEC_SCENE_ITEMS,TEXT("RenderVelocities"));

	if (!IsValidRef(GSceneRenderTargets.GetVelocitySurface()))
		return;

	UBOOL bWroteVelocities = FALSE;
	GSceneRenderTargets.BeginRenderingPrePass( TRUE );
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

		FViewInfo& View = Views(ViewIndex);
		if ( View.bRequiresVelocities )
		{
			const FSceneViewState* ViewState = (FSceneViewState*)View.State;

			// Set the device viewport for the view.
			RHISetViewport(GlobalContext,View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
			RHISetViewParameters( GlobalContext, &View, View.ViewProjectionMatrix, View.ViewOrigin );

			// Clear the velocity buffer.
			RHIClear(GlobalContext,TRUE,FLinearColor::Black,FALSE,1.0f,FALSE,0);
			bWroteVelocities = TRUE;

			// Opaque blending, depth tests and writes.
			RHISetBlendState(GlobalContext,TStaticBlendState<>::GetRHI());
			RHISetDepthState(GlobalContext,TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());

			// Build a map from static mesh ID to whether it should be used as an occluder.
			for(TSparseArray<FStaticMesh*>::TConstIterator StaticMeshIt(Scene->StaticMeshes);StaticMeshIt;++StaticMeshIt)
			{
				if(View.StaticMeshVisibilityMap(StaticMeshIt.GetIndex()))
				{
					const FStaticMesh* StaticMesh = *StaticMeshIt;
					if( !ViewState || !StaticMesh->PrimitiveSceneInfo->bStaticShadowing )
					{
						// Only use the static mesh as an occluder if it wasn't occluded the previous frame.
						View.StaticMeshVelocityMap.AccessCorrespondingBit(StaticMeshIt) = TRUE;
					}
				}
			}

			// Draw velocities for movable static meshes.
			bWroteVelocities |= Scene->DPGs[DPGIndex].VelocityDrawList.DrawVisible(GlobalContext,&View,View.StaticMeshVelocityMap);

			BspRendering_StartBatch(BSP_OVERRIDEMATERIAL);				

			extern void ParticleRendering_StartBatch( UBOOL bTranslucentPass );
			extern void ParticleRendering_EndBatch( FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex );

			ParticleRendering_StartBatch(FALSE);

			// Draw velocities for movable dynamic meshes.
			TDynamicPrimitiveDrawer<FVelocityDrawingPolicyFactory> Drawer(GlobalContext,&View,DPGIndex,FVelocityDrawingPolicyFactory::ContextType(),TRUE);
			for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
			{
				const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);
				const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

				const UBOOL bVisible = View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id);
				if( bVisible && !PrimitiveSceneInfo->bStaticShadowing && PrimitiveViewRelevance.GetDPG(DPGIndex) )
				{
					Drawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawDynamicElements(
						&Drawer,
						&View,
						DPGIndex
						);
				}
			}
			
			BspRendering_EndBatch( &Drawer );

			ParticleRendering_EndBatch( &Drawer, &View, DPGIndex );

			bWroteVelocities |= Drawer.IsDirty();

			// For each view we draw the foreground DPG dynamic objects into the velocity buffer so that we can do the motion blur shader after all DPGs
			// are finished.
			//
			// Note that we simply write a velocity of 0 without any regard to the depth buffer.

			static bool bDrawForegroundVelocities = true;
			if (bDrawForegroundVelocities)
			{
				RHISetDepthState(GlobalContext,TStaticDepthState<FALSE,CF_Always>::GetRHI());

				extern void ParticleRendering_StartBatch( UBOOL bTranslucentPass );
				extern void ParticleRendering_EndBatch( FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex );

				ParticleRendering_StartBatch(FALSE);

				FLOAT OldMaxVelocity = View.MotionBlurParameters.MaxVelocity;
				View.MotionBlurParameters.MaxVelocity = 0.f;

				// Draw velocities for movable dynamic meshes.
				TDynamicPrimitiveDrawer<FVelocityDrawingPolicyFactory> ForegroundDrawer(GlobalContext,&View,SDPG_Foreground,FVelocityDrawingPolicyFactory::ContextType(),TRUE);

				for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
				{
					const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);
					const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

					if( !PrimitiveSceneInfo->bStaticShadowing && PrimitiveViewRelevance.GetDPG(SDPG_Foreground) )
					{
						ForegroundDrawer.SetPrimitive(PrimitiveSceneInfo);
						PrimitiveSceneInfo->Proxy->DrawDynamicElements(&ForegroundDrawer,&View,SDPG_Foreground);
					}
				}

				ParticleRendering_EndBatch(&Drawer, &View, DPGIndex);

				bWroteVelocities |= Drawer.IsDirty();
				View.MotionBlurParameters.MaxVelocity = OldMaxVelocity;

				RHISetDepthState(GlobalContext,TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());
			}
		}
	}

	GSceneRenderTargets.FinishRenderingPrePass( bWroteVelocities );
}
