class avaSeqAction_GuildKick extends avaSeqAction;

var() string Nickname;

event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().GuildKick(Nickname);
}


defaultproperties
{
	ObjName="(Guild) Kick"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nickname",PropertyName=Nickname))

	ObjClassVersion=1
}

