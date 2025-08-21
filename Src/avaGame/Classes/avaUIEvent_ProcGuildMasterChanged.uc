class avaUIEvent_ProcGuildMasterChanged extends UIEvent;


defaultproperties
{
	ObjName="Proc Guild MasterChanged"
	ObjCategory="ProcGuild"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nick",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="AccountID",bWriteable=true))

	ObjClassVersion=1
}