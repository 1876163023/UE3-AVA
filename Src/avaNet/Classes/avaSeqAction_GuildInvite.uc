class avaSeqAction_GuildInvite extends avaSeqAction;

var() string Nickname;

event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().InviteGuild(Nickname);
}


defaultproperties
{
	ObjName="(Guild) Invite"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nickname",PropertyName=Nickname))

	ObjClassVersion=1
}

