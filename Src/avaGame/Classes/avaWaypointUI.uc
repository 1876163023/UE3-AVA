// !!! Expired avaWayPointUI !!!

class avaWaypointUI extends avaStateUI;

//function bool OnInputKey(int ControllerID, Name Key, int Number, EInputEvent Event)
//{
//	Local bool bSquadLeader;
//	Local PlayerReplicationInfo PRI;
////	Local int CursorX, CursorY;
////	Local GameViewportClient ViewportClient;
////	Local EMouseCursor CursorType;
//
//	if(!bOpened || GetLocalPC() == None)
//		return false;
//
//	PRI = GetLocalPC().PlayerReplicationInfo;
//	bSquadLeader = avaPlayerReplicationInfo(PRI).IsSquadLeader();
//
//	if( Key == 'F1' && bSquadLeader)
//	{
//		if(bCtrlPressed)
//			avaPlayerController(GetLocalPC()).ClearWaypoint(WPTeam_Blue);
//		else
//			avaPlayerController(GetLocalPC()).SetWaypointAround(WPTeam_Blue);
//	}
//	else if ( Key == 'F2' && bSquadLeader)
//	{
//		if(bCtrlPressed)
//			avaPlayerController(GetLocalPC()).ClearWaypoint(WPTeam_Yellow);
//		else
//			avaPlayerController(GetLocalPC()).SetWaypointAround(WPTeam_Yellow);
//	}
//
//	return false;
//}