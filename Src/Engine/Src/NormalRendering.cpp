/*========================================================================
	NormalRendering.cpp : Implementation for rendering viewspace normal.
	Copyright 2007 Redduck, inc. All Rights Reserved.
========================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "BSPDynamicBatch.h"

/**
 * A vertex shader for rendering viewspace normal.
 */
class FViewSpaceNormalVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FViewSpaceNormalVertexShader, MeshMaterial);
public:
	static UBOOL ShouldCache(EShaderPlatform Platform, const FMaterial *Material, const FVertexFactoryType *VertexFactoryType)
	{
		return	TRUE;
	}
	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("WORLD_COORDS"), TEXT("1")) ;
	}
	FViewSpaceNormalVertexShader( const ShaderMetaType::CompiledShaderInitializerType &Initializer ):
		FShader(Initializer),
		VertexFactoryParameters(Initializer.VertexFactoryType, Initializer.ParameterMap)
	{
		ViewProjectionMatrixParameter.Bind(Initializer.ParameterMap, TEXT("ViewProjectionMatrix"));
	}
	FViewSpaceNormalVertexShader() {}
	virtual void Serialize(FArchive &Ar)
	{
		FShader::Serialize(Ar);
		Ar << VertexFactoryParameters;
		Ar << ViewProjectionMatrixParameter;
	}
	void SetParameters(FCommandContextRHI *Context, const FVertexFactory *VertexFactory, const FMaterialInstance *MaterialInstance, const FSceneView *View )
	{
		VertexFactoryParameters.Set(Context, this, VertexFactory, View);
		SetVertexShaderValue( Context, GetVertexShader(), ViewProjectionMatrixParameter, View->ViewProjectionMatrix );
	}
	void SetLocalTransforms(FCommandContextRHI *Context, const FMatrix &LocalToWorld, const FMatrix &WorldToLocal )
	{
		VertexFactoryParameters.SetLocalTransforms( Context, this, LocalToWorld, WorldToLocal );
	}

private:
	FVertexFactoryParameterRef VertexFactoryParameters;
	FShaderParameter ViewProjectionMatrixParameter;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FViewSpaceNormalVertexShader, TEXT("ViewSpaceNormalShader"), TEXT("MainVertexShader"), SF_Vertex, 0, VER_AVA_VIEWSPACENORMAL );

/**
* A pixel shader for rendering texture density.
*/

class FViewSpaceNormalPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FViewSpaceNormalPixelShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return TRUE;
	}
	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("WORLD_COORDS"), TEXT("1")) ;
	}
	FViewSpaceNormalPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FShader(Initializer)
	{
		LocalToWorldParameter.Bind(Initializer.ParameterMap, TEXT("LocalToWorldMatrix"), TRUE );
		WorldToViewParameter.Bind(Initializer.ParameterMap, TEXT("WorldToViewMatrix"), TRUE );

		const FMaterial *Material = Initializer.Material;
		const FShaderParameterMap &ParameterMap = Initializer.ParameterMap;

		// Bind uniform scalar expression parameters.
		for(INT ParameterIndex = 0;ParameterIndex < Material->GetUniformScalarExpressions().Num();ParameterIndex++)
		{
			FShaderParameter ShaderParameter;
			FString ParameterName = FString::Printf(TEXT("UniformScalar_%u"),ParameterIndex);
			ShaderParameter.Bind(ParameterMap,*ParameterName,TRUE);
			if(ShaderParameter.IsBound())
			{
				FUniformParameter* UniformParameter = new(UniformParameters) FUniformParameter;
				UniformParameter->Type = MCT_Float;
				UniformParameter->Index = ParameterIndex;
				UniformParameter->ShaderParameter = ShaderParameter;
			}
		}

		// Bind uniform vector expression parameters.
		for(INT ParameterIndex = 0;ParameterIndex < Material->GetUniformVectorExpressions().Num();ParameterIndex++)
		{
			FShaderParameter ShaderParameter;
			FString ParameterName = FString::Printf(TEXT("UniformVector_%u"),ParameterIndex);
			ShaderParameter.Bind(ParameterMap,*ParameterName,TRUE);
			if(ShaderParameter.IsBound())
			{
				FUniformParameter* UniformParameter = new(UniformParameters) FUniformParameter;
				UniformParameter->Type = MCT_Float4;
				UniformParameter->Index = ParameterIndex;
				UniformParameter->ShaderParameter = ShaderParameter;
			}
		}

		// Bind uniform 2D texture parameters.
		for(INT ParameterIndex = 0;ParameterIndex < Material->GetUniform2DTextureExpressions().Num();ParameterIndex++)
		{
			FShaderParameter ShaderParameter;
			FString ParameterName = FString::Printf(TEXT("Texture2D_%u"),ParameterIndex);
			ShaderParameter.Bind(ParameterMap,*ParameterName,TRUE);
			if(ShaderParameter.IsBound())
			{
				FUniformParameter* UniformParameter = new(UniformParameters) FUniformParameter;
				UniformParameter->Type = MCT_Texture2D;
				UniformParameter->Index = ParameterIndex;
				UniformParameter->ShaderParameter = ShaderParameter;
			}
		}
	}

	FViewSpaceNormalPixelShader() {}

	void SetParameters(FCommandContextRHI* Context,const FVertexFactory* VertexFactory,const FMaterialInstance* MaterialInstance,const FSceneView* View, const FMaterialInstance* OriginalMaterialInstance)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialInstance, View->Family->CurrentWorldTime, View->Family->CurrentRealTime, View);
		SetPixelShaderValues( Context, GetPixelShader(), WorldToViewParameter, (FVector4*)&MaterialRenderContext.View->ViewMatrix, 3);

		// Set the uniform parameters.
		const FMaterial* Material = MaterialRenderContext.MaterialInstance->GetMaterial();
		for(INT ParameterIndex = 0;ParameterIndex < UniformParameters.Num();ParameterIndex++)
		{
			const FUniformParameter& UniformParameter = UniformParameters(ParameterIndex);
			switch(UniformParameter.Type)
			{
			case MCT_Float:
				{
					FLinearColor Value;
					if (!Material || (UniformParameter.Index >= Material->GetUniformScalarExpressions().Num()))
					{
						Value = FLinearColor(0.0f, 0.0f, 0.0f);
					}
					else
					{
						(Material->GetUniformScalarExpressions())(UniformParameter.Index)->GetNumberValue(MaterialRenderContext,Value);
					}
					SetPixelShaderValue(Context,GetPixelShader(),UniformParameter.ShaderParameter,Value.R);
					break;
				}
			case MCT_Float4:
				{
					FLinearColor Value;			
					if (!Material || (UniformParameter.Index >= Material->GetUniformVectorExpressions().Num()))
					{
						Value = FLinearColor(5.0f, 0.0f, 5.0f);
					}
					else
					{
						(Material->GetUniformVectorExpressions())(UniformParameter.Index)->GetNumberValue(MaterialRenderContext,Value);
					}
					SetPixelShaderValue(Context,GetPixelShader(),UniformParameter.ShaderParameter,Value);
					break;
				}
			case MCT_Texture2D:
				{
					const FTexture* Value = NULL;
					if (!Material || (UniformParameter.Index >= Material->GetUniform2DTextureExpressions().Num()))
					{
						Value = GWhiteTexture;
					}
					if (!Value)
					{
						(Material->GetUniform2DTextureExpressions())(UniformParameter.Index)->GetTextureValue(MaterialRenderContext,&Value);
					}
					if(!Value)
					{
						Value = GWhiteTexture;
					}
					SetTextureParameter(Context,GetPixelShader(),UniformParameter.ShaderParameter,Value);
					break;
				}
			};
		}
	}
	void SetLocalTransforms(FCommandContextRHI* Context,const FMaterialInstance* MaterialInstance,const FMatrix& LocalToWorld,UBOOL bBackFace)
	{
		SetPixelShaderValues(Context, GetPixelShader(), LocalToWorldParameter, (FVector4*)&LocalToWorld, 3);
	}
	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		Ar << UniformParameters;
		Ar << LocalToWorldParameter;
		Ar << WorldToViewParameter;
	}
private:
	//FMaterialPixelShaderParameters MaterialParameters;
	struct FUniformParameter
	{
		BYTE Type;
		INT Index;
		FShaderParameter ShaderParameter;
		friend FArchive& operator<<(FArchive& Ar,FUniformParameter& P)
		{
			return Ar << P.Type << P.Index << P.ShaderParameter;
		}
	};
	TArray<FUniformParameter> UniformParameters;
	FShaderParameter LocalToWorldParameter;
	FShaderParameter WorldToViewParameter;

};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FViewSpaceNormalPixelShader,TEXT("ViewSpaceNormalShader"),TEXT("MainPixelShader"),SF_Pixel, 0, VER_AVA_VIEWSPACENORMAL);

//=============================================================================
/** FViewSpaceNormalDrawingPolicy - Policy to wrap FMeshDrawingPolicy with new shaders. */

FNormalDrawingPolicy::FNormalDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialInstance* InMaterialInstance,
	const FMaterialInstance* InOriginalMaterialInstance
	)
	:	FMeshDrawingPolicy(InVertexFactory,InMaterialInstance)
	,	OriginalMaterialInstance(InOriginalMaterialInstance)
{
	const FMaterialShaderMap* MaterialShaderIndex = InMaterialInstance->GetMaterial()->GetShaderMap();
	const FMeshMaterialShaderMap* MeshShaderIndex = MaterialShaderIndex->GetMeshShaderMap(InVertexFactory->GetType());
	UBOOL HasVertexShader = MeshShaderIndex->HasShader(&FViewSpaceNormalVertexShader::StaticType);
	UBOOL HasPixelShader = MeshShaderIndex->HasShader(&FViewSpaceNormalPixelShader::StaticType);
	VertexShader = HasVertexShader ? MeshShaderIndex->GetShader<FViewSpaceNormalVertexShader>() : NULL;
	PixelShader = HasPixelShader ? MeshShaderIndex->GetShader<FViewSpaceNormalPixelShader>() : NULL;
}

void FNormalDrawingPolicy::DrawShared( FCommandContextRHI* Context,const FSceneView* SceneView, FBoundShaderStateRHIRef ShaderState ) const
{
	SCOPE_CYCLE_COUNTER(STAT_DepthDrawSharedTime);

	// NOTE: Assuming this cast is always safe!
	FViewInfo* View = (FViewInfo*) SceneView;

	// Set the depth-only shader parameters for the material.
	RHISetBoundShaderState( Context, ShaderState );
	VertexShader->SetParameters( Context, VertexFactory, MaterialInstance, View );
	PixelShader->SetParameters( Context, VertexFactory, MaterialInstance, View, OriginalMaterialInstance );

	// Set the shared mesh resources.
	FMeshDrawingPolicy::DrawShared( Context, View );
}

void FNormalDrawingPolicy::SetMeshRenderState(
	FCommandContextRHI* Context,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const ElementDataType& ElementData
	) const
{
	VertexShader->SetLocalTransforms(Context, Mesh.LocalToWorld, Mesh.WorldToLocal);
	PixelShader->SetLocalTransforms(Context, Mesh.MaterialInstance, Mesh.LocalToWorld, bBackFace);
	FMeshDrawingPolicy::SetMeshRenderState(Context, PrimitiveSceneInfo, Mesh,bBackFace, ElementData);
}

/** 
* Create bound shader state using the vertex decl from the mesh draw policy
* as well as the shaders needed to draw the mesh
* @param DynamicStride - optional stride for dynamic vertex data
* @return new bound shader state object
*/
FBoundShaderStateRHIRef FNormalDrawingPolicy::CreateBoundShaderState(DWORD DynamicStride)
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

INT Compare(const FNormalDrawingPolicy& A,const FNormalDrawingPolicy& B)
{
	COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
	COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
	COMPAREDRAWINGPOLICYMEMBERS(MaterialInstance);
	return 0;
}

//=============================================================================
/** Policy to wrap FMeshDrawingPolicy with new shaders. */

UBOOL FNormalDrawingPolicyFactory::DrawDynamicMesh(
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
	const FMaterialInstance* OriginalMaterialInstance = Mesh.MaterialInstance;
	const FMaterialInstance* MaterialInstance = OriginalMaterialInstance;
	const FMaterial* Material = MaterialInstance->GetMaterial();
	FNormalDrawingPolicy DrawingPolicy( Mesh.VertexFactory, MaterialInstance, OriginalMaterialInstance );

	if (!Mesh.bDrawnShared)
	{
		DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
	}
	DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FMeshDrawingPolicy::ElementDataType());
	DrawingPolicy.DrawMesh(Context,Mesh);
	return TRUE;
}

UBOOL FNormalDrawingPolicyFactory::DrawStaticMesh( FCommandContextRHI* Context, const FSceneView* View, const FStaticMesh* Mesh, UBOOL bDrawnShared )
{
	// Only draw opaque materials in the depth pass.
	const FMaterialInstance* OriginalMaterialInstance = Mesh->MaterialInstance;
	const FMaterialInstance* MaterialInstance = OriginalMaterialInstance;
	const FMaterial* Material = MaterialInstance->GetMaterial();
	UBOOL bBackFace = FALSE;
	FMeshElement MeshElement( *Mesh );
	FNormalDrawingPolicy DrawingPolicy( MeshElement.VertexFactory, MaterialInstance, OriginalMaterialInstance );

	DrawingPolicy.DrawShared(Context, View, DrawingPolicy.CreateBoundShaderState(MeshElement.GetDynamicVertexStride()));	
	DrawingPolicy.SetMeshRenderState(Context, Mesh->PrimitiveSceneInfo, MeshElement, bBackFace, FMeshDrawingPolicy::ElementDataType());
	DrawingPolicy.DrawMesh(Context, MeshElement);
	return TRUE;
}

/** Renders view space normal as color instead of the normal color. */
UBOOL FSceneRenderer::RenderViewSpaceNormals(UINT DPGIndex)
{
	SCOPED_DRAW_EVENT(EventViewSpaceNormal)(DEC_SCENE_ITEMS,TEXT("RenderViewSpaceNormals"));

	UBOOL bWorldDpg = (DPGIndex == SDPG_World);
	UBOOL bDirty = FALSE;

	// Opaque blending, enable depth tests and writes.
	RHISetBlendState(GlobalContext,TStaticBlendState<>::GetRHI());
	RHISetDepthState(GlobalContext,TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());

	FNormalDrawingPolicyFactory PolicyFactory;
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

		FViewInfo& View = Views(ViewIndex);
		const FSceneViewState* ViewState = (FSceneViewState*)View.State;

		// Set the device viewport for the view.
		RHISetViewport(GlobalContext,View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);

		if (GUseTilingCode && bWorldDpg)
		{
			RHIMSAAFixViewport();
		}

		// Draw texture density for static meshes.
		bDirty |= Scene->DPGs[DPGIndex].EmissiveNoLightMapDrawList.DrawVisible( GlobalContext, &View, View.StaticMeshVisibilityMap, PolicyFactory );
		bDirty |= Scene->DPGs[DPGIndex].EmissiveVertexLightMapDrawList.DrawVisible( GlobalContext, &View, View.StaticMeshVisibilityMap, PolicyFactory );
		bDirty |= Scene->DPGs[DPGIndex].EmissiveLightMapTextureDrawList.DrawVisible( GlobalContext, &View, View.StaticMeshVisibilityMap, PolicyFactory );
		
		BspRendering_StartBatch(BSP_OVERRIDEMATERIAL);		

		extern void ParticleRendering_StartBatch( UBOOL bTranslucentPass );
		extern void ParticleRendering_EndBatch( FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex );

		ParticleRendering_StartBatch(FALSE);

		// Draw texture density for dynamic meshes.
		TDynamicPrimitiveDrawer<FNormalDrawingPolicyFactory> Drawer(GlobalContext,&View,DPGIndex,FNormalDrawingPolicyFactory::ContextType(),TRUE);
		for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
		{
			const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);
			const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

			const UBOOL bVisible = View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id);
			if( bVisible &&		PrimitiveViewRelevance.GetDPG(DPGIndex) )
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

		ParticleRendering_EndBatch(&Drawer, &View, DPGIndex);

		bDirty |= Drawer.IsDirty();
	}

	return bDirty;
}
