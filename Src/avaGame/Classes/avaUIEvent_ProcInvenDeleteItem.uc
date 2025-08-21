class avaUIEvent_ProcInvenDeleteItem extends UIEvent;


defaultproperties
{
	ObjName="Proc Inven DeleteItem"
	ObjCategory="ProcInven"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="ItemName",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="ItemID",bWriteable=true))

	ObjClassVersion=1
}