/**
 * Absolute value material expression for user-defined materials
 *
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionAbs extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

/** Link to the input expression to be evaluated */
var ExpressionInput	Input;

cpptext
{
    /**
	 * Creates the new shader code chunk needed for the Abs expression
	 *
	 * @param	Compiler - Material compiler that knows how to handle this expression
	 * @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
	 */	
	virtual INT Compile( FMaterialCompiler* Compiler );

	/**
	 * Textual description for this material expression
	 *
	 * @return	Caption text
	 */	
	virtual FString GetCaption() const;

	/**
	 * Removes references to the passed in expression.
	 *
	 * @param	Expression		Expression to remove reference to
	 */
	virtual void RemoveReferenceTo( UMaterialExpression* Expression );
}
