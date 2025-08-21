class avaSeqAction_JoinRoom extends avaSeqAction;



var() int ListIndex;
var() string RoomPassword;


event Activated()
{
	local int RoomIndex;

	if (InputLinks[0].bHasImpulse)
	{
		if (class'avaNet.avaNetRequest'.static.GetAvaNetRequest().JoinRoom(ListIndex, RoomPassword))
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;
	}
	else if (InputLinks[1].bHasImpulse)
	{
		if (class'avaNet.avaNetRequest'.static.GetAvaNetRequest().QuickJoinRoom())
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;
	}
	else if (InputLinks[2].bHasImpulse)
	{
		RoomIndex = class'avaNet.avaNetRequest'.static.GetAvaNetRequest().GetCurrentRoomIndex();
		if (class'avaNet.avaNetRequest'.static.GetAvaNetRequest().JoinRoom(RoomIndex, RoomPassword))
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;
	}
}


defaultproperties
{
	ObjName="(Lobby) Join Room"

	InputLinks(0)=(LinkDesc="Join")
	InputLinks(1)=(LinkDesc="Quick Join")
	InputLinks(2)=(LinkDesc="Join Current")

	OutputLinks(0)=(LinkDesc="Successful")
	OutputLinks(1)=(LinkDesc="Failed")

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="List Index",PropertyName=ListIndex))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Password",PropertyName=RoomPassword))
	bAutoActivateOutputLinks=false

	ObjClassVersion=2
}

