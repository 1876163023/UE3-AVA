/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpression extends Object within Material
	native
	abstract;

struct ExpressionInput
{
	var MaterialExpression	Expression;
	var int					Mask,
							MaskR,
							MaskG,
							MaskB,
							MaskA;
	var int					GCC64_Padding; // @todo 64: if the C++ didn't mismirror this structure (with MaterialInput), we might not need this
};

var int		EditorX,
			EditorY;

/** If TRUE, an preview of the expression is generated in realtime in the material editor. */
var() bool					bRealtimePreview;

/** A reference to the compound expression this material expression belongs to. */
var const MaterialExpressionCompound	Compound;

/** A description that level designers can add (shows in the material editor UI). */
var() string				Desc;

cpptext
{
	// UObject interface.

	virtual void PostEditChange(UProperty* PropertyThatChanged);

	// UMaterialExpression interface.

	/**
	 * Removes references to the passed in expression.
	 *
	 * @param	Expression		Expression to remove reference to
	 */
	virtual void RemoveReferenceTo( UMaterialExpression* Expression ) {};

	virtual INT Compile(FMaterialCompiler* Compiler) { return INDEX_NONE; }
	virtual void GetOutputs(TArray<FExpressionOutput>& Outputs) const;
	virtual INT GetWidth() const;
	virtual INT GetHeight() const;
	virtual UBOOL UsesLeftGutter() const;
	virtual UBOOL UsesRightGutter() const;
	virtual FString GetCaption() const;
	virtual INT GetLabelPadding() { return 0; }
	
	virtual INT CompilerError(FMaterialCompiler* Compiler, const TCHAR* pcMessage);
}
