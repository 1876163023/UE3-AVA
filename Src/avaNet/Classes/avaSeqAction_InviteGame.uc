class avaSeqAction_InviteGame extends avaSeqAction;


var() string Nickname;

event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().InviteGame(Nickname);
}


defaultproperties
{
	ObjName="(Comm.) Invite To Game"

	InputLinks.Empty
	InputLinks(0)=(LinkDesc="In")

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nickname",PropertyName=Nickname))

	ObjClassVersion=1
}

