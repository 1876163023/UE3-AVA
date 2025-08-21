class avaSeqAction_RoomGetRoomInfo extends avaSeqAction;

var() int		RoomListIndex;
var() int		RoomID;
var() bool		bPassword;
var() string	HostName;
var() string	RoomName;


event Activated()
{
	Local int nPassword;

	if ( class'avaNetHandler'.static.GetAvaNetHandler().GetRoomInfo(RoomListIndex, RoomID, nPassword, RoomName, HostName) )
	{
		bPassword = bool(nPassword);
		OutputLinks[0].bHasImpulse = true;
	}
	else
	{
		OutputLinks[1].bHasImpulse = true;
	}
}


defaultproperties
{
	ObjName="(Room) Get RoomInfo"

	OutputLinks(0)=(LinkDesc="Successful")
	OutputLinks(1)=(LinkDesc="Failed")

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Room List Index",PropertyName=RoomListIndex))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Room ID",bWriteable=true,PropertyName=RoomID))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Locked",bWriteable=true,PropertyName=bPassword))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Host Name",bWriteable=true,PropertyName=HostName))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Room Name",bWriteable=true,PropertyName=RoomName))

	ObjClassVersion=1
}