class avaSeqAction_InviteGuildAns extends avaSeqAction;


var() string Nickname;

event Activated()
{
	if (InputLinks[0].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().InviteGuildAns(true, Nickname);
	}
	else
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().InviteGuildAns(false, Nickname);
	}
}


defaultproperties
{
	ObjName="(Comm.) Reply To Guild Invitation"

	InputLinks.Empty
	InputLinks(0)=(LinkDesc="Accept")
	InputLinks(1)=(LinkDesc="Reject")

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nickname",PropertyName=Nickname))

	ObjClassVersion=1
}

