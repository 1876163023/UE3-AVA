/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionTextureCoordinate extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

var() int	CoordinateIndex;
var() float	Tiling;

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual FString GetCaption() const;
}

defaultproperties
{
	Tiling=1.0
}
