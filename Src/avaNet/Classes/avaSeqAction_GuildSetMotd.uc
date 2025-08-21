class avaSeqAction_GuildSetMotd extends avaSeqAction;


var() string Motd;

event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().GuildSetMotd(Motd);
}


defaultproperties
{
	ObjName="(Guild) Set MotD"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="MotD",PropertyName=Motd))

	ObjClassVersion=1
}

