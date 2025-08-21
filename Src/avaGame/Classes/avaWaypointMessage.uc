// !!! Expired avaWayPointMessage !!! //

class avaWaypointMessage extends avaLocalMessage
	config(game);

//var localized string WaypointCommand[6];
//var localized string WaypointAction[3];
//
//var const Color WaypointCommandColor;
//
//enum WaypointCommandType
//{
//	WAYPOINTCOMMAND_SetWP1,
//	WAYPOINTCOMMAND_SetWP2,
//	WAYPOINTCOMMAND_ChangeWP1,
//	WAYPOINTCOMMAND_ChangeWP2,
//	WAYPOINTCOMMAND_CancelWP1,
//	WAYPOINTCOMMAND_CancelWP2,
//};
//
///* Ignored. Colored with avaLeaderMessage.*/
//static function color GetConsoleColor( PlayerReplicationInfo RelatedPRI_1 )
//{
//    return default.WaypointCommandColor;
//}
//
//
//static function ClientReceive(
//	PlayerController P,
//	optional int Switch,
//	optional PlayerReplicationInfo RelatedPRI_1,
//	optional PlayerReplicationInfo RelatedPRI_2,
//	optional Object OptionalObject
//	)
//{
//	Local avaHUD avaHUD;
//	Local avaMsgParam Param;
//	Local int nWaypoint, nAction;
//	Local string Key;
//
//	avaHUD = avaHUD(P.myHUD);
//
//	if( avaHUD == None )
//		return;
//
//	Param = avaMsgParam(OptionalObject);
//	nAction = Param.IntParam[0];
//	nWaypoint = Param.IntParam[1];
//	
//	Key = nAction $ nWaypoint;
//
//	Play( Key, RelatedPRI_1, P);
//	avaHUD.GameInfoMessage(default.WaypointCommand[GetMessageIndexOfKey(Key)]);
//}
//
//
//defaultproperties
//{
//	WaypointCommandColor=(R=220,G=220,B=220,A=220)
//	MessageData.Add(( Key="00", MsgIndex=WAYPOINTCOMMAND_SetWP1,SoundCue[0]=SoundCue'avaGameVoices.AutoVoices.Auto_CL_SetWP1_De',SoundCue[1]=SoundCue'avaGameVoices.AutoVoices.Auto_CL_SetWP1_Ru')
//	MessageData.Add(( Key="01", MsgIndex=WAYPOINTCOMMAND_SetWP2,SoundCue[0]=SoundCue'avaGameVoices.AutoVoices.Auto_CL_SetWP2_De',SoundCue[1]=SoundCue'avaGameVoices.AutoVoices.Auto_CL_SetWP2_Ru')
//	MessageData.Add(( Key="10", MsgIndex=WAYPOINTCOMMAND_ChangeWP1,SoundCue[0]=SoundCue'avaGameVoices.AutoVoices.Auto_CL_ChangeWP1_De',SoundCue[1]=SoundCue'avaGameVoices.AutoVoices.Auto_CL_ChangeWP1_Ru')
//	MessageData.Add(( Key="11", MsgIndex=WAYPOINTCOMMAND_ChangeWP2,SoundCue[0]=SoundCue'avaGameVoices.AutoVoices.Auto_CL_ChangeWP2_De',SoundCue[1]=SoundCue'avaGameVoices.AutoVoices.Auto_CL_ChangeWP2_Ru')
//	MessageData.Add(( Key="20", MsgIndex=WAYPOINTCOMMAND_CancelWP1,SoundCue[0]=SoundCue'avaGameVoices.AutoVoices.Auto_CL_CancelWP1_De',SoundCue[1]=SoundCue'avaGameVoices.AutoVoices.Auto_CL_CancelWP1_Ru')
//	MessageData.Add(( Key="21", MsgIndex=WAYPOINTCOMMAND_CancelWP2,SoundCue[0]=SoundCue'avaGameVoices.AutoVoices.Auto_CL_CancelWP2_De',SoundCue[1]=SoundCue'avaGameVoices.AutoVoices.Auto_CL_CancelWP2_Ru')
//}