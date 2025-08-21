/*=============================================================================
	LightRendering.h: Light rendering definitions.
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 */
class FNullLightShaderComponent
{
public:
	void Bind(const FShaderParameterMap& ParameterMap)
	{}
	void Serialize(FArchive& Ar)
	{}
};

/**
 * A shadowing policy for TMeshLightingDrawingPolicy which uses no static shadowing.
 */
class FNoStaticShadowingPolicy
{
public:
	typedef FNullLightShaderComponent VertexParametersType;
	typedef FNullLightShaderComponent PixelParametersType;
	typedef FMeshDrawingPolicy::ElementDataType ElementDataType;

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
	}

	void Set(
		FCommandContextRHI* Context,
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialInstance* MaterialInstance,
		const FSceneView* View
		) const
	{
		check(VertexFactory);
		VertexFactory->Set(Context);
	}

	/**
	* Get the decl and stream strides for this policy type and vertexfactory
	* @param VertexDeclaration - output decl 
	* @param StreamStrides - output array of vertex stream strides 
	* @param VertexFactory - factory to be used by this policy
	*/
	void GetVertexDeclarationInfo(FVertexDeclarationRHIParamRef &VertexDeclaration, DWORD *StreamStrides, const FVertexFactory* VertexFactory)
	{
		check(VertexFactory);
		VertexFactory->GetStreamStrides(StreamStrides);
		VertexDeclaration = VertexFactory->GetDeclaration();
	}

	void SetMesh(
		FCommandContextRHI* Context,
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* VertexShader,
		FShader* PixelShader,
		const FMeshElement& Mesh,
		const ElementDataType& ElementData
		) const
	{}
	friend UBOOL operator==(const FNoStaticShadowingPolicy A,const FNoStaticShadowingPolicy B)
	{
		return TRUE;
	}
	friend INT Compare(const FNoStaticShadowingPolicy&,const FNoStaticShadowingPolicy&)
	{
		return 0;
	}
};

/**
 * A shadowing policy for TMeshLightingDrawingPolicy which uses a 2D shadow texture.
 */
class FShadowTexturePolicy
{
public:

	class VertexParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
			ShadowCoordinateScaleBiasParameter.Bind(ParameterMap,TEXT("ShadowCoordinateScaleBias"),TRUE);
		}
		void SetCoordinateTransform(FCommandContextRHI* Context,FShader* VertexShader,const FVector2D& ShadowCoordinateScale,const FVector2D& ShadowCoordinateBias) const
		{
			SetVertexShaderValue(Context,VertexShader->GetVertexShader(),ShadowCoordinateScaleBiasParameter,FVector4(
				ShadowCoordinateScale.X,
				ShadowCoordinateScale.Y,
				ShadowCoordinateBias.Y,
				ShadowCoordinateBias.X
				));
		}
		void Serialize(FArchive& Ar)
		{
			Ar << ShadowCoordinateScaleBiasParameter;
		}
	private:
		FShaderParameter ShadowCoordinateScaleBiasParameter;
	};

	class PixelParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
			ShadowTextureParameter.Bind(ParameterMap,TEXT("ShadowTexture"),TRUE);
		}
		void SetShadowTexture(FCommandContextRHI* Context,FShader* PixelShader,const FTexture* ShadowTexture) const
		{
			SetTextureParameter(Context,PixelShader->GetPixelShader(),ShadowTextureParameter,ShadowTexture);
		}
		void Serialize(FArchive& Ar)
		{
			Ar << ShadowTextureParameter;
		}
	private:
		FShaderParameter ShadowTextureParameter;
	};

	struct ElementDataType : public FMeshDrawingPolicy::ElementDataType
	{
		FVector2D ShadowCoordinateScale;
		FVector2D ShadowCoordinateBias;

		/** Default constructor. */
		ElementDataType() {}

		/** Initialization constructor. */
		ElementDataType(const FVector2D& InShadowCoordinateScale,const FVector2D& InShadowCoordinateBias):
			ShadowCoordinateScale(InShadowCoordinateScale),
			ShadowCoordinateBias(InShadowCoordinateBias)
		{}
	};

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return VertexFactoryType->SupportsStaticLighting();
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("STATICLIGHTING_TEXTUREMASK"),TEXT("1"));
	}

	FShadowTexturePolicy(const UTexture2D* InTexture):
		Texture(InTexture)
	{}

	// FShadowingPolicyInterface.
	void Set(
		FCommandContextRHI* Context,
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialInstance* MaterialInstance,
		const FSceneView* View
		) const
	{
		check(VertexFactory);
		VertexFactory->Set(Context);
		PixelShaderParameters->SetShadowTexture(Context,PixelShader,Texture->Resource);
	}

	/**
	* Get the decl and stream strides for this policy type and vertexfactory
	* @param VertexDeclaration - output decl 
	* @param StreamStrides - output array of vertex stream strides 
	* @param VertexFactory - factory to be used by this policy
	*/
	void GetVertexDeclarationInfo(FVertexDeclarationRHIParamRef &VertexDeclaration, DWORD *StreamStrides, const FVertexFactory* VertexFactory)
	{
		check(VertexFactory);
		VertexFactory->GetStreamStrides(StreamStrides);
		VertexDeclaration = VertexFactory->GetDeclaration();
	}

	void SetMesh(
		FCommandContextRHI* Context,
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* VertexShader,
		FShader* PixelShader,
		const FMeshElement& Mesh,
		const ElementDataType& ElementData
		) const
	{
		VertexShaderParameters->SetCoordinateTransform(
			Context,
			VertexShader,
			ElementData.ShadowCoordinateScale,
			ElementData.ShadowCoordinateBias
			);
	}
	friend UBOOL operator==(const FShadowTexturePolicy A,const FShadowTexturePolicy B)
	{
		return A.Texture == B.Texture;
	}
	friend INT Compare(const FShadowTexturePolicy& A,const FShadowTexturePolicy& B)
	{
		COMPAREDRAWINGPOLICYMEMBERS(Texture);
		return 0;
	}

private:
	const UTexture2D* Texture;
};

/**
 * A shadowing policy for TMeshLightingDrawingPolicy which uses a shadow vertex buffer.
 */
class FShadowVertexBufferPolicy
{
public:

	typedef FNullLightShaderComponent VertexParametersType;
	typedef FNullLightShaderComponent PixelParametersType;
	typedef FMeshDrawingPolicy::ElementDataType ElementDataType;

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return VertexFactoryType->SupportsStaticLighting();
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("STATICLIGHTING_VERTEXMASK"),TEXT("1"));
	}

	FShadowVertexBufferPolicy(const FVertexBuffer* InVertexBuffer):
		VertexBuffer(InVertexBuffer)
	{}

	void Set(
		FCommandContextRHI* Context,
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialInstance* MaterialInstance,
		const FSceneView* View
		) const
	{
		check(VertexFactory);
		VertexFactory->SetVertexShadowMap(Context,VertexBuffer);
	}

	/**
	* Get the decl and stream strides for this policy type and vertexfactory
	* @param VertexDeclaration - output decl 
	* @param StreamStrides - output array of vertex stream strides 
	* @param VertexFactory - factory to be used by this policy
	*/
	void GetVertexDeclarationInfo(FVertexDeclarationRHIParamRef &VertexDeclaration, DWORD *StreamStrides, const FVertexFactory* VertexFactory)
	{
		check(VertexFactory);
		VertexFactory->GetVertexShadowMapStreamStrides(StreamStrides);
		VertexDeclaration = VertexFactory->GetVertexShadowMapDeclaration();
	}

	void SetMesh(
		FCommandContextRHI* Context,
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* VertexShader,
		FShader* PixelShader,
		const FMeshElement& Mesh,
		const ElementDataType& ElementData
		) const
	{}
	friend UBOOL operator==(const FShadowVertexBufferPolicy A,const FShadowVertexBufferPolicy B)
	{
		return A.VertexBuffer == B.VertexBuffer;
	}
	friend INT Compare(const FShadowVertexBufferPolicy& A,const FShadowVertexBufferPolicy& B)
	{
		COMPAREDRAWINGPOLICYMEMBERS(VertexBuffer);
		return 0;
	}

private:
	const FVertexBuffer* VertexBuffer;
};

/**
 * The pixel shader used to draw the effect of a light on a mesh.
 */
template<typename LightTypePolicy,typename ShadowingTypePolicy>
class TLightPixelShader :
	public FShader,
	public LightTypePolicy::PixelParametersType,
	public ShadowingTypePolicy::PixelParametersType
{
	DECLARE_SHADER_TYPE(TLightPixelShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return	!IsTranslucentBlendMode(Material->GetBlendMode()) &&
				Material->GetLightingModel() != MLM_Unlit &&
				ShadowingTypePolicy::ShouldCache(Platform,Material,VertexFactoryType);
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		ShadowingTypePolicy::ModifyCompilationEnvironment(OutEnvironment);
	}

	TLightPixelShader() {}

	TLightPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		LightTypePolicy::PixelParametersType::Bind(Initializer.ParameterMap);
		ShadowingTypePolicy::PixelParametersType::Bind(Initializer.ParameterMap);
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
		LightAttenuationTextureParameter.Bind(Initializer.ParameterMap,TEXT("LightAttenuationTexture"),TRUE);

		//<@ ava specific ; 2007. 7. 4 changmin
		// hdr integer path
		TonemapTextureParameter.Bind(Initializer.ParameterMap,TEXT("TonemapTexture"),TRUE);
		TonemapRangeParameter.Bind(Initializer.ParameterMap,TEXT("TonemapRange"),TRUE);
		//>@ ava
	}

	void SetParameters(FCommandContextRHI* Context,const FMaterialInstance* MaterialInstance,const FSceneView* View)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialInstance, View->Family->CurrentWorldTime, View->Family->CurrentRealTime, View);
		MaterialParameters.Set(Context,this,MaterialRenderContext);

		if(LightAttenuationTextureParameter.IsBound())
		{
			SetTextureParameter(
				Context,
				GetPixelShader(),
				LightAttenuationTextureParameter,
				TStaticSamplerState<SF_Nearest,AM_Wrap,AM_Wrap,AM_Wrap>::GetRHI(),
				GSceneRenderTargets.GetEffectiveLightAttenuationTexture()
				);
		}

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

	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		LightTypePolicy::PixelParametersType::Serialize(Ar);
		ShadowingTypePolicy::PixelParametersType::Serialize(Ar);
		Ar << MaterialParameters;
		Ar << LightAttenuationTextureParameter;

		//removed duplicated parameter
		if (Ar.Ver() < VER_FP_BLENDING_FALLBACK)
		{
			FShaderParameter DummyScreenPositionScaleBiasParameter;
			Ar << DummyScreenPositionScaleBiasParameter;
		}

		//<@ ava specific ; 2007. 7. 4 changmin
		Ar << TonemapTextureParameter;
		Ar << TonemapRangeParameter;
		//>@ ava
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
	FShaderParameter LightAttenuationTextureParameter;

	//<@ ava specific ; 2007. 7. 4 changmin
	// hdr integer path
	FShaderParameter TonemapTextureParameter;
	FShaderParameter TonemapRangeParameter;
	//>@ ava
};

/**
 * The vertex shader used to draw the effect of a light on a mesh.
 */
template<typename LightTypePolicy,typename ShadowingTypePolicy>
class TLightVertexShader :
	public FShader,
	public LightTypePolicy::VertexParametersType,
	public ShadowingTypePolicy::VertexParametersType
{
	DECLARE_SHADER_TYPE(TLightVertexShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return	!IsTranslucentBlendMode(Material->GetBlendMode()) &&
				Material->GetLightingModel() != MLM_Unlit &&
				ShadowingTypePolicy::ShouldCache(Platform,Material,VertexFactoryType);
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		ShadowingTypePolicy::ModifyCompilationEnvironment(OutEnvironment);
	}

	TLightVertexShader() {}

	TLightVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer),
		VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{
		LightTypePolicy::VertexParametersType::Bind(Initializer.ParameterMap);
		ShadowingTypePolicy::VertexParametersType::Bind(Initializer.ParameterMap);		
	}

	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		LightTypePolicy::VertexParametersType::Serialize(Ar);
		ShadowingTypePolicy::VertexParametersType::Serialize(Ar);
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
* Implements the vertex shader and 2 versions of the pixel shader for a given light type.  
* One pixel shader is used when floating point blending is supported, the other does blending in the shader.
*/
#define IMPLEMENT_LIGHTSHADOWING_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,VertexShaderFilename,PixelShaderFilename,ShadowingPolicyType,MinPackageVersion,MinLicenseePackageVersion) \
	typedef TLightVertexShader<LightPolicyType,ShadowingPolicyType> TLightVertexShader##LightPolicyFriendlyName##ShadowingPolicyType; \
	IMPLEMENT_MATERIAL_SHADER_TYPE( \
		template<>, \
		TLightVertexShader##LightPolicyFriendlyName##ShadowingPolicyType, \
		VertexShaderFilename, \
		TEXT("Main"), \
		SF_Vertex, \
		MinPackageVersion, \
		MinLicenseePackageVersion \
	); \
	typedef TLightPixelShader<LightPolicyType,ShadowingPolicyType> TLightPixelShader##LightPolicyFriendlyName##ShadowingPolicyType##TRUE; \
	IMPLEMENT_MATERIAL_SHADER_TYPE( \
		template<>, \
		TLightPixelShader##LightPolicyFriendlyName##ShadowingPolicyType##TRUE, \
		PixelShaderFilename, \
		TEXT("Main"), \
		SF_Pixel, \
		MinPackageVersion, \
		MinLicenseePackageVersion \
	);

//<@ ava specific ; 2007. 10. 19 changmin
class FBumpLightingShadowingPolicy
{
public:

	typedef FNullLightShaderComponent VertexParametersType;
	class PixelParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
			DiffuseSpecularScaleParameter.Bind(ParameterMap, TEXT("DiffuseSpecularScale"), TRUE);
		}
		void Set(FCommandContextRHI *Context, FShader *PixelShader) const
		{
			if(DiffuseSpecularScaleParameter.IsBound())
			{
				extern INT GBumpState;
				switch(GBumpState)
				{
				case 1:	// diffuse
					SetPixelShaderValue(Context, PixelShader->GetPixelShader(), DiffuseSpecularScaleParameter, FVector2D(1.0f, 0.0f));
					break;
				case 2:	// specular
					SetPixelShaderValue(Context, PixelShader->GetPixelShader(), DiffuseSpecularScaleParameter, FVector2D(0.0f, 1.0f));
					break;
				case 3:	// specular
					SetPixelShaderValue(Context, PixelShader->GetPixelShader(), DiffuseSpecularScaleParameter, FVector2D(0.0f, 0.0f));
					break;
				default:
					SetPixelShaderValue(Context, PixelShader->GetPixelShader(), DiffuseSpecularScaleParameter, FVector2D(1.0f, 1.0f));
					break;
				}
			}
		}
		void Serialize(FArchive& Ar)
		{
			Ar << DiffuseSpecularScaleParameter;
		}
	private:
		FShaderParameter DiffuseSpecularScaleParameter;
	};
	typedef FMeshDrawingPolicy::ElementDataType ElementDataType;

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
	}

	void Set(
		FCommandContextRHI* Context,
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialInstance* MaterialInstance,
		const FSceneView* View
		) const
	{
		check(VertexFactory);
		VertexFactory->Set(Context);
		PixelShaderParameters->Set(Context, PixelShader);
	}

	/**
	* Get the decl and stream strides for this policy type and vertexfactory
	* @param VertexDeclaration - output decl 
	* @param StreamStrides - output array of vertex stream strides 
	* @param VertexFactory - factory to be used by this policy
	*/
	void GetVertexDeclarationInfo(FVertexDeclarationRHIParamRef &VertexDeclaration, DWORD *StreamStrides, const FVertexFactory* VertexFactory)
	{
		check(VertexFactory);
		VertexFactory->GetStreamStrides(StreamStrides);
		VertexDeclaration = VertexFactory->GetDeclaration();
	}

	void SetMesh(
		FCommandContextRHI* Context,
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* VertexShader,
		FShader* PixelShader,
		const FMeshElement& Mesh,
		const ElementDataType& ElementData
		) const
	{}
	friend UBOOL operator==(const FBumpLightingShadowingPolicy A,const FBumpLightingShadowingPolicy B)
	{
		return TRUE;
	}
	friend INT Compare(const FBumpLightingShadowingPolicy&,const FBumpLightingShadowingPolicy&)
	{
		return 0;
	}
};
#define IMPLEMENT_BUMPLIGHT_SHADER_TYPE(LightPolicyType, LightPolicyFriendlyName, VertexShaderFilename,PixelShaderFilename,MinPackageVersion,MinLicenseePackageVersion) \
	typedef TLightVertexShader<LightPolicyType,FBumpLightingShadowingPolicy> TLightVertexShader##LightPolicyFriendlyName; \
	IMPLEMENT_MATERIAL_SHADER_TYPE( \
	template<>, \
	TLightVertexShader##LightPolicyFriendlyName, \
	VertexShaderFilename, \
	TEXT("Main"), \
	SF_Vertex, \
	MinPackageVersion, \
	MinLicenseePackageVersion \
	); \
	typedef TLightPixelShader<LightPolicyType,FBumpLightingShadowingPolicy> TLightPixelShader##LightPolicyFriendlyName; \
	IMPLEMENT_MATERIAL_SHADER_TYPE( \
	template<>, \
	TLightPixelShader##LightPolicyFriendlyName, \
	PixelShaderFilename, \
	TEXT("Main"), \
	SF_Pixel, \
	MinPackageVersion, \
	MinLicenseePackageVersion \
	);
//>@ ava

/** 
* Implements a version of TBranchingPCFModProjectionPixelShader
*/
#define IMPLEMENT_LIGHT_BPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,EntryFunctionName,BranchingPCFPolicy,MinPackageVersion,MinLicenseePackageVersion) \
	typedef TBranchingPCFModProjectionPixelShader<LightPolicyType,BranchingPCFPolicy> TBranchingPCFModProjectionPixelShader##LightPolicyFriendlyName##BranchingPCFPolicy; \
	IMPLEMENT_SHADER_TYPE( \
		template<>, \
		TBranchingPCFModProjectionPixelShader##LightPolicyFriendlyName##BranchingPCFPolicy, \
		TEXT("BranchingPCFModProjectionPixelShader"), \
		EntryFunctionName, \
		SF_Pixel, \
		MinPackageVersion, \
		MinLicenseePackageVersion \
	);

/** 
* Implements a version of TModShadowVolumePixelShader
*/
#define IMPLEMENT_LIGHT_VOLUME_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,EntryFunctionName,MinPackageVersion,MinLicenseePackageVersion) \
	typedef TModShadowVolumePixelShader<LightPolicyType> TModShadowVolumePixelShader##LightPolicyFriendlyName; \
	IMPLEMENT_SHADER_TYPE( \
		template<>, \
		TModShadowVolumePixelShader##LightPolicyFriendlyName, \
		TEXT("ModShadowVolumePixelShader"), \
		EntryFunctionName, \
		SF_Pixel, \
		MinPackageVersion, \
		MinLicenseePackageVersion \
	);

/** 
* Implements a version of TModShadowProjectionPixelShader
*/
#define IMPLEMENT_LIGHT_UNIFORMPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,EntryFunctionName,UniformPCFPolicy,MinPackageVersion,MinLicenseePackageVersion) \
	typedef TModShadowProjectionPixelShader<LightPolicyType,UniformPCFPolicy> TModShadowProjectionPixelShader##LightPolicyFriendlyName##UniformPCFPolicy; \
	IMPLEMENT_SHADER_TYPE( \
		template<>, \
		TModShadowProjectionPixelShader##LightPolicyFriendlyName##UniformPCFPolicy, \
		TEXT("ModShadowProjectionPixelShader"), \
		EntryFunctionName, \
		SF_Pixel, \
		MinPackageVersion, \
		MinLicenseePackageVersion \
	);
/**
* Implements all of the shader types which must be compiled for a particular light policy type. 
*
* A IMPLEMENT_LIGHTSHADOWING_SHADER_TYPE for each static shadowing policy
* A IMPLEMENT_LIGHT_VOLUME_SHADER_TYPE, which is the shadow volume pixel shader used with modulative shadows
* 4 IMPLEMENT_LIGHT_UNIFORMPCF_SHADER_TYPE's for support for Hardware PCF and a fallback for SM2.
* 6 IMPLEMENT_LIGHT_BPCF_SHADER_TYPE's for Hardware PCF and 3 quality levels.
*/

#define IMPLEMENT_LIGHT_SHADER_TYPE_WITH_FRIENDLY_NAME(LightPolicyType,LightPolicyFriendlyName,VertexShaderFilename,PixelShaderFilename,MinPackageVersion,MinLicenseePackageVersion) \
	IMPLEMENT_LIGHTSHADOWING_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,VertexShaderFilename,PixelShaderFilename,FNoStaticShadowingPolicy,Max(VER_EPIC_MERGE_APR_6,MinPackageVersion),MinLicenseePackageVersion) \
	IMPLEMENT_LIGHTSHADOWING_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,VertexShaderFilename,PixelShaderFilename,FShadowTexturePolicy,Max(VER_EPIC_MERGE_APR_6,MinPackageVersion),MinLicenseePackageVersion) \
	IMPLEMENT_LIGHTSHADOWING_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,VertexShaderFilename,PixelShaderFilename,FShadowVertexBufferPolicy,Max(VER_EPIC_MERGE_APR_6,MinPackageVersion),MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_VOLUME_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("Main"),Max(VER_FP_BLENDING_FALLBACK,MinPackageVersion),MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_UNIFORMPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("HardwarePCFMain"),F4SampleHwPCF,Max(VER_EPIC_MERGE_APR_6,MinPackageVersion),MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_UNIFORMPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("Main"),F4SampleManualPCF,Max(VER_EPIC_MERGE_APR_6,MinPackageVersion),MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_UNIFORMPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("HardwarePCFMain"),F16SampleHwPCF,Max(VER_EPIC_MERGE_APR_6,MinPackageVersion),MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_UNIFORMPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("Fetch4Main"),F16SampleFetch4PCF,MinPackageVersion,MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_UNIFORMPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("Main"),F16SampleManualPCF,Max(VER_EPIC_MERGE_APR_6,MinPackageVersion),MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_BPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("HardwarePCFMain"),FLowQualityHwPCF,Max(VER_EPIC_MERGE_APR_6,MinPackageVersion),MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_BPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("HardwarePCFMain"),FMediumQualityHwPCF,Max(VER_EPIC_MERGE_APR_6,MinPackageVersion),MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_BPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("HardwarePCFMain"),FHighQualityHwPCF,Max(VER_EPIC_MERGE_APR_6,MinPackageVersion),MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_BPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("Fetch4Main"),FLowQualityFetch4PCF,MinPackageVersion,MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_BPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("Fetch4Main"),FMediumQualityFetch4PCF,MinPackageVersion,MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_BPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("Fetch4Main"),FHighQualityFetch4PCF,MinPackageVersion,MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_BPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("Main"),FLowQualityManualPCF,Max(VER_EPIC_MERGE_APR_6,MinPackageVersion),MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_BPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("Main"),FMediumQualityManualPCF,Max(VER_EPIC_MERGE_APR_6,MinPackageVersion),MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_BPCF_SHADER_TYPE(LightPolicyType,LightPolicyFriendlyName,TEXT("Main"),FHighQualityManualPCF,Max(VER_EPIC_MERGE_APR_6,MinPackageVersion),MinLicenseePackageVersion) 

#define IMPLEMENT_LIGHT_SHADER_TYPE(LightPolicyType,VertexShaderFilename,PixelShaderFilename,MinPackageVersion,MinLicenseePackageVersion) \
	IMPLEMENT_LIGHT_SHADER_TYPE_WITH_FRIENDLY_NAME(LightPolicyType,LightPolicyType,VertexShaderFilename,PixelShaderFilename,MinPackageVersion,MinLicenseePackageVersion)

/**
 * Draws a light's interaction with a mesh.
 */
template<typename ShadowPolicyType,typename LightPolicyType>
class TMeshLightingDrawingPolicy : public FMeshDrawingPolicy
{
public:

	typedef typename ShadowPolicyType::ElementDataType ElementDataType;
	typedef typename LightPolicyType::SceneInfoType LightSceneInfoType;

	typedef TLightVertexShader<LightPolicyType,ShadowPolicyType> VertexShaderType;

	typedef TLightPixelShader<LightPolicyType,ShadowPolicyType> PixelShaderType;

	void FindShaders(const FVertexFactory* InVertexFactory,const FMaterialInstance* InMaterialInstance)
	{		
		// Find the shaders used to render this material/vertexfactory/light combination.
		FVertexFactoryType* VertexFactoryType = InVertexFactory->GetType();
		const FMaterialShaderMap* MaterialShaderIndex = InMaterialInstance->GetMaterial()->GetShaderMap();
		const FMeshMaterialShaderMap* MeshShaderIndex = MaterialShaderIndex->GetMeshShaderMap(VertexFactoryType);
		VertexShader = *TShaderMapRef<VertexShaderType>(MeshShaderIndex);
		PixelShader = *TShaderMapRef<PixelShaderType>(MeshShaderIndex);
	}

	/** Initialization constructor. */
	TMeshLightingDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialInstance* InMaterialInstance,
		const LightSceneInfoType* InLight,
		ShadowPolicyType InShadowPolicy
		):
		FMeshDrawingPolicy(InVertexFactory,InMaterialInstance),
		Light(InLight),
		ShadowPolicy(InShadowPolicy)
	{
		FindShaders(InVertexFactory,InMaterialInstance);
	}

	TMeshLightingDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialInstance* InMaterialInstance,
		const LightSceneInfoType* InLight
		):
		FMeshDrawingPolicy(InVertexFactory,InMaterialInstance),
		Light(InLight)
	{
		FindShaders(InVertexFactory,InMaterialInstance);
	}

	// FMeshDrawingPolicy interface.
	UBOOL Matches(const TMeshLightingDrawingPolicy& Other) const
	{
		return FMeshDrawingPolicy::Matches(Other) &&
			VertexShader == Other.VertexShader &&
			PixelShader == Other.PixelShader &&
			Light == Other.Light &&
			ShadowPolicy == Other.ShadowPolicy;
	}
	void SetMeshRenderState(
		FCommandContextRHI* Context,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		const ElementDataType& ElementData
		) const
	{
		

		ShadowPolicy.SetMesh(Context,VertexShader,PixelShader,VertexShader,PixelShader,Mesh,ElementData);
		PixelShader->SetLocalTransforms(Context,Mesh.MaterialInstance,Mesh.LocalToWorld,bBackFace);
		VertexShader->SetLocalTransforms(Context,Mesh.LocalToWorld,Mesh.WorldToLocal);
		
		FMeshDrawingPolicy::SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FMeshDrawingPolicy::ElementDataType());
	}

	void DrawShared(FCommandContextRHI* Context,const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
	{
		SCOPE_CYCLE_COUNTER(STAT_LightingDrawSharedTime);

		ShadowPolicy.Set(Context,VertexShader,PixelShader,PixelShader,VertexFactory,MaterialInstance,View);
		PixelShader->SetParameters(Context,MaterialInstance,View);
		PixelShader->SetLight(Context,PixelShader,Light);

		VertexShader->SetParameters(Context,VertexFactory,MaterialInstance,View);
		VertexShader->SetLight(Context,VertexShader,Light);
		

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

		ShadowPolicy.GetVertexDeclarationInfo(VertexDeclaration, StreamStrides, VertexFactory);
		if (DynamicStride)
		{
			StreamStrides[0] = DynamicStride;
		}

		return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, VertexShader->GetVertexShader(), PixelShader->GetPixelShader());
	}

	friend INT Compare(const TMeshLightingDrawingPolicy& A,const TMeshLightingDrawingPolicy& B)
	{
		COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
		COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
		COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
		COMPAREDRAWINGPOLICYMEMBERS(MaterialInstance);
		return Compare(A.ShadowPolicy,B.ShadowPolicy);
	}

private:
	VertexShaderType* VertexShader;
	PixelShaderType* PixelShader;

	const LightSceneInfoType* Light;
	ShadowPolicyType ShadowPolicy;
};



/** Information about a light's effect on a scene's DPG. */
template<class LightPolicyType>
class TLightSceneDPGInfo : public FLightSceneDPGInfoInterface
{
public:

	// FLightSceneDPGInfoInterface
	virtual UBOOL DrawStaticMeshes(
		FCommandContextRHI* Context,
		const FSceneView* View,
		const FBitArray& StaticMeshVisibilityMap
		) const
	{
		UBOOL bDirty = FALSE;
		bDirty |= NoStaticShadowingDrawList.DrawVisible(Context,View,StaticMeshVisibilityMap);
		bDirty |= ShadowTextureDrawList.DrawVisible(Context,View,StaticMeshVisibilityMap);
		bDirty |= ShadowVertexBufferDrawList.DrawVisible(Context,View,StaticMeshVisibilityMap);
		return bDirty;
	}

	//<@ ava specific ; 2007. 10. 18 changmin
	// add viewmode bumplighting
	// FLightSceneDPGInfoInterface
	virtual UBOOL DrawStaticMeshes(
		FCommandContextRHI* Context,
		const FSceneView* View,
		const FBitArray& StaticMeshVisibilityMap,
		const FLightSceneInfo *LightSceneInfo
		) const
	{
		UBOOL bDirty = FALSE;
		TMeshBumpLightingDrawingPolicyFactory<LightPolicyType> PolicyFactory(LightSceneInfo);
		bDirty |= NoStaticShadowingDrawList.DrawVisible(Context,View,StaticMeshVisibilityMap, PolicyFactory);
		bDirty |= ShadowTextureDrawList.DrawVisible(Context,View,StaticMeshVisibilityMap, PolicyFactory);
		bDirty |= ShadowVertexBufferDrawList.DrawVisible(Context,View,StaticMeshVisibilityMap, PolicyFactory);
		return bDirty;
	}
	//>@ ava

	virtual ELightInteractionType AttachStaticMesh(const FLightSceneInfo* LightSceneInfo,FStaticMesh* Mesh)
	{
		// Check for cached shadow data.
		FLightInteraction CachedInteraction = FLightInteraction::Uncached();
		if(Mesh->LCI)
		{
			CachedInteraction = Mesh->LCI->GetInteraction(LightSceneInfo);
		}

		// Add the mesh to the appropriate draw list for the cached shadow data type.
		switch(CachedInteraction.GetType())
		{
		case LIT_CachedShadowMap1D:
			{
				ShadowVertexBufferDrawList.AddMesh(
					Mesh,
					FShadowVertexBufferPolicy::ElementDataType(),
					TMeshLightingDrawingPolicy<FShadowVertexBufferPolicy,LightPolicyType>(
						Mesh->VertexFactory,
						Mesh->MaterialInstance,
						(typename LightPolicyType::SceneInfoType*)LightSceneInfo,
						FShadowVertexBufferPolicy(CachedInteraction.GetShadowVertexBuffer())
						)
					);
				break;
			}
		case LIT_CachedShadowMap2D:
			{
				ShadowTextureDrawList.AddMesh(
					Mesh,
					FShadowTexturePolicy::ElementDataType(
						CachedInteraction.GetShadowCoordinateScale(),
						CachedInteraction.GetShadowCoordinateBias()
						),
					TMeshLightingDrawingPolicy<FShadowTexturePolicy,LightPolicyType>(
						Mesh->VertexFactory,
						Mesh->MaterialInstance,
						(typename LightPolicyType::SceneInfoType*)LightSceneInfo,
						FShadowTexturePolicy(CachedInteraction.GetShadowTexture())
						)
					);
				break;
			}
		case LIT_Uncached:
			{
				NoStaticShadowingDrawList.AddMesh(
					Mesh,
					FNoStaticShadowingPolicy::ElementDataType(),
					TMeshLightingDrawingPolicy<FNoStaticShadowingPolicy,LightPolicyType>(
						Mesh->VertexFactory,
						Mesh->MaterialInstance,
						(typename LightPolicyType::SceneInfoType*)LightSceneInfo,
						FNoStaticShadowingPolicy()
						)
					);
				break;
			}
		case LIT_CachedIrrelevant:
		case LIT_CachedLightMap:
		default:
			break;
		};

		return CachedInteraction.GetType();
	}

	virtual UBOOL DrawDynamicMesh(
		FCommandContextRHI* Context,
		const FSceneView* View,
		const FLightSceneInfo* LightSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		) const
	{
		// Check for cached shadow data.
		FLightInteraction CachedInteraction = FLightInteraction::Uncached();
		if(Mesh.LCI)
		{
			CachedInteraction = Mesh.LCI->GetInteraction(LightSceneInfo);
		}

		// Add the mesh to the appropriate draw list for the cached shadow data type.
		switch(CachedInteraction.GetType())
		{
		case LIT_CachedShadowMap1D:
			{
				TMeshLightingDrawingPolicy<FShadowVertexBufferPolicy,LightPolicyType> DrawingPolicy(
					Mesh.VertexFactory,
					Mesh.MaterialInstance,
					(typename LightPolicyType::SceneInfoType*)LightSceneInfo,
					FShadowVertexBufferPolicy(CachedInteraction.GetShadowVertexBuffer())
					);

				if (!Mesh.bDrawnShared)
				{
					DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
				}
				DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FShadowVertexBufferPolicy::ElementDataType());
				DrawingPolicy.DrawMesh(Context,Mesh);
				return TRUE;
			}
		case LIT_CachedShadowMap2D:
			{
				TMeshLightingDrawingPolicy<FShadowTexturePolicy,LightPolicyType> DrawingPolicy(
					Mesh.VertexFactory,
					Mesh.MaterialInstance,
					(typename LightPolicyType::SceneInfoType*)LightSceneInfo,
					FShadowTexturePolicy(CachedInteraction.GetShadowTexture())
					);

				if (!Mesh.bDrawnShared)
				{
					DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
				}
				DrawingPolicy.SetMeshRenderState(
					Context,
					PrimitiveSceneInfo,
					Mesh,
					bBackFace,
					FShadowTexturePolicy::ElementDataType(
						CachedInteraction.GetShadowCoordinateScale(),
						CachedInteraction.GetShadowCoordinateBias()
						)
					);
				DrawingPolicy.DrawMesh(Context,Mesh);
				return TRUE;
			}
		case LIT_Uncached:
			{
				//<@ ava specific ; 2007. 10 .19 changmin
				if( View->Family->ShowFlags & SHOW_BumpLighting )
				{
					TMeshLightingDrawingPolicy<FBumpLightingShadowingPolicy,LightPolicyType> DrawingPolicy(
						Mesh.VertexFactory,
						Mesh.MaterialInstance,
						(typename LightPolicyType::SceneInfoType*)LightSceneInfo,
						FBumpLightingShadowingPolicy()
						);
					if(!Mesh.bDrawnShared)
					{
						DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
					}
					DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FBumpLightingShadowingPolicy::ElementDataType());
					DrawingPolicy.DrawMesh(Context,Mesh);
					return TRUE;
				}
				else
				//>@ ava specific 
				{
					TMeshLightingDrawingPolicy<FNoStaticShadowingPolicy,LightPolicyType> DrawingPolicy(
						Mesh.VertexFactory,
						Mesh.MaterialInstance,
						(typename LightPolicyType::SceneInfoType*)LightSceneInfo,
						FNoStaticShadowingPolicy()
						);
					if(!Mesh.bDrawnShared)
					{
						DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
					}
					DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FNoStaticShadowingPolicy::ElementDataType());
					DrawingPolicy.DrawMesh(Context,Mesh);
					return TRUE;
				}
			}
		case LIT_CachedIrrelevant:
		case LIT_CachedLightMap:
		default:
			return FALSE;
		};
	}

private:
	TStaticMeshDrawList<TMeshLightingDrawingPolicy<FNoStaticShadowingPolicy,LightPolicyType> > NoStaticShadowingDrawList;
	TStaticMeshDrawList<TMeshLightingDrawingPolicy<FShadowTexturePolicy,LightPolicyType> > ShadowTextureDrawList;
	TStaticMeshDrawList<TMeshLightingDrawingPolicy<FShadowVertexBufferPolicy,LightPolicyType> > ShadowVertexBufferDrawList;
};
