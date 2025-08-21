class avaUIEvent_SimpleTextChanged extends UIEvent
	native;

defaultproperties
{
	ObjName="SimpleTextChanged"
	ObjCategory="Value Changed"

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Text",bWriteable=true));

	ObjClassVersion=1
}