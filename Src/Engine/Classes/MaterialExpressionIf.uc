/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionIf extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

var ExpressionInput A;
var ExpressionInput B;

var ExpressionInput AGreaterThanB;
var ExpressionInput AEqualsB;
var ExpressionInput ALessThanB;

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
}
