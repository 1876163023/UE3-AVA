/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionSceneTexture extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

/** 
 * MaterialExpressionSceneTexture: 
 * samples the current scene texture (lighting,etc)
 * for use in a material
 */

/** texture coordinate inputt expression for this node */
var ExpressionInput	Coordinates;

var() enum ESceneTextureType
{
	// 16bit component lighting target
	SceneTex_Lighting
} SceneTextureType;

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual void GetOutputs(TArray<FExpressionOutput>& Outputs) const;
	virtual void RemoveReferenceTo( UMaterialExpression* Expression );

	virtual FString GetCaption() const;
}

defaultproperties
{
	SceneTextureType=SceneTex_Lighting
}
