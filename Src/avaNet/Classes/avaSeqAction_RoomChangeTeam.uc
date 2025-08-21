class avaSeqAction_RoomChangeTeam extends avaSeqAction;



event Activated()
{
	if (InputLinks[0].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomChangeTeam(0);
	}
	else if (InputLinks[1].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomChangeTeam(1);
	}
	else if (InputLinks[2].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomChangeTeamToggle();
	}
	else if (InputLinks[3].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomChangeTeam(2);
	}
}


defaultproperties
{
	ObjName="(Room) Change Team"

	InputLinks(0)=(LinkDesc="EU")
	InputLinks(1)=(LinkDesc="NRF")
	InputLinks(2)=(LinkDesc="Toggle")
	InputLinks(3)=(LinkDesc="Spectator")

    VariableLinks.Empty

	ObjClassVersion=2
}

