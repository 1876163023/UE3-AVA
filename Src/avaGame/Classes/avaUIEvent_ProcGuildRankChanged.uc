class avaUIEvent_ProcGuildRankChanged extends UIEvent;


defaultproperties
{
	ObjName="Proc Guild RankChanged"
	ObjCategory="ProcGuild"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nick",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="AccountID",bWriteable=true))

	ObjClassVersion=1
}