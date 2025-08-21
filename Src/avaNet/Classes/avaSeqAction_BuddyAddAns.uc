class avaSeqAction_BuddyAddAns extends avaSeqAction;


var() string Nickname;

event Activated()
{
	if (InputLinks[0].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().BuddyAddAns(true, Nickname);
	}
	else
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().BuddyAddAns(false, Nickname);
	}
}


defaultproperties
{
	ObjName="(Buddy) Reply To Add"

	InputLinks.Empty
	InputLinks(0)=(LinkDesc="Accept")
	InputLinks(1)=(LinkDesc="Reject")

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nickname",PropertyName=Nickname))

	ObjClassVersion=1
}

