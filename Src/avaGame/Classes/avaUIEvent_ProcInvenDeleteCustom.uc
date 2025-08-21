class avaUIEvent_ProcInvenDeleteCustom extends UIEvent;


defaultproperties
{
	ObjName="Proc Inven DeleteCustom"
	ObjCategory="ProcInven"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="CustomName",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="CustomID",bWriteable=true))

	ObjClassVersion=1
}