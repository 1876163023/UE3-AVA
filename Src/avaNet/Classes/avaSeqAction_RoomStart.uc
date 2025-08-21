class avaSeqAction_RoomStart extends avaSeqAction;




event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomStart();
}


defaultproperties
{
	ObjName="(Room) Start Game"

    VariableLinks.Empty
}

