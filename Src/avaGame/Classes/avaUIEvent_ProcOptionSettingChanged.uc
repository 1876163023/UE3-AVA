class avaUIEvent_ProcOptionSettingChanged extends UIEvent;


defaultproperties
{
	ObjName="Proc Option Setting Changed"
	ObjCategory="ProcOption"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="WhatChanged",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="bImmediately",bWriteable=true))

	ObjClassVersion=1
}