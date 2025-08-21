class avaUIEvent_ProcGuildMOTD extends UIEvent;


defaultproperties
{
	ObjName="Proc Guild MOTD"
	ObjCategory="ProcGuild"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Response",bWriteable=true))

	ObjClassVersion=1
}