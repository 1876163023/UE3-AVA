class avaSeqAction_GuildLeave extends avaSeqAction;

event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().GuildLeave();
}


defaultproperties
{
	ObjName="(Guild) Leave"

    VariableLinks.Empty

	ObjClassVersion=1
}

