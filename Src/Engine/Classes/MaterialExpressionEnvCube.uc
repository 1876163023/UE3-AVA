class MaterialExpressionEnvCube extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

/** texture coordinate inputt expression for this node */
var ExpressionInput	Coordinates;

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual void GetOutputs(TArray<FExpressionOutput>& Outputs) const;
	virtual void RemoveReferenceTo( UMaterialExpression* Expression );

	virtual FString GetCaption() const;
}

defaultproperties
{	
}
