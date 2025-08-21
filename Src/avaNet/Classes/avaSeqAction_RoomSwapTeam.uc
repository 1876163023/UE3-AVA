class avaSeqAction_RoomSwapTeam extends avaSeqAction;



event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomSwapTeam(1);
}


defaultproperties
{
	ObjName="(Room) Swap Team"

    VariableLinks.Empty

	ObjClassVersion=1
}

