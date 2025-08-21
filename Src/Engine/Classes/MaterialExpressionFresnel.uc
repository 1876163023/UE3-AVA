/**
 * Copyright 2006 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Allows the artists to quickly set up a Fresnel term. Returns:
 *
 *		pow(1 - max(Normal dot Camera,0),Exponent)
 */
class MaterialExpressionFresnel extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

/** The exponent to pass into the pow() function */
var() float Exponent;

/** The normal to dot with the camera vector */
var ExpressionInput	Normal;

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual FString GetCaption() const
	{
		return FString(TEXT("Fresnel"));
	}

	/**
	 * Removes references to the passed in expression.
	 *
	 * @param	Expression		Expression to remove reference to
	 */
	virtual void RemoveReferenceTo( UMaterialExpression* Expression );
}

defaultproperties
{
	Exponent=3.0
}