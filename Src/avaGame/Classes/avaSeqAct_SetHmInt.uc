/**
 * Copyright 2004-2006 Epic Games, Inc. All Rights Reserved.
 */
class avaSeqAct_SetHmInt extends SeqAct_SetSequenceVariable
	native(Sequence);

cpptext
{
	void Activated();
};

/** Value to apply */
var() int Value;

defaultproperties
{
	ObjName="HmInt"

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'avaSeqVar_HmInt',LinkDesc="Target",bWriteable=true)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Value",MinVars=0)
}
