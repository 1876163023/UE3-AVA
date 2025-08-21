/*=============================================================================
	EmissiveRendering.h: Emissive rendering definitions.
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "LightMapRendering.h"

/**
 * A vertex shader for rendering the emissive color, and light-mapped/ambient lighting of a mesh.
 */
template<class LightMapPolicyType>
class TEmissiveVertexShader : public FShader, public LightMapPolicyType::VertexParametersType
{
	DECLARE_SHADER_TYPE(TEmissiveVertexShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return	!IsTranslucentBlendMode(Material->GetBlendMode()) &&
				LightMapPolicyType::ShouldCache(Platform,Material,VertexFactoryType);
	}

	TEmissiveVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer),
		VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{
		LightMapPolicyType::VertexParametersType::Bind(Initializer.ParameterMap);
	}
	TEmissiveVertexShader() {}

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
	}

	void SetParameters(FCommandContextRHI* Context,const FVertexFactory* VertexFactory,const FMaterialInstance* MaterialInstance,const FSceneView* View)
	{
		VertexFactoryParameters.Set(Context,this,VertexFactory,View);		
	}

	void SetLocalTransforms(FCommandContextRHI* Context,const FMatrix& LocalToWorld,const FMatrix& WorldToLocal)
	{
		VertexFactoryParameters.SetLocalTransforms(Context,this,LocalToWorld,WorldToLocal);
	}

private:
	FVertexFactoryParameterRef VertexFactoryParameters;	
};

/**
 * A pixel shader for rendering the emissive color, and light-mapped/ambient lighting of a mesh.
 */
template<class LightMapPolicyType>
class TEmissivePixelShader : public FShader, public LightMapPolicyType::PixelParametersType
{
	DECLARE_SHADER_TYPE(TEmissivePixelShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return	!IsTranslucentBlendMode(Material->GetBlendMode()) &&
				LightMapPolicyType::ShouldCache(Platform,Material,VertexFactoryType);
	}

	TEmissivePixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		LightMapPolicyType::PixelParametersType::Bind(Initializer.ParameterMap);
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
		UpperSkyColorParameter.Bind(Initializer.ParameterMap,TEXT("UpperSkyColor"),TRUE);
		LowerSkyColorParameter.Bind(Initializer.ParameterMap,TEXT("LowerSkyColor"),TRUE);
		//<@ ava specific ; 2007. 7. 4 changmin
		// hdr integer path
		TonemapTextureParameter.Bind(Initializer.ParameterMap,TEXT("TonemapTexture"),TRUE);
		TonemapRangeParameter.Bind(Initializer.ParameterMap,TEXT("TonemapRange"),TRUE);
		//>@ ava
	}
	TEmissivePixelShader() {}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_LIGHTMAP_COEFFICIENTS"),*FString::Printf(TEXT("%u"),NUM_LIGHTMAP_COEFFICIENTS));
		LightMapPolicyType::ModifyCompilationEnvironment(OutEnvironment);
	}

	void SetParameters(FCommandContextRHI* Context,const FVertexFactory* VertexFactory,const FMaterialInstance* MaterialInstance,const FSceneView* View)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialInstance, View->Family->CurrentWorldTime, View->Family->CurrentRealTime, View);
		MaterialParameters.Set(Context,this,MaterialRenderContext);

		//<@ ava specific ; 2007. 7. 4 changmin
		// hdr integer path
		FSceneViewState *ViewState = (FSceneViewState*)(View->State);
		if( TonemapTextureParameter.IsBound() && TonemapRangeParameter.IsBound() && ViewState )
		{
			const FExposureData& ExposureData = ViewState->ExposureData;
			SetTextureParameter(
				Context,
				GetPixelShader(),
				TonemapTextureParameter,
				TStaticSamplerState<SF_Linear>::GetRHI(),
				ExposureData.LuminanceHistogram.TonemapTexture );

			SetPixelShaderValue(
				Context,
				GetPixelShader(),
				TonemapRangeParameter,
				FLinearColor(
					ExposureData.LuminanceHistogram.WorldLuminaceScale / (ExposureData.LuminanceHistogram.WorldLuminaceMax +1e-6f),
					0,
					0,
					View->Opacity)
				);
		}
		//>@ ava
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
		Ar << MaterialParameters;
		Ar << AmbientColorAndSkyFactorParameter;
		Ar << UpperSkyColorParameter;
		Ar << LowerSkyColorParameter;

		//<@ ava specific ; 2007. 7. 4 changmin
		Ar << TonemapTextureParameter;
		Ar << TonemapRangeParameter;
		//>@ ava
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
	FShaderParameter AmbientColorAndSkyFactorParameter;
	FShaderParameter UpperSkyColorParameter;
	FShaderParameter LowerSkyColorParameter;
	//<@ ava specific ; 2007. 7. 4 changmin
	// hdr integer path
	FShaderParameter TonemapTextureParameter;
	FShaderParameter TonemapRangeParameter;
	//>@ ava
};

/**
 * Draws the emissive color and the light-map of a mesh.
 */
template<class LightMapPolicyType>
class TEmissiveDrawingPolicy : public FMeshDrawingPolicy
{
public:

	typedef typename LightMapPolicyType::ElementDataType ElementDataType;

	TEmissiveDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialInstance* InMaterialInstance,
		LightMapPolicyType InLightMapPolicy
		):
		FMeshDrawingPolicy(InVertexFactory,InMaterialInstance),
		LightMapPolicy(InLightMapPolicy)
	{
		const FMaterialShaderMap* MaterialShaderIndex = InMaterialInstance->GetMaterial()->GetShaderMap();
		const FMeshMaterialShaderMap* MeshShaderIndex = MaterialShaderIndex->GetMeshShaderMap(InVertexFactory->GetType());
		VertexShader = MeshShaderIndex->GetShader<TEmissiveVertexShader<LightMapPolicyType> >();
		PixelShader = MeshShaderIndex->GetShader<TEmissivePixelShader<LightMapPolicyType> >();
	}

	// FMeshDrawingPolicy interface.

	UBOOL Matches(const TEmissiveDrawingPolicy& Other) const
	{
		return FMeshDrawingPolicy::Matches(Other) &&
			VertexShader == Other.VertexShader &&
			PixelShader == Other.PixelShader &&
			LightMapPolicy == Other.LightMapPolicy;
	}

	void DrawShared(FCommandContextRHI* Context,const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
	{
		SCOPE_CYCLE_COUNTER(STAT_EmissiveDrawSharedTime);

		EnvCube_PrepareDraw();

		// Set the emissive shader parameters for the material.
		VertexShader->SetParameters(Context,VertexFactory,MaterialInstance,View);
		PixelShader->SetParameters(Context,VertexFactory,MaterialInstance,View);

		// Set the light-map policy.
		LightMapPolicy.Set(Context,VertexShader,PixelShader,PixelShader,VertexFactory,MaterialInstance,View);

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

		LightMapPolicy.GetVertexDeclarationInfo(VertexDeclaration, StreamStrides, VertexFactory);
		if (DynamicStride)
		{
			StreamStrides[0] = DynamicStride;
		}

		return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, VertexShader->GetVertexShader(), PixelShader->GetPixelShader());
	}

	void SetMeshRenderState(
		FCommandContextRHI* Context,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		const ElementDataType& ElementData
		) const
	{
		// Set the light-map policy's mesh-specific settings.
		LightMapPolicy.SetMesh(Context,VertexShader,PixelShader,VertexShader,PixelShader,VertexFactory,MaterialInstance,ElementData);

		VertexShader->SetLocalTransforms(Context,Mesh.LocalToWorld,Mesh.WorldToLocal);
		PixelShader->SetLocalTransforms(Context,Mesh.MaterialInstance,Mesh.LocalToWorld,bBackFace);
		if(PrimitiveSceneInfo)
		{
			PixelShader->SetSkyColor(Context,PrimitiveSceneInfo->UpperSkyLightColor,PrimitiveSceneInfo->LowerSkyLightColor);
		}
		else
		{
			PixelShader->SetSkyColor(Context,FLinearColor::Black,FLinearColor::Black);
		}
		/* EnvCube */
		if (PrimitiveSceneInfo != NULL && GEnvCube_IsRequired)			
			EnvCube_Bind(Context, PixelShader->GetPixelShader(), PrimitiveSceneInfo );
		/* EnvCube */
		FMeshDrawingPolicy::SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FMeshDrawingPolicy::ElementDataType());
	}

	friend INT Compare(const TEmissiveDrawingPolicy& A,const TEmissiveDrawingPolicy& B)
	{
		COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
		COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
		COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
		COMPAREDRAWINGPOLICYMEMBERS(MaterialInstance);
		return Compare(A.LightMapPolicy,B.LightMapPolicy);
	}

private:
	TEmissiveVertexShader<LightMapPolicyType>* VertexShader;
	TEmissivePixelShader<LightMapPolicyType>* PixelShader;

	LightMapPolicyType LightMapPolicy;
};

/**
 * A drawing policy factory for the emissive drawing policy.
 */
class FEmissiveDrawingPolicyFactory
{
public:

	enum { bAllowSimpleElements = TRUE };
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

	static UBOOL IsMaterialIgnored(const FMaterialInstance* MaterialInstance)
	{
		// Ignore non-opaque materials in the opaque base pass.
		return MaterialInstance && IsTranslucentBlendMode(MaterialInstance->GetMaterial()->GetBlendMode());
	}
};



/**
* A pixel shader for rendering the emissive color, and light-mapped/ambient lighting of a mesh.
*/
class TUnlitPixelShader : public FShader, public FNoLightMapPolicy::PixelParametersType
{
	DECLARE_SHADER_TYPE(TUnlitPixelShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
#if FINAL_RELEASE
		return FALSE;
#else
		return !IsTranslucentBlendMode(Material->GetBlendMode()) &&
			FNoLightMapPolicy::ShouldCache(Platform,Material,VertexFactoryType);
#endif
	}

	TUnlitPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FShader(Initializer)
	{
		FNoLightMapPolicy::PixelParametersType::Bind(Initializer.ParameterMap);
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
		AmbientColorAndSkyFactorParameter.Bind(Initializer.ParameterMap,TEXT("AmbientColorAndSkyFactor"),TRUE);
		UpperSkyColorParameter.Bind(Initializer.ParameterMap,TEXT("UpperSkyColor"),TRUE);
		LowerSkyColorParameter.Bind(Initializer.ParameterMap,TEXT("LowerSkyColor"),TRUE);		
	}
	TUnlitPixelShader() {}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_LIGHTMAP_COEFFICIENTS"),*FString::Printf(TEXT("%u"),NUM_LIGHTMAP_COEFFICIENTS));
		FNoLightMapPolicy::ModifyCompilationEnvironment(OutEnvironment);
	}

	void SetParameters(FCommandContextRHI* Context,const FVertexFactory* VertexFactory,const FMaterialInstance* MaterialInstance,const FSceneView* View)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialInstance, View->Family->CurrentWorldTime, View->Family->CurrentRealTime, View);
		MaterialParameters.Set(Context,this,MaterialRenderContext);
		SetPixelShaderValue(
			Context,
			GetPixelShader(),
			AmbientColorAndSkyFactorParameter,
			(View->Family->ShowFlags & SHOW_Lighting) ? FLinearColor(0,0,0,1) : FLinearColor(1,1,1,0)
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
		FNoLightMapPolicy::PixelParametersType::Serialize(Ar);
		Ar << MaterialParameters;
		Ar << AmbientColorAndSkyFactorParameter;
		Ar << UpperSkyColorParameter;
		Ar << LowerSkyColorParameter;		
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
	FShaderParameter AmbientColorAndSkyFactorParameter;
	FShaderParameter UpperSkyColorParameter;
	FShaderParameter LowerSkyColorParameter;	
};

/**
* Draws the emissive color and the light-map of a mesh.
*/
class TUnlitDrawingPolicy : public FMeshDrawingPolicy
{
public:

	typedef FNoLightMapPolicy::ElementDataType ElementDataType;

	TUnlitDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialInstance* InMaterialInstance,
		FNoLightMapPolicy InLightMapPolicy
		):
	FMeshDrawingPolicy(InVertexFactory,InMaterialInstance),
		LightMapPolicy(InLightMapPolicy)
	{
		const FMaterialShaderMap* MaterialShaderIndex = InMaterialInstance->GetMaterial()->GetShaderMap();
		const FMeshMaterialShaderMap* MeshShaderIndex = MaterialShaderIndex->GetMeshShaderMap(InVertexFactory->GetType());
		VertexShader = MeshShaderIndex->GetShader<TEmissiveVertexShader<FNoLightMapPolicy> >();
		PixelShader = MeshShaderIndex->GetShader<TUnlitPixelShader>();
	}

	// FMeshDrawingPolicy interface.

	UBOOL Matches(const TUnlitDrawingPolicy& Other) const
	{
		return FMeshDrawingPolicy::Matches(Other) &&
			VertexShader == Other.VertexShader &&
			PixelShader == Other.PixelShader &&
			LightMapPolicy == Other.LightMapPolicy;
	}

	void DrawShared(FCommandContextRHI* Context,const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
	{
		SCOPE_CYCLE_COUNTER(STAT_EmissiveDrawSharedTime);

		EnvCube_PrepareDraw();

		// Set the emissive shader parameters for the material.
		VertexShader->SetParameters(Context,VertexFactory,MaterialInstance,View);
		PixelShader->SetParameters(Context,VertexFactory,MaterialInstance,View);

		// Set the light-map policy.
		LightMapPolicy.Set(Context,VertexShader,PixelShader,PixelShader,VertexFactory,MaterialInstance,View);

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

		LightMapPolicy.GetVertexDeclarationInfo(VertexDeclaration, StreamStrides, VertexFactory);
		if (DynamicStride)
		{
			StreamStrides[0] = DynamicStride;
		}

		return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, VertexShader->GetVertexShader(), PixelShader->GetPixelShader());
	}

	void SetMeshRenderState(
		FCommandContextRHI* Context,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		const ElementDataType& ElementData
		) const
	{
		// Set the light-map policy's mesh-specific settings.
		LightMapPolicy.SetMesh(Context,VertexShader,PixelShader,VertexShader,PixelShader,VertexFactory,MaterialInstance,ElementData);

		VertexShader->SetLocalTransforms(Context,Mesh.LocalToWorld,Mesh.WorldToLocal);
		PixelShader->SetLocalTransforms(Context,Mesh.MaterialInstance,Mesh.LocalToWorld,bBackFace);
		if(PrimitiveSceneInfo)
		{
			PixelShader->SetSkyColor(Context,PrimitiveSceneInfo->UpperSkyLightColor,PrimitiveSceneInfo->LowerSkyLightColor);
		}
		else
		{
			PixelShader->SetSkyColor(Context,FLinearColor::Black,FLinearColor::Black);
		}
		/* EnvCube */
		if (PrimitiveSceneInfo != NULL && GEnvCube_IsRequired)			
			EnvCube_Bind(Context, PixelShader->GetPixelShader(), PrimitiveSceneInfo );
		/* EnvCube */
		FMeshDrawingPolicy::SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FMeshDrawingPolicy::ElementDataType());
	}

	friend INT Compare(const TUnlitDrawingPolicy& A,const TUnlitDrawingPolicy& B)
	{
		COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
		COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
		COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
		COMPAREDRAWINGPOLICYMEMBERS(MaterialInstance);
		return Compare(A.LightMapPolicy,B.LightMapPolicy);
	}

private:
	TEmissiveVertexShader<FNoLightMapPolicy>* VertexShader;
	TUnlitPixelShader* PixelShader;

	FNoLightMapPolicy LightMapPolicy;
};

/**
* A drawing policy factory for the emissive drawing policy.
*/
class FUnlitDrawingPolicyFactory
{
public:

	enum { bAllowSimpleElements = TRUE };
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

	static UBOOL IsMaterialIgnored(const FMaterialInstance* MaterialInstance)
	{
		return FALSE;
	}
};