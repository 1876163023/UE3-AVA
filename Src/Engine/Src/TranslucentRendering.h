/*=============================================================================
	TranslucentRendering.h: Translucent rendering definitions.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


/**
* Translucent draw policy factory.
* Creates the policies needed for rendering a mesh based on its material
*/
class FTranslucencyDrawingPolicyFactory
{
public:
	enum { bAllowSimpleElements = FALSE };
	struct ContextType {};

	/**
	* Render a dynamic mesh using a translucent draw policy 
	* @return TRUE if the mesh rendered
	*/
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

	/**
	* Render a dynamic mesh using a translucent draw policy 
	* @return TRUE if the mesh rendered
	*/
	static UBOOL DrawStaticMesh(
		FCommandContextRHI* Context,
		const FViewInfo* View,
		ContextType DrawingContext,
		const FStaticMesh& StaticMesh,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		);

	static UBOOL IsMaterialIgnored(const FMaterialInstance* MaterialInstance)
	{
		return !IsTranslucentBlendMode(MaterialInstance->GetMaterial()->GetBlendMode());
	}
};
