/*=============================================================================
LightMapRendering.h: Light map rendering definitions.
Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __LIGHTMAPRENDERING_H__
#define __LIGHTMAPRENDERING_H__

/**
*/
class FNullLightMapShaderComponent
{
public:
	void Bind(const FShaderParameterMap& ParameterMap)
	{}
	void Serialize(FArchive& Ar)
	{}
};

/**
* A policy for emissive shaders without a light-map.
*/
class FNoLightMapPolicy
{
public:

	typedef FNullLightMapShaderComponent VertexParametersType;
	typedef FNullLightMapShaderComponent PixelParametersType;
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
		const FVertexFactory* VertexFactory,
		const FMaterialInstance* MaterialInstance,
		const ElementDataType& ElementData
		) const
	{}

	friend UBOOL operator==(const FNoLightMapPolicy A,const FNoLightMapPolicy B)
	{
		return TRUE;
	}

	friend INT Compare(const FNoLightMapPolicy&,const FNoLightMapPolicy&)
	{
		return 0;
	}
};

/**
* A policy for emissive shaders with a vertex light-map.
*/
class FVertexLightMapPolicy
{
public:

	typedef const FLightMap1D* ElementDataType;

	class VertexParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
			LightMapScaleParameter.Bind(ParameterMap,TEXT("LightMapScale"));
		}
		void SetLightMapScale(FCommandContextRHI* Context,FShader* VertexShader,const FLightMap1D* LightMap) const
		{
			SetVertexShaderValues(Context,VertexShader->GetVertexShader(),LightMapScaleParameter,LightMap->GetScaleArray(),NUM_LIGHTMAP_COEFFICIENTS);
		}
		void Serialize(FArchive& Ar)
		{
			Ar << LightMapScaleParameter;
		}
	private:
		FShaderParameter LightMapScaleParameter;
	};

	typedef FNullLightMapShaderComponent PixelParametersType;

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return Material->GetLightingModel() != MLM_Unlit && VertexFactoryType->SupportsStaticLighting();
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("VERTEX_LIGHTMAP"),TEXT("1"));
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
	{}

	/**
	* Get the decl and stream strides for this policy type and vertexfactory
	* @param VertexDeclaration - output decl 
	* @param StreamStrides - output array of vertex stream strides 
	* @param VertexFactory - factory to be used by this policy
	*/
	void GetVertexDeclarationInfo(FVertexDeclarationRHIParamRef &VertexDeclaration, DWORD *StreamStrides, const FVertexFactory* VertexFactory)
	{
		check(VertexFactory);
		VertexFactory->GetVertexLightMapStreamStrides(StreamStrides);
		VertexDeclaration = VertexFactory->GetVertexLightMapDeclaration();
	}

	void SetMesh(
		FCommandContextRHI* Context,
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* VertexShader,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialInstance* MaterialInstance,
		const FLightMap1D* LightMap
		) const
	{
		check(VertexFactory);
		VertexFactory->SetVertexLightMap(Context,LightMap);
		VertexShaderParameters->SetLightMapScale(Context,VertexShader,LightMap);
	}

	friend UBOOL operator==(const FVertexLightMapPolicy A,const FVertexLightMapPolicy B)
	{
		return TRUE;
	}

	friend INT Compare(const FVertexLightMapPolicy&,const FVertexLightMapPolicy&)
	{
		return 0;
	}
};

/**
* A policy for emissive shaders with a light-map texture.
*/
class FLightMapTexturePolicy
{
public:

	typedef const FLightMap2D* ElementDataType;

	class PixelParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
			LightMapTexturesParameter.Bind(ParameterMap,TEXT("LightMapTextures"),TRUE);
		}
		void SetLightMapTextures(FCommandContextRHI* Context,FShader* PixelShader,const FLightMapTexturePolicy& LightMapPolicy) const
		{
			//<@ ava specific ; 2007. 1. 20 changmin
			//for(INT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
			for(INT CoefficientIndex = 1;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				//>@ ava
			{
				SetTextureParameter(
					Context,
					PixelShader->GetPixelShader(),
					LightMapTexturesParameter,
					LightMapPolicy.LightMapTextures[CoefficientIndex]->Resource,
					CoefficientIndex - 1
					);
			}
			//>@ ava
		}

		//<@ ava specific ; 2007. 1. 18 changmin
		void SetLightMapTexture(FCommandContextRHI* Context,FShader* PixelShader,const FLightMapTexturePolicy& LightMapPolicy) const
		{
			SetTextureParameter(
				Context,
				PixelShader->GetPixelShader(),
				LightMapTexturesParameter,
				LightMapPolicy.LightMapTextures[0]->Resource,
				0
				);
		}		
		//>@ ava		

		void Serialize(FArchive& Ar)
		{
			Ar << LightMapTexturesParameter;			
		}
	private:
		FShaderParameter LightMapTexturesParameter;		
	};

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

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return Material->GetLightingModel() != MLM_Unlit && VertexFactoryType->SupportsStaticLighting();
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("TEXTURE_LIGHTMAP"),TEXT("1"));
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
		//<@ ava specific ; 2007. 1. 18 changmin
		const FMaterial *Material = MaterialInstance->GetMaterial();
		if( Material && Material->NeedsBumpedLightmap() )
		{
#if !FINAL_RELEASE
			if( IsSM2Platform(GRHIShaderPlatform) && GIsEditor )
			{
				debugf(NAME_Warning, TEXT("Material %s use bumped light in sm2 platform"), *Material->GetFriendlyName() );
			}
#endif
			PixelShaderParameters->SetLightMapTextures(Context, PixelShader, *this);	// 이 라인이 original
		}
		else
		{
#if !FINAL_RELEASE
			if( !IsSM2Platform(GRHIShaderPlatform) && GIsEditor )
			{
				//debugf(NAME_Warning, TEXT("Material %s does not use bumped light in sm3 platform"), *Material->GetFriendlyName() );
			}
#endif
			PixelShaderParameters->SetLightMapTexture(Context,PixelShader,*this);
		}
		//>@ ava
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
		const FVertexFactory* VertexFactory,
		const FMaterialInstance* MaterialInstance,
		const FLightMap2D* LightMap
		) const
	{
		VertexShaderParameters->SetCoordinateTransform(
			Context,
			VertexShader,
			LightMap->GetCoordinateScale(),
			LightMap->GetCoordinateBias()
			);		
	}

	/** Initialization constructor. */
	FLightMapTexturePolicy(const FLightMap2D* LightMap)
	{
		//for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
		//<@ ava specific ; 2007. 1. 20 changmin
		for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
			//>@ ava
		{
			LightMapTextures[CoefficientIndex] = LightMap->GetTexture(CoefficientIndex);
		}
	}

	friend UBOOL operator==(const FLightMapTexturePolicy A,const FLightMapTexturePolicy B)
	{
		//for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
		//<@ ava specific ; 2007. 1. 20 changmin
		for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
			//>@ ava
		{
			if(A.LightMapTextures[CoefficientIndex] != B.LightMapTextures[CoefficientIndex])
			{
				return FALSE;
			}
		}
		return TRUE;
	}

	friend INT Compare(const FLightMapTexturePolicy& A,const FLightMapTexturePolicy& B)
	{
		COMPAREDRAWINGPOLICYMEMBERS(LightMapTextures[0]);
		COMPAREDRAWINGPOLICYMEMBERS(LightMapTextures[1]);
		COMPAREDRAWINGPOLICYMEMBERS(LightMapTextures[2]);
		//<@ ava specific ; 2007. 1. 20 changmin
		COMPAREDRAWINGPOLICYMEMBERS(LightMapTextures[3]);
		//>@ ava
		return 0;
	}

private:
	//const UTexture2D* LightMapTextures[NUM_LIGHTMAP_COEFFICIENTS];
	//<@ ava specific ; 2007. 1. 20 changmin
	const UTexture2D* LightMapTextures[NUM_AVA_LIGHTMAPS];
	//>@ ava
};
#endif // __LIGHTMAPRENDERING_H__
