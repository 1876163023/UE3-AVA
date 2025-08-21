/*=============================================================================
	TextureDensityRendering.h: Definitions for rendering texture density.
	Copyright 1998-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * 
 */
class FTextureDensityDrawingPolicy : public FMeshDrawingPolicy
{
public:
	FTextureDensityDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialInstance* InMaterialInstance,
		const FMaterialInstance* InOriginalMaterialInstance
		);

	// FMeshDrawingPolicy interface.
	UBOOL Matches(const FTextureDensityDrawingPolicy& Other) const
	{
		return FMeshDrawingPolicy::Matches(Other) && VertexShader == Other.VertexShader && PixelShader == Other.PixelShader;
	}
	void DrawShared( FCommandContextRHI* Context, const FSceneView* View, FBoundShaderStateRHIRef ShaderState ) const;
	void SetMeshRenderState(
		FCommandContextRHI* Context,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		const ElementDataType& ElementData
		) const;

	FBoundShaderStateRHIRef CreateBoundShaderState(DWORD DynamicStride = 0);

	friend INT Compare(const FTextureDensityDrawingPolicy& A,const FTextureDensityDrawingPolicy& B);

private:
	class FTextureDensityVertexShader*	VertexShader;
	class FTextureDensityPixelShader*	PixelShader;
	const FMaterialInstance*			OriginalMaterialInstance;
};

/**
 * A drawing policy factory for rendering texture density.
 */
class FTextureDensityDrawingPolicyFactory
{
public:
	enum { bAllowSimpleElements = FALSE };
	struct ContextType {};

	static UBOOL DrawStaticMesh( FCommandContextRHI* Context, const FSceneView* View, const FStaticMesh* Mesh, UBOOL bDrawnShared );

	static UBOOL DrawDynamicMesh(
		FCommandContextRHI* Context,
		const FViewInfo* View,
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
