class avaSeqAction_RoomGetRandomName extends avaSeqAction;



var string RoomName;


event Activated()
{
	RoomName = class'avaNet.avaNetRequest'.static.GetAvaNetRequest().GetRandomRoomName();
}


defaultproperties
{
	ObjName="(Room) Get Random Room Name"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Room Name",bWriteable=true,PropertyName=RoomName))

	ObjClassVersion=1
}

