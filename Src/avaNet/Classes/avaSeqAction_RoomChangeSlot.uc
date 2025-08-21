class avaSeqAction_RoomChangeSlot extends avaSeqAction;


var int Slot;

event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomChangeSlot(Slot);
}


defaultproperties
{
	ObjName="(Room) Change Slot"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Slot",PropertyName=Slot))

	ObjClassVersion=3
}

