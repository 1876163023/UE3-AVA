class avaSeqAction_InviteGuild extends avaSeqAction;


var() string Nickname;

event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().InviteGuild(Nickname);
}


defaultproperties
{
	ObjName="(Comm.) Invite To Guild"

	InputLinks.Empty
	InputLinks(0)=(LinkDesc="In")

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nickname",PropertyName=Nickname))

	ObjClassVersion=1
}

