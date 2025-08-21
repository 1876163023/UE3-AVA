class avaUIEvent_ProcInvenDeleteEffect extends UIEvent;


defaultproperties
{
	ObjName="Proc Inven DeleteEffect"
	ObjCategory="ProcInven"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="CustomName",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="CustomID",bWriteable=true))

	ObjClassVersion=1
}