class avaSeqAction_RoomCancelStart extends avaSeqAction;




event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomCancelStart();
}


defaultproperties
{
	ObjName="(Room) Cancel Count Down"

    VariableLinks.Empty
}

