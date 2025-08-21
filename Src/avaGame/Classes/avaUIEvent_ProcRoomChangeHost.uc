class avaUIEvent_ProcRoomChangeHost extends UIEvent;


defaultproperties
{
	ObjName="Proc Room ChangeHost"
	ObjCategory="ProcRoom"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="NewHostName",bWriteable=true))

	ObjClassVersion=1
}