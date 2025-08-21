/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionCosine extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

var ExpressionInput	Input;

var() float	Period;

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

defaultproperties
{
	Period=1.f
}
