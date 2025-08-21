/*================================================================
	NormalRendering.h : Definitions for rendering viewspace normal.
	Copyright 2007 Redduck, inc. All Rights Reserved.
====================================================================*/

class FNormalDrawingPolicy : public FMeshDrawingPolicy
{
public:
	FNormalDrawingPolicy(
		const FVertexFactory	*InVertexFactory,
		const FMaterialInstance	*InMaterialInstance,
		const FMaterialInstance	*InOriginalMaterialInstance
		);

	// FMeshDrawingPolicy interface.
	UBOOL Matches( const FNormalDrawingPolicy &Other ) const
	{
		return FMeshDrawingPolicy::Matches( Other ) && VertexShader == Other.VertexShader && PixelShader == Other.PixelShader;
	}
	void DrawShared( FCommandContextRHI *Context, const FSceneView *View, FBoundShaderStateRHIRef ShaderState ) const;
	void SetMeshRenderState(
		FCommandContextRHI *Context,
		const FPrimitiveSceneInfo *PrimitiveSceneInfo,
		const FMeshElement &Mesh,
		UBOOL bBackFace,
		const ElementDataType& ElementData ) const;
	FBoundShaderStateRHIRef CreateBoundShaderState( DWORD DynamicStride = 0 );
	friend INT Compare( const FNormalDrawingPolicy &A, const FNormalDrawingPolicy &B );
private:
	class FViewSpaceNormalVertexShader	*VertexShader;
	class FViewSpaceNormalPixelShader	*PixelShader;
	const FMaterialInstance		*OriginalMaterialInstance;
};

/*
 * A drawing policy factory for rendering viewspace normal
 */
class FNormalDrawingPolicyFactory
{
public:
	enum { bAllowSimpleElements = FALSE };
	struct ContextType {};

	static UBOOL DrawStaticMesh(
		FCommandContextRHI *Context,
		const FSceneView *View,
		const FStaticMesh *Mesh,
		UBOOL bDrawShared );

	static UBOOL DrawDynamicMesh(
		FCommandContextRHI *Context,
		const FViewInfo *View,
		ContextType DrawingContext,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo *PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		);

	static UBOOL IsMaterialIgnored(const FMaterialInstance* MaterialInstance)
	{
		return FALSE;
	}
};
