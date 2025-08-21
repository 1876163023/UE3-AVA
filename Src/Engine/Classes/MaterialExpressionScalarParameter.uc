/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionScalarParameter extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

var() name	ParameterName;
var() float	DefaultValue;

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual FString GetCaption() const;
}
