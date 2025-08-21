/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionTransform extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

/** input expression for this transform */
var ExpressionInput	Input;

/** type of transform to apply to the input expression */
var() const enum EMaterialVectorCoordTransform
{
	// transform from tangent space to world space
	TRANSFORM_World,
	// transform from tangent space to view space
	TRANSFORM_View,
	// transform from tangent space to local space
	TRANSFORM_Local
} TransformType;

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual FString GetCaption() const;

	/**
	 * Removes references to the passed in expression.
	 *
	 * @param	Expression		Expression to remove reference to
	 */
	virtual void RemoveReferenceTo( UMaterialExpression* Expression );
}
