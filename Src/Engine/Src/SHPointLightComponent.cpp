/*=============================================================================
SHPointLightComponent.cpp: SHPointLightComponent implementation.
Copyright 2006 Red duck, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "LightRendering.h"
#include "PointLightSceneInfo.h"
#include "Half.h"

IMPLEMENT_CLASS(USHPointLightComponent);

#define USE_FP16_CUBE 1

class FIrradianceCubeTexture : public FTextureResource
{
public:
	FTextureCubeRHIRef TextureCubeRHI;

	virtual void ReleaseRHI()
	{
		FTextureResource::ReleaseRHI();
		
		TextureCubeRHI.Release();
	}

	// FResource interface.
	virtual void InitRHI()
	{
		DWORD CreationFlags=0;

		if (GSystemSettings->DiffuseCubeResolution > 0)
		{
#if USE_FP16_CUBE
			TextureCubeRHI = RHICreateTextureCube(GSystemSettings->DiffuseCubeResolution,PF_FloatRGBA,1,CreationFlags);
#else
			TextureCubeRHI = RHICreateTextureCube(GSystemSettings->DiffuseCubeResolution,PF_A8R8G8B8,1,CreationFlags);
#endif
			TextureRHI = TextureCubeRHI;

			// Create the sampler state RHI resource.
			FSamplerStateInitializerRHI SamplerStateInitializer =
			{
				SF_Linear,
				AM_Wrap,
				AM_Wrap,
				AM_Wrap
			};
			SamplerStateRHI = RHICreateSamplerState(SamplerStateInitializer);
		}		
	}

	/** Returns the width of the texture in pixels. */
	virtual UINT GetSizeX() const
	{
		return GSystemSettings->DiffuseCubeResolution;
	}

	/** Returns the height of the texture in pixels. */
	virtual UINT GetSizeY() const
	{
		return GSystemSettings->DiffuseCubeResolution;
	}

	#define MAX_HDR_OVERBRIGHT								16.0f
	static FColor EncodeRGBL( FLOAT R, FLOAT G, FLOAT B )
	{			
		FLOAT M = Max( Max( R, G ), B );
		if( M > 1.0f )
		{			
			return FColor( (BYTE)(0xff * (R / M)), (BYTE)(0xff * (G / M)), (BYTE)(0xff * (B / M)), (BYTE)(0xff * Min( M / MAX_HDR_OVERBRIGHT, 1.0f ) ));			
		}
		else
		{
			return FColor( (BYTE)(0xff * Min( R, 1.0f )), (BYTE)(0xff * Min( G, 1.0f )), (BYTE)(0xff * Min( B, 1.0f )), (BYTE)(0xff / MAX_HDR_OVERBRIGHT) );			
		}
	}

	/** Dot product operator. */
	static FLOAT Irr(const FSHVector& A,const FSHVector& B,FLOAT CosTheta)	
	{
		FLOAT Result = 0.0f;
		static const FLOAT K_A[MAX_SH_BASIS] = {3.141593f,2.094395f,2.094395f,2.094395f};

		for(INT BasisIndex = 0;BasisIndex < MAX_SH_BASIS;BasisIndex++)
		{
			Result += A.V[BasisIndex] * B.V[BasisIndex] * K_A[BasisIndex];
		}

		return Result;
	}

	static FVector ComputeIrradianceFromSHRadiance( const FSHVectorRGB& SHRadiance, const FVector& Direction )
	{
		const FLOAT X = Direction.X, Y = Direction.Y, Z = Direction.Z;
		FLOAT CosTheta = Z;

		// E = Sum for all L,M (^Al * Llm * Ylm)

		FSHVector Basis = SHBasisFunction( Direction );		

		return FVector( 
			Irr( SHRadiance.R, Basis, Direction.Z ),
			Irr( SHRadiance.G, Basis, Direction.Z ),
			Irr( SHRadiance.B, Basis, Direction.Z ) );
	}
	
	void UpdateIrradiance( const FPrimitiveSceneInfo* PrimitiveSceneInfo, const FMeshElement& Mesh )
	{		
		UINT DestStride;
		for (INT FaceIndex=0; FaceIndex<6; ++FaceIndex)
		{
			BYTE* Buffer = (BYTE*)RHILockTextureCubeFace(TextureCubeRHI, FaceIndex, 0, TRUE, DestStride);

			FVector U, V, W;

			switch (FaceIndex)
			{
			case 0 : U = FVector(0,0,-1); V = FVector(0,-1,0); W = FVector(+1,0,0); break;
			case 1 : U = FVector(0,0,+1); V = FVector(0,-1,0); W = FVector(-1,0,0); break;			
			case 2 : U = FVector(+1,0,0); V = FVector(0,0,+1); W = FVector(0,+1,0); break;			
			case 3 : U = FVector(+1,0,0); V = FVector(0,0,-1); W = FVector(0,-1,0); break;
			case 4 : U = FVector(+1,0,0); V = FVector(0,-1,0); W = FVector(0,0,+1); break;
			case 5 : U = FVector(-1,0,0); V = FVector(0,-1,0); W = FVector(0,0,-1); break;
			}

			W = W - U - V;
			U = U * 2.0f / (GSystemSettings->DiffuseCubeResolution-1);
			V = V * 2.0f / (GSystemSettings->DiffuseCubeResolution-1);

			for (INT y=0; y<GSystemSettings->DiffuseCubeResolution; ++y)
			{
#if USE_FP16_CUBE
				WORD* Dest = (WORD*)( Buffer + y * DestStride );
#else
				FColor* Dest = (FColor*)(Buffer + y * DestStride);
#endif

				for (INT x=0; x<GSystemSettings->DiffuseCubeResolution; ++x)
				{
					FVector NormalVector = FVector( U * x + V * y + W ).UnsafeNormal();

					FVector RGB = ComputeIrradianceFromSHRadiance( PrimitiveSceneInfo->IrradianceSH, NormalVector );

#if USE_FP16_CUBE					
					*Dest++ = half_from_float( *((UINT*)&RGB.X) );
					*Dest++ = half_from_float( *((UINT*)&RGB.Y) );
					*Dest++ = half_from_float( *((UINT*)&RGB.Z) );
					*Dest++ = 0;					
					
#else
					*Dest++ = EncodeRGBL( RGB );
#endif
				}
			}

			RHIUnlockTextureCubeFace(TextureCubeRHI, FaceIndex, 0);
		}		
	}
};

FIrradianceCubeTexture* GIrradianceCubeTexture = new TGlobalResource<FIrradianceCubeTexture>;

class FTwoWaySkyLightingPolicy
{
public :
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("SKYLIGHT_TWOWAY"),TEXT("1"));
	}

	class PixelParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{			
			UpperSkyColorParameter.Bind(ParameterMap,TEXT("UpperSkyColor"),TRUE);
			LowerSkyColorParameter.Bind(ParameterMap,TEXT("LowerSkyColor"),TRUE);
		}

		void Serialize(FArchive& Ar)
		{			
			Ar << UpperSkyColorParameter;
			Ar << LowerSkyColorParameter;
		}

		void SetSkyColor(FCommandContextRHI* Context, FShader* PixelShader, const FLinearColor& UpperSkyColor,const FLinearColor& LowerSkyColor) const
		{
			SetPixelShaderValue(Context,PixelShader->GetPixelShader(),UpperSkyColorParameter,UpperSkyColor);
			SetPixelShaderValue(Context,PixelShader->GetPixelShader(),LowerSkyColorParameter,LowerSkyColor);
		}

		void SetSkyLighting(FCommandContextRHI* Context, FShader* PixelShader, const FPrimitiveSceneInfo* PrimitiveSceneInfo, const FMeshElement& Mesh) const
		{
			// set sky color
			if(PrimitiveSceneInfo)
			{
				SetSkyColor(Context, PixelShader, PrimitiveSceneInfo->UpperSkyLightColor,PrimitiveSceneInfo->LowerSkyLightColor);
			}
			else
			{
				SetSkyColor(Context, PixelShader, FLinearColor::Black,FLinearColor::Black);
			}
		}

		FShaderParameter UpperSkyColorParameter;
		FShaderParameter LowerSkyColorParameter;
	};
	
};

class FIrradianceCubeSkyLightingPolicy
{
public :
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return Platform == SP_PCD3D;
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("SKYLIGHT_CUBE"), TEXT("1"));
		OutEnvironment.Definitions.Set(TEXT("WORLD_COORDS"), TEXT("1"));
	}

	class PixelParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{			
			IrradianceCubeParameter.Bind(ParameterMap,TEXT("IrradianceCube"),TRUE);
		}

		void Serialize(FArchive& Ar)
		{			
			Ar << IrradianceCubeParameter;
		}		

		void SetSkyLighting(FCommandContextRHI* Context, FShader* PixelShader, const FPrimitiveSceneInfo* PrimitiveSceneInfo, const FMeshElement& Mesh) const
		{
			// set sky color
			GIrradianceCubeTexture->UpdateIrradiance( PrimitiveSceneInfo, Mesh );			

			SetTextureParameter(Context,PixelShader->GetPixelShader(),IrradianceCubeParameter,GIrradianceCubeTexture->SamplerStateRHI,GIrradianceCubeTexture->TextureRHI);
		}

		FShaderParameter IrradianceCubeParameter;
	};
};

template <typename SkyLightingPolicy>
class FPointAndSkyLightPolicy
{
public:	
	typedef SkyLightingPolicy SkyLightingPolicyType;
	typedef FPointAndSkyLightPolicy<SkyLightingPolicyType> LightingPolicyType;
	typedef TPointLightSceneInfo<LightingPolicyType> SceneInfoType;

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return (VertexFactoryType->SupportsStaticLighting() || Material->IsUsedWithSkeletalMesh() || Material->IsSpecialEngineMaterial()) && SkyLightingPolicyType::ShouldCache( Platform, Material, VertexFactoryType );
		//return !Material->IsUsedWithParticleSystem() && !Material->IsSpecialEngineMaterial();
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("ONE_PASS_DYNAMIC"),TEXT("1"));
	}

	class VertexParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
			LightPositionAndInvRadiusParameter.Bind(ParameterMap,TEXT("LightPositionAndInvRadius"));
		}
		void SetLight(FCommandContextRHI* Context,FShader* VertexShader,const TPointLightSceneInfo<LightingPolicyType>* Light) const;
		void Serialize(FArchive& Ar)
		{
			Ar << LightPositionAndInvRadiusParameter;
		}
	private:
		FShaderParameter LightPositionAndInvRadiusParameter;
	};

	class PixelParametersType : public SkyLightingPolicyType::PixelParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
			SkyLightingPolicy::PixelParametersType::Bind(ParameterMap);
			LightColorAndFalloffExponentParameter.Bind(ParameterMap,TEXT("LightColorAndFalloffExponent"),TRUE);									
		}
		void SetLight(FCommandContextRHI* Context, FShader* PixelShader,const TPointLightSceneInfo<LightingPolicyType>* Light, const FSceneView* View) const
		{
			SetPixelShaderValue(
				Context,
				PixelShader->GetPixelShader(),
				LightColorAndFalloffExponentParameter,
				FVector4(
				Light->Color.R,
				Light->Color.G,
				Light->Color.B,
				Light->FalloffExponent
				)
				);
		}
		void Serialize(FArchive& Ar)
		{
			SkyLightingPolicy::PixelParametersType::Serialize(Ar);
			Ar << LightColorAndFalloffExponentParameter;						
		}

	private:
		FShaderParameter LightColorAndFalloffExponentParameter;		
	};

	/**
	* Modulated shadow shader params associated with this light policy
	*/
	class ModShadowPixelParamsType
	{
	public:
		void Bind( const FShaderParameterMap& ParameterMap )
		{
			LightPositionParam.Bind(ParameterMap,TEXT("LightPosition"));
			FalloffExponentParam.Bind(ParameterMap,TEXT("FalloffExponent"));
		}
		void SetModShadowLight( FCommandContextRHI* Context, FShader* PixelShader, const TPointLightSceneInfo<LightingPolicyType>* Light ) const
		{
			// set world light position and falloff rate
			SetPixelShaderValue( Context, PixelShader->GetPixelShader(), LightPositionParam, FVector4(Light->GetOrigin(), 1.0f / Light->Radius) );
			SetPixelShaderValue( Context, PixelShader->GetPixelShader(), FalloffExponentParam, Light->ShadowFalloffExponent );
		}
		void Serialize( FArchive& Ar )
		{
			Ar << LightPositionParam;
			Ar << FalloffExponentParam;
		}
		static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
		{
			OutEnvironment.Definitions.Set(TEXT("MODSHADOW_LIGHTTYPE_POINT"),TEXT("1"));
		}
	private:
		/** world position of light casting a shadow. Note: w = 1.0 / Radius */
		FShaderParameter LightPositionParam;
		/** attenuation exponent for light casting a shadow */
		FShaderParameter FalloffExponentParam;
	};
};

/**
* specialize The pixel shader used to draw the effect of a light on a mesh.
*/
template<typename ShadowingTypePolicy,typename SkyLightingPolicy>
class TLightPixelShader<FPointAndSkyLightPolicy<SkyLightingPolicy>, ShadowingTypePolicy> :
	public FShader,
	public FPointAndSkyLightPolicy<SkyLightingPolicy>::PixelParametersType,
	public ShadowingTypePolicy::PixelParametersType
{
	DECLARE_SHADER_TYPE(TLightPixelShader,MeshMaterial);
public:
	typedef FPointAndSkyLightPolicy<SkyLightingPolicy> LightingTypePolicy;
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return	!IsTranslucentBlendMode(Material->GetBlendMode())
			&& Material->GetLightingModel() != MLM_Unlit
			&& ShadowingTypePolicy::ShouldCache(Platform,Material,VertexFactoryType)
			&& LightingTypePolicy::ShouldCache(Platform, Material, VertexFactoryType)
			&& SkyLightingPolicy::ShouldCache(Platform, Material, VertexFactoryType);
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		SkyLightingPolicy::ModifyCompilationEnvironment(OutEnvironment);
		ShadowingTypePolicy::ModifyCompilationEnvironment(OutEnvironment);
		LightingTypePolicy::ModifyCompilationEnvironment(OutEnvironment);		
	}

	TLightPixelShader() {}

	TLightPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FShader(Initializer)
	{
		LightingTypePolicy::PixelParametersType::Bind(Initializer.ParameterMap);
		ShadowingTypePolicy::PixelParametersType::Bind(Initializer.ParameterMap);		
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
		LightAttenuationTextureParameter.Bind(Initializer.ParameterMap,TEXT("LightAttenuationTexture"),TRUE);
		ScreenPositionScaleBiasParameter.Bind(Initializer.ParameterMap,TEXT("ScreenPositionScaleBias"),TRUE);

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
		if(ScreenPositionScaleBiasParameter.IsBound())
		{
			SetPixelShaderValue(Context,GetPixelShader(),ScreenPositionScaleBiasParameter,FVector4(
				View->SizeX / GSceneRenderTargets.GetBufferSizeX() / +2.0f,
				View->SizeY / GSceneRenderTargets.GetBufferSizeY() / -2.0f,
				(View->SizeY / 2.0f + GPixelCenterOffset + View->RenderTargetY) / GSceneRenderTargets.GetBufferSizeY(),
				(View->SizeX / 2.0f + GPixelCenterOffset + View->RenderTargetX) / GSceneRenderTargets.GetBufferSizeX()
				));
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
		const FMaterial* Material = MaterialInstance->GetMaterial();		

		extern UBOOL GShouldUploadLocalToWorldPixelShaderParameters;

		GShouldUploadLocalToWorldPixelShaderParameters = TRUE;

		MaterialParameters.SetLocalTransforms(Context,this,MaterialInstance,LocalToWorld,bBackFace);		

		GShouldUploadLocalToWorldPixelShaderParameters = FALSE;		
	}

	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		LightingTypePolicy::PixelParametersType::Serialize(Ar);
		ShadowingTypePolicy::PixelParametersType::Serialize(Ar);
		
		Ar << MaterialParameters;
		Ar << LightAttenuationTextureParameter;
		Ar << ScreenPositionScaleBiasParameter;

		//<@ ava specific ; 2007. 7. 4 changmin
		Ar << TonemapTextureParameter;
		Ar << TonemapRangeParameter;
		//>@ ava
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
	FShaderParameter LightAttenuationTextureParameter;
	FShaderParameter ScreenPositionScaleBiasParameter;

	//<@ ava specific ; 2007. 7. 4 changmin
	// hdr integer path
	FShaderParameter TonemapTextureParameter;
	FShaderParameter TonemapRangeParameter;
	//>@ ava
};

/**
* specialized The vertex shader used to draw the effect of a light on a mesh.
*/
template<typename ShadowingTypePolicy,typename SkyLightingPolicy>
class TLightVertexShader<FPointAndSkyLightPolicy<SkyLightingPolicy>, ShadowingTypePolicy> :
	public FShader,
	public FPointAndSkyLightPolicy<SkyLightingPolicy>::VertexParametersType,
	public ShadowingTypePolicy::VertexParametersType
{
	DECLARE_SHADER_TYPE(TLightVertexShader,MeshMaterial);
public:
	typedef FPointAndSkyLightPolicy<SkyLightingPolicy> LightingTypePolicy;

	// specialize this function
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return	!IsTranslucentBlendMode(Material->GetBlendMode()) &&
			Material->GetLightingModel() != MLM_Unlit &&
			ShadowingTypePolicy::ShouldCache(Platform,Material,VertexFactoryType) &&
			LightingTypePolicy::ShouldCache(Platform, Material, VertexFactoryType) &&
			SkyLightingPolicy::ShouldCache(Platform, Material, VertexFactoryType);
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		SkyLightingPolicy::ModifyCompilationEnvironment(OutEnvironment);
		ShadowingTypePolicy::ModifyCompilationEnvironment(OutEnvironment);
		LightingTypePolicy::ModifyCompilationEnvironment(OutEnvironment);
	}

	TLightVertexShader() {}

	TLightVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FShader(Initializer),
		VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{
		LightingTypePolicy::VertexParametersType::Bind(Initializer.ParameterMap);
		ShadowingTypePolicy::VertexParametersType::Bind(Initializer.ParameterMap);
		ViewProjectionMatrixParameter.Bind(Initializer.ParameterMap,TEXT("ViewProjectionMatrix"),TRUE);
		CameraPositionParameter.Bind(Initializer.ParameterMap,TEXT("CameraPosition"),TRUE);
	}

	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		LightingTypePolicy::VertexParametersType::Serialize(Ar);
		ShadowingTypePolicy::VertexParametersType::Serialize(Ar);
		Ar << VertexFactoryParameters;
		Ar << ViewProjectionMatrixParameter;
		Ar << CameraPositionParameter;
	}

	void SetParameters(FCommandContextRHI* Context,const FVertexFactory* VertexFactory,const FMaterialInstance* MaterialInstance,const FSceneView* View) 
	{
		VertexFactoryParameters.Set(Context,this,VertexFactory,View);
		SetVertexShaderValue(Context,GetVertexShader(),ViewProjectionMatrixParameter,View->ViewProjectionMatrix);
		SetVertexShaderValue(Context,GetVertexShader(),CameraPositionParameter,View->ViewOrigin);
	}
	void SetLocalTransforms(FCommandContextRHI* Context,const FMatrix& LocalToWorld,const FMatrix& WorldToLocal) 
	{
		VertexFactoryParameters.SetLocalTransforms(Context,this,LocalToWorld,WorldToLocal);
	}

private:
	FVertexFactoryParameterRef VertexFactoryParameters;
	FShaderParameter ViewProjectionMatrixParameter;
	FShaderParameter CameraPositionParameter;
};

// sky light version이 더 높으므로 그것을 씁니다.
IMPLEMENT_LIGHT_SHADER_TYPE_WITH_FRIENDLY_NAME(FPointAndSkyLightPolicy<FTwoWaySkyLightingPolicy>,FPointAndSkyLightPolicy_TwoWay,TEXT("PointAndSkyLightVertexShader"),TEXT("PointAndSkyLightPixelShader"),VER_LATEST_ENGINE,VER_AVA_SHADER_TRANSFORM_OPTIMIZED);
//IMPLEMENT_LIGHT_SHADER_TYPE_WITH_FRIENDLY_NAME(FPointAndSkyLightPolicy<FIrradianceCubeSkyLightingPolicy>,FPointAndSkyLightPolicy_IrradianceCube,TEXT("PointAndSkyLightVertexShader"),TEXT("PointAndSkyLightPixelShader"),VER_LATEST_ENGINE,VER_AVA_IRRADIANCE_ENVMAP);

//<@ ava specific ; 2007. 10. 19 changmin : add viewmode bump lighting
IMPLEMENT_BUMPLIGHT_SHADER_TYPE(FPointAndSkyLightPolicy<FTwoWaySkyLightingPolicy>,FPointAndSkyLightPolicy_TwoWay,TEXT("PointAndSkyLightVertexShader"),TEXT("PointAndSkyLightBumpPixelShader"),VER_SKYLIGHT_LOWERHEMISPHERE_SHADER_RECOMPILE,VER_AVA_ADD_BUMPONLY_VIEWMODE);
//>@ ava

/**
* Specialized MeshLightingDrawingPolicy for Point And SkyLighting Shader
* 2006. 9. 18 changmin
*/
template<typename ShadowPolicyType,typename SkyLightingPolicy>
class TMeshLightingDrawingPolicy<ShadowPolicyType, FPointAndSkyLightPolicy<SkyLightingPolicy>>
	: public FMeshDrawingPolicy
{
public:

	typedef typename ShadowPolicyType::ElementDataType ElementDataType;
	typedef typename FPointAndSkyLightPolicy<SkyLightingPolicy>::SceneInfoType LightSceneInfoType;	

	typedef TLightVertexShader<FPointAndSkyLightPolicy<SkyLightingPolicy>,ShadowPolicyType> VertexShaderType;
	typedef TLightPixelShader<FPointAndSkyLightPolicy<SkyLightingPolicy>,ShadowPolicyType> PixelShaderType;

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
		VertexShader->SetLocalTransforms(Context,Mesh.LocalToWorld,Mesh.WorldToLocal);
		PixelShader->SetLocalTransforms(Context,Mesh.MaterialInstance,Mesh.LocalToWorld,bBackFace);

		PixelShader->SetSkyLighting(Context, PixelShader, PrimitiveSceneInfo, Mesh);


		// call parent implementation

		/* EnvCube */
		if (PrimitiveSceneInfo != NULL && GEnvCube_IsRequired)			
			EnvCube_Bind(Context, PixelShader->GetPixelShader(), PrimitiveSceneInfo );
		/* EnvCube */
		FMeshDrawingPolicy::SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FMeshDrawingPolicy::ElementDataType());
	}

	void DrawShared(FCommandContextRHI* Context,const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
	{
		SCOPE_CYCLE_COUNTER(STAT_LightingDrawSharedTime);

		ShadowPolicy.Set(Context,VertexShader,PixelShader,PixelShader,VertexFactory,MaterialInstance,View);

		VertexShader->SetParameters(Context,VertexFactory,MaterialInstance,View);
		VertexShader->SetLight(Context,VertexShader,Light);

		PixelShader->SetParameters(Context,MaterialInstance,View);

		// specialized for FPointAndSkyLight
		PixelShader->SetLight(Context,PixelShader,Light, View);

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

//
template <typename SkyLightingPolicy>
void FPointAndSkyLightPolicy<SkyLightingPolicy>::VertexParametersType::SetLight(FCommandContextRHI* Context,FShader* VertexShader,const TPointLightSceneInfo<FPointAndSkyLightPolicy<SkyLightingPolicy> >* Light) const
{
	SetVertexShaderValue(Context,VertexShader->GetVertexShader(),LightPositionAndInvRadiusParameter,FVector4(Light->GetOrigin(),Light->InvRadius));
}




template <typename SkyLightingPolicy>
class FSHPointLightSceneInfo : public TPointLightSceneInfo<FPointAndSkyLightPolicy<SkyLightingPolicy> >
{
public :
	FSHPointLightSceneInfo(const UPointLightComponent* Component)
	   : TPointLightSceneInfo<FPointAndSkyLightPolicy<SkyLightingPolicy> >( Component )
	{
	}

    /* SH는 light의 bound와 상관 없이 다 그려야함 */
	virtual void SetDepthBounds(FCommandContextRHI* Context,const FSceneView* View) const
	{
	}

	virtual void SetScissorRect(FCommandContextRHI* Context,const FSceneView* View) const
	{
	}
};

FLightSceneInfo* USHPointLightComponent::CreateSceneInfo() const
{
/*	if (GSystemSettings->DiffuseCubeResolution > 0)
		return new FSHPointLightSceneInfo<FIrradianceCubeSkyLightingPolicy>(this);
	else*/
		return new FSHPointLightSceneInfo<FTwoWaySkyLightingPolicy>(this);
}


ELightComponentType USHPointLightComponent::GetLightType() const
{
	return LightType_SHPoint;
}

UBOOL USHPointLightComponent::IsDepthDrawingLight() const
{
	return TRUE;
}
