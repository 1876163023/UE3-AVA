/*=============================================================================
	TranslucentRendering.cpp: Translucent rendering implementation.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"

/**
* Pixel shader used for combining LDR translucency with scene color when floating point blending isn't supported
*/
class FLDRTranslucencyCombinePixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FLDRTranslucencyCombinePixelShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return Platform == SP_PCD3D_SM2_POOR;
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
	}

	/** Default constructor. */
	FLDRTranslucencyCombinePixelShader() {}

public:

	FShaderParameter LDRTranslucencyTextureParameter;
	FSceneTextureShaderParameters SceneTextureParameters;

	/** Initialization constructor. */
	FLDRTranslucencyCombinePixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
		SceneTextureParameters.Bind(Initializer.ParameterMap);
		LDRTranslucencyTextureParameter.Bind(Initializer.ParameterMap,TEXT("LDRTranslucencyTexture"),TRUE);
	}

	// FGlobalShader interface.
	virtual void Serialize(FArchive& Ar)
	{
		FGlobalShader::Serialize(Ar);
		Ar << SceneTextureParameters;
		Ar << LDRTranslucencyTextureParameter;
	}
};

IMPLEMENT_SHADER_TYPE(,FLDRTranslucencyCombinePixelShader,TEXT("LDRTranslucencyCombinePixelShader"),TEXT("Main"),SF_Pixel,0,VER_AVA_SHADER_TRANSFORM_OPTIMIZED);

/**
* Vertex shader used for combining LDR translucency with scene color when floating point blending isn't supported
*/
class FLDRTranslucencyCombineVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FLDRTranslucencyCombineVertexShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return Platform == SP_PCD3D_SM2_POOR;
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
	}

	FLDRTranslucencyCombineVertexShader() {}

public:

	FLDRTranslucencyCombineVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
	}
};

IMPLEMENT_SHADER_TYPE(,FLDRTranslucencyCombineVertexShader,TEXT("LDRTranslucencyCombineVertexShader"),TEXT("Main"),SF_Vertex,0,VER_AVA_SHADER_TRANSFORM_OPTIMIZED);

FGlobalBoundShaderStateRHIRef FSceneRenderer::LDRCombineBoundShaderState;

extern TGlobalResource<FFilterVertexDeclaration> GFilterVertexDeclaration;

/**
* Base translucency blend policy
*/
class FBlendPolicy
{
public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// only cache for unlit translucent materials
		return Material &&
			(Material->GetLightingModel() == MLM_Unlit || Material->GetLightingModel() == MLM_Phong) &&
			IsTranslucentBlendMode(Material->GetBlendMode());
	}
};

/**
* Translucent blend policy 
*/
class FBlendTranslucentPolicy : public FBlendPolicy
{
public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// only cache translucent materials
		return Material && 
			Material->GetBlendMode() == BLEND_Translucent &&
			FBlendPolicy::ShouldCache(Platform,Material,VertexFactoryType);
	}

	FORCEINLINE static void SetBlendState(FCommandContextRHI* Context)
	{
		
		if (GSupportsFPBlending || GIsLowEndHW)
		{
			// Alpha blend color = SrcColor * Alpha + DestColor * (1-Alpha)
			// Don't alter alpha channel
			RHISetBlendState(Context,TStaticBlendState<BO_Add,BF_SourceAlpha,BF_InverseSourceAlpha,BO_Add,BF_Zero,BF_One>::GetRHI());
		}
		else
		{
			// Alpha blend color = SrcColor * Alpha + DestColor * (1-Alpha)
			// Alpha channel = DestAlpha * (1 - Alpha) - this is used to weight in scene color during the LDR translucency combine
			RHISetBlendState(Context,TStaticBlendState<BO_Add,BF_SourceAlpha,BF_InverseSourceAlpha,BO_Add,BF_Zero,BF_InverseSourceAlpha>::GetRHI());
		}
	}

	/**
	 * @return		TRUE if this blending mode can be rendered after fog has been applied.
	 */
	FORCEINLINE static UBOOL CanRenderPostFog()
	{
		return TRUE;
	}

	FORCEINLINE static UBOOL RequiresManualBlend()
	{
		return FALSE;
	}
};

/**
* Translucent manual blend policy 
*/
class FManualBlendTranslucentPolicy : public FBlendPolicy
{
public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// only cache translucent materials
		return Material && 
			Material->GetBlendMode() == BLEND_Translucent &&
			Platform == SP_PCD3D_SM2_POOR &&
			FBlendPolicy::ShouldCache(Platform,Material,VertexFactoryType);
	}

	FORCEINLINE static void SetBlendState(FCommandContextRHI* Context)
	{
		RHISetBlendState(Context,TStaticBlendState<>::GetRHI());
	}

	/**
	* @return		TRUE if this blending mode can be rendered after fog has been applied.
	*/
	FORCEINLINE static UBOOL CanRenderPostFog()
	{
		return TRUE;
	}

	FORCEINLINE static UBOOL RequiresManualBlend()
	{
		return TRUE;
	}
};

/**
* Additive blend policy
*/
class FBlendAdditivePolicy : public FBlendPolicy
{
public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// only cache additive materials
		return Material &&
			Material->GetBlendMode() == BLEND_Additive &&
			FBlendPolicy::ShouldCache(Platform,Material,VertexFactoryType);
	}

	FORCEINLINE static void SetBlendState(FCommandContextRHI* Context)
	{
		// Additive color = SrcColor * Alpha + DestColor * 1
		RHISetBlendState(Context,TStaticBlendState<BO_Add,BF_SourceAlpha,BF_One,BO_Add,BF_Zero,BF_One>::GetRHI());
	}

	/**
	 * @return		TRUE if this blending mode can be rendered after fog has been applied.
	 */
	FORCEINLINE static UBOOL CanRenderPostFog()
	{
		return TRUE;
	}

	FORCEINLINE static UBOOL RequiresManualBlend()
	{
		return FALSE;
	}
};

/**
* Additive manual blend policy
*/
class FManualBlendAdditivePolicy : public FBlendPolicy
{
public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// only cache additive materials
		return Material &&
			Material->GetBlendMode() == BLEND_Additive &&
			Platform == SP_PCD3D_SM2_POOR &&
			FBlendPolicy::ShouldCache(Platform,Material,VertexFactoryType);
	}

	FORCEINLINE static void SetBlendState(FCommandContextRHI* Context)
	{
		// Additive color = SrcColor * Alpha + DestColor * 1
		RHISetBlendState(Context,TStaticBlendState<>::GetRHI());
	}

	/**
	* @return		TRUE if this blending mode can be rendered after fog has been applied.
	*/
	FORCEINLINE static UBOOL CanRenderPostFog()
	{
		return TRUE;
	}

	FORCEINLINE static UBOOL RequiresManualBlend()
	{
		return TRUE;
	}
};

/**
* Modulated blend policy
*/
class FBlendModulatePolicy : public FBlendPolicy
{
public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// only cache modulated decal materials
		return Material &&
			Material->GetBlendMode() == BLEND_Modulate &&
			FBlendPolicy::ShouldCache(Platform,Material,VertexFactoryType);
	}

	FORCEINLINE static void SetBlendState(FCommandContextRHI* Context)
	{
		// Modulate color = SrcColor * 0 + SrcColor * DestColor
		RHISetBlendState(Context,TStaticBlendState<BO_Add,BF_Zero,BF_SourceColor,BO_Add,BF_Zero,BF_One>::GetRHI());
	}

	/**
	 * @return		TRUE if this blending mode can be rendered after fog has been applied.
	 */
	FORCEINLINE static UBOOL CanRenderPostFog()
	{
		return FALSE;
	}

	FORCEINLINE static UBOOL RequiresManualBlend()
	{
		return FALSE;
	}
};

/**
* Modulated manual blend policy
*/
class FManualBlendModulatePolicy : public FBlendPolicy
{
public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// only cache modulated decal materials
		return Material &&
			Material->GetBlendMode() == BLEND_Modulate &&
			Platform == SP_PCD3D_SM2_POOR &&
			FBlendPolicy::ShouldCache(Platform,Material,VertexFactoryType);
	}

	FORCEINLINE static void SetBlendState(FCommandContextRHI* Context)
	{
		// Modulate color = SrcColor * 0 + SrcColor * DestColor
		RHISetBlendState(Context,TStaticBlendState<>::GetRHI());
	}

	/**
	* @return		TRUE if this blending mode can be rendered after fog has been applied.
	*/
	FORCEINLINE static UBOOL CanRenderPostFog()
	{
		return FALSE;
	}

	FORCEINLINE static UBOOL RequiresManualBlend()
	{
		return TRUE;
	}
};

/**
* A vertex shader for rendering translucent blended meshes
*/
template<class BlendPolicyType, class LightMapPolicyType>
class TTranslucencyVertexShader : public FShader, public LightMapPolicyType::VertexParametersType
{
	DECLARE_SHADER_TYPE(TTranslucencyVertexShader,MeshMaterial);

public:
	void GetLightMapParameters(typename LightMapPolicyType::VertexParametersType*& params) { params = this; }

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return BlendPolicyType::ShouldCache(Platform,Material,VertexFactoryType)
			&& LightMapPolicyType::ShouldCache(Platform,Material,VertexFactoryType);
	}

	TTranslucencyVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FShader(Initializer)
		,	VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{
		LightMapPolicyType::VertexParametersType::Bind(Initializer.ParameterMap);

		HeightFogParameters.Bind(Initializer.ParameterMap);
	}

	TTranslucencyVertexShader()
	{
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_LIGHTMAP_COEFFICIENTS"),*FString::Printf(TEXT("%u"),NUM_LIGHTMAP_COEFFICIENTS));
		LightMapPolicyType::ModifyCompilationEnvironment(OutEnvironment);
	}

	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		LightMapPolicyType::VertexParametersType::Serialize(Ar);
		Ar << VertexFactoryParameters;		
		Ar << HeightFogParameters;

	}

	void SetParameters(
		FCommandContextRHI* Context,
		const FVertexFactory* VertexFactory,
		const FMaterialInstance* MaterialInstance,
		const FViewInfo& View,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo)
	{
		VertexFactoryParameters.Set(Context,this,VertexFactory,&View);		
		HeightFogParameters.Set(Context, MaterialInstance, View, this);
	}

	void SetLocalTransforms(FCommandContextRHI* Context,const FMatrix& LocalToWorld,const FMatrix& WorldToLocal)
	{
		VertexFactoryParameters.SetLocalTransforms(Context,this,LocalToWorld,WorldToLocal);
	}

private:
	FVertexFactoryParameterRef VertexFactoryParameters;	

	/** The parameters needed to calculate the fog contribution from height fog layers. */
	FHeightFogVertexShaderParameters HeightFogParameters;
};

/**
* A pixel shader for rendering translucent blended meshes
*/
template<class BlendPolicyType, class LightMapPolicyType>
class TTranslucencyPixelShader : public FShader, public LightMapPolicyType::PixelParametersType
{
	DECLARE_SHADER_TYPE(TTranslucencyPixelShader,MeshMaterial);

public:
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return BlendPolicyType::ShouldCache(Platform,Material,VertexFactoryType)
			&& LightMapPolicyType::ShouldCache(Platform,Material,VertexFactoryType);
	}

	TTranslucencyPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FShader(Initializer)
	{
		LightMapPolicyType::PixelParametersType::Bind(Initializer.ParameterMap);
		AmbientColorAndSkyFactorParameter.Bind(Initializer.ParameterMap,TEXT("AmbientColorAndSkyFactor"),TRUE);
		UpperSkyColorParameter.Bind(Initializer.ParameterMap,TEXT("UpperSkyColor"),TRUE);
		LowerSkyColorParameter.Bind(Initializer.ParameterMap,TEXT("LowerSkyColor"),TRUE);

		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
	}

	TTranslucencyPixelShader()
	{

	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_LIGHTMAP_COEFFICIENTS"),*FString::Printf(TEXT("%u"),NUM_LIGHTMAP_COEFFICIENTS));
		LightMapPolicyType::ModifyCompilationEnvironment(OutEnvironment);

		//tell the shader to do blending manually if required by the blend policy
		OutEnvironment.Definitions.Set(TEXT("MANUAL_FP_BLEND"),*FString::Printf(TEXT("%u"),(UINT)BlendPolicyType::RequiresManualBlend()));
	}

	void SetParameters(FCommandContextRHI* Context,const FVertexFactory* VertexFactory,const FMaterialInstance* MaterialInstance,const FViewInfo& View)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialInstance, View.Family->CurrentWorldTime, View.Family->CurrentRealTime, &View);
		MaterialParameters.Set(Context,this,MaterialRenderContext);

		SetPixelShaderValue(
			Context,
			GetPixelShader(),
			AmbientColorAndSkyFactorParameter,
			(View.Family->ShowFlags & SHOW_Lighting) ? FLinearColor(0,0,0,1) : FLinearColor(1,1,1,0)
			);
	}

	void SetLocalTransforms(FCommandContextRHI* Context,const FMaterialInstance* MaterialInstance,const FMatrix& LocalToWorld,UBOOL bBackFace)
	{
		MaterialParameters.SetLocalTransforms(Context,this,MaterialInstance,LocalToWorld,bBackFace);
	}

	void SetSkyColor(FCommandContextRHI* Context,const FLinearColor& UpperSkyColor,const FLinearColor& LowerSkyColor)
	{
		SetPixelShaderValue(Context,GetPixelShader(),UpperSkyColorParameter,UpperSkyColor);
		SetPixelShaderValue(Context,GetPixelShader(),LowerSkyColorParameter,LowerSkyColor);
	}

	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		LightMapPolicyType::PixelParametersType::Serialize(Ar);
		Ar << AmbientColorAndSkyFactorParameter;
		Ar << UpperSkyColorParameter;
		Ar << LowerSkyColorParameter;
		Ar << MaterialParameters;
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;

	FShaderParameter AmbientColorAndSkyFactorParameter;
	FShaderParameter UpperSkyColorParameter;
	FShaderParameter LowerSkyColorParameter;
};

/** A logical exclusive or function. */
static UBOOL XOR(UBOOL A,UBOOL B)
{
	return (A && !B) || (!A && B);
}

/**
* Translucent drawing policy
*/
template<class BlendPolicyType, class LightMapPolicyType>
class TTranslucencyDrawingPolicy : public FMeshDrawingPolicy
{
public:
	/** context type */
	typedef FMeshDrawingPolicy::ElementDataType ElementDataType;

	/** light map context type */
	typedef typename LightMapPolicyType::ElementDataType LightMapPolicyType_ElementDataType;

	/**
	* Constructor
	* @param InIndexBuffer - index buffer for rendering
	* @param InVertexFactory - vertex factory for rendering
	* @param InMaterialInstance - material instance for rendering
	*/
	TTranslucencyDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialInstance* InMaterialInstance,
		LightMapPolicyType InLightMapPolicy)
		:	FMeshDrawingPolicy(InVertexFactory,InMaterialInstance),
			LightMapPolicy(InLightMapPolicy)
	{
		const FMaterialShaderMap* MaterialShaderIndex = InMaterialInstance->GetMaterial()->GetShaderMap();
		const FMeshMaterialShaderMap* MeshShaderIndex = MaterialShaderIndex->GetMeshShaderMap(InVertexFactory->GetType());
		// get cached shaders
		VertexShader = MeshShaderIndex->GetShader<TTranslucencyVertexShader<BlendPolicyType, LightMapPolicyType> >();	
		PixelShader = MeshShaderIndex->GetShader<TTranslucencyPixelShader<BlendPolicyType, LightMapPolicyType> >();		
	}

	// FMeshDrawingPolicy interface.

	/**
	* Match two draw policies
	* @param Other - draw policy to compare
	* @return TRUE if the draw policies are a match
	*/
	UBOOL Matches(const TTranslucencyDrawingPolicy& Other) const
	{
		return FMeshDrawingPolicy::Matches(Other) &&
			VertexShader == Other.VertexShader &&
			PixelShader == Other.PixelShader &&
			LightMapPolicy == Other.LightMapPolicy;
	}

	/**
	* Executes the draw commands which can be shared between any meshes using this drawer.
	* @param CI - The command interface to execute the draw commands on.
	* @param View - The view of the scene being drawn.
	*/
	void DrawShared(FCommandContextRHI* Context,const FViewInfo& View,FBoundShaderStateRHIParamRef BoundShaderState,const FPrimitiveSceneInfo* PrimitiveSceneInfo) const
	{
		// Set the translucent shader parameters for the material instance
		VertexShader->SetParameters(Context,VertexFactory,MaterialInstance,View,PrimitiveSceneInfo);

		PixelShader->SetParameters(Context,VertexFactory,MaterialInstance,View);
		// Set blend mode
		BlendPolicyType::SetBlendState(Context);
		// Set vertex factory
		FMeshDrawingPolicy::DrawShared(Context,&View);

		// Set the light-map policy.
		typename LightMapPolicyType::VertexParametersType* lightMapVertexParams = NULL;
		VertexShader->GetLightMapParameters(lightMapVertexParams);
		LightMapPolicy.Set(Context,lightMapVertexParams,PixelShader,PixelShader,VertexFactory,MaterialInstance,&View);

		// Set the actual shader & vertex declaration state
		RHISetBoundShaderState(Context, BoundShaderState);
	}

	/** 
	* Create bound shader state using the vertex decl from the mesh draw policy
	* as well as the shaders needed to draw the mesh
	* @param DynamicStride - optional stride for dynamic vertex data
	* @return new bound shader state object
	*/
	FBoundShaderStateRHIRef CreateBoundShaderState(DWORD DynamicStride = 0)
	{
		FVertexDeclarationRHIParamRef VertexDeclaration;
		DWORD StreamStrides[MaxVertexElementCount];

		//FMeshDrawingPolicy::GetVertexDeclarationInfo(VertexDeclaration, StreamStrides);
		LightMapPolicy.GetVertexDeclarationInfo(VertexDeclaration, StreamStrides, VertexFactory);
		if (DynamicStride)
		{
			StreamStrides[0] = DynamicStride;
		}

		return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, VertexShader->GetVertexShader(), PixelShader->GetPixelShader());	
	}

	/**
	* Sets the render states for drawing a mesh.
	* @param Context - command context
	* @param PrimitiveSceneInfo - The primitive drawing the dynamic mesh.  If this is a view element, this will be NULL.
	* @param Mesh - mesh element with data needed for rendering
	* @param ElementData - context specific data for mesh rendering
	*/
	void SetMeshRenderState(
		FCommandContextRHI* Context,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		const ElementDataType& ElementData,
		const LightMapPolicyType_ElementDataType& LightMapElementData
		) const
	{
		// Set the light-map policy's mesh-specific settings.
		typename LightMapPolicyType::VertexParametersType* lightMapVertexParams = NULL;
		VertexShader->GetLightMapParameters(lightMapVertexParams);
		LightMapPolicy.SetMesh(Context,lightMapVertexParams,PixelShader,VertexShader,PixelShader,VertexFactory,MaterialInstance,LightMapElementData);

		// Set the upper and lower sky colors for the hemisphere lighting
		if(PrimitiveSceneInfo)
		{
			PixelShader->SetSkyColor(Context,PrimitiveSceneInfo->UpperSkyLightColor,PrimitiveSceneInfo->LowerSkyLightColor);
		}
		else
		{
			PixelShader->SetSkyColor(Context,FLinearColor::Black,FLinearColor::Black);
		}

		// Set transforms
		VertexShader->SetLocalTransforms(Context,Mesh.LocalToWorld,Mesh.WorldToLocal);
		PixelShader->SetLocalTransforms(Context,Mesh.MaterialInstance,Mesh.LocalToWorld,bBackFace);
	
		extern UBOOL GInvertCullMode;
		const FRasterizerStateInitializerRHI Initializer = {
			(Mesh.bWireframe || IsWireframe()) ? FM_Wireframe : FM_Solid,
			IsTwoSided() ? CM_None : (XOR(bBackFace, Mesh.ReverseCulling||GInvertCullMode) ? CM_CCW : CM_CW),
			Mesh.DepthBias,
			Mesh.SlopeScaleDepthBias
		};
		RHISetRasterizerStateImmediate(Context, Initializer);
	}

private:
	/** vertex shader based on policy type */
	TTranslucencyVertexShader<BlendPolicyType, LightMapPolicyType>* VertexShader;
	/** pixel shader based on policy type */
	TTranslucencyPixelShader<BlendPolicyType, LightMapPolicyType>* PixelShader;	
	/** Light-map policy */
	LightMapPolicyType LightMapPolicy;
};

/**
* Render a dynamic mesh using the templated light map policy and templated blend policy,
* or the fallback blend policy if floating point blending is not supported.
* @return TRUE if the mesh rendered
*/
template<class BlendPolicy, class FallbackBlendPolicy, class LightMapPolicyType>
UBOOL BlendPolicyDrawDynamicMesh(
								 FCommandContextRHI* Context,
								 const FViewInfo* View,
								 FTranslucencyDrawingPolicyFactory::ContextType DrawingContext,
								 const FMeshElement& Mesh,
								 UBOOL bBackFace,
								 UBOOL bPreFog,
								 const FPrimitiveSceneInfo* PrimitiveSceneInfo,
								 FHitProxyId HitProxyId,
								 UBOOL bIsDecalMaterial,
								 const LightMapPolicyType& lightMapPolicy,
								 const typename LightMapPolicyType::ElementDataType& lightMapPolicyElementData
								 )
{
	UBOOL bDirty = FALSE;		

	// Draw decals before fog if floating point blending is not supported
	if (!(GSupportsFPBlending||GIsLowEndHW) && bIsDecalMaterial && bPreFog)
	{
		//resolve the scene color surface to the texture so that it can be used for blending in the shader
		GSceneRenderTargets.FinishRenderingSceneColor();
		GSceneRenderTargets.BeginRenderingSceneColor();
		//set states that are reset during the resolve
		RHISetDepthState( Context, TStaticDepthState<FALSE,CF_LessEqual>::GetRHI() );
		RHISetViewport(Context,View->RenderTargetX,View->RenderTargetY,0.0f,View->RenderTargetX + View->RenderTargetSizeX,View->RenderTargetY + View->RenderTargetSizeY,1.0f);

		//use a manual blending policy since floating point blending isn't supported
		TTranslucencyDrawingPolicy<FallbackBlendPolicy, LightMapPolicyType> DrawingPolicy(
			Mesh.VertexFactory,
			Mesh.MaterialInstance,			
			lightMapPolicy
			);

		if (!Mesh.bDrawnShared)
		{
			DrawingPolicy.DrawShared(Context,*View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()), PrimitiveSceneInfo);
		}

		DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,TTranslucencyDrawingPolicy<BlendPolicy, LightMapPolicyType>::ElementDataType(),lightMapPolicyElementData);
		DrawingPolicy.DrawMesh(Context,Mesh);
		
		bDirty=TRUE;
	}
	// Draw non-decals (post-fog), or decals pre-fog, or decals post-fog if the blend policy allows it.
	else if ( !bIsDecalMaterial || ( bPreFog || BlendPolicy::CanRenderPostFog() ) )
	{
		TTranslucencyDrawingPolicy<BlendPolicy, LightMapPolicyType> DrawingPolicy(
			Mesh.VertexFactory,
			Mesh.MaterialInstance,			
			lightMapPolicy
			);

		if (!Mesh.bDrawnShared)
		{
			DrawingPolicy.DrawShared(Context,*View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()), PrimitiveSceneInfo);
		}

		DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,TTranslucencyDrawingPolicy<BlendPolicy, LightMapPolicyType>::ElementDataType(),lightMapPolicyElementData);
		DrawingPolicy.DrawMesh(Context,Mesh);
		
		bDirty=TRUE;
	}
	return bDirty;
}

/**
* Render a dynamic mesh using a translucent draw policy
* @return TRUE if the mesh rendered
*/
UBOOL FTranslucencyDrawingPolicyFactory::DrawDynamicMesh(
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
	UBOOL bDirty=FALSE;
	if(Mesh.IsTranslucent() && !bBackFace)
	{
		const FMaterial* CurrentMaterial = Mesh.MaterialInstance->GetMaterial();

		// Decals render pre fog, non-decals render post-fog.
		const UBOOL bIsDecalMaterial = CurrentMaterial->IsDecalMaterial();

		// Retrieve blend mode
		const EBlendMode BlendMode = CurrentMaterial->GetBlendMode();

		// Check for a cached light-map.
		const UBOOL bIsLitMaterial = CurrentMaterial->GetLightingModel() != MLM_Unlit && (View->Family->ShowFlags & SHOW_Lighting);
		const FLightMap* LightMap = Mesh.LCI ? Mesh.LCI->GetLightMap() : NULL;
		if(LightMap && (!LightMap->IsValid() || Mesh.MaterialInstance->GetMaterial()->GetLightingModel() == MLM_Unlit))
		{
			LightMap = NULL;
		}		

		// Handle unlit translucency case
		if(LightMap == NULL)
		{			
			// If we're drawing before fog, don't render non-decals.
			if ( bPreFog && !bIsDecalMaterial )
			{
			}
			else
			{
				// Use the right draw policy based on the material blend mode
				if( BlendMode == BLEND_Translucent )
				{
					bDirty = BlendPolicyDrawDynamicMesh<FBlendTranslucentPolicy, FManualBlendTranslucentPolicy, FNoLightMapPolicy>
						(Context, View, DrawingContext, Mesh, bBackFace, bPreFog, PrimitiveSceneInfo, HitProxyId, bIsDecalMaterial,
						FNoLightMapPolicy(),
						FNoLightMapPolicy::ElementDataType());
				}
				else if( BlendMode == BLEND_Additive )
				{
					bDirty = BlendPolicyDrawDynamicMesh<FBlendAdditivePolicy, FManualBlendAdditivePolicy, FNoLightMapPolicy>
						(Context, View, DrawingContext, Mesh, bBackFace, bPreFog, PrimitiveSceneInfo, HitProxyId, bIsDecalMaterial,
						FNoLightMapPolicy(),
						FNoLightMapPolicy::ElementDataType());
				}
				else if( BlendMode == BLEND_Modulate )
				{
					bDirty = BlendPolicyDrawDynamicMesh<FBlendModulatePolicy, FManualBlendModulatePolicy, FNoLightMapPolicy>
						(Context, View, DrawingContext, Mesh, bBackFace, bPreFog, PrimitiveSceneInfo, HitProxyId, bIsDecalMaterial,
						FNoLightMapPolicy(),
						FNoLightMapPolicy::ElementDataType());
				}
			}
		}

		// Handle lightmapped translucency case
		else if(CurrentMaterial->GetLightingModel() == MLM_Phong)
		{
			const FLightMap1D* LightMap1D = LightMap->GetLightMap1D();
			const FLightMap2D* LightMap2D = LightMap->GetLightMap2D();

			// If we're drawing before fog, don't render non-decals.
			if ( bPreFog && !bIsDecalMaterial )
			{
			}
			else
			{
				// Use the right draw policy based on the material blend mode and light map type
				if(LightMap1D)
				{
					if( BlendMode == BLEND_Translucent )
					{
						bDirty = BlendPolicyDrawDynamicMesh<FBlendTranslucentPolicy, FManualBlendTranslucentPolicy, FVertexLightMapPolicy>
							(Context, View, DrawingContext, Mesh, bBackFace, bPreFog, PrimitiveSceneInfo, HitProxyId, bIsDecalMaterial,
							FVertexLightMapPolicy(),
							LightMap1D);
					}
					else if( BlendMode == BLEND_Additive )
					{
						bDirty = BlendPolicyDrawDynamicMesh<FBlendAdditivePolicy, FManualBlendAdditivePolicy, FVertexLightMapPolicy>
							(Context, View, DrawingContext, Mesh, bBackFace, bPreFog, PrimitiveSceneInfo, HitProxyId, bIsDecalMaterial,
							FVertexLightMapPolicy(),
							LightMap1D);
					}
					else if( BlendMode == BLEND_Modulate )
					{
						bDirty = BlendPolicyDrawDynamicMesh<FBlendModulatePolicy, FManualBlendModulatePolicy, FVertexLightMapPolicy>
							(Context, View, DrawingContext, Mesh, bBackFace, bPreFog, PrimitiveSceneInfo, HitProxyId, bIsDecalMaterial,
							FVertexLightMapPolicy(),
							LightMap1D);
					}
				}
				else if(LightMap2D)
				{
					if( BlendMode == BLEND_Translucent )
					{
						bDirty = BlendPolicyDrawDynamicMesh<FBlendTranslucentPolicy, FManualBlendTranslucentPolicy, FLightMapTexturePolicy>
							(Context, View, DrawingContext, Mesh, bBackFace, bPreFog, PrimitiveSceneInfo, HitProxyId, bIsDecalMaterial,
							FLightMapTexturePolicy(LightMap2D),
							LightMap2D);
					}
					else if( BlendMode == BLEND_Additive )
					{
						bDirty = BlendPolicyDrawDynamicMesh<FBlendAdditivePolicy, FManualBlendAdditivePolicy, FLightMapTexturePolicy>
							(Context, View, DrawingContext, Mesh, bBackFace, bPreFog, PrimitiveSceneInfo, HitProxyId, bIsDecalMaterial,
							FLightMapTexturePolicy(LightMap2D),
							LightMap2D);
					}
					else if( BlendMode == BLEND_Modulate )
					{
						bDirty = BlendPolicyDrawDynamicMesh<FBlendModulatePolicy, FManualBlendModulatePolicy, FLightMapTexturePolicy>
							(Context, View, DrawingContext, Mesh, bBackFace, bPreFog, PrimitiveSceneInfo, HitProxyId, bIsDecalMaterial,
							FLightMapTexturePolicy(LightMap2D),
							LightMap2D);
					}
				}
			}
		}
	}
	return bDirty;
}
/**
 * Render a static mesh using a translucent draw policy 
 * @return TRUE if the mesh rendered
 */
UBOOL FTranslucencyDrawingPolicyFactory::DrawStaticMesh(
	FCommandContextRHI* Context,
	const FViewInfo* View,
	ContextType DrawingContext,
	const FStaticMesh& StaticMesh,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	// this does the same thing as DrawDynamicMesh, except without back-facing option
	return DrawDynamicMesh(Context, View, DrawingContext, StaticMesh, FALSE, bPreFog, PrimitiveSceneInfo, HitProxyId);
}

#define IMPLEMENT_TRANSLUCENT_MATERIAL_SHADER_TYPE(BlendPolicy,MinPackageVersion,MinLicenseePackageVersion) \
	typedef TTranslucencyVertexShader<BlendPolicy,FNoLightMapPolicy> TTranslucencyVertexShader##BlendPolicy##FNoLightMapPolicy; \
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TTranslucencyVertexShader##BlendPolicy##FNoLightMapPolicy,TEXT("TranslucencyVertexShader"),TEXT("Main"),SF_Vertex,MinPackageVersion,MinLicenseePackageVersion); \
	typedef TTranslucencyPixelShader<BlendPolicy,FNoLightMapPolicy> TTranslucencyPixelShader##BlendPolicy##FNoLightMapPolicy; \
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TTranslucencyPixelShader##BlendPolicy##FNoLightMapPolicy,TEXT("TranslucencyPixelShader"),TEXT("Main"),SF_Pixel,MinPackageVersion,MinLicenseePackageVersion); \
	\
	typedef TTranslucencyVertexShader<BlendPolicy,FVertexLightMapPolicy> TTranslucencyVertexShader##BlendPolicy##FVertexLightMapPolicy; \
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TTranslucencyVertexShader##BlendPolicy##FVertexLightMapPolicy,TEXT("EmissiveVertexShader"),TEXT("Main"),SF_Vertex,MinPackageVersion,MinLicenseePackageVersion); \
	typedef TTranslucencyPixelShader<BlendPolicy,FVertexLightMapPolicy> TTranslucencyPixelShader##BlendPolicy##FVertexLightMapPolicy; \
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TTranslucencyPixelShader##BlendPolicy##FVertexLightMapPolicy,TEXT("EmissivePixelShader"),TEXT("Main"),SF_Pixel,MinPackageVersion,MinLicenseePackageVersion); \
	\
	typedef TTranslucencyVertexShader<BlendPolicy,FLightMapTexturePolicy> TTranslucencyVertexShader##BlendPolicy##FLightMapTexturePolicy; \
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TTranslucencyVertexShader##BlendPolicy##FLightMapTexturePolicy,TEXT("EmissiveVertexShader"),TEXT("Main"),SF_Vertex,MinPackageVersion,MinLicenseePackageVersion); \
	typedef TTranslucencyPixelShader<BlendPolicy,FLightMapTexturePolicy> TTranslucencyPixelShader##BlendPolicy##FLightMapTexturePolicy; \
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TTranslucencyPixelShader##BlendPolicy##FLightMapTexturePolicy,TEXT("EmissivePixelShader"),TEXT("Main"),SF_Pixel,MinPackageVersion,MinLicenseePackageVersion);

/** translucent blend shader type */
IMPLEMENT_TRANSLUCENT_MATERIAL_SHADER_TYPE(FBlendTranslucentPolicy,VER_EPIC_MERGE_APR_26,0);
/** translucent manual blend shader type */
IMPLEMENT_TRANSLUCENT_MATERIAL_SHADER_TYPE(FManualBlendTranslucentPolicy,VER_EPIC_MERGE_APR_26,0);
/** additive blend shader type */
IMPLEMENT_TRANSLUCENT_MATERIAL_SHADER_TYPE(FBlendAdditivePolicy,VER_EPIC_MERGE_APR_26,0);
/** additive manual blend shader type */
IMPLEMENT_TRANSLUCENT_MATERIAL_SHADER_TYPE(FManualBlendAdditivePolicy,VER_EPIC_MERGE_APR_26,0);
/** modulated blend shader type */
IMPLEMENT_TRANSLUCENT_MATERIAL_SHADER_TYPE(FBlendModulatePolicy,VER_EPIC_MERGE_APR_26,0);
/** modulated manual blend shader type */
IMPLEMENT_TRANSLUCENT_MATERIAL_SHADER_TYPE(FManualBlendModulatePolicy,VER_EPIC_MERGE_APR_26,0);


/*-----------------------------------------------------------------------------
FTranslucentPrimSet
-----------------------------------------------------------------------------*/

/** 
* Iterate over the sorted list of prims and draw them
* @param Context - command context
* @param ViewInfo - current view used to draw items
* @param DPGIndex - current DPG used to draw items
* @param bPreFog - TRUE if the draw call is occurring before fog has been rendered.
* @return TRUE if anything was drawn
*/
UBOOL FTranslucentPrimSet::Draw(FCommandContextRHI* Context,
		  const FViewInfo* View,
		  UINT DPGIndex,
		  UBOOL bPreFog
		  )
{
	UBOOL bDirty=FALSE;

	// Draw the view's mesh elements with the translucent drawing policy.
	bDirty |= DrawViewElements<FTranslucencyDrawingPolicyFactory>(Context,View,FTranslucencyDrawingPolicyFactory::ContextType(),DPGIndex,bPreFog);

	if( SortedPrims.Num() )
	{
		extern void ParticleRendering_StartBatch( UBOOL bTranslucentPass );
		extern void ParticleRendering_SetBurstMode( UBOOL bBurstMode );
		extern void ParticleRendering_EndBatch( FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex );

		// For drawing scene prims with dynamic relevance.
		TDynamicPrimitiveDrawer<FTranslucencyDrawingPolicyFactory> Drawer(
			Context,
			View,
			DPGIndex,
			FTranslucencyDrawingPolicyFactory::ContextType(),
			bPreFog
			);

		ParticleRendering_StartBatch(TRUE);

		// Draw sorted scene prims
		for( INT PrimIdx=0; PrimIdx < SortedPrims.Num(); PrimIdx++ )
		{
			FPrimitiveSceneInfo* PrimitiveSceneInfo = SortedPrims(PrimIdx).PrimitiveSceneInfo;			

			// Check that the primitive hasn't been occluded.
			if(View->PrimitiveVisibilityMap(PrimitiveSceneInfo->Id))
			{
				const FPrimitiveViewRelevance& ViewRelevance = View->PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

				// Render dynamic scene prim
				if( ViewRelevance.bDynamicRelevance )
				{				
					Drawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawDynamicElements(
						&Drawer,
						View,
						DPGIndex
						);
				}
				// Render static scene prim
				if( ViewRelevance.bStaticRelevance )
				{
					// Render static meshes from static scene prim
					for( INT StaticMeshIdx=0; StaticMeshIdx < PrimitiveSceneInfo->StaticMeshes.Num(); StaticMeshIdx++ )
					{
						FStaticMesh& StaticMesh = PrimitiveSceneInfo->StaticMeshes(StaticMeshIdx);

						// Only render static mesh elements using translucent materials
						if( StaticMesh.IsTranslucent() )
						{
							bDirty |= FTranslucencyDrawingPolicyFactory::DrawStaticMesh(
								Context,
								View,
								FTranslucencyDrawingPolicyFactory::ContextType(),
								StaticMesh,
								bPreFog,
								PrimitiveSceneInfo,
								StaticMesh.HitProxyId
								);
						}
					}
				}
			}
		}

		ParticleRendering_EndBatch( &Drawer, View, DPGIndex );

		// Mark dirty if dynamic drawer rendered
		bDirty |= Drawer.IsDirty();
	}

    // render any translucent prims that sample the scene color texture after all other translucent prims
	if( SortedSceneColorPrims.Num() )
	{
		if( bDirty )
		{
			// if any translucent prims were rendered then resolve scene color texture
//@todo.SAS/SZ-Determine why this breaks emitters on PC...
#if CONSOLE
            GSceneRenderTargets.ResolveSceneColor();
#endif
		}

		// For drawing scene prims with dynamic relevance.
		TDynamicPrimitiveDrawer<FTranslucencyDrawingPolicyFactory> Drawer(
			Context,
			View,
			DPGIndex,
			FTranslucencyDrawingPolicyFactory::ContextType(),
			bPreFog
			);

		extern void ParticleRendering_StartBatch( UBOOL bTranslucentPass );		
		extern void ParticleRendering_EndBatch( FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex );

		ParticleRendering_StartBatch(TRUE);

		// Draw sorted scene prims
		for( INT PrimIdx=0; PrimIdx < SortedSceneColorPrims.Num(); PrimIdx++ )
		{
			FPrimitiveSceneInfo* PrimitiveSceneInfo = SortedSceneColorPrims(PrimIdx).PrimitiveSceneInfo;

			// Check that the primitive hasn't been occluded.
			if(View->PrimitiveVisibilityMap(PrimitiveSceneInfo->Id))
			{
				const FPrimitiveViewRelevance& ViewRelevance = View->PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

				// Render dynamic scene prim
				if( ViewRelevance.bDynamicRelevance )
				{				
					Drawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawDynamicElements(
						&Drawer,
						View,
						DPGIndex
						);
				}
				// Render static scene prim
				if( ViewRelevance.bStaticRelevance )
				{
					// Render static meshes from static scene prim
					for( INT StaticMeshIdx=0; StaticMeshIdx < PrimitiveSceneInfo->StaticMeshes.Num(); StaticMeshIdx++ )
					{
						FStaticMesh& StaticMesh = PrimitiveSceneInfo->StaticMeshes(StaticMeshIdx);

						// Only render static mesh elements using translucent materials
						if( StaticMesh.IsTranslucent() )
						{
							bDirty |= FTranslucencyDrawingPolicyFactory::DrawStaticMesh(
								Context,
								View,
								FTranslucencyDrawingPolicyFactory::ContextType(),
								StaticMesh,
								bPreFog,
								PrimitiveSceneInfo,
								StaticMesh.HitProxyId
								);
						}
					}
				}
			}
		}

		ParticleRendering_EndBatch( &Drawer, View, DPGIndex );

		// Mark dirty if dynamic drawer rendered
		bDirty |= Drawer.IsDirty();
	}

	return bDirty;
}

/**
* Add a new primitive to the list of sorted prims
* @param PrimitiveSceneInfo - primitive info to add. Origin of bounds is used for sort.
* @param ViewInfo - used to transform bounds to view space
* @param bUsesSceneColor - primitive samples from scene color
*/
void FTranslucentPrimSet::AddScenePrimitive(FPrimitiveSceneInfo* PrimitivieSceneInfo,const FViewInfo& ViewInfo, UBOOL bUsesSceneColor)
{
	// sort based on view space depth
	const FLOAT SortKey = ViewInfo.ViewMatrix.TransformFVector(PrimitivieSceneInfo->Bounds.Origin).Z;

	if( bUsesSceneColor )
	{
		// add to list of translucent prims that use scene color
		new(SortedSceneColorPrims) FSortedPrim(PrimitivieSceneInfo,SortKey);
	}
	else
	{
		// add to list of translucent prims
		new(SortedPrims) FSortedPrim(PrimitivieSceneInfo,SortKey);
	}
	
}

/**
* Sort any primitives that were added to the set back-to-front
*/
void FTranslucentPrimSet::SortPrimitives()
{
	// sort prims based on depth
	Sort<USE_COMPARE_CONSTREF(FSortedPrim,TranslucentRender)>(&SortedPrims(0),SortedPrims.Num());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FSceneRenderer::RenderTranslucency
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** 
 * Renders the scene's translucency.
 *
 * @param	DPGIndex	Current DPG used to draw items.
 * @return				TRUE if anything was drawn.
 */
UBOOL FSceneRenderer::RenderTranslucency(UINT DPGIndex)
{
	SCOPED_DRAW_EVENT(EventTranslucent)(DEC_SCENE_ITEMS,TEXT("Translucency"));

	UBOOL bRender=FALSE;
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		const FViewInfo& View = Views(ViewIndex);
		const UBOOL bViewHasTranslucentViewMeshElements = View.bHasTranslucentViewMeshElements & (1<<DPGIndex);
		if( View.TranslucentPrimSet[DPGIndex].NumPrims() > 0 || bViewHasTranslucentViewMeshElements )
		{
			bRender=TRUE;
			break;
		}
	}

	UBOOL bDirty = FALSE;
	if( bRender )
	{
		
		if (GSupportsFPBlending||GIsLowEndHW)
		{
			// Use the scene color buffer.
			GSceneRenderTargets.BeginRenderingSceneColor();
		}
		else
		{
			// render to the LDR scene color surface
			GSceneRenderTargets.BeginRenderingSceneColorLDR();
			// initialize the surface to (0,0,0,1).  The alpha channel determines how much scene color will be weighted
			// in during the combine, which starts out at 1 so that all scene color will be used.
			RHIClear(GlobalContext,TRUE,FLinearColor(0.0f, 0.0f, 0.0f, 1.0f),FALSE,0.0f,FALSE,0);
		}

		// Enable depth test, disable depth writes.
		RHISetDepthState( GlobalContext, TStaticDepthState<FALSE,CF_LessEqual>::GetRHI() );

		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);		

			FViewInfo& View = Views(ViewIndex);
			// viewport to match view size
			RHISetViewport(GlobalContext,View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
			RHISetViewParameters( GlobalContext, &View, View.ViewProjectionMatrix, View.ViewOrigin );

			// draw only translucent prims
			bDirty |= View.TranslucentPrimSet[DPGIndex].Draw(GlobalContext,&View,DPGIndex,FALSE);
		}
	}
	return bDirty;
}

/** 
* Combines the recently rendered LDR translucency with scene color.
*
* @param	Context	 The RHI command context the primitives are being rendered to. 
*/
void FSceneRenderer::CombineLDRTranslucency(FCommandContextRHI* Context)
{
	SCOPED_DRAW_EVENT(EventCombineLDRTranslucency)(DEC_SCENE_ITEMS,TEXT("CombineLDRTranslucency"));
	
	// Render to the scene color buffer.
	GSceneRenderTargets.BeginRenderingSceneColor();

	TShaderMapRef<FLDRTranslucencyCombineVertexShader> CombineVertexShader(GetGlobalShaderMap());
	TShaderMapRef<FLDRTranslucencyCombinePixelShader> CombinePixelShader(GetGlobalShaderMap());

	SetGlobalBoundShaderState(Context, LDRCombineBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *CombineVertexShader, *CombinePixelShader, sizeof(FFilterVertex));

	//sample the LDR translucency texture
	SetTextureParameter(
		Context,
		CombinePixelShader->GetPixelShader(),
		CombinePixelShader->LDRTranslucencyTextureParameter,
		TStaticSamplerState<SF_Nearest>::GetRHI(),
		GSceneRenderTargets.GetSceneColorLDRTexture()
		);

	RHISetRasterizerState(GlobalContext,TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	//opaque, blending will be done in the shader since there is no floating point blending support on this path
	RHISetBlendState(Context,TStaticBlendState<>::GetRHI());

	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		SCOPED_DRAW_EVENT(CombineEventView)(DEC_SCENE_ITEMS,TEXT("CombineView%d"),ViewIndex);

		FViewInfo& View = Views(ViewIndex);

		RHISetViewParameters( GlobalContext, &View, View.ViewProjectionMatrix, View.ViewOrigin );
		CombinePixelShader->SceneTextureParameters.Set(Context,&View,*CombinePixelShader);

		const UINT SceneColorBufferSizeX = GSceneRenderTargets.GetBufferSizeX();
		const UINT SceneColorBufferSizeY = GSceneRenderTargets.GetBufferSizeY();

		DrawDenormalizedQuad(
			Context,
			View.RenderTargetX,View.RenderTargetY,
			View.RenderTargetSizeX,View.RenderTargetSizeY,
			1,1,
			View.RenderTargetSizeX,View.RenderTargetSizeY,
			SceneColorBufferSizeX,SceneColorBufferSizeY,
			SceneColorBufferSizeX,SceneColorBufferSizeY
			);
	}

	// Resolve the scene color buffer since it has been dirtied.
	GSceneRenderTargets.FinishRenderingSceneColor();

}
