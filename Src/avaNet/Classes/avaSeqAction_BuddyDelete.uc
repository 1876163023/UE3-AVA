class avaSeqAction_BuddyDelete extends avaSeqAction;

var() string Nickname;


event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().BuddyDelete(Nickname);
}


defaultproperties
{
	ObjName="(Buddy) Delete"

	InputLinks(0)=(LinkDesc="In")

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nickname",PropertyName=Nickname))

	ObjClassVersion=1
}

