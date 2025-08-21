/**
 * Copyright 2004-2006 Epic Games, Inc. All Rights Reserved.
 */
class avaSeqAct_SetHm extends SeqAct_SetSequenceVariable
	native(Sequence);

cpptext
{
	void Activated();
};

var() bool DefaultValue;

defaultproperties
{
	ObjName="Hm"

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'avaSeqVar_Hm',LinkDesc="Target",bWriteable=true)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Bool',LinkDesc="Value",MinVars=0)
}
