class avaQuickChatUI extends avaStateUI
	within avaGameViewportClient
	config(Game);


var const Color ActColor;
var const Color DeactColor;

var int			CommandLeader_Team;
var int			CommandLeader_Strategy;
var int			CommandLeader_Waypoint;

var const float	DefaultLifeTime;
var const Color DefaultColor;

var float StartPendingTime;
var const float PendingDuration;

/* Non-State IO Procedure. 
On the Other State such as RadioCommand , below function is ignored */
function bool OnInputKey(int ControllerID, Name Key, int Number, EInputEvent Event)
{
	local PlayerReplicationInfo PRI;
	local PlayerController		PC;

	PC = GetLocalPC();

	if ( !bOpened || PC == None )
		return false;

	if ( PC.Pawn == None )				return false;
	if ( bEnableQuickChat == false )	return false;

	//MyNetLocation = class'avaNetHandler'.static.GetAvaNetHandler().CheckMyLocation();
	//// MyNet Location '3' means that you're playing games
	//if( MyNetLocation != 3 && MyNetLocation != 0)
	//{
	//	ClearMenu();
	//	return false;
	//}

	PRI = PC.PlayerReplicationInfo;

	if( Key == 'QuickChatCommonCommand' )
	{
		if( PeekMenuName() == "RadioCommand")
			ClearMenu();
		else
		{
			ClearMenu();
			PushMenu("RadioCommand");
		}
		return true;
	}
	else if (Key == 'QuickChatCommonResponse' )
	{
		if( PeekMenuName() == "RadioResponse")
			ClearMenu();
		else
		{	
			ClearMenu();
			PushMenu("RadioResponse");
		}
		return true;
	}
	else if ( Key == 'QuickMemberCommand' )
	{
		if( InStr( PeekMenuName() , "CommandLeader" ) >= 0 || InStr( PeekMenuName(), "CommandMember") >= 0)
		{
			ClearMenu();
		}
		else
		{
			ClearMenu();
			if ( avaPlayerReplicationInfo(PRI).IsSquadLeader() )
				PushMenu("CommandLeader_Team");
			else
				PushMenu("CommandMember");
		}
		return true;
	}
	return false;
}

function OnEndState(string NextStateName)
{
	ClearMessage();
}

function OnBeginRadioCommand(string PrevStateName)
{
	Local int i;
	Local array<String> Msg;

	Msg.Length = 8 + 1;
	for(i = 0; i < 8 ; i++)
		Msg[i] = (i + 1)$"."@class'avaRadioCommandMessage'.Default.RadioCommand[i];
	
	Msg[Msg.Length-1] = "0."@class'avaLocalizedMessage'.default.Cancel;
	AddMessage(Msg, 0, DefaultColor);
}

function bool OnKeyRadioCommand(int ControllerID, Name Key, int Number, EInputEvent Event)
{
	OnInputKey(ControllerID,Key, Number, Event);

	switch(Number)
	{
	case 1: case 2:	case 3:	case 4:	case 5:	case 6:	case 7:	case 8:	
		BroadcastAll(class'avaRadioCommandMessage', Number); 
		ClearMenu();						break;
	case 0: ClearMenu();					break;
	}

	if( 0 <= Number && Number <= 9)
		return true;

	return false;
}

function OnBeginRadioResponse(string PrevStateName)
{
	Local int i;
	Local array<String> Msg;
	Local string Label;

	Msg.Length = 7 + 1;
	for(i = 0; i < 7 ; i++)
	{
		Label = ( i >= 6 ) ? "["$class'avaRadioResponseMessage'.Default.RadioResponse_Label_BroadCast$"]" : "";
		Msg[i] = (i + 1)$"."@Label@class'avaRadioResponseMessage'.Default.RadioResponse[i];
	}
	Msg[Msg.Length - 1] = "0."@class'avaLocalizedMessage'.default.Cancel;

	AddMessage(Msg, 0, DefaultColor);
}

function bool OnKeyRadioResponse(int ControllerID, Name Key, int Number, EInputEvent Event)
{
	OnInputKey(ControllerID,Key, Number, Event);
	if ( Number == 0 )
	{
		ClearMenu();
		return true;
	}
	else if ( Number >=1 && Number <=7 )
	{
		BroadcastAll(class'avaRadioResponseMessage', Number); 
		ClearMenu();
		return true;
	}
	return false;
}

function OnBeginCommandLeader_Target(string PrevStateName)
{
	local array<string>	Msg;
	local int			i;
	local avaHUD		hud;
	hud = avaHUD(GetLocalPC().MyHUD);
	for ( i = 0 ; i < hud.POIList.length ; ++ i )
	{
		Msg.Add(1);
		Msg[Msg.length] = ""$(i+1)$"."@class'avaHUD'.default.PlaceOfInterest[hud.POIList[i].NameIndex];
	}
	Msg.Add(1);
	Msg[Msg.length]	=	"9."@class'avaLocalizedMessage'.default.TargetCancel;
	Msg.Add(1);
	Msg[Msg.length]	=	"0."@class'avaLocalizedMessage'.default.Cancel;
	AddMessage( Msg, 0, DefaultColor );	
}

function bool OnKeyCommandLeader_Target(int ControllerID, Name Key, int Number, EInputEvent Event)
{
	local avaMsgParam	Param;
	local avaHUD		hud;
	hud = avaHUD(GetLocalPC().MyHUD);
	if ( OnInputKey(ControllerID,Key, Number, Event) )
		return false;

	if ( Number == 0 )
	{
		ClearMenu();
		return true;
	}
	else if ( Number == 9 )
	{
		Param = class'avaMsgParam'.static.Create();
		Param.SetInt(Number-1);
		BroadcastAll(class'avaLeaderMessage', 1, Param);
		ClearMenu();
		return true;
	}
	else if ( Number >= 1 )
	{	
		if ( Number-1 < hud.POIList.length )
		{
			Param = class'avaMsgParam'.static.Create();
			Param.SetInt(Number-1);
			BroadcastAll(class'avaLeaderMessage', 1, Param);
			ClearMenu();	
		}
		return true;
	}
	return false;
}

function OnBeginCommandLeader_Team(string PrevStateName)
{
	Local int i, nOperation, MsgIndex;
	Local array<String> Msg;
	Local bool bAttackTeam;
	Local string Key;
	Local avaGameReplicationInfo GRI;
	Local avaPlayerReplicationInfo PRI;

	GRI = avaGameReplicationInfo(GetLocalPC().WorldInfo.GRI);
	PRI = avaPlayerReplicationinfo(GetLocalPC().PlayerReplicationInfo);

	Assert(GRI != None && PRI != None);
	nOperation = GRI.GetMissionCategory();
	bAttackTeam = GRI.IsAttackTeam(PRI);

	for(i = 0; i < 6 ; i++)
	{
		Msg.Length = Msg.Length + 1;
		Msg[Msg.Length - 1] = (Msg.Length)$"."@class'avaLeaderMessage'.default.LeaderCommand[i];
	}

	for( i = 0 ; i < 2 ; i ++)
	{
		Key = (i+7) $ nOperation $ (bAttackTeam ? "A" : "D");
		MsgIndex = class'avaLeaderMessage'.static.GetMessageIndexOfKey(key);
		if ( MsgIndex >= 0 )
		{
			Msg.Length = Msg.Length + 1;
			Msg[Msg.Length - 1] = (Msg.Length)$"."@class'avaLeaderMessage'.default.LeaderCommand[ MsgIndex ];
		}
	}
	
	Msg.Length = Msg.Length + 1;	
	Msg[Msg.Length - 1] = "0."@class'avaLocalizedMessage'.default.Cancel;

	AddMessage(Msg, 0, DefaultColor);
}

function bool OnKeyCommandLeader_Team(int ControllerID, Name Key, int Number, EInputEvent Event)
{
	//Local avaPlayerController PC;
	Local avaMsgParam Param;
	Local avaPlayerReplicationInfo PRI;
	Local avaGameReplicationInfo GRI;

	OnInputKey(ControllerID, Key, Number,Event);

	//PC = avaPlayerController(GetLocalPC());
	PRI = avaPlayerReplicationInfo(GetLocalPC().PlayerReplicationInfo);
	GRI = avaGameReplicationInfo(GetLocalPC().WorldInfo.GRI);

	switch(Number)
	{
	//case 1: 
	//	PC.SetWaypointTeam(WPTeam_Blue, Number);
	//	ClearMenu();																	break;
	//case 2: 
	//	PC.SetWaypointTeam(WPTeam_Yellow, Number);
	//	ClearMenu();																	break;
	//case 3:
	//	PC.SetWaypointTeam(WPTeam_MAX, Number);
	//	ClearMenu();																	break;
	case 1: 
		PushMenu( "CommandLeader_Target" );
		break;
	case 2: case 3:
	case 4:	case 5: case 6:
		Param = class'avaMsgParam'.static.Create();
		BroadcastAll(class'avaLeaderMessage', Number, Param);
		ClearMenu();																	break;
	case 7: case 8:
		if ( Messages.length <= Number )
		{
			ClearMenu();
			Param = class'avaMsgParam'.static.Create();
			Param.SetInt(GRI.GetMissionCategory());
			Param.SetBool(GRI.IsAttackTeam(PRI));
			BroadcastAll(class'avaLeaderMessage', Number, Param);
			ClearMenu();																
		}
		break;	
	case 0: 
		ClearMenu();																	break;
	}

	if( 0 <= Number && Number <= 8 )
		return true;

	return false;
}

//function OnBeginCommandLeader_Strategy(string PrevStateName)
//{
//	Local int i , nOperation, Index;
//	Local array<String> Msg;
//	Local bool bAttackTeam;
//	Local avaGameReplicationInfo GRI;
//	Local avaPlayerReplicationInfo PRI;
//
//	GRI = avaGameReplicationInfo(GetLocalPC().WorldInfo.GRI);
//	PRI = avaPlayerReplicationinfo(GetLocalPC().PlayerReplicationInfo);
//
//	Assert(GRI != None && PRI != None);
//	nOperation = GRI.GetMissionCategory();
//	bAttackTeam = GRI.IsAttackTeam(PRI);
//
//	for(i = 0; i < 7 ; i++)
//	{
//		Msg.Length = Msg.Length + 1;
//		Msg[i] = (Msg.Length)$"."@class'avaLeaderMessage'.Default.LeaderStrategy[i];
//	}
//	for( i = 0 ; i < 2 ; i++ )
//	{
//		Msg.Length = Msg.Length + 1;
//		Index = 2*2*nOperation + (bAttackTeam ? 0 : 2) + i;
//		Msg[Msg.Length - 1] = (Msg.Length)$"."@class'avaLeaderMessage'.Default.LeaderOperation[Index];
//	}
//
//	Msg.Length = Msg.Length + 1;
//	Msg[Msg.Length - 1] = "0."@class'avaLocalizedMessage'.default.Cancel;
//
//	AddMessage(Msg, 0, DefaultColor);
//}
//
//function bool OnKeyCommandLeader_Strategy(int ControllerID, Name Key, int Number, EInputEvent Event)
//{
//	Local avaMsgParam Param;
//	Local avaGameReplicationInfo GRI;
//
//	GRI = avaGameReplicationInfo(GetLocalPC().WorldInfo.GRI);
//
//	OnInputKey(ControllerID, Key, Number, Event);
//
//	switch(Number)
//	{
//	case 1:	case 2:	case 3:	
//		CommandLeader_Strategy = Number;	PushMenu("CommandLeader_Waypoint");	break;
//
//	case 4:	case 5:	case 6: case 7:
//		Param = class'avaMsgParam'.static.Create();
//		CommandLeader_Strategy = Number;
//		Param.SetInt(CommandLeader_Strategy);
//		BroadcastTeam(class'avaLeaderMessage', CommandLeader_Team, Param);
//		ClearMenu();														break;
//
//	case 8: case 9:
//		Param = class'avaMsgParam'.static.Create();
//		CommandLeader_Strategy = Number;
//		Param.SetInt(CommandLeader_Strategy, GRI.GetMissionCategory());
//		Param.SetBool(GRI.IsAttackTeam(GetLocalPC().PlayerReplicationInfo));
//		BroadcastTeam(class'avaLeaderMessage', CommandLeader_Team, Param);
//		ClearMenu();
//
//		break;
//
//	case 0:	
//		ClearMenu();													break;
//	}
//
//	if( 0 <= Number && Number <= 9)
//		return true;
//
//	return false;
//
//}
//function OnBeginCommandLeader_Waypoint(string PrevStateName)
//{
//	Local int i;
//	Local string Msg;
//	Local avaGameReplicationInfo GRI;
//	Local avaPlayerReplicationInfo PRI;
//	Local bool bActivate;
//
//	PRI = avaPlayerReplicationInfo(GetLocalPC().PlayerReplicationInfo);
//	GRI = avaGameReplicationInfo(GetLocalPC().WorldInfo.GRI);
//
//	for(i = 0; i < WPTeam_MAX ; i++)
//	{
//		bActivate = !GRI.IsEmptyWaypoint(EWaypointTeamType(i),PRI);
//		Msg = (i+1)$"."@"Waypoint"$(i + 1);
//		AppendMessage(Msg, 0, bActivate ? ActColor : DeactColor); 
//	}
//	Msg = "0."@class'avaLocalizedMessage'.default.Cancel;
//	AppendMessage(Msg, 0, ActColor); 
//}

//function bool OnKeyCommandLeader_Waypoint(int ControllerID, Name Key, int Number, EInputEvent Event)
//{
//	Local avaMsgParam Param;
//	Local avaGameReplicationInfo GRI;
//	Local avaPlayerReplicationInfo PRI;
//	Local bool bActivate;
//
//	PRI = avaPlayerReplicationInfo(GetLocalPC().PlayerReplicationInfo);
//	GRI = avaGameReplicationInfo(GetLocalPC().WorldInfo.GRI);
//
//	OnInputKey(ControllerID, Key, Number, Event);
//
//	switch(Number)
//	{
//	case 1: case 2:
//		bActivate = !GRI.IsEmptyWaypoint(EWaypointTeamType(Number-1), PRI);
//		if(!bActivate)
//			return false;
//		Param = class'avaMsgParam'.static.Create();
//		CommandLeader_Waypoint = Number;	
//		Param.SetInt(CommandLeader_Strategy, CommandLeader_Waypoint);
////		`Log("Param = "$Param);
//		BroadcastTeam(class'avaLeaderMessage', CommandLeader_Team, Param);		
//		ClearMenu();																	break;
//
//	case 0: 
//		ClearMenu();																		break;
//	}
//
//	if( 0 <= Number && Number <= 9)
//		return true;
//
//	return false;
//}

function OnBeginCommandMember(string PrevStateName)
{
	Local int i, nOperation, MsgIndex;
	Local array<String> Msg;
	Local bool bAttackTeam;
	Local string Key;
	Local avaGameReplicationInfo GRI;
	Local avaPlayerReplicationInfo PRI;


	GRI = avaGameReplicationInfo(GetLocalPC().WorldInfo.GRI);
	PRI = avaPlayerReplicationinfo(GetLocalPC().PlayerReplicationInfo);

	Assert(GRI != None && PRI != None);
	nOperation = GRI.GetMissionCategory();
	bAttackTeam = GRI.IsAttackTeam(PRI);

	for(i = 0; i < 6 ; i++)
	{
		Msg.Length = Msg.Length + 1;
		Msg[Msg.Length - 1] = (Msg.Length)$"."@class'avaMemberMessage'.default.MemberCommand[i];
	}

	for( i = 0 ; i < 2 ; i ++)
	{
		Key = (i+7) $ nOperation $ (bAttackTeam ? "A" : "D");
		MsgIndex = class'avaMemberMessage'.static.GetMessageIndexOfKey(key);
		if ( MsgIndex >= 0 )
		{
			Msg.Length = Msg.Length + 1;
			Msg[Msg.Length - 1] = (Msg.Length)$"."@class'avaMemberMessage'.default.MemberCommand[ MsgIndex ];
		}
	}
	
	Msg.Length = Msg.Length + 1;	
	Msg[Msg.Length - 1] = "0."@class'avaLocalizedMessage'.default.Cancel;

	AddMessage(Msg, 0, DefaultColor);
}

function bool OnKeyCommandMember(int ControllerID, Name Key, int Number, EInputEvent Event)
{
	//Local avaPlayerController PC;
	Local avaMsgParam Param;
	Local avaPlayerReplicationInfo PRI;
	Local avaGameReplicationInfo GRI;

	OnInputKey(ControllerID, Key, Number,Event);

	//PC = avaPlayerController(GetLocalPC());
	PRI = avaPlayerReplicationInfo(GetLocalPC().PlayerReplicationInfo);
	GRI = avaGameReplicationInfo(GetLocalPC().WorldInfo.GRI);

	switch(Number)
	{
	//case 1: 
	//	PC.SetWaypointTeam(WPTeam_Blue, Number);
	//	ClearMenu();																	break;
	//case 2: 
	//	PC.SetWaypointTeam(WPTeam_Yellow, Number);
	//	ClearMenu();																	break;
	//case 3:
	//	PC.SetWaypointTeam(WPTeam_MAX, Number);
	//	ClearMenu();																	break;
	case 1: case 2: case 3:
	case 4:	case 5: case 6:
		Param = class'avaMsgParam'.static.Create();
		BroadcastAll(class'avaMemberMessage', Number, Param);
		ClearMenu();																	break;
	case 7: case 8:
		if ( Messages.length <= Number )
		{
			ClearMenu();
			Param = class'avaMsgParam'.static.Create();
			Param.SetBool(GRI.IsAttackTeam(PRI));
			Param.SetInt(GRI.GetMissionCategory());
			BroadcastAll(class'avaMemberMessage', Number, Param);
			ClearMenu();																
		}
		break;	
	case 0: 
		ClearMenu();																	break;
	}

	if( 0 <= Number && Number <= 8 )
		return true;

	return false;
}


/* State'Waypoint' Specific function, Refresh 'CommandLeader_Waypoint' State */
function NotifyModifiedWaypoint()
{
	if(PeekMenuName() == "CommandLeader_Waypoint")
	{
		ClearMenu();
		PushMenu("CommandLeader_Waypoint");
	}
}


/* BroadCast */
function BroadcastTeam(Class<avaLocalMessage> MessageClassName, int Switch, optional avaMsgParam Param)
{
	Local avaPlayerController PC;
	PC = avaPlayerController(GetLocalPC());

	// World를 두번이상 띄웠을때 StartPendingTime은 그대로인대 WorldTime은 0으로 초기화 된다.
	// 따라서 StartPendingTime을 초기화해줘야함.
	if( PC.WorldInfo.TimeSeconds < StartPendingTime )
		StartPendingTime = 0.0f - PendingDuration;
	if( PC.WorldInfo.TimeSeconds < StartPendingTime + PendingDuration )
		return;
	avaPlayerController(GetLocalPC()).BroadcastTeamParam(MessageClassName, Switch, Param);
	SetCurrentPendingTime();
}

function BroadcastAll(Class<avaLocalMessage> MessageClassName, int Switch, optional avaMsgParam Param)
{
	Local avaPlayerController PC;
	PC = avaPlayerController(GetLocalPC());

	// World를 두번이상 띄웠을때 StartPendingTime은 그대로인대 WorldTime은 0으로 초기화 된다.
	// 따라서 StartPendingTime을 초기화해줘야함.
	if( PC.WorldInfo.TimeSeconds < StartPendingTime )
		StartPendingTime = 0.0f - PendingDuration;
	if( PC.WorldInfo.TimeSeconds < StartPendingTime + PendingDuration )
		return;
	avaPlayerController(GetLocalPC()).BroadcastLocalizedAll(MessageClassName, Switch, Param);
	SetCurrentPendingTime();
}

/* State Initialization, 추가를 원하는 사용자 스테이트는 여기서 Super.Initilized()이후에 추가해야한다. */
function Initialized()
{
	Super.Initialized();

	//Assert(avaHUD(GetLocalPC().MyHUD) != None);
	//RedirectMessages( avaHUD(GetLocalPC().MyHUD).QuickChatMessages );

	AddState("RadioCommand", OnKeyRadioCommand, OnBeginRadioCommand, OnEndState);
	AddState("RadioResponse", OnKeyRadioResponse, OnBeginRadioResponse, OnEndState);
	
	AddState("CommandLeader_Team", OnKeyCommandLeader_Team, OnBeginCommandLeader_Team, OnEndState);
	AddState("CommandLeader_Target", OnKeyCommandLeader_Target, OnBeginCommandLeader_Target, OnEndState);



	//AddState("CommandLeader_Strategy", OnKeyCommandLeader_Strategy, OnBeginCommandLeader_Strategy, OnEndState);
	//AddState("CommandLeader_Waypoint", OnKeyCommandLeader_Waypoint, OnBeginCommandLeader_Waypoint, OnEndState);
	
	AddState("CommandMember", OnKeyCommandMember, OnBeginCommandMember, OnEndState);
}

function RenderMenu( Canvas Canvas )
{
	// Positioning before Rendering Text
	SetRenderPos(15 , Canvas.ClipY - 200);
	SetLineDown(false);
	Super.RenderMenu( Canvas );
}

function NotifyPawnDied()
{
	ClearMenu();
	ClearMessage();
}

function bool CheckInputCondition()
{
	Local avaHUD HUD;
	Local avaPlayerController PC;
	Local UIInteraction UIController;

	PC = avaPlayerController(GetLocalPC());
	if( PC == None )
		return false;
	
	HUD = avaHUD(PC.MyHUD);
	if( HUD == None )
		return false;

	UIController = LocalPlayer(PC.Player).ViewportClient.UIController;

	if( UIController != None && 
		(UIController.FindSceneByTag( avaHUD(PC.MyHUD).ChatScene.SceneTag ) != none || 
		UIController.FindSceneByTag( avaHUD(PC.MyHUD).TeamChatScene.SceneTag ) != none) )
		return false;

	return true;
}

function SetCurrentPendingTime()
{
	Local PlayerController PC;
	PC = GetLocalPC();

	if( PC != None )
		StartPendingTime = PC.WorldInfo.TimeSeconds;
	else
		StartPendingTime = 0.0f;
}

function ClearMenu()
{
	bEnableVoteUI = true;
	Super.ClearMenu();
}

function bool PushMenu(string StateName)
{
	bEnableVoteUI = false;
	return Super.PushMenu( StateName );
}


defaultproperties
{
	DefaultLifeTime = 3.0
	DefaultColor = (R=248,G=197,B=103,A=255)
	ActColor=(R=0,G=230,B=100,A=170)
	DeactColor=(R=170,G=170,B=170,A=170)

	bOpened=true

	PendingDuration=2.0
}
