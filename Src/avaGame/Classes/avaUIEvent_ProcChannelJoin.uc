class avaUIEvent_ProcChannelJoin extends UIEvent;


defaultproperties
{
	ObjName="Proc ChannelJoin"
	ObjCategory="ProcChannel"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Response",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="bGuildChannel",bWriteable=true))

	ObjClassVersion=2
}