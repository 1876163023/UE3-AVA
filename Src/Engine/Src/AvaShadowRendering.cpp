/*==
AvaShadowRendering.cpp: Cascaded Shadow Rendering implementation
Copyright 2006-2007 Redduck, Inc. All Rights Reserved.
==*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "UnTextureLayout.h"
#include "BspDynamicBatch.h"
#include "SceneFilterRendering.h"

//<@ ava specific ; 2007. 11. 6 changmin
// add cascaded shadow
FLOAT GSliceValues[8]	= { 100.0f, 200.0f, 400.0f, 800.0f, 1600.0f, 3200.0f, 6400.0f, 25600.0f };
INT GNumCascadedShadow = 8;
//>@ ava

/**
* Returns the material which should be used to draw the shadow depth for the given material.
* If the material is masked, it returns the given material; otherwise it returns GEngine->DefaultMaterial.
*/
static const FMaterialInstance* GetShadowDepthMaterial(const FMaterialInstance* MaterialInstance)
{
	return MaterialInstance->GetMaterial()->IsMasked() ? MaterialInstance :	GEngine->DefaultMaterial->GetInstanceInterface(FALSE);
}

/**
* A vertex shader for rendering the depth of a mesh.
*/
UINT GSliceIndex = 0;
FLOAT GCascadedDepthBias = 0.001f;
class FCascadedShadowDepthVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FCascadedShadowDepthVertexShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only compile the shadow depth shaders for the default material, and masked materials.
		return	Material->IsSpecialEngineMaterial() && !IsSM2Platform(Platform);
	}

	FCascadedShadowDepthVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FShader(Initializer),
		VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{
		ShadowMatrixParameter.Bind(Initializer.ParameterMap,TEXT("ShadowMatrix"));
		ProjectionMatrixParameter.Bind(Initializer.ParameterMap,TEXT("ProjectionMatrix"),TRUE);
	}
	FCascadedShadowDepthVertexShader() {}
	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		Ar << VertexFactoryParameters;
		Ar << ShadowMatrixParameter;
		Ar << ProjectionMatrixParameter;
	}
	void SetParameters(
		FCommandContextRHI* Context,
		const FVertexFactory* VertexFactory,
		const FMaterialInstance* MaterialInstance,
		const FSceneView* View,
		const FCascadedShadowInfo* ShadowInfo
		)
	{
		FMatrix ShadowDepthMatrix = ShadowInfo->SubjectMatrix[GSliceIndex]
		* FScaleMatrix( FVector( 1.0f, 1.0f, 1.0f / ShadowInfo->MaxSubjectDepth[GSliceIndex]) )
			* FTranslationMatrix( FVector( 0.0f, 0.0f, GCascadedDepthBias) );

		VertexFactoryParameters.Set(Context,this,VertexFactory,View);
		SetVertexShaderValue(Context,GetVertexShader(),ShadowMatrixParameter,ShadowDepthMatrix);
		
	}
	void SetLocalTransforms(FCommandContextRHI* Context,const FMatrix& LocalToWorld,const FMatrix& WorldToLocal)
	{
		VertexFactoryParameters.SetLocalTransforms(Context,this,LocalToWorld,WorldToLocal);
	}

	virtual EShaderRecompileGroup GetRecompileGroup() const
	{
		return SRG_GLOBAL_MISC_SHADOW;
	}
private:
	FVertexFactoryParameterRef VertexFactoryParameters;
	FShaderParameter ShadowMatrixParameter;
	FShaderParameter ProjectionMatrixParameter;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FCascadedShadowDepthVertexShader,TEXT("CascadedShadowDepthVertexShader"),TEXT("Main"),SF_Vertex,0,VER_AVA_CASCADEDSHADOWDEPTH_RECOMPILE);

class FCascadedShadowMaskedDepthVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FCascadedShadowMaskedDepthVertexShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only compile the shadow depth shaders for the default material, and masked materials.
		return	Material->IsMasked() && !IsSM2Platform(Platform);
	}

	FCascadedShadowMaskedDepthVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FShader(Initializer),
		VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{
		ShadowMatrixParameter.Bind(Initializer.ParameterMap,TEXT("ShadowMatrix"));
		ProjectionMatrixParameter.Bind(Initializer.ParameterMap,TEXT("ProjectionMatrix"),TRUE);
	}
	FCascadedShadowMaskedDepthVertexShader() {}
	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		Ar << VertexFactoryParameters;
		Ar << ShadowMatrixParameter;
		Ar << ProjectionMatrixParameter;
	}
	void SetParameters(
		FCommandContextRHI* Context,
		const FVertexFactory* VertexFactory,
		const FMaterialInstance* MaterialInstance,
		const FSceneView* View,
		const FCascadedShadowInfo* ShadowInfo
		)
	{
		VertexFactoryParameters.Set(Context,this,VertexFactory,View);
		SetVertexShaderValue(Context,GetVertexShader(),ShadowMatrixParameter,ShadowInfo->SubjectAndReceiverMatrix[GSliceIndex]);
		SetVertexShaderValue(Context,GetVertexShader(),ProjectionMatrixParameter,ShadowInfo->SubjectMatrix[GSliceIndex]);
	}
	void SetLocalTransforms(FCommandContextRHI* Context,const FMatrix& LocalToWorld,const FMatrix& WorldToLocal)
	{
		VertexFactoryParameters.SetLocalTransforms(Context,this,LocalToWorld,WorldToLocal);
	}

	virtual EShaderRecompileGroup GetRecompileGroup() const
	{
		return SRG_GLOBAL_MISC_SHADOW;
	}
private:
	FVertexFactoryParameterRef VertexFactoryParameters;
	FShaderParameter ShadowMatrixParameter;
	FShaderParameter ProjectionMatrixParameter;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FCascadedShadowMaskedDepthVertexShader,TEXT("ShadowDepthVertexShader"),TEXT("Main"),SF_Vertex,0,VER_AVA_CASCADEDSHADOWDEPTH_RECOMPILE);

/**
* A pixel shader for rendering the depth of a mesh.
*/

class FCascadedShadowDepthPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FCascadedShadowDepthPixelShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only compile the shadow depth shaders for the default material, and masked materials.
		return	Material->IsSpecialEngineMaterial() && !IsSM2Platform(Platform);
	}

	FCascadedShadowDepthPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
		InvMaxSubjectDepthParameter.Bind(Initializer.ParameterMap,TEXT("InvMaxSubjectDepth"), TRUE);
		DepthBiasParameter.Bind(Initializer.ParameterMap,TEXT("DepthBias"),TRUE);
	}

	FCascadedShadowDepthPixelShader() {}

	void SetParameters(
		FCommandContextRHI* Context,
		const FVertexFactory* VertexFactory,
		const FMaterialInstance* MaterialInstance,
		const FSceneView* View,
		const FCascadedShadowInfo* ShadowInfo
		)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialInstance, View->Family->CurrentWorldTime, View->Family->CurrentRealTime, View);
		MaterialParameters.Set(Context,this,MaterialRenderContext);
		if( InvMaxSubjectDepthParameter.IsBound() )
			SetPixelShaderValue(Context,GetPixelShader(),InvMaxSubjectDepthParameter, 1.0f / ShadowInfo->MaxSubjectDepth[GSliceIndex]);
		//FLOAT DepthBias = GEngine->DepthBias;
		if( DepthBiasParameter.IsBound() )
			SetPixelShaderValue(Context,GetPixelShader(),DepthBiasParameter, GCascadedDepthBias);
	}

	void SetLocalTransforms(FCommandContextRHI* Context,const FMaterialInstance* MaterialInstance,const FMatrix& LocalToWorld,UBOOL bBackFace)
	{
		MaterialParameters.SetLocalTransforms(Context,this,MaterialInstance,LocalToWorld,bBackFace);
	}

	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		Ar << MaterialParameters;
		Ar << InvMaxSubjectDepthParameter;
		if( Ar.Ver() >= VER_SHADOW_DEPTH_SHADER_RECOMPILE )
		{
			Ar << DepthBiasParameter;
		}
	}

	virtual EShaderRecompileGroup GetRecompileGroup() const
	{
		return SRG_GLOBAL_MISC_SHADOW;
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
	FShaderParameter InvMaxSubjectDepthParameter;
	FShaderParameter DepthBiasParameter;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FCascadedShadowDepthPixelShader,TEXT("CascadedShadowDepthPixelShader"),TEXT("Main"),SF_Pixel,VER_SHADOW_DEPTH_SHADER_RECOMPILE,VER_AVA_CASCADEDSHADOWDEPTH_RECOMPILE);

// for masked material
class FCascadedShadowMaskedDepthPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FCascadedShadowMaskedDepthPixelShader, MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only compile the shadow depth shaders for the default material, and masked materials.
		return	Material->IsMasked() && !IsSM2Platform(Platform);
	}

	FCascadedShadowMaskedDepthPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
		InvMaxSubjectDepthParameter.Bind(Initializer.ParameterMap,TEXT("InvMaxSubjectDepth"), TRUE);
		DepthBiasParameter.Bind(Initializer.ParameterMap,TEXT("DepthBias"),TRUE);
	}

	FCascadedShadowMaskedDepthPixelShader() {}

	void SetParameters(
		FCommandContextRHI* Context,
		const FVertexFactory* VertexFactory,
		const FMaterialInstance* MaterialInstance,
		const FSceneView* View,
		const FCascadedShadowInfo* ShadowInfo)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialInstance, View->Family->CurrentWorldTime, View->Family->CurrentRealTime, View);
		MaterialParameters.Set(Context,this,MaterialRenderContext);
		if( InvMaxSubjectDepthParameter.IsBound() )
			SetPixelShaderValue(Context,GetPixelShader(),InvMaxSubjectDepthParameter, 1.0f / ShadowInfo->MaxSubjectDepth[GSliceIndex]);
		//FLOAT DepthBias = GEngine->DepthBias;
		if( DepthBiasParameter.IsBound() )
			SetPixelShaderValue(Context,GetPixelShader(),DepthBiasParameter, GCascadedDepthBias);
	}

	void SetLocalTransforms(FCommandContextRHI* Context,const FMaterialInstance* MaterialInstance,const FMatrix& LocalToWorld,UBOOL bBackFace)
	{
		MaterialParameters.SetLocalTransforms(Context,this,MaterialInstance,LocalToWorld,bBackFace);
	}

	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		Ar << MaterialParameters;
		Ar << InvMaxSubjectDepthParameter;
		if( Ar.Ver() >= VER_SHADOW_DEPTH_SHADER_RECOMPILE )
		{
			Ar << DepthBiasParameter;
		}
	}

	virtual EShaderRecompileGroup GetRecompileGroup() const
	{
		return SRG_GLOBAL_MISC_SHADOW;
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
	FShaderParameter InvMaxSubjectDepthParameter;
	FShaderParameter DepthBiasParameter;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FCascadedShadowMaskedDepthPixelShader,TEXT("ShadowDepthPixelShader"),TEXT("Main"),SF_Pixel,VER_SHADOW_DEPTH_SHADER_RECOMPILE,VER_AVA_CASCADEDSHADOWDEPTH_RECOMPILE);

// Shader Model 2 compatible version that uses Hardware PCF
IMPLEMENT_SHADER_TYPE(template<>, TCascadedShadowProjectionPixelShader<F4SampleHwPCF>, TEXT("CascadedShadowProjectionPixelShader"), TEXT("HardwarePCFMain"), SF_Pixel, VER_HARDWARE_PCF, 0);

IMPLEMENT_SHADER_TYPE(template<>, TCascadedShadowProjectionPixelShader<F4SampleNearHwPCF>, TEXT("CascadedShadowProjectionPixelShader"), TEXT("NearHardwarePCFMain"), SF_Pixel, VER_HARDWARE_PCF, 0);

// Shader Model 2 compatible version
IMPLEMENT_SHADER_TYPE(template<>, TCascadedShadowProjectionPixelShader<F4SampleManualPCF>, TEXT("CascadedShadowProjectionPixelShader"), TEXT("Main"), SF_Pixel, VER_HARDWARE_PCF, 0);

// Full version that uses Hardware PCF
IMPLEMENT_SHADER_TYPE(template<>, TCascadedShadowProjectionPixelShader<F16SampleHwPCF>, TEXT("CascadedShadowProjectionPixelShader"), TEXT("HardwarePCFMain"), SF_Pixel, VER_HARDWARE_PCF, 0);

IMPLEMENT_SHADER_TYPE(template<>, TCascadedShadowProjectionPixelShader<F16SampleNearHwPCF>, TEXT("CascadedShadowProjectionPixelShader"), TEXT("NearHardwarePCFMain"), SF_Pixel, VER_HARDWARE_PCF, 0);
// Full version that uses Fetch4
IMPLEMENT_SHADER_TYPE(template<>, TCascadedShadowProjectionPixelShader<F16SampleFetch4PCF>, TEXT("CascadedShadowProjectionPixelShader"), TEXT("Fetch4Main"), SF_Pixel, 0, 0);
// Full version
IMPLEMENT_SHADER_TYPE(template<>, TCascadedShadowProjectionPixelShader<F16SampleManualPCF>, TEXT("CascadedShadowProjectionPixelShader"), TEXT("Main"), SF_Pixel, VER_HARDWARE_PCF, 0);

/**
* Get the version of TCascadedShadowPixelShader that should be used based on the hardware's capabilities
* @return a pointer to the chosen shader
*/
FCascadedShadowProjectionPixelShaderInterface* GetCascadedShadowProjectionPixelShaderRef(UBOOL bIsNear)
{
	// shader 2 model은 지원하지도 않음. shader가 instruction limit를 초과.
	check( IsSM2Platform(GRHIShaderPlatform) == FALSE );

	FCascadedShadowProjectionPixelShaderInterface *PixelShader = NULL;

	
	//if( GEngine->ShadowFilterQuality == SFQ_Low || IsSM2Platform(GRHIShaderPlatform) )
	{
		if( GSupportsHardwarePCF )
		{
			if( bIsNear )
			{
				TShaderMapRef<TCascadedShadowProjectionPixelShader<F4SampleNearHwPCF> > FourSamplePixelShader(GetGlobalShaderMap());
				PixelShader = *FourSamplePixelShader;
			}
			else
			{
				TShaderMapRef<TCascadedShadowProjectionPixelShader<F4SampleHwPCF> > FourSamplePixelShader(GetGlobalShaderMap());
				PixelShader = *FourSamplePixelShader;
			}
		}
		else
		{
			TShaderMapRef<TCascadedShadowProjectionPixelShader<F4SampleManualPCF> > FourSamplePixelShader(GetGlobalShaderMap());
			PixelShader = *FourSamplePixelShader;
		}
	}
	//else
	//{
	//	if( GSupportsHardwarePCF )
	//	{
	//		if( bIsNear )
	//		{
	//			TShaderMapRef<TCascadedShadowProjectionPixelShader<F16SampleNearHwPCF> > SixteenSamplePixelShader(GetGlobalShaderMap());
	//			PixelShader = *SixteenSamplePixelShader;
	//		}
	//		else
	//		{
	//			TShaderMapRef<TCascadedShadowProjectionPixelShader<F16SampleHwPCF> > SixteenSamplePixelShader(GetGlobalShaderMap());
	//			PixelShader = *SixteenSamplePixelShader;
	//		}
	//	}
	//	else if( GSupportsFetch4 )
	//	{
	//		TShaderMapRef<TCascadedShadowProjectionPixelShader<F16SampleFetch4PCF> > SixteenSamplePixelShader(GetGlobalShaderMap());
	//		PixelShader = *SixteenSamplePixelShader;
	//	}
	//	else
	//	{
	//		TShaderMapRef<TCascadedShadowProjectionPixelShader<F16SampleManualPCF> > SixteenSamplePixelShader(GetGlobalShaderMap());
	//		PixelShader = *SixteenSamplePixelShader;
	//	}
	//}
	return PixelShader;
}

//-- FCascadedShadowDepthDrawingPolicy
FCascadedShadowDepthDrawingPolicy::FCascadedShadowDepthDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialInstance* InMaterialInstance,
	const FCascadedShadowInfo* InShadowInfo ):
FMeshDrawingPolicy(InVertexFactory, InMaterialInstance),
ShadowInfo(InShadowInfo)
{
	const FMaterialShaderMap *MaterialShaderIndex = InMaterialInstance->GetMaterial()->GetShaderMap();
	const FMeshMaterialShaderMap *MeshShaderIndex = MaterialShaderIndex->GetMeshShaderMap(InVertexFactory->GetType());
	
	if( MaterialInstance->GetMaterial()->IsMasked() )
	{
		VertexShader = NULL;
		PixelShader = NULL;
		MaskedVertexShader = MeshShaderIndex->GetShader<FCascadedShadowMaskedDepthVertexShader>();
		MaskedPixelShader = MeshShaderIndex->GetShader<FCascadedShadowMaskedDepthPixelShader>();
	}
	else
	{
		VertexShader = MeshShaderIndex->GetShader<FCascadedShadowDepthVertexShader>();
		PixelShader = MeshShaderIndex->GetShader<FCascadedShadowDepthPixelShader>();
		MaskedVertexShader = NULL;
		MaskedPixelShader = NULL;
		
	}
}

void FCascadedShadowDepthDrawingPolicy::DrawShared(FCommandContextRHI* Context,const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
{
	// Set the depth-only shader parameters for the material
	if( PixelShader )
	{
		VertexShader->SetParameters(Context, VertexFactory, MaterialInstance, View, ShadowInfo);
		PixelShader->SetParameters(Context, VertexFactory, MaterialInstance, View, ShadowInfo);
	}
	if( MaskedPixelShader )
	{
		MaskedVertexShader->SetParameters(Context, VertexFactory, MaterialInstance, View, ShadowInfo);
		MaskedPixelShader->SetParameters(Context, VertexFactory, MaterialInstance, View, ShadowInfo);
	}
	// Set the shared mesh resources.
	FMeshDrawingPolicy::DrawShared( Context, View );

	// Set the actual shader & vertex declaration state
	RHISetBoundShaderState( Context, BoundShaderState );
}

FBoundShaderStateRHIRef FCascadedShadowDepthDrawingPolicy::CreateBoundShaderState(DWORD DynamicStride)
{
	FVertexDeclarationRHIParamRef VertexDeclaration;
	DWORD StreamStrides[MaxVertexElementCount];

	FMeshDrawingPolicy::GetVertexDeclarationInfo(VertexDeclaration, StreamStrides);
	if (DynamicStride)
	{
		StreamStrides[0] = DynamicStride;
	}

	if( PixelShader )
	{
		return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, VertexShader->GetVertexShader(), PixelShader->GetPixelShader());	
	}
	else
	{
		return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, MaskedVertexShader->GetVertexShader(), MaskedPixelShader->GetPixelShader());
	}
}

void FCascadedShadowDepthDrawingPolicy::SetMeshRenderState(
	FCommandContextRHI* Context,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const ElementDataType& ElementData
	) const
{
	
	if( PixelShader )
	{
		VertexShader->SetLocalTransforms(Context,Mesh.LocalToWorld,Mesh.WorldToLocal);
		PixelShader->SetLocalTransforms(Context,Mesh.MaterialInstance,Mesh.LocalToWorld,bBackFace);
	}
	else
	{
		MaskedVertexShader->SetLocalTransforms(Context,Mesh.LocalToWorld,Mesh.WorldToLocal);
		MaskedPixelShader->SetLocalTransforms(Context,Mesh.MaterialInstance,Mesh.LocalToWorld,bBackFace);
	}
	FMeshDrawingPolicy::SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,ElementData);
}

INT Compare(const FCascadedShadowDepthDrawingPolicy& A,const FCascadedShadowDepthDrawingPolicy& B)
{
	COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
	COMPAREDRAWINGPOLICYMEMBERS(MaskedVertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(MaskedPixelShader);
	COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
	COMPAREDRAWINGPOLICYMEMBERS(MaterialInstance);
	return 0;
}


//FCascadedShadowDepthDrawingPolicy --



UBOOL FCascadedShadowDepthDrawingPolicyFactory::DrawDynamicMesh(
	FCommandContextRHI* Context,
	const FSceneView* View,
	const FCascadedShadowInfo* ShadowInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId 
	)
{
	FCascadedShadowDepthDrawingPolicy DrawingPolicy(Mesh.VertexFactory, GetShadowDepthMaterial(Mesh.MaterialInstance), ShadowInfo);
	DrawingPolicy.DrawShared(Context, View, DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
	DrawingPolicy.SetMeshRenderState(Context, PrimitiveSceneInfo, Mesh, bBackFace, FMeshDrawingPolicy::ElementDataType());
	DrawingPolicy.DrawMesh(Context, Mesh);
	return TRUE;
}

FCascadedShadowInfo::FCascadedShadowInfo(
	const FLightSceneInfo* InLightSceneInfo,
	const FProjectedShadowInitializer& Initializer0,
	const FProjectedShadowInitializer& Initializer1,
	const FProjectedShadowInitializer& Initializer2,
	const FProjectedShadowInitializer& Initializer3,
	const FProjectedShadowInitializer& Initializer4,
	const FProjectedShadowInitializer& Initializer5,
	const FProjectedShadowInitializer& Initializer6,
	const FProjectedShadowInitializer& Initializer7,
	UINT InResolution ):
LightSceneInfo( InLightSceneInfo ),
Resolution(InResolution )
{
	SubjectAndReceiverMatrix[0] = Initializer0.SubjectMatrix;
	SubjectAndReceiverMatrix[1] = Initializer1.SubjectMatrix;
	SubjectAndReceiverMatrix[2] = Initializer2.SubjectMatrix;
	SubjectAndReceiverMatrix[3] = Initializer3.SubjectMatrix;
	SubjectAndReceiverMatrix[4] = Initializer4.SubjectMatrix;
	SubjectAndReceiverMatrix[5] = Initializer5.SubjectMatrix;
	SubjectAndReceiverMatrix[6] = Initializer6.SubjectMatrix;
	SubjectAndReceiverMatrix[7] = Initializer7.SubjectMatrix;

	SubjectMatrix[0] = Initializer0.SubjectMatrix;
	SubjectMatrix[1] = Initializer1.SubjectMatrix;
	SubjectMatrix[2] = Initializer2.SubjectMatrix;
	SubjectMatrix[3] = Initializer3.SubjectMatrix;
	SubjectMatrix[4] = Initializer4.SubjectMatrix;
	SubjectMatrix[5] = Initializer5.SubjectMatrix;
	SubjectMatrix[6] = Initializer6.SubjectMatrix;
	SubjectMatrix[7] = Initializer7.SubjectMatrix;

	ReceiverMatrix[0] = Initializer0.PostSubjectMatrix;
	ReceiverMatrix[1] = Initializer1.PostSubjectMatrix;
	ReceiverMatrix[2] = Initializer2.PostSubjectMatrix;
	ReceiverMatrix[3] = Initializer3.PostSubjectMatrix;
	ReceiverMatrix[4] = Initializer4.PostSubjectMatrix;
	ReceiverMatrix[5] = Initializer5.PostSubjectMatrix;
	ReceiverMatrix[6] = Initializer6.PostSubjectMatrix;
	ReceiverMatrix[7] = Initializer7.PostSubjectMatrix;

	InvReceiverMatrix[0] = ReceiverMatrix[0].Inverse();
	InvReceiverMatrix[1] = ReceiverMatrix[1].Inverse();
	InvReceiverMatrix[2] = ReceiverMatrix[2].Inverse();
	InvReceiverMatrix[3] = ReceiverMatrix[3].Inverse();
	InvReceiverMatrix[4] = ReceiverMatrix[4].Inverse();
	InvReceiverMatrix[5] = ReceiverMatrix[5].Inverse();
	InvReceiverMatrix[6] = ReceiverMatrix[6].Inverse();
	InvReceiverMatrix[7] = ReceiverMatrix[7].Inverse();

	MaxSubjectDepth[0] = Initializer0.MaxSubjectDepth;
	MaxSubjectDepth[1] = Initializer1.MaxSubjectDepth;
	MaxSubjectDepth[2] = Initializer2.MaxSubjectDepth;
	MaxSubjectDepth[3] = Initializer3.MaxSubjectDepth;
	MaxSubjectDepth[4] = Initializer4.MaxSubjectDepth;
	MaxSubjectDepth[5] = Initializer5.MaxSubjectDepth;
	MaxSubjectDepth[6] = Initializer6.MaxSubjectDepth;
	MaxSubjectDepth[7] = Initializer7.MaxSubjectDepth;

	SubjectFrustum[0] = GetViewFrustumBounds(SubjectMatrix[0], FALSE);
	SubjectFrustum[1] = GetViewFrustumBounds(SubjectMatrix[1], FALSE);
	SubjectFrustum[2] = GetViewFrustumBounds(SubjectMatrix[2], FALSE);
	SubjectFrustum[3] = GetViewFrustumBounds(SubjectMatrix[3], FALSE);
	SubjectFrustum[4] = GetViewFrustumBounds(SubjectMatrix[4], FALSE);
	SubjectFrustum[5] = GetViewFrustumBounds(SubjectMatrix[5], FALSE);
	SubjectFrustum[6] = GetViewFrustumBounds(SubjectMatrix[6], FALSE);
	SubjectFrustum[7] = GetViewFrustumBounds(SubjectMatrix[7], FALSE);
}

void FCascadedShadowInfo::Initialize(const FProjectedShadowInitializer& Initializer0,
									 const FProjectedShadowInitializer& Initializer1,
									 const FProjectedShadowInitializer& Initializer2,
									 const FProjectedShadowInitializer& Initializer3,
									 const FProjectedShadowInitializer& Initializer4,
									 const FProjectedShadowInitializer& Initializer5,
									 const FProjectedShadowInitializer& Initializer6,
									 const FProjectedShadowInitializer& Initializer7)
{
	SubjectAndReceiverMatrix[0] = Initializer0.SubjectMatrix;
	SubjectAndReceiverMatrix[1] = Initializer1.SubjectMatrix;
	SubjectAndReceiverMatrix[2] = Initializer2.SubjectMatrix;
	SubjectAndReceiverMatrix[3] = Initializer3.SubjectMatrix;
	SubjectAndReceiverMatrix[4] = Initializer4.SubjectMatrix;
	SubjectAndReceiverMatrix[5] = Initializer5.SubjectMatrix;
	SubjectAndReceiverMatrix[6] = Initializer6.SubjectMatrix;
	SubjectAndReceiverMatrix[7] = Initializer7.SubjectMatrix;

	SubjectMatrix[0] = Initializer0.SubjectMatrix;
	SubjectMatrix[1] = Initializer1.SubjectMatrix;
	SubjectMatrix[2] = Initializer2.SubjectMatrix;
	SubjectMatrix[3] = Initializer3.SubjectMatrix;
	SubjectMatrix[4] = Initializer4.SubjectMatrix;
	SubjectMatrix[5] = Initializer5.SubjectMatrix;
	SubjectMatrix[6] = Initializer6.SubjectMatrix;
	SubjectMatrix[7] = Initializer7.SubjectMatrix;

	ReceiverMatrix[0] = Initializer0.PostSubjectMatrix;
	ReceiverMatrix[1] = Initializer1.PostSubjectMatrix;
	ReceiverMatrix[2] = Initializer2.PostSubjectMatrix;
	ReceiverMatrix[3] = Initializer3.PostSubjectMatrix;
	ReceiverMatrix[4] = Initializer4.PostSubjectMatrix;
	ReceiverMatrix[5] = Initializer5.PostSubjectMatrix;
	ReceiverMatrix[6] = Initializer6.PostSubjectMatrix;
	ReceiverMatrix[7] = Initializer7.PostSubjectMatrix;

	InvReceiverMatrix[0] = ReceiverMatrix[0].Inverse();
	InvReceiverMatrix[1] = ReceiverMatrix[1].Inverse();
	InvReceiverMatrix[2] = ReceiverMatrix[2].Inverse();
	InvReceiverMatrix[3] = ReceiverMatrix[3].Inverse();
	InvReceiverMatrix[4] = ReceiverMatrix[4].Inverse();
	InvReceiverMatrix[5] = ReceiverMatrix[5].Inverse();
	InvReceiverMatrix[6] = ReceiverMatrix[6].Inverse();
	InvReceiverMatrix[7] = ReceiverMatrix[7].Inverse();

	MaxSubjectDepth[0] = Initializer0.MaxSubjectDepth;
	MaxSubjectDepth[1] = Initializer1.MaxSubjectDepth;
	MaxSubjectDepth[2] = Initializer2.MaxSubjectDepth;
	MaxSubjectDepth[3] = Initializer3.MaxSubjectDepth;
	MaxSubjectDepth[4] = Initializer4.MaxSubjectDepth;
	MaxSubjectDepth[5] = Initializer5.MaxSubjectDepth;
	MaxSubjectDepth[6] = Initializer6.MaxSubjectDepth;
	MaxSubjectDepth[7] = Initializer7.MaxSubjectDepth;

	SubjectFrustum[0] = GetViewFrustumBounds(SubjectMatrix[0], FALSE);
	SubjectFrustum[1] = GetViewFrustumBounds(SubjectMatrix[1], FALSE);
	SubjectFrustum[2] = GetViewFrustumBounds(SubjectMatrix[2], FALSE);
	SubjectFrustum[3] = GetViewFrustumBounds(SubjectMatrix[3], FALSE);
	SubjectFrustum[4] = GetViewFrustumBounds(SubjectMatrix[4], FALSE);
	SubjectFrustum[5] = GetViewFrustumBounds(SubjectMatrix[5], FALSE);
	SubjectFrustum[6] = GetViewFrustumBounds(SubjectMatrix[6], FALSE);
	SubjectFrustum[7] = GetViewFrustumBounds(SubjectMatrix[7], FALSE);
}

//<@ 2008. 1. 24 changmin
class AvaBspShadowDepthDrawer : public FPrimitiveDrawInterface
{
public:
	FCommandContextRHI* Context;
	FPrimitiveDrawInterface* PDI;
	FCascadedShadowInfo* ShadowInfo;
	UBOOL IsNear;
	
	AvaBspShadowDepthDrawer( FCommandContextRHI* InContext, FPrimitiveDrawInterface* InPDI, FCascadedShadowInfo* InShadowInfo, UBOOL InIsNear)
		: FPrimitiveDrawInterface(InPDI->View), Context(InContext), PDI(InPDI), ShadowInfo(InShadowInfo), IsNear(InIsNear)
	{}

	virtual UBOOL IsHitTesting()
	{
		return PDI->IsHitTesting();
	}
	virtual void SetHitProxy( HHitProxy* HitProxy )
	{
		PDI->SetHitProxy(HitProxy);
	}
	virtual void RegisterDynamicResource(FDynamicPrimitiveResource* DynamicResource)
	{
		PDI->RegisterDynamicResource(DynamicResource);
	}
	virtual void DrawSprite(
		const FVector& Position,
		FLOAT SizeX,
		FLOAT SizeY,
		const FTexture* Sprite,
		const FLinearColor& Color,
		BYTE DepthPriorityGroup)
	{}
	virtual void DrawLine(
		const FVector& Start,
		const FVector& End,
		const FLinearColor& Color,
		BYTE DepthPriorityGroup)
	{}

	virtual void DrawPoint(
		const FVector& Position,
		const FLinearColor& Color,
		FLOAT PointSize,
		BYTE DepthPriorityGroup) 
	{}
	virtual INT Ava_AddVertex( const FVector& Position, const FVector2D& UV, const FLinearColor& Color, BYTE DepthPriorityGroup )
	{
		return INDEX_NONE;
	}
	virtual void Ava_AddTriangle( INT V0, INT V1, INT V2, const FTexture* Texture, EBlendMode BlendMode, BYTE DepthPriorityGroup)
	{

	}

	/**
	* Determines whether a particular material will be ignored in this context.
	* @param MaterialInstance - The render proxy of the material to check.
	* @return TRUE if meshes using the material will be ignored in this context.
	*/
	virtual UBOOL IsMaterialIgnored(const FMaterialInstance* MaterialInstance) const
	{
		return PDI->IsMaterialIgnored(MaterialInstance);
	}

	/**
	* Draw a mesh element.
	* This should only be called through the DrawMesh function.
	*/
	virtual void DrawMesh(const FMeshElement& Mesh)
	{
		check( Mesh.NumPrimitives <= 4096);
		INT StartIndex = 0;
		INT EndIndex = 4;
		if( !IsNear )
		{
			StartIndex = 4;
			EndIndex = GNumCascadedShadow;
		}
		for( INT SliceIndex = StartIndex; SliceIndex < EndIndex; ++SliceIndex )
		{
			if( ShadowInfo->bHasReceiver[SliceIndex] )
			{
				ShadowInfo->SetViewport( Context, SliceIndex );
				GSliceIndex = SliceIndex;	// shader에서 참조할 겁니다..
				PDI->DrawMesh(Mesh);
			}
		}
	}
};
//>@ 2008. 1. 24

void FCascadedShadowInfo::RenderDepth( FCommandContextRHI *Context, const FSceneRenderer *SceneRenderer, BYTE DepthPriorityGroup, UBOOL IsNear )
{
	INT StartIndex = 0;
	INT EndIndex = 4;
	if( !IsNear )
	{
		StartIndex = 4;
		EndIndex = GNumCascadedShadow;
	}

	// Choose an arbitrary view where this shadow is in the right DPG to use for rendering the depth.
	const FViewInfo* View = NULL;
	for(INT ViewIndex = 0;ViewIndex < SceneRenderer->Views.Num();ViewIndex++)
	{
		if(GetDepthPriorityGroup(&SceneRenderer->Views(ViewIndex)) == DepthPriorityGroup)
		{
			View = &SceneRenderer->Views(ViewIndex);
			break;
		}
	}
	check(View);

	// 4개의 shadow depth를 그립니다.

	// clear shadow depth
	for( INT SliceIndex = StartIndex; SliceIndex < EndIndex; ++SliceIndex )
	{
		if( bHasReceiver[SliceIndex] )
		{
			// Set the viewport for the shadow
			// clear 하는 viewport는 shadow border까지 포함합니다.
			// this->SetViewport는 shadow border를 제외한 viewport입니다.
			RHISetViewport(
				Context,
				X[SliceIndex],
				Y[SliceIndex],
				0.0f,
				X[SliceIndex] + SHADOW_BORDER * 2 + Resolution,
				Y[SliceIndex] + SHADOW_BORDER * 2 + Resolution,
				1.0f);

			if( GSupportsDepthTextures || GSupportsHardwarePCF || GSupportsFetch4 )
			{
				// Clear depth only
				RHIClear( Context, FALSE, FColor(255, 255, 255), TRUE, 1.0f, FALSE, 0);
			}
			else
			{
				// Clear color and depth
				RHIClear( Context, TRUE, FColor(255, 255, 255), TRUE, 1.0f, FALSE, 0);
			}
		}
	}

	// Opaque blending, depth tests and writes, solid rasterization w/ back-face culling.
	RHISetBlendState(Context,TStaticBlendState<>::GetRHI());	
	RHISetDepthState(Context,TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());
	RHISetRasterizerState(Context,TStaticRasterizerState<FM_Solid,CM_CW>::GetRHI());

	//@HACK deif :)
	extern UBOOL GDrawingShadowDepth;
	GDrawingShadowDepth = TRUE;
	//@HACK end
	
	// render bsp 
	{
		SCOPE_CYCLE_COUNTER(STAT_AVA_CascadedShadow_BspRenderDepth);

		const UINT ListIndex = (IsNear ? 0:1);
		BspRendering_StartBatch(BSP_OVERRIDEMATERIAL);
		// Draw the subject's dynamic elements.
		TDynamicPrimitiveDrawer<FCascadedShadowDepthDrawingPolicyFactory> Drawer(Context,View,DepthPriorityGroup,this,TRUE);
		for(INT PrimitiveIndex = 0;PrimitiveIndex < BspPrimitives[ListIndex].Num();PrimitiveIndex++)
		{
			Drawer.SetPrimitive(BspPrimitives[ListIndex](PrimitiveIndex));
			BspPrimitives[ListIndex](PrimitiveIndex)->Proxy->DrawDynamicElements(&Drawer,View,DepthPriorityGroup);
		}
		AvaBspShadowDepthDrawer BspShadowDepthDrawer(Context, &Drawer, this, IsNear);
		BspRendering_EndBatch(&BspShadowDepthDrawer);
	}

	// render all except bsp
	{
		SCOPE_CYCLE_COUNTER(STAT_AVA_CascadedShadow_OtherRenderDepth);

		for( INT SliceIndex = StartIndex; SliceIndex < EndIndex; ++SliceIndex )
		{
			if( bHasReceiver[SliceIndex] )
			{
				SetViewport(Context, SliceIndex);

				//@Hack for depth vertex/pixel shader
				// 좀더 나은 방법...
				GSliceIndex = SliceIndex;

				// Draw the subject's static elements.
				SubjectMeshElements[SliceIndex].AVA_DrawAllShadowDepth(Context,View);

				extern void ParticleRendering_StartBatch( UBOOL bTranslucentPass );
				extern void ParticleRendering_EndBatch( FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex );
				ParticleRendering_StartBatch(FALSE);
				// Draw the subject's dynamic elements.
				TDynamicPrimitiveDrawer<FCascadedShadowDepthDrawingPolicyFactory> Drawer(Context,View,DepthPriorityGroup,this,TRUE);
				for(INT PrimitiveIndex = 0;PrimitiveIndex < SubjectPrimitives[SliceIndex].Num();PrimitiveIndex++)
				{
					Drawer.SetPrimitive(SubjectPrimitives[SliceIndex](PrimitiveIndex));
					SubjectPrimitives[SliceIndex](PrimitiveIndex)->Proxy->DrawDynamicElements(&Drawer,View,DepthPriorityGroup);
				}
				ParticleRendering_EndBatch(&Drawer, View, DepthPriorityGroup);
			}

		} // end loop of slices
	}

	//@HACK deif :)
	GDrawingShadowDepth = FALSE;
	//@HACK end
}
extern TGlobalResource<FShadowFrustumVertexDeclaration> GShadowFrustumVertexDeclaration;

FBoundShaderStateRHIRef FCascadedShadowInfo::MaskBoundShaderState;
FGlobalBoundShaderStateRHIRef FCascadedShadowInfo::ShadowProjectionBoundShaderState;
FGlobalBoundShaderStateRHIRef FCascadedShadowInfo::NearShadowProjectionBoundShaderState;


/** Encapsulates the cascaded vertex shader. */
class FCascadedShadowProjectionVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FCascadedShadowProjectionVertexShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return !IsSM2Platform(Platform);
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
	}

	/** Default constructor. */
	FCascadedShadowProjectionVertexShader() {}

public:

	FCascadedShadowProjectionVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
	}
};
IMPLEMENT_SHADER_TYPE(,FCascadedShadowProjectionVertexShader,TEXT("CascadedShadowProjectionVertexShader"),TEXT("Main"),SF_Vertex,0,0);

extern TGlobalResource<FFilterVertexDeclaration> GFilterVertexDeclaration;

void DrawDenormalizedQuadWithDepth(
						  FCommandContextRHI* Context,
						  FLOAT X,
						  FLOAT Y,
						  FLOAT Z,
						  FLOAT W,
						  FLOAT SizeX,
						  FLOAT SizeY,
						  FLOAT U,
						  FLOAT V,
						  FLOAT SizeU,
						  FLOAT SizeV,
						  UINT TargetSizeX,
						  UINT TargetSizeY,
						  UINT TextureSizeX,
						  UINT TextureSizeY
						  )
{
	// Set up the vertices.
	FFilterVertex Vertices[4];

	Vertices[0].Position = FVector4(X,			Y,			Z,	W);
	Vertices[1].Position = FVector4(X + SizeX,	Y,			Z,	W);
	Vertices[2].Position = FVector4(X,			Y + SizeY,	Z,	W);
	Vertices[3].Position = FVector4(X + SizeX,	Y + SizeY,	Z,	W);

	Vertices[0].UV = FVector2D(U,			V);
	Vertices[1].UV = FVector2D(U + SizeU,	V);
	Vertices[2].UV = FVector2D(U,			V + SizeV);
	Vertices[3].UV = FVector2D(U + SizeU,	V + SizeV);

	for(INT VertexIndex = 0;VertexIndex < 4;VertexIndex++)
	{
		Vertices[VertexIndex].Position.X = -1.0f + 2.0f * (Vertices[VertexIndex].Position.X - GPixelCenterOffset) / (FLOAT)TargetSizeX;
		Vertices[VertexIndex].Position.Y = +1.0f - 2.0f * (Vertices[VertexIndex].Position.Y - GPixelCenterOffset) / (FLOAT)TargetSizeY;

		Vertices[VertexIndex].Position.X *= W;
		Vertices[VertexIndex].Position.Y *= W;



		Vertices[VertexIndex].UV.X = Vertices[VertexIndex].UV.X / (FLOAT)TextureSizeX;
		Vertices[VertexIndex].UV.Y = Vertices[VertexIndex].UV.Y / (FLOAT)TextureSizeY;
	}

	static WORD Indices[] =
	{
		0, 1, 3,
		0, 3, 2
	};

	RHIDrawIndexedPrimitiveUP(Context,PT_TriangleList,0,4,2,Indices,sizeof(Indices[0]),Vertices,sizeof(Vertices[0]));
}

UBOOL GNearCascadedShadowProjection = FALSE;
void FCascadedShadowInfo::RenderProjection( FCommandContextRHI* Context, const class FViewInfo* View, UBOOL bIsNear, UBOOL bWorldDpg ) const
{
	SCOPE_CYCLE_COUNTER(STAT_AVA_CascadedShadow_RenderProjection);

	GNearCascadedShadowProjection = bIsNear;

	// Determine the shadow's DPG.
	BYTE ShadowDPG = GetDepthPriorityGroup(View);

	// Find the projection shaders.
	TShaderMapRef<FCascadedShadowProjectionVertexShader> VertexShader(GetGlobalShaderMap());

	// no depth test or writes, solid rasterization w/ back-face culling.
	//RHISetDepthState(Context,TStaticDepthState<FALSE,CF_Always>::GetRHI());
	RHISetDepthState(Context,TStaticDepthState<FALSE,CF_Less>::GetRHI());
	RHISetColorWriteEnable(Context,TRUE);
	RHISetRasterizerState(Context, TStaticRasterizerState<FM_Solid, CM_None>::GetRHI());
	RHISetBlendState(Context,TStaticBlendState<>::GetRHI());
	FCascadedShadowProjectionPixelShaderInterface *PixelShader = GetCascadedShadowProjectionPixelShaderRef( bIsNear );
	PixelShader->SetParameters(Context,View,this);

	if( bIsNear )
	{
		if( GIsATI )
		{
			if( bWorldDpg )
				RHISetStencilState(Context,
				TStaticStencilState<
				TRUE,CF_Equal,SO_Keep,SO_Keep,SO_Keep,
				FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
				0xff,0xff,2>::GetRHI());
			else
				RHISetStencilState(Context,
				TStaticStencilState<
				TRUE,CF_Equal,SO_Keep,SO_Keep,SO_Keep,
				FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
				0xff,0xff,1>::GetRHI());
		}

		SetGlobalBoundShaderState(Context,
			NearShadowProjectionBoundShaderState,
			GFilterVertexDeclaration.VertexDeclarationRHI, 
			*VertexShader,
			PixelShader,
			sizeof(FFilterVertex));
	}
	else
	{
		if( GIsATI )
		{
			RHISetStencilState(Context,
				TStaticStencilState<
				TRUE,CF_Equal,SO_Keep,SO_Keep,SO_Keep,
				FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
				0xff,0xff,1>::GetRHI());
		}

		SetGlobalBoundShaderState(Context,
			ShadowProjectionBoundShaderState,
			GFilterVertexDeclaration.VertexDeclarationRHI, 
			*VertexShader,
			PixelShader,
			sizeof(FFilterVertex));
	}

	// Draw a quad mapping the blurred pixels in the filter buffer to the scene color buffer.
	DrawDenormalizedQuad(
		Context,
		View->X,View->Y,
		View->SizeX,View->SizeY,
		View->RenderTargetX,View->RenderTargetY,
		View->RenderTargetSizeX,View->RenderTargetSizeY,
		View->Family->RenderTarget->GetSizeX(),View->Family->RenderTarget->GetSizeY(),
		GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY());
}

void FCascadedShadowInfo::RenderFrustumWireframe( FPrimitiveDrawInterface *PDI ) const
{
	for( INT SliceIndex = 0; SliceIndex < FCascadedShadowInfo::NumShadows; ++SliceIndex )
	{
		DrawFrustumWireframe(
			PDI,
			ReceiverMatrix[SliceIndex],
			FColor(255, 255, 255),
			SDPG_Foreground);
	}
}

void FCascadedShadowInfo::AddSubjectPrimitive( FPrimitiveSceneInfo *PrimitiveSceneInfo, UINT SliceIndex )
{
	if( !PrimitiveSceneInfo->StaticMeshes.Num() )
	{
		// Add the primitive to the subject primitive list.
		SubjectPrimitives[SliceIndex].AddItem(PrimitiveSceneInfo);
	}
	else
	{
		// Add the primitive's static mesh elements to the draw lists.
		for( INT MeshIndex = 0; MeshIndex < PrimitiveSceneInfo->StaticMeshes.Num(); ++MeshIndex )
		{
			FStaticMesh *StaticMesh = &PrimitiveSceneInfo->StaticMeshes(MeshIndex);
			if( StaticMesh->MaterialInstance->GetMaterial()->GetBlendMode() == BLEND_Opaque 
			|| StaticMesh->MaterialInstance->GetMaterial()->GetBlendMode() == BLEND_Masked)
			{
				SubjectMeshElements[SliceIndex].AddMesh(
					StaticMesh,
					FCascadedShadowDepthDrawingPolicy::ElementDataType(),
					FCascadedShadowDepthDrawingPolicy(StaticMesh->VertexFactory, GetShadowDepthMaterial(StaticMesh->MaterialInstance),this)
					);
			}
		}
	}
}

void FCascadedShadowInfo::AddModelPrimitive( FPrimitiveSceneInfo* PrimitiveSceneInfo, UINT ListIndex )
{
	BspPrimitives[ListIndex].AddItem(PrimitiveSceneInfo);
}

void FCascadedShadowInfo::GetSubjectPrimitives( TArray<const FPrimitiveSceneInfo*>& OutSubjectPrimitives )
{
	//OutSubjectPrimitives += SubjectPrimitives;
}

//<@ ava specific ; 2008. 1. 15 changmin
void CreateShadowFrustum( const FLightSceneInfo *LightSceneInfo,  const FBox& Bounds, FConvexVolume* ShadowFrustum, FMatrix* CasterMatrix )
{
	struct FShadowProjectionMatrix: FMatrix
	{
		FShadowProjectionMatrix(FLOAT MinZ,FLOAT MaxZ,const FVector4& WAxis):
	FMatrix(
		FPlane(1,	0,	0,													WAxis.X),
		FPlane(0,	1,	0,													WAxis.Y),
		FPlane(0,	0,	(WAxis.Z * MaxZ + WAxis.W) / (MaxZ - MinZ),			WAxis.Z),
		FPlane(0,	0,	-MinZ * (WAxis.Z * MaxZ + WAxis.W) / (MaxZ - MinZ),	WAxis.W))
	{}
	};

	// compute light matrix
	FVector XAxis, YAxis;
	const FVector FaceDirection = FVector( 1.0f, 0.0f, 0.0f );
	FaceDirection.FindBestAxisVectors( XAxis, YAxis );
	const FBasisVectorMatrix BasisMatrix = FBasisVectorMatrix( -XAxis, YAxis, FaceDirection.SafeNormal(), FVector(0,0,0));
	const FMatrix WorldToLightSpace = FInverseRotationMatrix(
		FVector(LightSceneInfo->WorldToLight.M[0][2],
		LightSceneInfo->WorldToLight.M[1][2],
		LightSceneInfo->WorldToLight.M[2][2]).SafeNormal().Rotation()) * BasisMatrix;
	const FMatrix LightToWorldSpace = WorldToLightSpace.Inverse();

	FBox BoundsInLightSpace = Bounds.TransformBy(WorldToLightSpace);
	const FLOAT XRange = BoundsInLightSpace.Max.X - BoundsInLightSpace.Min.X;
	const FLOAT YRange = BoundsInLightSpace.Max.Y - BoundsInLightSpace.Min.Y;
	const FLOAT ZRange = BoundsInLightSpace.Max.Z - BoundsInLightSpace.Min.Z;
	const FLOAT	NearLightPlane	= -(ZRange * 0.5f);

	const FLOAT FarLightPlane = HALF_WORLD_MAX;

	const FVector CenterPosition = LightToWorldSpace.TransformFVector( BoundsInLightSpace.GetCenter() );
	FMatrix WorldToLight = FTranslationMatrix(-CenterPosition)
		* FInverseRotationMatrix(
			FVector(LightSceneInfo->WorldToLight.M[0][2],
					LightSceneInfo->WorldToLight.M[1][2],
					LightSceneInfo->WorldToLight.M[2][2]).SafeNormal().Rotation())
		* FScaleMatrix( FVector( 1.0f, 1.0f / (YRange*0.5f), 1.0f / (XRange*0.5f)));

	FMatrix WorldToFace = WorldToLight * BasisMatrix;
	FVector4 WAxis = FVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	FMatrix ResultMatrix = WorldToFace * FShadowProjectionMatrix(NearLightPlane, FarLightPlane, WAxis);
	if( CasterMatrix )
		*CasterMatrix = ResultMatrix;

	// 면이 4개 밖에 안 생긴다.. 조사할 것..
	if( ShadowFrustum )
		*ShadowFrustum = GetViewFrustumBounds( ResultMatrix, TRUE );
}
//>@

//<@ ava specific ; 2008. 1. 18 changmin
// add cascaded shadow
AvaCasterOcclusionQueryBatcher::AvaCasterOcclusionQueryBatcher(class FSceneViewState* ViewState, UINT InMaxBatchedPrimitives )
: CurrentBatchOcclusionQuery(FOcclusionQueryRHIRef())
, MaxBatchedPrimitives(InMaxBatchedPrimitives)
, NumBatchedPrimitives(0)
, OcclusionQueryPool(ViewState ? &ViewState->OcclusionQueryPool : NULL)
{}

AvaCasterOcclusionQueryBatcher::~AvaCasterOcclusionQueryBatcher()
{
	check(!Primitives.Num());
}

void AvaCasterOcclusionQueryBatcher::Flush(FCommandContextRHI* Context)
{
	if( BatchOcclusionQueries.Num())
	{
		WORD* BakedIndices = new WORD[MaxBatchedPrimitives * 12 * 3];
		for( UINT PrimitiveIndex = 0; PrimitiveIndex < MaxBatchedPrimitives;PrimitiveIndex++)
		{
			for( INT Index = 0; Index < 12*3;Index++)
			{
				BakedIndices[PrimitiveIndex*12*3 + Index] = PrimitiveIndex * 8 + GCubeIndices[Index];
			}
		}

		// Draw the batches.
		for( INT BatchIndex = 0; BatchIndex < BatchOcclusionQueries.Num(); ++BatchIndex )
		{
			FOcclusionQueryRHIParamRef BatchOcclusionQuery = BatchOcclusionQueries(BatchIndex);
			const INT NumPrimitivesInBatch = Min<INT>(Primitives.Num() - BatchIndex * MaxBatchedPrimitives, MaxBatchedPrimitives);
			INT NumChildPrimitivesInBatch = 0;
			for( INT PrimitiveIndex = 0 ; PrimitiveIndex < NumPrimitivesInBatch ; ++PrimitiveIndex)
			{
				const FPrimitive& Primitive = Primitives(BatchIndex * MaxBatchedPrimitives + PrimitiveIndex );
				NumChildPrimitivesInBatch += Primitive.NumChildren;
			}
			const INT NumTotalPrimitivesInBatch = NumPrimitivesInBatch + NumChildPrimitivesInBatch;
			RHIBeginOcclusionQuery(Context, BatchOcclusionQuery );
			
			FLOAT* RESTRICT Vertices;
			WORD* RESTRICT Indices;
			RHIBeginDrawIndexedPrimitiveUP(Context, PT_TriangleList, NumTotalPrimitivesInBatch * 12, NumTotalPrimitivesInBatch * 8,
				sizeof(FVector), *(void**)&Vertices, 0, NumTotalPrimitivesInBatch * 12 * 3, sizeof(WORD), *(void**)&Indices);

			for( INT PrimitiveIndex = 0; PrimitiveIndex < NumPrimitivesInBatch; PrimitiveIndex ++ )
			{
				const FPrimitive& Primitive = Primitives(BatchIndex * MaxBatchedPrimitives + PrimitiveIndex);
				const UINT BaseVertexIndex = PrimitiveIndex * 8;

				if( Primitive.PrimitiveSceneInfo->ShadowVolumeVerticesId != INDEX_NONE )
				{
					// Generate vertices for the shadow's frustum.
					FVector* ShadowFrustumVertices = Primitive.PrimitiveSceneInfo->Scene->StaticSunShadowVertices(Primitive.PrimitiveSceneInfo->ShadowVolumeVerticesId).Vertices;
					for(UINT VertexIndex = 0; VertexIndex < 4; VertexIndex++)
					{
						*Vertices++ = ShadowFrustumVertices->X;
						*Vertices++ = ShadowFrustumVertices->Y;
						*Vertices++ = ShadowFrustumVertices->Z;
						ShadowFrustumVertices++;

						*Vertices++ = ShadowFrustumVertices->X;
						*Vertices++ = ShadowFrustumVertices->Y;
						*Vertices++ = ShadowFrustumVertices->Z;
						ShadowFrustumVertices++;
					}
				}
				else
				{
					FMatrix CasterMatrix;
					CreateShadowFrustum( Primitive.LightSceneInfo, Primitive.BoundingBox, NULL, &CasterMatrix );
					FMatrix InvCasterMatrix = CasterMatrix.Inverse();

					// Generate vertices for the shadow's frustum.
					for(UINT Z = 0;Z < 2;Z++)
					{
						for(UINT Y = 0;Y < 2;Y++)
						{
							for(UINT X = 0;X < 2;X++)
							{
								const FVector4 UnprojectedVertex = InvCasterMatrix.TransformFVector4(
									FVector4(
									(X ? -1.0f : 1.0f),
									(Y ? -1.0f : 1.0f),
									(Z ?  1.0f : 0.0f),
									1.0f
									)
									);
								const FVector ProjectedVertex = UnprojectedVertex / UnprojectedVertex.W;
								Vertices[GetCubeVertexIndex(X,Y,Z)*3] = ProjectedVertex.X;
								Vertices[GetCubeVertexIndex(X,Y,Z)*3+1] = ProjectedVertex.Y;
								Vertices[GetCubeVertexIndex(X,Y,Z)*3+2] = ProjectedVertex.Z;
							}
						}
					}
					Vertices += 24;
				}



				for (INT Child = Primitive.FirstChild; Child >= 0; Child = ChildPrimitives(Child).NextChild)
				{
					const FPrimitive& Primitive = ChildPrimitives(Child);

					FMatrix CasterMatrix;
					CreateShadowFrustum( Primitive.LightSceneInfo, Primitive.BoundingBox, NULL, &CasterMatrix );
					FMatrix InvCasterMatrix = CasterMatrix.Inverse();

					const UINT BaseVertexIndex = PrimitiveIndex * 8;

					// Generate vertices for the shadow's frustum.
					for(UINT Z = 0;Z < 2;Z++)
					{
						for(UINT Y = 0;Y < 2;Y++)
						{
							for(UINT X = 0;X < 2;X++)
							{
								const FVector4 UnprojectedVertex = InvCasterMatrix.TransformFVector4(
									FVector4(
									(X ? -1.0f : 1.0f),
									(Y ? -1.0f : 1.0f),
									(Z ?  1.0f : 0.0f),
									1.0f
									)
									);
								const FVector ProjectedVertex = UnprojectedVertex / UnprojectedVertex.W;
								Vertices[GetCubeVertexIndex(X,Y,Z)*3] = ProjectedVertex.X;
								Vertices[GetCubeVertexIndex(X,Y,Z)*3+1] = ProjectedVertex.Y;
								Vertices[GetCubeVertexIndex(X,Y,Z)*3+2] = ProjectedVertex.Z;
							}
						}
					}

					Vertices += 24;
				}
			}

			const INT CopyablePrimitives = Min<INT>( NumTotalPrimitivesInBatch, MaxBatchedPrimitives );

			appMemcpy(Indices,BakedIndices,sizeof(WORD) * CopyablePrimitives * 12 * 3);

			for (INT PrimitiveIndex=CopyablePrimitives; PrimitiveIndex<NumTotalPrimitivesInBatch; ++PrimitiveIndex)
			{				
				for(INT Index = 0;Index < 12*3;Index++)
				{
					Indices[PrimitiveIndex * 12 * 3 + Index] = PrimitiveIndex * 8 + GCubeIndices[Index];
				}
			}

			RHIEndDrawIndexedPrimitiveUP(Context);
			RHIEndOcclusionQuery(Context,BatchOcclusionQuery);
		}

		delete[] BakedIndices;

		INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_OcclusionQueries,BatchOcclusionQueries.Num());

		// Reset the batch state.
		BatchOcclusionQueries.Empty();
		Primitives.Empty();
		ChildPrimitives.Empty();
		CurrentBatchOcclusionQuery = FOcclusionQueryRHIRef();
	}
}

INT AvaCasterOcclusionQueryBatcher::BatchPrimitive( const FBoxSphereBounds& Bounds, const FLightSceneInfo* LightSceneInfo, const FPrimitiveSceneInfo* PrimitiveSceneInfo )
{
	// Check if the current batch is full.
	if( !IsValidRef(CurrentBatchOcclusionQuery) || NumBatchedPrimitives >= MaxBatchedPrimitives )
	{
		check(OcclusionQueryPool);
		CurrentBatchOcclusionQuery = OcclusionQueryPool->Allocate();
		BatchOcclusionQueries.AddItem(CurrentBatchOcclusionQuery);
		NumBatchedPrimitives = 0;
	}

	// add the primitive to the current batch
	FPrimitive * const Primitive = new(Primitives) FPrimitive;
	Primitive->BoundingBox = Bounds.GetBox();
	Primitive->LightSceneInfo = LightSceneInfo;
	Primitive->OcclusionQuery = CurrentBatchOcclusionQuery;
	Primitive->FirstChild = -1;
	Primitive->NumChildren = 0;
	Primitive->PrimitiveSceneInfo = PrimitiveSceneInfo;
	NumBatchedPrimitives++;

	return Primitives.Num() - 1;
}

UBOOL AvaCasterOcclusionQueryBatcher::BatchChildPrimitive(INT ParentIndex, const FBoxSphereBounds& Bounds, const FLightSceneInfo* LightSceneInfo, const FPrimitiveSceneInfo* PrimitiveSceneInfo)
{
	if( ParentIndex < 0 || ParentIndex >= Primitives.Num() )
	{
		return FALSE;
	}
	FPrimitive& Parent = Primitives(ParentIndex);

	// Add the primitive to the current batch.
	FPrimitive* const Primitive = new(ChildPrimitives) FPrimitive;
	Primitive->BoundingBox = Bounds.GetBox();
	Primitive->LightSceneInfo = LightSceneInfo;
	Primitive->NextChild = Parent.FirstChild;
	Primitive->PrimitiveSceneInfo = PrimitiveSceneInfo;
	Parent.FirstChild = ChildPrimitives.Num() - 1;
	Parent.NumChildren++;

	return TRUE;
}



//>@ ava

//<@ ava specific ; 2008. 1. 14 changmin
void FCascadedShadowInfo::BeginOcclusionTest( FViewInfo& View, FCommandContextRHI *GlobalContext )
{
	FSceneViewState* ViewState = (FSceneViewState*)View.State;
	if( ViewState && !View.bDisableQuerySubmissions )
	{
		ViewState->Ava_TrimCasterOcclusionHistory( View.Family->CurrentRealTime - GEngine->PrimitiveProbablyVisibleTime, View.Family->CurrentRealTime );
	}
}
//>@ ava



//<@ ava specific ; 2007. 9. 20 changmin
// add cascaded shadow
void FSceneRenderer::Ava_CreateCascadedShadows( const FLightSceneInfo *LightSceneInfo )
{
	SCOPE_CYCLE_COUNTER(STAT_AVA_CascadedShadow_Create);

	struct FShadowProjectionMatrix: FMatrix
	{
		FShadowProjectionMatrix(FLOAT MinZ,FLOAT MaxZ,const FVector4& WAxis):
		FMatrix(
			FPlane(1,	0,	0,													WAxis.X),
			FPlane(0,	1,	0,													WAxis.Y),
			FPlane(0,	0,	(WAxis.Z * MaxZ + WAxis.W) / (MaxZ - MinZ),			WAxis.Z),
			FPlane(0,	0,	-MinZ * (WAxis.Z * MaxZ + WAxis.W) / (MaxZ - MinZ),	WAxis.W))
	{}
	};

	// compute light matrix
	FVector XAxis, YAxis;
	const FVector FaceDirection = FVector( 1.0f, 0.0f, 0.0f );
	FaceDirection.FindBestAxisVectors( XAxis, YAxis );
	const FBasisVectorMatrix BasisMatrix = FBasisVectorMatrix( -XAxis, YAxis, FaceDirection.SafeNormal(), FVector(0,0,0));
	const FMatrix WorldToLightSpace =
		FInverseRotationMatrix(
			FVector(LightSceneInfo->WorldToLight.M[0][2],
					LightSceneInfo->WorldToLight.M[1][2],
					LightSceneInfo->WorldToLight.M[2][2]).SafeNormal().Rotation()) * BasisMatrix;
	const FMatrix LightToWorldSpace = WorldToLightSpace.Inverse();
	const FVector4 WAxis = FVector4( 0.0f, 0.0f, 0.0f, 1.0f );

	for( INT ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex )
	{
		FViewInfo &View = Views( ViewIndex );

		// create shadow
		FMatrix WorldToLight[FCascadedShadowInfo::NumShadows];
		FMatrix WorldToFace[FCascadedShadowInfo::NumShadows];
		FLOAT NearLightPlane[FCascadedShadowInfo::NumShadows];
		FLOAT FarLightPlane[FCascadedShadowInfo::NumShadows];
		FConvexVolume SubjectFrustum[FCascadedShadowInfo::NumShadows];
		appMemzero( NearLightPlane, sizeof(FLOAT) * FCascadedShadowInfo::NumShadows );
		appMemzero( FarLightPlane, sizeof(FLOAT) * FCascadedShadowInfo::NumShadows );

		// compute light frustum of each splited view frustum
		UBOOL bInitShadows = TRUE;
		FProjectedShadowInitializer ShadowInitializer[FCascadedShadowInfo::NumShadows];
		for( INT SplitIndex = 0; SplitIndex < FCascadedShadowInfo::NumShadows; ++SplitIndex )
		{
			// compute light space split bounds
			View.Ava_SplitBounds[SplitIndex].Init();
			for( INT VertexIndex = 0; VertexIndex < 8; ++VertexIndex )
			{
				FVector Vertex = WorldToLightSpace.TransformFVector(View.Ava_SplitedFrustumVertices[SplitIndex * 8 + VertexIndex]);
				View.Ava_SplitBounds[SplitIndex] += Vertex;
			}

			// compute intersection between splitbounds and recevier bounds
			if( View.Ava_ReceiverBounds[SplitIndex].IsValid )
			{
				FVector& MinPoint = View.Ava_SplitBounds[SplitIndex].Min;
				FVector& MaxPoint = View.Ava_SplitBounds[SplitIndex].Max;

				MinPoint.X = Max<FLOAT>( MinPoint.X, View.Ava_ReceiverBounds[SplitIndex].Min.X);
				MinPoint.Y = Max<FLOAT>( MinPoint.Y, View.Ava_ReceiverBounds[SplitIndex].Min.Y);
				MinPoint.Z = Max<FLOAT>( MinPoint.Z, View.Ava_ReceiverBounds[SplitIndex].Min.Z);

				MaxPoint.X = Min<FLOAT>( MaxPoint.X, View.Ava_ReceiverBounds[SplitIndex].Max.X);
				MaxPoint.Y = Min<FLOAT>( MaxPoint.Y, View.Ava_ReceiverBounds[SplitIndex].Max.Y);
				MaxPoint.Z = Min<FLOAT>( MaxPoint.Z, View.Ava_ReceiverBounds[SplitIndex].Max.Z);
			}

			// get split bounds
			FBoxSphereBounds Bounds( View.Ava_SplitBounds[SplitIndex] );

			const FLOAT XRange = View.Ava_SplitBounds[SplitIndex].Max.X - View.Ava_SplitBounds[SplitIndex].Min.X;
			const FLOAT YRange = View.Ava_SplitBounds[SplitIndex].Max.Y - View.Ava_SplitBounds[SplitIndex].Min.Y;
			const FLOAT ZRange = View.Ava_SplitBounds[SplitIndex].Max.Z - View.Ava_SplitBounds[SplitIndex].Min.Z;
			NearLightPlane[SplitIndex]	= -(ZRange * 0.5f);
			FarLightPlane[SplitIndex]	= +(ZRange * 0.5f);

			Bounds.Origin = LightToWorldSpace.TransformFVector( Bounds.Origin );

			WorldToLight[SplitIndex] = FTranslationMatrix(-(FVector)Bounds.GetSphere())
										* FInverseRotationMatrix(
											FVector(LightSceneInfo->WorldToLight.M[0][2],
													LightSceneInfo->WorldToLight.M[1][2],
													LightSceneInfo->WorldToLight.M[2][2]).SafeNormal().Rotation())
										* FScaleMatrix( FVector( 1.0f, 1.0f / (YRange*0.5f), 1.0f / (XRange*0.5f)));
									
			WorldToFace[SplitIndex] = WorldToLight[SplitIndex] * BasisMatrix;
			FLOAT MaxSubjectZ = WorldToFace[SplitIndex].TransformFVector(Bounds.GetSphere()).Z + Bounds.GetSphere().W;
			FLOAT MinLightW = -Bounds.GetSphere().W * 2;
			FMatrix SubjectMatrix = WorldToFace[SplitIndex] * FShadowProjectionMatrix( MinLightW, MaxSubjectZ, WAxis);
			SubjectFrustum[SplitIndex] = GetViewFrustumBounds(SubjectMatrix, FALSE);	// near plane은 사용하지 않는다. direction light..

			// initialize shadows
			if( !LightSceneInfo->GetProjectedShadowInitializer( Bounds.GetSphere(), ShadowInitializer[SplitIndex] ) )
			{
				bInitShadows = FALSE;
			}


		}

		// create shadow
		// add primitives as subject
		INT AllSubjectCount = 0;
		INT NearBspCasters= 0;
		INT FarBspCasters = 0;
		INT SubjectCount[8];
		INT CasterCount = 0;
		INT VisibleCasterCount = 0;
		INT OccludedCount = 0;
		memset( SubjectCount, 0, sizeof(INT)*8);

		if( bInitShadows )
		{
			const INT ShadowResolution = 1014;
			//const INT ShadowResolution = 512;
			FCascadedShadowInfo *CascadedShadowInfo = new FCascadedShadowInfo(
				LightSceneInfo,
				ShadowInitializer[0],
				ShadowInitializer[1],
				ShadowInitializer[2],
				ShadowInitializer[3],
				ShadowInitializer[4],
				ShadowInitializer[5],
				ShadowInitializer[6],
				ShadowInitializer[7],
				ShadowResolution);
			CascadedShadows.AddRawItem( CascadedShadowInfo );

			// TO DO : 전체 box를 계산해서 near을 땡기거나 미는 작업을 해야 한다..........
			//  Shadow Frustum 의 Width 및 Height를 Light Space에서 할 경우..... View의 회전에 따라 그림자가 흔들릴 수 있다..
			// World Space에서 하고.... Near Far 를 고정시키면... Static 하게 보이지 않을까?

			// compute receiver and caster bounds / receiver list
			FBox CasterBounds[FCascadedShadowInfo::NumShadows];
			for( INT ShadowIndex = 0; ShadowIndex < FCascadedShadowInfo::NumShadows; ++ShadowIndex )
			{
				CasterBounds[ShadowIndex].Init();
			}
			for( TSparseArray<FPrimitiveSceneInfoCompact>::TConstIterator PrimitiveIt(Scene->Primitives);PrimitiveIt;++PrimitiveIt)
			{
				const FPrimitiveSceneInfoCompact &CompactPrimitiveSceneInfo = *PrimitiveIt;
				if( CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Proxy->GetDepthPriorityGroup(&View) != SDPG_World
				|| !CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Proxy->bCastSunShadow
				|| (CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Component->HiddenGame && !CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Component->bCastHiddenShadow))
				{
					continue;
				}

				CasterCount++;

				const UBOOL bShadowIsOccluded = !View.bDisableQuerySubmissions
												&& View.State
												&& ((FSceneViewState*)View.State)->Ava_UpdateCasterOcclusion( CompactPrimitiveSceneInfo, View, View.Family->CurrentRealTime, LightSceneInfo );

				const UBOOL IsBsp = CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Component->IsA(UModelComponent::StaticClass());
				const UBOOL AddToCasterBounds = IsBsp || CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Component->IsA(UStaticMeshComponent::StaticClass());

				if( !bShadowIsOccluded )
				{
					if( IsBsp )
					{
						// near
						UBOOL bAddToNear = FALSE;
						UBOOL bAddToFar = FALSE;
						for( INT SplitIndex = 0; SplitIndex < FCascadedShadowInfo::NumShadows; ++SplitIndex )
						{
							// Check if this primitive is in the shadow's frustum.
							if( View.Ava_ReceiverBounds[SplitIndex].IsValid
								&&	SubjectFrustum[SplitIndex].IntersectBox(CompactPrimitiveSceneInfo.Bounds.Origin, CompactPrimitiveSceneInfo.Bounds.BoxExtent) )
							{
								if( SplitIndex < 4 )
								{
									bAddToNear = TRUE;
									SplitIndex = 3;	// jump to far
								}
								else
								{
									bAddToFar = TRUE;
									break;
								}
							}
						}
						if( bAddToNear )
						{
							NearBspCasters++;
							AllSubjectCount++;

							CascadedShadowInfo->AddModelPrimitive( CompactPrimitiveSceneInfo.PrimitiveSceneInfo, 0 );

							for( INT SplitIndex = 0; SplitIndex < 4; ++SplitIndex)
							{
								FBox BoundingBox = CompactPrimitiveSceneInfo.Bounds.GetBox();
								FBox LightspaceBox = BoundingBox.TransformBy( WorldToFace[SplitIndex] );
								CasterBounds[SplitIndex] += LightspaceBox;
							}
						}
						if( bAddToFar )
						{
							FarBspCasters++;
							AllSubjectCount++;
							CascadedShadowInfo->AddModelPrimitive( CompactPrimitiveSceneInfo.PrimitiveSceneInfo, 1 );
							for( INT SplitIndex = 4; SplitIndex < FCascadedShadowInfo::NumShadows; ++SplitIndex)
							{
								FBox BoundingBox = CompactPrimitiveSceneInfo.Bounds.GetBox();
								FBox LightspaceBox = BoundingBox.TransformBy( WorldToFace[SplitIndex] );
								CasterBounds[SplitIndex] += LightspaceBox;
							}
						}
					}
					else
					{
						for( INT SplitIndex = 0; SplitIndex < FCascadedShadowInfo::NumShadows; ++SplitIndex )
						{		
							// Check if this primitive is in the shadow's frustum.
							if( View.Ava_ReceiverBounds[SplitIndex].IsValid
								&&	SubjectFrustum[SplitIndex].IntersectBox(CompactPrimitiveSceneInfo.Bounds.Origin, CompactPrimitiveSceneInfo.Bounds.BoxExtent) )
							{
								SubjectCount[SplitIndex]++;
								AllSubjectCount++;

								CascadedShadowInfo->AddSubjectPrimitive( CompactPrimitiveSceneInfo.PrimitiveSceneInfo, SplitIndex );

								if( AddToCasterBounds )
								{
									FBox BoundingBox = CompactPrimitiveSceneInfo.Bounds.GetBox();
									FBox LightspaceBox = BoundingBox.TransformBy( WorldToFace[SplitIndex] );
									CasterBounds[SplitIndex] += LightspaceBox;
								}
							}
						}
					}
				}
				else
				{
					OccludedCount++;
				}
			}

			// update stats
			INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_NearBspCasters, NearBspCasters);
			INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_FarBspCasters, FarBspCasters );
			INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_CasterCount0, SubjectCount[0]);
			INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_CasterCount1, SubjectCount[1]);
			INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_CasterCount2, SubjectCount[2]);
			INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_CasterCount3, SubjectCount[3]);
			INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_CasterCount4, SubjectCount[4]);
			INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_CasterCount5, SubjectCount[5]);
			INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_CasterCount6, SubjectCount[6]);
			INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_CasterCount7, SubjectCount[7]);
			INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_CasterCountSum, AllSubjectCount );

			INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_CasterCount, CasterCount);
			INC_DWORD_STAT_BY(STAT_AVA_CascadedShadow_OccludedShadow, OccludedCount);


			for( INT SplitIndex = 0; SplitIndex < FCascadedShadowInfo::NumShadows; ++SplitIndex )
			{
				// get split bounds
				FBoxSphereBounds Bounds( View.Ava_SplitBounds[SplitIndex] );
				Bounds.Origin = LightToWorldSpace.TransformFVector( Bounds.Origin );
				if( CasterBounds[SplitIndex].IsValid )
				{
					NearLightPlane[SplitIndex] = CasterBounds[SplitIndex].Min.Z;
					FarLightPlane[SplitIndex] = Min<FLOAT>(FarLightPlane[SplitIndex], CasterBounds[SplitIndex].Max.Z);
					ShadowInitializer[SplitIndex].CalcTransforms(
						WorldToLight[SplitIndex],
						FVector(1,0,0),
						Bounds.GetSphere(),
						FVector4(0,0,0,1),
						NearLightPlane[SplitIndex],
						FarLightPlane[SplitIndex],
						TRUE);
				}
				CascadedShadowInfo->bHasReceiver[SplitIndex] = View.Ava_ReceiverBounds[SplitIndex].IsValid;
			}
			CascadedShadowInfo->Initialize(
				ShadowInitializer[0],
				ShadowInitializer[1],
				ShadowInitializer[2],
				ShadowInitializer[3],
				ShadowInitializer[4],
				ShadowInitializer[5],
				ShadowInitializer[6],
				ShadowInitializer[7]);
		}
	}
}
//>@ ava

UBOOL FSceneRenderer::Ava_RenderCascadedShadows( const FLightSceneInfo *LightSceneInfo, UBOOL bRenderDepth )
{
	UBOOL bAttenuationBufferDirty = FALSE;

	// Find the cascaded shadows cast by this light
	TArray<FCascadedShadowInfo*> Shadows;
	for( INT ShadowIndex = 0; ShadowIndex < CascadedShadows.Num(); ++ShadowIndex )
	{
		FCascadedShadowInfo *CascadedShadowInfo = &CascadedShadows(ShadowIndex);
		if( CascadedShadowInfo->LightSceneInfo == LightSceneInfo )
		{
			Shadows.AddItem( CascadedShadowInfo );
		}
	}

	//check( Shadows.Num() == 1 );

	if( Shadows.Num() )
	{
		FCascadedShadowInfo *CascadedShadowInfo = Shadows(0);

		// world dpg 에서만 depth를 갱신한다.
		// forground dpg는 near shadow depth만 사용하게 될 것이다.
		if( bRenderDepth )
		{
			// Allocate shadow texture space to the shadow
			UBOOL bAllocCascadedShadow = TRUE;
			const UINT ShadowBufferResolution = GSceneRenderTargets.GetShadowDepthTextureResolution();
			FTextureLayout ShadowLayout( 1, 1, ShadowBufferResolution, ShadowBufferResolution );
			FTextureLayout NearShadowLayout( 1, 1, ShadowBufferResolution, ShadowBufferResolution );
			
			for( INT SliceIndex = 0; SliceIndex < FCascadedShadowInfo::NumShadows; ++SliceIndex )
			{
				if( SliceIndex < 4 )
				{
					if( !NearShadowLayout.AddElement(
						&CascadedShadowInfo->X[SliceIndex],
						&CascadedShadowInfo->Y[SliceIndex],
						CascadedShadowInfo->Resolution + SHADOW_BORDER * 2,
						CascadedShadowInfo->Resolution + SHADOW_BORDER * 2))
					{
						bAllocCascadedShadow = FALSE;
					}
				}
				else
				{
					if( !ShadowLayout.AddElement(
						&CascadedShadowInfo->X[SliceIndex],
						&CascadedShadowInfo->Y[SliceIndex],
						CascadedShadowInfo->Resolution + SHADOW_BORDER * 2,
						CascadedShadowInfo->Resolution + SHADOW_BORDER * 2))
					{
						bAllocCascadedShadow = FALSE;
					}
				}
			}

			// 항상 alloc 되어야 합니다.
			if( !bAllocCascadedShadow )
			{
				return FALSE;
			}

			// render the shadow depths
			SCOPED_DRAW_EVENT(EventShadowDepths)(DEC_SCENE_ITEMS, TEXT("Shadow Depths"));

			GSceneRenderTargets.AVA_BeginRenderingNearShadowDepth();
			CascadedShadowInfo->RenderDepth( GlobalContext, this, SDPG_World, TRUE );
			GSceneRenderTargets.AVA_FinishRenderingNearShadowDepth();
			if( GNumCascadedShadow > 4 )
			{
				GSceneRenderTargets.BeginRenderingShadowDepth();
				// render depth for cascaded shadow
				CascadedShadowInfo->RenderDepth( GlobalContext, this, SDPG_World, FALSE );
				GSceneRenderTargets.FinishRenderingShadowDepth();
			}
		}

		// render the cascaded shadow projection
		GSceneRenderTargets.BeginRenderingLightAttenuation();
		for( INT ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex )
		{
			SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS, TEXT("Viwe%d"), ViewIndex);
			const FViewInfo &View = Views( ViewIndex );

			// st the device viewport for the view
			RHISetViewport( GlobalContext,
				View.RenderTargetX, View.RenderTargetY, 0.0f,
				View.RenderTargetX + View.RenderTargetSizeX, View.RenderTargetY + View.RenderTargetSizeY, 1.0f);
			RHISetViewParameters( GlobalContext, &View, View.ViewProjectionMatrix, View.ViewOrigin );

			// Set the light's scissor rectangle.
			//LightSceneInfo->SetScissorRect(GlobalContext, &View);

			const UBOOL bWorldDpg = bRenderDepth;


			// masking near region
			// use early stencil rejection for ati graphics cards
			if( GIsATI )
			{
				// clear stencil buffer
				RHIClear(GlobalContext, FALSE, FLinearColor::Black, FALSE, 0, TRUE, 0);

				// Depth test wo/ writes, no color writing.
				RHISetDepthState(GlobalContext,TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
				RHISetColorWriteEnable(GlobalContext, FALSE);

				// Solid rasterization wo/ backface culling.
				RHISetRasterizerState(GlobalContext,TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());

				// depth fail Set stencil to one.
				// depth fail은 mask plane평면보다 가까운 object의 pixel이 보이는 것이므로, near region이 된다.
				// 0 = sky region ( all pass )
				// 1 = far region ( one pass )
				// 2 = near region ( two pass )

				RHISetStencilState(GlobalContext,
					TStaticStencilState<TRUE,	CF_Always,	SO_Keep,	SO_Increment,	SO_Keep,
					FALSE,	CF_Always,	SO_Keep,	SO_Keep,		SO_Keep,
					0xff,0xff>::GetRHI());

				TShaderMapRef<FCascadedShadowProjectionVertexShader> VertexShader(GetGlobalShaderMap());
				// Cache the bound shader state
				if( !IsValidRef(FCascadedShadowInfo::MaskBoundShaderState) )
				{
					DWORD Strides[MaxVertexElementCount];
					appMemzero(Strides, sizeof(Strides));
					Strides[0] = sizeof(FFilterVertex);
					FCascadedShadowInfo::MaskBoundShaderState = RHICreateBoundShaderState(
						GFilterVertexDeclaration.VertexDeclarationRHI,
						Strides,
						VertexShader->GetVertexShader(),
						FPixelShaderRHIRef() );
				}

				RHISetBoundShaderState(GlobalContext, FCascadedShadowInfo::MaskBoundShaderState);

				//<@ 2007. 11. 19 changmin
				FLOAT MaskPlaneDepths[2];
				MaskPlaneDepths[0] = bWorldDpg ? GSliceValues[3] : GSliceValues[1];
				MaskPlaneDepths[1] = GSliceValues[7];

				for( INT MaskPlaneIndex = 0; MaskPlaneIndex < (bWorldDpg ? 2 : 1); ++MaskPlaneIndex )
				{
					const FVector ViewSpaceDepth(0.0f, 0.0f, MaskPlaneDepths[MaskPlaneIndex]);
					FVector4 ProjDepth = View.ProjectionMatrix.TransformFVector(ViewSpaceDepth);
					DrawDenormalizedQuadWithDepth(
						GlobalContext,
						View.X,View.Y, ProjDepth.Z, ProjDepth.W,
						View.SizeX,View.SizeY,
						View.RenderTargetX,View.RenderTargetY,
						View.RenderTargetSizeX,View.RenderTargetSizeY,
						View.Family->RenderTarget->GetSizeX(),View.Family->RenderTarget->GetSizeY(),
						GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY());
				}
				//>@
			}

			if( GIsNVidia )
			{
				const FVector ClipNearDepth(0.0f, 0.0f, View.NearClippingDistance);
				const FVector ClipFarDepth(0.0f, 0.0f, bWorldDpg ? GSliceValues[3] : GSliceValues[1] );
				//transform both near and far positions into clip space
				FVector4 ClipSpaceNearPos = View.ProjectionMatrix.TransformFVector(ClipNearDepth);
				FVector4 ClipSpaceFarPos = View.ProjectionMatrix.TransformFVector(ClipFarDepth);

				//be sure to disable depth bounds test after drawing!
				RHISetDepthBoundsTest(GlobalContext, TRUE, ClipSpaceNearPos, ClipSpaceFarPos);
			}
			CascadedShadowInfo->RenderProjection(GlobalContext, &View, TRUE, bWorldDpg );	// near
			if( bWorldDpg && GNumCascadedShadow )
			{
				if( GIsNVidia )
				{
					const FVector ClipNearDepth(0.0f, 0.0f, GSliceValues[3]);
					const FVector ClipFarDepth(0.0f, 0.0f, GSliceValues[7] );

					//transform both near and far positions into clip space
					FVector4 ClipSpaceNearPos = View.ProjectionMatrix.TransformFVector(ClipNearDepth);
					FVector4 ClipSpaceFarPos = View.ProjectionMatrix.TransformFVector(ClipFarDepth);

					//be sure to disable depth bounds test after drawing!
					RHISetDepthBoundsTest(GlobalContext, TRUE, ClipSpaceNearPos, ClipSpaceFarPos);
				}
				CascadedShadowInfo->RenderProjection(GlobalContext, &View, FALSE, bWorldDpg );	// far
			}
			if( GIsNVidia )
			{
				//disable depthbounds test
				RHISetDepthBoundsTest(GlobalContext, FALSE, FVector4(0.0f,0.0f,0.0f,1.0f), FVector4(0.0f,0.0f,1.0f,1.0f));

			}

			// Reset the scissor rectangle.
			//RHISetScissorRect( GlobalContext, FALSE, 0, 0, 0, 0);

			// Mark the attenuation buffer as dirty.
			bAttenuationBufferDirty = TRUE;
		}

		if( GIsATI )
		{
			// Reset the stencil state.
			RHISetStencilState(GlobalContext,TStaticStencilState<>::GetRHI());

			// clear stencil buffer
			RHIClear(GlobalContext, FALSE, FLinearColor::Black, FALSE, 0, TRUE, 0);
		}
	}

	return bAttenuationBufferDirty;
}