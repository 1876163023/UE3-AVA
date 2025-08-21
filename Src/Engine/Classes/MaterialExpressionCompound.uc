/**
 * Copyright 2006 Epic Games, Inc. All Rights Reserved.
 *
 * A compound material expression representing several material expressions collapsed
 * into one node.  An editor-only concept; this node does not generate shader code.
 */
class MaterialExpressionCompound extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

/** Array of material expressions encapsulated by this node. */
var const array<MaterialExpression>		MaterialExpressions;

/** Textual descrption for this compound expression; appears in the expression title. */
var() string							Caption;

/** IF TRUE, the nodes encapsulated by compound expression are drawn in the material editor. */
var() bool								bExpanded;
 
cpptext
{
	/**
	 * Textual description for this material expression
	 *
	 * @return	Caption text
	 */	
	virtual FString GetCaption() const;
	
	/**
	 * Recursively gathers all UMaterialExpression objects referenced by this expression.
	 * Including self.
	 *
	 * @param	Expressions	- Reference to array of material expressions to add to
	 */	
	virtual void GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const;

	/**
	 * Removes references to the passed in expression.
	 *
	 * @param	Expression		Expression to remove reference to
	 */
	virtual void RemoveReferenceTo( UMaterialExpression* Expression );
}
