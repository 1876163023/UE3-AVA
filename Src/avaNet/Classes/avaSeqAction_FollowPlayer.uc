/*
	다른 플레이어 따라서 해당 방으로 들어가기 위한 액션.
*/
class avaSeqAction_FollowPlayer extends avaSeqAction;

var() string Nickname;
var() string Password;

event Activated()
{
	if ( InputLinks[0].bHasImpulse )
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().FollowPlayer(Nickname);
	else if ( InputLinks[1].bHasImpulse )
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().SetFollowPlayerPwd(Password);
}

defaultproperties
{
	ObjName="(Comm.) Follow Player"

	InputLinks.Empty
	InputLinks(0)=(LinkDesc="In Nickname")
	InputLinks(1)=(LinkDesc="In Password")

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nickname",PropertyName=Nickname))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Password",PropertyName=Password))

	ObjClassVersion=2
}

