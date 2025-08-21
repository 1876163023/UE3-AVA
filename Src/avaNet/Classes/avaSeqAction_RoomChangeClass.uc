class avaSeqAction_RoomChangeClass extends avaSeqAction;



event Activated()
{
	if (InputLinks[0].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomChangeClass(0);
	}
	else if (InputLinks[1].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomChangeClass(1);
	}
	else if (InputLinks[2].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomChangeClass(2);
	}
}


defaultproperties
{
	ObjName="(Room/Inventory) Change Class"

	//InputLinks(0)=(LinkDesc="Pointman")
	//InputLinks(1)=(LinkDesc="Rifleman")
	InputLinks(0)=(LinkDesc="Pointman")
	InputLinks(1)=(LinkDesc="Rifleman")
	InputLinks(2)=(LinkDesc="Sniper")

    VariableLinks.Empty

	ObjClassVersion=3
}

