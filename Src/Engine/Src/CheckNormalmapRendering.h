/*=========================================================================================
	CheckNormalmapRendering.h : Definitions for rendering normalmap lighting result.
	Copyright 2007 2007 Redduck, inc. All Rights Reserved.
==========================================================================================*/

#include "LightMapRendering.h"

/**
* A Vertex Shader for rendering bump result.
*/
template<class LightMapPolicyType>
class TBumpOnlyVertexShader : public FShader, public LightMapPolicyType::VertexParametersType
{
	DECLARE_SHADER_TYPE(TBumpOnlyVertexShader, MeshMaterial);
public:
	//
	static UBOOL ShouldCache( EShaderPlatform Platform, const FMaterial *Material, const FVertexFactoryType *VertexFactoryType )
	{
#if FINAL_RELEASE
		return FALSE;
#else
		return !IsTranslucentBlendMode(Material->GetBlendMode())
			&& LightMapPolicyType::ShouldCache( Platform, Material, VertexFactoryType );
#endif
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_LIGHTMAP_COEFFICIENTS"), *FString::Printf(TEXT("%u"), NUM_LIGHTMAP_COEFFICIENTS));
		LightMapPolicyType::ModifyCompilationEnvironment(OutEnvironment);
	}

	// constructors
	TBumpOnlyVertexShader( const ShaderMetaType::CompiledShaderInitializerType &Initializer ):
	FShader(Initializer),
		VertexFactoryParameters( Initializer.VertexFactoryType, Initializer.ParameterMap)
	{
		LightMapPolicyType::VertexParametersType::Bind(Initializer.ParameterMap);
	}
	TBumpOnlyVertexShader() {}

	//
	virtual void Serialize(FArchive &Ar)
	{
		FShader::Serialize(Ar);
		LightMapPolicyType::VertexParametersType::Serialize(Ar);
		Ar << VertexFactoryParameters;
	}

	// 
	void SetParameters(FCommandContextRHI *Context, const FVertexFactory *VertexFactory, const FMaterialInstance *MaterialInstance, const FSceneView *View )
	{
		VertexFactoryParameters.Set(Context, this, VertexFactory, View);
	}
	void SetLocalTransforms(FCommandContextRHI *Context, const FMatrix &LocalToWorld, const FMatrix &WorldToLocal )
	{
		VertexFactoryParameters.SetLocalTransforms( Context, this, LocalToWorld, WorldToLocal );
	}

private:
	FVertexFactoryParameterRef VertexFactoryParameters;
};

/**
* a pixel shader for rendering bump only lighting result.
*/
template<class LightMapPolicyType>
class TBumpOnlyPixelShader : public FShader, public LightMapPolicyType::PixelParametersType
{
	DECLARE_SHADER_TYPE(TBumpOnlyPixelShader, MeshMaterial);

public:
	//
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
#if FINAL_RELEASE
		return FALSE;
#else
		return	!IsTranslucentBlendMode(Material->GetBlendMode())
			&&	LightMapPolicyType::ShouldCache(Platform, Material, VertexFactoryType);
#endif
	}
	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_LIGHTMAP_COEFFICIENTS"), *FString::Printf(TEXT("%u"), NUM_LIGHTMAP_COEFFICIENTS));
		LightMapPolicyType::ModifyCompilationEnvironment(OutEnvironment);
	}
	//
	TBumpOnlyPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FShader(Initializer)
	{
		LightMapPolicyType::PixelParametersType::Bind(Initializer.ParameterMap);
		MaterialParameters.Bind(Initializer.Material, Initializer.ParameterMap);
		DiffuseSpecularScaleParameter.Bind(Initializer.ParameterMap, TEXT("DiffuseSpecularScale"),TRUE);
	}

	TBumpOnlyPixelShader() {}

	void SetParameters(FCommandContextRHI* Context,const FVertexFactory* VertexFactory,const FMaterialInstance* MaterialInstance,const FSceneView* View)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialInstance, View->Family->CurrentWorldTime, View->Family->CurrentRealTime, View);
		MaterialParameters.Set(Context, this, MaterialRenderContext);

		if( DiffuseSpecularScaleParameter.IsBound() )
		{
			extern INT GBumpState;
			switch(GBumpState)
			{
				// diffuse
			case 1:
				SetPixelShaderValue(Context, GetPixelShader(), DiffuseSpecularScaleParameter,FVector2D(1.0f, 0.0f));
				break;
			case 2:
				// specular
				SetPixelShaderValue(Context, GetPixelShader(), DiffuseSpecularScaleParameter,FVector2D(0.0f, 1.0f));
				break;
			default:
				// diffuse + specular
				SetPixelShaderValue(Context, GetPixelShader(), DiffuseSpecularScaleParameter,FVector2D(1.0f, 1.0f));
				break;
			}
		}
	}
	void SetLocalTransforms(FCommandContextRHI* Context,const FMaterialInstance* MaterialInstance,const FMatrix& LocalToWorld,UBOOL bBackFace)
	{
		MaterialParameters.SetLocalTransforms(Context, this, MaterialInstance, LocalToWorld, bBackFace);
	}
	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		LightMapPolicyType::PixelParametersType::Serialize(Ar);
		Ar << MaterialParameters;
		Ar << DiffuseSpecularScaleParameter;
	}
private:
	FMaterialPixelShaderParameters MaterialParameters;
	FShaderParameter DiffuseSpecularScaleParameter;
};

// draws the bump lighting of a mesh
template<class LightMapPolicyType>
class TBumpOnlyDrawingPolicy : public FMeshDrawingPolicy
{
public:
	typedef typename LightMapPolicyType::ElementDataType ElementDataType;

	TBumpOnlyDrawingPolicy(
		const FVertexFactory	*InVertexFactory,
		const FMaterialInstance *InMaterialInstance,
		LightMapPolicyType		InLightMapPolicy):
	FMeshDrawingPolicy(InVertexFactory, InMaterialInstance),
		LightMapPolicy(InLightMapPolicy)
	{
		const FMaterialShaderMap *MaterialShaderIndex = InMaterialInstance->GetMaterial()->GetShaderMap();
		const FMeshMaterialShaderMap *MeshShaderIndex = MaterialShaderIndex->GetMeshShaderMap(InVertexFactory->GetType());
		VertexShader = MeshShaderIndex->GetShader<TBumpOnlyVertexShader<LightMapPolicyType> >();
		PixelShader = MeshShaderIndex->GetShader<TBumpOnlyPixelShader<LightMapPolicyType> >();
	}
	
	UBOOL Matches( const TBumpOnlyDrawingPolicy &Other ) const
	{
		return FMeshDrawingPolicy::Matches( Other )
			&& VertexShader == Other.VertexShader
			&& PixelShader == Other.PixelShader;
	}

	void DrawShared( FCommandContextRHI *Context, const FSceneView *View, FBoundShaderStateRHIRef BoundShaderState ) const
	{
		// set the emissive shader parameters for the material.
		VertexShader->SetParameters(Context, VertexFactory, MaterialInstance, View);
		PixelShader->SetParameters(Context, VertexFactory, MaterialInstance, View);

		// Set the lightmap policy.
		LightMapPolicy.Set(Context, VertexShader, PixelShader, PixelShader, VertexFactory, MaterialInstance, View);

		// Set the actual shader and vertex declaration state
		RHISetBoundShaderState( Context, BoundShaderState );
	}

	FBoundShaderStateRHIRef CreateBoundShaderState( DWORD DynamicStride = 0 )
	{
		FVertexDeclarationRHIParamRef VertexDeclaration;
		DWORD StreamStrides[MaxVertexElementCount];

		LightMapPolicy.GetVertexDeclarationInfo( VertexDeclaration, StreamStrides, VertexFactory );
		if( DynamicStride )
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
		const ElementDataType& ElementData ) const
	{
		// Set the light-map policy's mesh-specific settings.
		LightMapPolicy.SetMesh(Context, VertexShader, PixelShader, VertexShader, PixelShader, VertexFactory, MaterialInstance, ElementData );

		VertexShader->SetLocalTransforms( Context, Mesh.LocalToWorld, Mesh.WorldToLocal );
		PixelShader->SetLocalTransforms( Context, Mesh.MaterialInstance, Mesh.LocalToWorld, bBackFace );

		FMeshDrawingPolicy::SetMeshRenderState( Context, PrimitiveSceneInfo, Mesh, bBackFace, FMeshDrawingPolicy::ElementDataType());
	}
	
	friend INT Compare( const TBumpOnlyDrawingPolicy &A, const TBumpOnlyDrawingPolicy &B )
	{
		COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
		COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
		COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
		COMPAREDRAWINGPOLICYMEMBERS(MaterialInstance);
		return Compare(A.LightMapPolicy, B.LightMapPolicy);
	}
private:
	class TBumpOnlyVertexShader<LightMapPolicyType>	*VertexShader;
	class TBumpOnlyPixelShader<LightMapPolicyType>	*PixelShader;
	LightMapPolicyType LightMapPolicy;
};

// emissive
class FBumpOnlyDrawingPolicyFactory
{
public:
	enum { bAllowSimpleElements = FALSE };
	struct ContextType {};

	static UBOOL DrawStaticMesh(
		FCommandContextRHI	*Context,
		const FSceneView	*View,
		const FStaticMesh	*Mesh,
		UBOOL bDrawShared );

	static UBOOL DrawDynamicMesh(
		FCommandContextRHI	*Context,
		const FViewInfo		*View,
		ContextType			DrawingContext,
		const FMeshElement	&Mesh,
		UBOOL				bBackFace,
		UBOOL				bPreFog,
		const FPrimitiveSceneInfo	*PrimitiveSceneInfo,
		FHitProxyId			HitProxyId);

	static UBOOL IsMaterialIgnored( const FMaterialInstance* MaterialInstance )
	{
		return MaterialInstance && MaterialInstance->GetMaterial()->GetLightingModel() == MLM_Unlit;
	}
};



template<typename LightPolicyType>
class TMeshBumpLightingDrawingPolicyFactory
{
public:
	enum { bAllowSimpleElements = FALSE };
	typedef const FLightSceneInfo* ContextType;

	TMeshBumpLightingDrawingPolicyFactory()
	{
		LightSceneInfo = NULL;
	}

	TMeshBumpLightingDrawingPolicyFactory( const FLightSceneInfo *InLightSceneInfo )
	{
		LightSceneInfo = InLightSceneInfo;
	}

	UBOOL DrawStaticMesh(
		FCommandContextRHI	*Context,
		const FSceneView	*View,
		const FStaticMesh	*StaticMesh,
		UBOOL bDrawShared)
	{
		FMeshElement Mesh( *StaticMesh );
		// Check for cached shadow data.
		FLightInteraction CachedInteraction = FLightInteraction::Uncached();
		if( Mesh.LCI )
		{
			CachedInteraction = Mesh.LCI->GetInteraction(LightSceneInfo);
		}
		UBOOL bBackFace = FALSE;
		// render the mesh with the cached shadow data type.
		switch(CachedInteraction.GetType())
		{
		case LIT_Uncached:
			{
				TMeshLightingDrawingPolicy<FBumpLightingShadowingPolicy, LightPolicyType> DrawingPolicy(
					Mesh.VertexFactory,
					Mesh.MaterialInstance,
					(typename LightPolicyType::SceneInfoType*)LightSceneInfo,
					FBumpLightingShadowingPolicy());

				DrawingPolicy.DrawShared(Context, View, DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
				DrawingPolicy.SetMeshRenderState(Context, StaticMesh->PrimitiveSceneInfo, Mesh, bBackFace, FBumpLightingShadowingPolicy::ElementDataType());
				DrawingPolicy.DrawMesh(Context, Mesh);
				return TRUE;
			}
		case LIT_CachedShadowMap1D:
		case LIT_CachedShadowMap2D:
		case LIT_CachedIrrelevant:
		case LIT_CachedLightMap:
		default:
			return FALSE;

		}
	}

	static UBOOL DrawDynamicMesh(
		FCommandContextRHI* Context,
		const FSceneView* View,
		const FLightSceneInfo* InLightSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		)
	{}

	static UBOOL IsMaterialIgnored( const FMaterialInstance* MaterialInstance )
	{
		return MaterialInstance && MaterialInstance->GetMaterial()->GetLightingModel() == MLM_Unlit;
	}
private:
	const FLightSceneInfo *LightSceneInfo;

};

