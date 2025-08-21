class avaSeqAction_RoomReady extends avaSeqAction;



var() bool bReady;

event Activated()
{
	if (InputLinks[0].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomReady(bReady);
	}
	else if (InputLinks[1].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomReadyToggle();
	}
}


defaultproperties
{
	ObjName="(Room) Ready"

	InputLinks(0)=(LinkDesc="Set")
	InputLinks(1)=(LinkDesc="Toggle")

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Ready",PropertyName=bReady))
}

