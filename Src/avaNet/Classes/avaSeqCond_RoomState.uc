class avaSeqCond_RoomState extends avaSeqCondition;



event Activated()
{
	switch ( class'avaNet.avaNetRequest'.static.GetAvaNetRequest().GetCurrentRoomState() )
	{
	case 0:		OutputLinks[0].bHasImpulse = true;		break;
	case 1:		`log( "Current Room State is Waiting" );	OutputLinks[1].bHasImpulse = true;		break;
	case 2:		`log( "Current Room State is Playing" );	OutputLinks[2].bHasImpulse = true;		break;
	}
}


defaultproperties
{
	ObjCategory="avaNet"
	ObjName="(Room) State"

	OutputLinks(0)=(LinkDesc="No Room")
	OutputLinks(1)=(LinkDesc="Waiting")
	OutputLinks(2)=(LinkDesc="Playing")

	VariableLinks.Empty
}
