class avaSeqAction_BuddyList extends avaSeqAction;



event Activated()
{
	if (InputLinks[0].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().BuddyListBegin();
	}
	else
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().BuddyListEnd();
	}
}


defaultproperties
{
	ObjName="(Buddy) List"

	InputLinks.Empty
	InputLinks(0)=(LinkDesc="Begin")
	InputLinks(1)=(LinkDesc="End")

	VariableLinks.Empty

	ObjClassVersion=1
}

