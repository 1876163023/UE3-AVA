/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionReflectionVector extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual FString GetCaption() const;
}
