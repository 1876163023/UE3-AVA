/*=============================================================================
	DepthRendering.cpp: Depth rendering implementation.
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TDepthOnlyVertexShader<TRUE>,TEXT("PositionOnlyDepthVertexShader"),TEXT("Main"),SF_Vertex,0,0);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TDepthOnlyVertexShader<FALSE>,TEXT("DepthOnlyVertexShader"),TEXT("Main"),SF_Vertex,0,0);

FDepthDrawingPolicy::FDepthDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialInstance* InMaterialInstance
	):
FMeshDrawingPolicy(InVertexFactory,InMaterialInstance)
{
	const FMaterialShaderMap* MaterialShaderIndex = InMaterialInstance->GetMaterial()->GetShaderMap();
	const FMeshMaterialShaderMap* MeshShaderIndex = MaterialShaderIndex->GetMeshShaderMap(InVertexFactory->GetType());
	VertexShader = MeshShaderIndex->GetShader<TDepthOnlyVertexShader<FALSE> >();
}

void FDepthDrawingPolicy::DrawShared(FCommandContextRHI* Context,const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
{
	// Set the depth-only shader parameters for the material.
	VertexShader->SetParameters(Context,VertexFactory,MaterialInstance,View);

	// Set the shared mesh resources.
	FMeshDrawingPolicy::DrawShared(Context,View);

	// Set the actual shader & vertex declaration state
	RHISetBoundShaderState(Context, BoundShaderState);
}

/** 
* Create bound shader state using the vertex decl from the mesh draw policy
* as well as the shaders needed to draw the mesh
* @param DynamicStride - optional stride for dynamic vertex data
* @return new bound shader state object
*/
FBoundShaderStateRHIRef FDepthDrawingPolicy::CreateBoundShaderState(DWORD DynamicStride)
{
	FVertexDeclarationRHIParamRef VertexDeclaration;
	DWORD StreamStrides[MaxVertexElementCount];

	FMeshDrawingPolicy::GetVertexDeclarationInfo(VertexDeclaration, StreamStrides);

	if (DynamicStride)
	{
		StreamStrides[0] = DynamicStride;
	}	

	// Use a NULL pixel shader for opaque materials.
	checkSlow(MaterialInstance->GetMaterial()->GetBlendMode() == BLEND_Opaque);
	return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, VertexShader->GetVertexShader(), FPixelShaderRHIRef());
}

void FDepthDrawingPolicy::SetMeshRenderState(
	FCommandContextRHI* Context,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const ElementDataType& ElementData
	) const
{
	VertexShader->SetLocalTransforms(Context,Mesh.LocalToWorld,Mesh.WorldToLocal);
	FMeshDrawingPolicy::SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,ElementData);
}

INT Compare(const FDepthDrawingPolicy& A,const FDepthDrawingPolicy& B)
{
	COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
	COMPAREDRAWINGPOLICYMEMBERS(MaterialInstance);
	return 0;
}


FPositionOnlyDepthDrawingPolicy::FPositionOnlyDepthDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialInstance* InMaterialInstance
	):
FMeshDrawingPolicy(InVertexFactory,InMaterialInstance)
{
	const FMaterialShaderMap* MaterialShaderIndex = InMaterialInstance->GetMaterial()->GetShaderMap();
	const FMeshMaterialShaderMap* MeshShaderIndex = MaterialShaderIndex->GetMeshShaderMap(InVertexFactory->GetType());
	VertexShader = MeshShaderIndex->GetShader<TDepthOnlyVertexShader<TRUE> >();
}

void FPositionOnlyDepthDrawingPolicy::DrawShared(FCommandContextRHI* Context,const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
{
	// Set the depth-only shader parameters for the material.
	VertexShader->SetParameters(Context,VertexFactory,MaterialInstance,View);

	// Set the shared mesh resources.
	VertexFactory->SetPositionStream(Context);

	// Set the actual shader & vertex declaration state
	RHISetBoundShaderState(Context, BoundShaderState);
}

/** 
* Create bound shader state using the vertex decl from the mesh draw policy
* as well as the shaders needed to draw the mesh
* @param DynamicStride - optional stride for dynamic vertex data
* @return new bound shader state object
*/
FBoundShaderStateRHIRef FPositionOnlyDepthDrawingPolicy::CreateBoundShaderState(DWORD DynamicStride)
{
	FVertexDeclarationRHIParamRef VertexDeclaration;
	DWORD StreamStrides[MaxVertexElementCount];

	VertexFactory->GetPositionStreamStride(StreamStrides);
	VertexDeclaration = VertexFactory->GetPositionDeclaration();

	if (DynamicStride)
	{
		StreamStrides[0] = DynamicStride;
	}	

	checkSlow(MaterialInstance->GetMaterial()->GetBlendMode() == BLEND_Opaque);
	return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, VertexShader->GetVertexShader(), FPixelShaderRHIRef());
}

void FPositionOnlyDepthDrawingPolicy::SetMeshRenderState(
	FCommandContextRHI* Context,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const ElementDataType& ElementData
	) const
{
	VertexShader->SetLocalTransforms(Context,Mesh.LocalToWorld,Mesh.WorldToLocal);
	FMeshDrawingPolicy::SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,ElementData);
}

INT Compare(const FPositionOnlyDepthDrawingPolicy& A,const FPositionOnlyDepthDrawingPolicy& B)
{
	COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
	COMPAREDRAWINGPOLICYMEMBERS(MaterialInstance);
	return 0;
}

void FDepthDrawingPolicyFactory::AddStaticMesh(FScene* Scene,FStaticMesh* StaticMesh,ContextType)
{
	// Only draw opaque materials in the depth pass.
	if(StaticMesh->MaterialInstance->GetMaterial()->GetBlendMode() == BLEND_Opaque)
	{
		if (StaticMesh->VertexFactory->SupportsPositionOnlyStream())
		{
			// Add the static mesh to the depth-only draw list.
			Scene->DPGs[StaticMesh->DepthPriorityGroup].PositionOnlyDepthDrawList.AddMesh(
				StaticMesh,
				FPositionOnlyDepthDrawingPolicy::ElementDataType(),
				FPositionOnlyDepthDrawingPolicy(StaticMesh->VertexFactory,GEngine->DefaultMaterial->GetInstanceInterface(FALSE))
				);
		}
		else
		{
			// Add the static mesh to the depth-only draw list.
			Scene->DPGs[StaticMesh->DepthPriorityGroup].DepthDrawList.AddMesh(
				StaticMesh,
				FDepthDrawingPolicy::ElementDataType(),
				FDepthDrawingPolicy(StaticMesh->VertexFactory,GEngine->DefaultMaterial->GetInstanceInterface(FALSE))
				);
		}
	}
}

UBOOL FDepthDrawingPolicyFactory::DrawDynamicMesh(
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
	// Only draw opaque materials in the depth pass.
	if(Mesh.MaterialInstance->GetMaterial()->GetBlendMode() == BLEND_Opaque)
	{
		if (Mesh.VertexFactory->SupportsPositionOnlyStream())
		{
			FPositionOnlyDepthDrawingPolicy DrawingPolicy(Mesh.VertexFactory,GEngine->DefaultMaterial->GetInstanceInterface(FALSE));

			if (!Mesh.bDrawnShared)
			{
				DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
			}
			DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FPositionOnlyDepthDrawingPolicy::ElementDataType());
			DrawingPolicy.DrawMesh(Context,Mesh);
		}
		else 
		{
			FDepthDrawingPolicy DrawingPolicy(Mesh.VertexFactory,GEngine->DefaultMaterial->GetInstanceInterface(FALSE));

			if (!Mesh.bDrawnShared)
			{
				DrawingPolicy.DrawShared(Context,View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
			}
			DrawingPolicy.SetMeshRenderState(Context,PrimitiveSceneInfo,Mesh,bBackFace,FMeshDrawingPolicy::ElementDataType());
			DrawingPolicy.DrawMesh(Context,Mesh);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

UBOOL FDepthDrawingPolicyFactory::IsMaterialIgnored(const FMaterialInstance* MaterialInstance)
{
	return IsTranslucentBlendMode(MaterialInstance->GetMaterial()->GetBlendMode());
}
