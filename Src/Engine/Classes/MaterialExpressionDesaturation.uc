/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionDesaturation extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

// Outputs: Lerp(Input,dot(Input,LuminanceFactors)),Percent)

var ExpressionInput	Input;
var ExpressionInput	Percent;
var() LinearColor	LuminanceFactors;	// Color component factors for converting a color to greyscale.

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual FString GetCaption() const
	{
		return TEXT("Desaturation");
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
	LuminanceFactors=(R=0.3,G=0.59,B=0.11,A=0)
}
