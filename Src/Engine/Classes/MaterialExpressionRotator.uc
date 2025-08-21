/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionRotator extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

var ExpressionInput	Coordinate;
var ExpressionInput	Time;

var() float	CenterX,
			CenterY,
			Speed;

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
	CenterX=0.5
	CenterY=0.5
	Speed=0.25
}
