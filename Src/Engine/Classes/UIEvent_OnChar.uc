class UIEvent_OnChar extends UIEvent
	native(inherit);


defaultproperties
{
	ObjName="On Char"
	ObjCategory="UI"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="InChar",bWriteable=true,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="bComposing",bWriteable=true,bHidden=true))

	ObjClassVersion=1
}