class avaSeqAction_BuddyAdd extends avaSeqAction;

var() bool bForce;
var() string Nickname;


event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().BuddyAdd(bForce, Nickname);
}


defaultproperties
{
	ObjName="(Buddy) Add"

	InputLinks(0)=(LinkDesc="In")

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Force",PropertyName=bForce))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nickname",PropertyName=Nickname))

	ObjClassVersion=1
}

