class avaSeqAction_GetRoomKickedState extends avaSeqAction;

var() string KickState;

event Activated()
{
	KickState =	class'avaNetHandler'.static.GetAvaNetHandler().CheckRoomKickedState();
}

defaultproperties
{
	ObjName="Get RoomKickedState"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="State",PropertyName=KickState,bWriteable=true))

	ObjClassVersion=1
}