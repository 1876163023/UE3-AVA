/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaGame

	Name: avaNetEntryGame.uc

	Description: Game class for interacting with AVA Servers via avaNet.

***/
class avaNetEntryGameEx extends avaNetEntryGame;

`include(avaGame/avaGame.uci)

`define AVANET

struct EventDescStrType
{
	var string	Desc;
	var string	StringValue;
};

struct EventDescIntType
{
	var string	Desc;
	var int		IntValue;
};

struct EventDescFloatType
{
	var string	Desc;
	var float	FloatValue;
};

struct EventDescBoolType
{
	var string	Desc;
	var bool	BoolValue;
};

`if( `isdefined(AVANET) )

var int nCurrentTickerCnt;

//! UI공지 이미지를 패치해주는 객체.
var avaUINoticePatcher	NoticePatcher;

var UIScene				PreloadedScene;		//!< 미리 로딩된 장면.
var string				PreloadedSceneName;	//!< 미리 로딩된 장면 이름.

// 항상 떠있는 씬. (친구 추가 응답과 같이 장면에 상관없이 열어야하는 씬의 이벤트를 받는다.)
var string				GlobalSceneName;
var UIScene				GlobalScene;

/*! @brief Main Entry Point(??)
	@note
		처음 실행시, 게임 플레이 종료 후(or 알 수 없는 이유로) 채널로 돌아올 경우
		가장 먼머 처리되어 초기화 되는 부분.

		avaGame(채널) -> avaSWGame(게임) -> avaGame(채널)
*/
event PostLogin( PlayerController NewPlayer )
{
	Local avaGameViewportClient ViewportClient;
	
	Super.PostLogin( NewPlayer );
	`log( "avaNetEntryGame.PostLogin" @class'avaNetHandler'.static.GetAvaNetHandler().CheckMyLocation() );

	// UIController 초기화
	UIController = LocalPlayer(NewPlayer.Player).ViewportClient.UIController;
	if (UIController == NONE)
	{
		`warn( "Failed to set UIController" );
		return;
	}

	// 호스트마이그레이션 플래그 초기화 ( 플래그는 로딩화면에 나오는 '호스트를 옮기는 중입니다'를 찍어줄지 말지 결정)
	ViewportClient = avaGameViewportClient(LocalPlayer(NewPlayer.Player).ViewportClient);
	if( ViewportClient != None )
		ViewportClient.SetHostMigration(false);

	// GlobalScene 초기화 , GlobalScene은 열려있는 Scene에 상관없이 Event를 받을 수 있는 Scene임 
	// 예를 들어 친구추가 요청과 같이 상태에 상관없이 처리해줘야하는 메세지를 처리.
	GlobalScene = LoadScene( GlobalSceneName );
	if( GlobalScene == None )
		`warn("Can't open global scene, some popup in general purpose is going to be disabled");
	else
		UIController.OpenScene(GlobalScene);

	if ( IsInState('MatchPending') )
	{
		class'avaNetHandler'.static.GetAvaNetHandler().InitWeaponIDList();
		InitScenes();

		// 현재 위치에 따라서 '카메라 위치'와 'UI장면'을 선정한다.
		switch ( class'avaNetHandler'.static.GetAvaNetHandler().CheckMyLocation() )
		{
		case 0:
			LoadScene(TitleSceneName);
			
			GotoScene("Title");
			//GotoScene("ChannelList");
			break;

		case 1:	// lobby
			// NetState는 이미 avaMsgProc에서 LOBBY로 변경해주므로 특별한 처리를 하지 않음.
			UpdatePlayerInfo();
			ActivateRemoteEvent('SetLobbyCamera');
			if ( class'avaNetHandler'.static.GetAvaNetHandler().IsInClanLobby() ) 
				GotoScene("ClanLobby", true);
			else
				GotoScene("Lobby", true);
			//class'avaNetHandler'.static.GetAvaNetHandler().ProcRoomMessage(EMsg_Room_Leave, "ok", "", 0, 0);
			break;

		case 2:	// ready room
			UpdatePlayerInfo();
			ActivateRemoteEvent('SetRoomCamera');
			if ( class'avaNetHandler'.static.GetAvaNetHandler().IsGameResultValid() )
			{
				if( class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentMapMissionType() == 2/*NMT_MilitaryDrill*/ )
				{
					GotoScene("ResultMD", true);
				}
				else
				{
					GotoScene("Result", true);
				}
			}
			else
			{
				class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ReadyRoom', , , true);
			}
			break;

		case 3:	// in game
			// in case of host crash
			UpdatePlayerInfo();
			ActivateRemoteEvent('SetRoomCamera');
			class'avaNetHandler'.static.GetAvaNetHandler().ProcHostCrash();
			GotoScene("Room", true );
			PopUpMessage("<Strings:AVANET.UIPopUpMsg.Msg_General_HostConnectionLostInGame>", "None" );
			break;

		default:
			GotoScene("Title");
		}
	}
	
	UpdateTickerMsg();
	//SetTimer( 10, false, 'UpdateTickerMsg' );
}

//! UI관련 공지파일을 다운로드 받는다.
function PatchNotice()
{
	if ( NoticePatcher != None)
		return ;

//	NoticePatcher = new(self) class'avaUINoticePatcher';

//	NoticePatcher = spawn(class'avaUINoticePatcher', self);
//	NoticePatcher.Download();
}

//! 공지로 사용될 이미지들을 다운받은 후 갱신된 경우 호출되는 함수.
function OnDownloaded()
{
//	`log("avaNetEntryGameEx.OnDownloaded()");

	// UIScene에 갱신 이벤트를 활성해 시켜준다.
	ActivateUIRemoteEvent('RefreshNoticeEvent');
}

//! 빈번하게 쓰이는 캐릭터 정보를 미리 로딩한다.
function PrecacheCharacters()
{
	local avaUICharacterCache	CharCache;
	local Vector				loc;
	local Rotator				rot;

	// precache안되게...
	return ;

	`log("### PrecacheCharacters() - Count:" @class'avaCache'.static.GetCount());

	// 등록된 오브젝트가 하나도 없는 경우.
	if ( class'avaCache'.static.GetCount() == 0 )
	{
		// 배경과 충돌하지 않게...?
		loc.x = 0;
		loc.y = 0;
		loc.z = 20;

		// 1회 생성해서 등록한다.
		CharCache = spawn(class'avaUICharacterCache',self,,loc,rot);
		CharCache.LoadObjects();
	}
}

function class<Pawn> GetDefaultPlayerClass(Controller C)
{
	return None;
}

function UpdateTickerMsg()
{
	local array<UIEvent>	Events;
	local int i;
	local string msg;
	
	GetSupportedEvent( class'avaUIEvent_UpdateTicker', Events );

	++nCurrentTickerCnt;
	if ( nCurrentTickerCnt >= class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentTickerCount() )
		nCurrentTickerCnt = 0;

	for ( i = 0 ; i < Events.length ; ++ i ) 
	{
		msg = GetCurrentTickerMsg();
		avaUIEvent_UpdateTicker(Events[i]).Trigger( msg );
	}
	SetTimer( 10, false, 'UpdateTickerMsg' );
}

function string GetCurrentTickerMsg()
{
	return "<AVANET:TickerMsg"$nCurrentTickerCnt$">";
}


// utilities

`devexec function GotoScene( string InScene, optional bool bDirect, optional name InNextUIEventName, optional bool bPreload )
{
	Local array<UIEvent>	EventsToActivate;
	local UIScene			LoadedScene;

	local string			NextSceneName;
	local class<UIEvent>	NextUIEventClass;
	local int			i;

	// Popup 이 떠있는가 Check 한다...
	if ( bPopUpMsgOpen == true )
	{
		if ( NextScene == "" || NextScene == "None" )
		{
			NextScene		=	InScene;
		}
		return;
	}

	switch (InScene)
	{
	case "NeowizLogo":		NextSceneName		= LogoSceneName;					break;
	case "RedduckLogo":		NextSceneName		= RedduckLogoSceneName;				break;
	case "AvaLogo":			NextSceneName		= AvaLogoSceneName;					break;
	case "Title":			NextSceneName		= TitleSceneName;					break;
	case "ChannelList":		NextSceneName		= ChannelListSceneName;
							NextUIEventClass	= class'avaUIEvent_ChannelList';	break;
	case "Lobby":			NextSceneName		= LobbySceneName;
							NextUIEventClass	= class'avaUIEvent_Lobby';			break;
	case "ClanLobby":		NextSceneName		= ClanLobbySceneName;
							NextUIEventClass	= class'avaUIEvent_Lobby';			break;

	case "Result":			NextSceneName		= ResultSceneName;					break;

	// Military Drill (개인전)
	case "ResultMD":		NextSceneName		= ResultMDSceneName;				break;
	case "Room":			NextSceneName		= RoomSceneName;
							NextUIEventClass	= class'avaUIEvent_ReadyRoom';		break;
	case "Inventory":																break;
	case "Shop":																	break;
	case "NewCharacter":	NextSceneName		= NewCharacterSceneName;			break;
							NextUIEventClass	= class'avaUIEvent_CreateCharacter';	break;
	case "Exit":			NextSceneName		= ExitSceneName;					break;
	default:				break;
	}

	// 미리 로딩만 필요하다면 실행 후 리턴.
	if ( bPreload )
	{
		if ( NextSceneName != "" )
		{
			// 이름 저장.
			PreloadedSceneName = NextSceneName;

			PreloadedScene = LoadScene( PreloadedSceneName );
			if ( PreloadedScene == None )
				`log("Cannot preload UIScene." @PreloadedSceneName);
			else
				`log("Preloaded UIScene:" @PreloadedSceneName);
		}

		return ;
	}

	// 바로 로딩해야 하거나, 이벤트 클래스가 없는 경우에는.
	if ( bDirect == true || NextUIEventClass == None )
	{
		if ( NextSceneName != "" )
		{
			// 이미 로딩된 장면이 있다면 얻어온다.
			if ( PreloadedSceneName == NextSceneName )
				LoadedScene = PreloadedScene;
			else
				LoadedScene = LoadScene( NextSceneName );

			if ( LoadedScene != None )
			{
				UIController.OpenScene( LoadedScene );
			}
		}
	}
	else
	{
		// 이벤트 클래스가 있는 경우에는 해당 이벤트들을 활성화 시켜준다.
		if ( NextUIEventClass != None )
		{
			GetSupportedEvent( NextUIEventClass, EventsToActivate );
			if ( EventsToActivate.Length != 0 )
			{
				ActivateEventByValue( EventsToActivate );

				for ( i = 0; i < EventsToActivate.Length; ++i )
					`log("Scene:" @InScene @" - EventsToActivate[" @i @"]" @EventsToActivate[i]);
			}
			else
			{
				// 이벤트 클래스와 같은 종류(자기자신 or 상속된 클래스)를 아무것도 찾지 못한 경우.
				`warn("There's no event events preparing for the event");
			}
		}
	}
	`log("GotoScene:" @InScene @", InNextUIEventName" @InNextUIEventName );

	if ( InNextUIEventName != '' )
		ActivateUIRemoteEvent( InNextUIEventName );
}

function UpdateRealTimeNotice()
{
	ActivateSingleEvent( class'avaUIEvent_UpdateRTNotice' );
	ActivateSingleEvent( class'avaUIEvent_OnChatMessage' );
	ClearTimer( 'ExpireUpdateRTNotice' );
	SetTimer( 60, FALSE, 'ExpireUpdateRTNotice' );
}

function ExpireUpdateRTNotice()
{
	class'avaNetHandler'.static.GetAvaNetHandler().ClearRTNotice();
	ActivateSingleEvent( class'avaUIEvent_UpdateRTNotice' );
}

/** 
 *	List update functions below ( RoomList, ChannelList, ChatMessageList ... )
 *
 **/
function Chat(string Nickname, string Msg)
{
	ActivateSingleEvent(class'avaUIEvent_OnChatMessage');
}

/** Channel functions below */
function UpdateChannelList()
{
	ActivateSingleEvent(class'avaUIEvent_OnChannelList');
}

/** Lobby functions below */
function UpdateLobbyPlayerList()
{
	ActivateSingleEvent(class'avaUIEvent_OnLobbyPlayer');
}

function AddLobbyPlayerList(int idAccount)
{
	ActivateSingleEvent(class'avaUIEvent_OnLobbyPlayer');
}

function RemoveLobbyPlayerList(int idAccount)
{
	ActivateSingleEvent(class'avaUIEvent_OnLobbyPlayer');
}

function UpdateLobbySelectedPlayer()
{
	ActivateSingleEvent(class'avaUIEvent_OnLobbySelectPlayer');
}

function UpdateLobbyRoomList()	
{
	ActivateSingleEvent(class'avaUIEvent_OnLobbyRoom');
}

function UpdateLobbySelectedRoom()
{
	ActivateSingleEvent(class'avaUIEvent_OnSelectRoom');
}

/** (Ready)Room functions below */
//function UpdateRoomPlayerList()
//{
//	ActivateSingleEvent(class'avaUIEvent_RoomPlayer');
//}

function AddRoomList(int idRoom)
{
	UpdateLobbyRoomList();
	UpdateLobbySelectedRoom();
}

function RemoveRoomList(int idRoom)
{
	UpdateLobbyRoomList();
	UpdateLobbySelectedRoom();
}

function UpdateRoomState(int idRoom)
{
	UpdateLobbyRoomList();
	UpdateLobbySelectedRoom();
}

function UpdateRoomSetting(int idRoom)
{
	UpdateLobbyRoomList();
	UpdateLobbySelectedRoom();
}


//function UpdateRoomTitle(string Text)
//{
//	RoomTitle = Text;
//}

function UpdateRoomPlayerList()
{
	// EU/NRF 리스트에 보여지는 플레이어 정보를 갱신한다.
	ActivateSingleEvent(class'avaUIEvent_OnReadyRoomPlayer');
}

function UpdateRoomSelectedPlayer()
{
	// EU/NRF 리스트에서 선택된 플레이어의 정보를 갱신한다.
	ActivateSingleEvent(class'avaUIEvent_OnReadyRoomSelectedPlayer');
}

function UpdateRoomButtons()
{
	// 버튼들의 Visible과 Enable상태를 재설정 해준다.
	ActivateSingleEvent(class'avaUIEvent_OnReadyRoomButton');
}

function AddRoomPlayerList(int idPlayer)
{
	UpdateRoomPlayerList();
	UpdateRoomButtons();
}

function RemoveRoomPlayerList(int idPlayer)
{
	UpdateRoomPlayerList();
	UpdateRoomButtons();
}

function RoomChangeHost(int idPlayer)
{
	UpdateRoomPlayerList();
	UpdateRoomButtons();
}

// @deprecated	avaNetHandler에서 모두 처리
//function RoomChangeSetting()
//{
//	// 맵정보에 대한 설정을 갱신한다.
//	ActivateSingleEvent(class'avaUIEvent_OnReadyRoomSetting');
//
//	// 관전 버튼의 Visible유무가 갱신 되어야 한다.
//	UpdateRoomButtons();
//}

function RoomChangeState()
{
	UpdateRoomPlayerList();
	// Playing상태의 Host가 게임을 종료하고 Lobby로 돌아온 경우
	// '시작'을 '준비'로 변경 해줘야 한다.
	UpdateRoomButtons();
}

/*
	@question
		자신과 자신 이외의 모든 강퇴에 대해서 적용되나?
		만약 자신이 강퇴당할 경우에는 RoomLeave가 자동으로 오는 듯 싶은데...
*/
function RoomKick(int idPlayer)
{
	UpdateRoomPlayerList();

	// 강퇴당하고 리스트가 갱신되면 Client로 변경이 가능할 수 있다.
	UpdateRoomButtons();
}

function RoomReady(int idPlayer, int Ready)
{
	UpdateRoomPlayerList();
	UpdateRoomButtons();
}

function RoomChangeClass(int idPlayer)
{
	UpdateRoomPlayerList();
	ActivateRemoteEvent('ChangeServerCharacter');
}

function RoomChangeWeapon(int idPlayer)
{
	UpdateRoomPlayerList();
}

function RoomChangeTeam(int idPlayer, int idTeam)
{
	ActivateRemoteEvent('ChangeServerCharacter');
	UpdateRoomPlayerList();
	UpdateRoomButtons();
}

// @deprecated
//function RoomNameChanged()
//{
//	// 방이름이 바뀌었으므로 방설정을 변경해준다.
//	ActivateSingleEvent(class'avaUIEvent_OnReadyRoomSetting');
//}

function UpdateRoomResult(string param)
{
	switch (param)
	{
		case "level":
		case "supply":
		case "money":
		case "award":
		case "skill":
		case "player result":
		case "room result":
			break;
	}

	ActivateUIRemoteEvent('RefreshResultValue');
}

/*! @brief 게임 시작을 위한 메시지 핸들러.
	@param bStart
		Host, Ready상태의 Client, Spectator의 경우 시작이며,
		Ready상태가 아닌 Client의 경우는 버튼만 갱신하게 된다.
*/
function GameStarted(int bStart)
{
	if (bStart > 0)
	{
		ActivateSingleEvent(class'avaUIEvent_StartGame');
	}
	else
	{
		UpdateRoomButtons();
	}
}

/*! @brief 카운트다운.
	@param bStart
		true	카운트다운을 시작하게 한다.
		false	카운트다운을 취소하고 멈추게 한다.
*/
function GameCountDown(bool bStart)
{
	if (bStart)
	{
		`log("### Start CountDown ###");

		if ( !IsTimerActive('Timer_CountDown') )
		{
			SetTimer(1, true, 'Timer_CountDown');
		}
	}
	else
	{
		`log("### Cancel CountDown ###");

		if ( IsTimerActive('Timer_CountDown') )
		{
			ClearTimer('Timer_CountDown');

			`log("Cancel CountDown");
		}
	}

	UpdateRoomButtons();
	// AutoTeamBalance가 설정된 방에서
	// Client가 시작버튼을 눌러서 EU에서 NRF로 간 후에 '시작취소'를 누르면
	// Ready상태로 그대로 남는 것을 갱신하기 위해서 호출.(2007/02/26)
	UpdateRoomPlayerList();
}

function Timer_CountDown()
{
	// 혹시 무기 설정창이 띄어져 있다면 닫아준다.
	// 무기 설정창이 활성화 중이면 대기방은 비활성화 되서 할 필요가 없다.
//	ActivateUIRemoteEvent('CloseWeaponInventoryScene');

	// 카운트다운이 끝나기 전까지 true를 리턴한다.
	if ( class'avaNetHandler'.static.GetAvaNetHandler().ProcCountDown() )
	{
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGameCountDown',,,true);
		Chat("", "");
	}
	else
	{
		// 카운트다운이 끝나고 나면 현재 타이머 함수가 호출되지 않도록 한다.
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGameCountDown',,,false);
		ClearTimer('Timer_CountDown');
	}
}

/** 각 상태 (채널로비, 레디룸, 인벤토리) 에사 요청(REQ)에 대한 응답(ACK/NAK)이 돌아왔을때 불린다*/
function ResponseChannelLobbyMessage()
{
	ActivateSingleEvent(class'avaUIEvent_ResponseChannelLobby');
}

function ResponseRoomMessage()
{
	// 모든 버튼의 상태를 Enable로 만들어 준다.
	ActivateSingleEvent(class'avaUIEvent_ResponseReadyRoom');
}

function ResponceInventoryMessage()
{
	ActivateSingleEvent(class'avaUIEvent_ResponseInventory');
}


/*! @brief Level Kismet에 RemoteEvent를 발생시켜 준다.
	@param EventName
		RemoteEvent중에 발생시켜야 할 이벤트 이름.
	@note
		avaNetHandler.uc에서 위치를 옮김.
*/
function ActivateRemoteEvent( name EventName )
{
	local Sequence					GameSeq;
	local int						i;
	local array<SequenceObject>		AllSeqEvents;
	local SeqEvent_RemoteEvent		RemoteEvent;

	// World상의 Level Kismet의 Sequence를 얻는다.
	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != None )
	{
		// RemoteEvent를 모두 찾아서.
		GameSeq.FindSeqObjectsByClass( class'SeqEvent_RemoteEvent', true, AllSeqEvents );

		for ( i = 0; i < AllSeqEvents.Length; ++ i )
		{
			RemoteEvent = SeqEvent_RemoteEvent( AllSeqEvents[i] );

			if ( RemoteEvent == None )
				continue;

			if ( RemoteEvent.bEnabled != true )
				continue;

			// 발생하고 싶은 이벤트 이름의 RemoteEvent만 발생시킨다.
			if( RemoteEvent.EventName == EventName )
			{
				`log("### RemoteEvent - " @EventName @"###");
				RemoteEvent.CheckActivate( WorldInfo, None );
			}
		}
	}

}

/*! @brief UI Kismet에 RemoteEvent를 발생시켜 준다.
	@param EventName
		RemoteEvent중에 발생시켜야 할 이벤트 이름.
*/
function ActivateUIRemoteEvent( name EventName )
{
	local int						i, j;
	local array<UIEvent>			AllUIEvents;
	local avaUIEvent_UIRemoteEvent	Event;
	local UIScene					ActiveScene;
	local int						numEvents;

	`log("### ActivateUIRemoteEvent(" @EventName @")  ###");

	if ( UIController.SceneClient == None )
	{
		`log("Failed ActivateUIRemoteEvent - ( UIController.SceneClient != None )");
		return ;
	}

	numEvents = 0;

	// 모든 활성화된 장면에 대해서 찾는다.
	for( i = 0 ; i < UIController.SceneClient.ActiveScenes.Length ; ++i )
	{
		ActiveScene = UIController.SceneClient.ActiveScenes[i];

		if( ActiveScene != None )
			ActiveScene.FindEventsOfClass(class'avaUIEvent_UIRemoteEvent', AllUIEvents);

		for ( j = 0; j < AllUIEvents.Length; ++j )
		{
			Event = avaUIEvent_UIRemoteEvent( AllUIEvents[j] );
			if ( Event == None )
				continue;

			// 같은 이름의 UI RemoteEvent만 활성화 시켜준다.
			if ( Event.EventName == EventName )
			{
				Event.ActivateUIEvent(0, ActiveScene);
				`log("### Event.ActivateUIEvent(" @EventName @")  ###");

				numEvents++;
			}
		}
	}

	// 만약 해당하는 이벤특가 하나도 없는 경우에 한해서 로그를 남겨준다.
	if ( numEvents == 0 )
		`log("no match an event -" @EventName);
}

/*! @brief 로그인 후 서버에서 얻은 플레이어 정보를 (avaPlayerModifierInfo에) 갱신한다.
	@note
		3D World의 캐릭터의 장비도 같이 갱신해 준다.
*/
function UpdatePlayerInfo()
{
	local PlayerController			PC;
	local avaPlayerController		LocalPC;
	local avaPlayerReplicationInfo	avaPRI;
	local int						i;

	// 값이 없을때만 1번에 한해서 실행해 준다.
	if ( NoticePatcher == None)
		PatchNotice();

	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		LocalPC = avaPlayerController(PC);
		break;
	}

	avaPRI = avaPlayerReplicationInfo( LocalPC.PlayerReplicationInfo );

	avaPRI.CharacterItem	= class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("ChItem");
	avaPRI.PointManItem		= class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("SkillP");
	avaPRI.RifleManItem		= class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("SkillR");
	avaPRI.SniperItem		= class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("SkillS");
	avaPRI.WeaponPointMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("WeaponP");
	avaPRI.WeaponRifleMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("WeaponR");
	avaPRI.WeaponSniperMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("WeaponS");

	avaPRI.LastClass     = int( class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("LastClass") );
	avaPRI.LastTeam      = int( class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("Team") );
	avaPRI.bUpdate       = true;

	`log("ChItem=" @ avaPRI.CharacterItem);
	`log("SkillP=" @ avaPRI.PointManItem);
	`log("SkillR=" @ avaPRI.RifleManItem);
	`log("SkillS=" @ avaPRI.SniperItem);
	`log("WeaponP=" @ avaPRI.WeaponPointMan);
	`log("WeaponR=" @ avaPRI.WeaponRifleMan);
	`log("WeaponS=" @ avaPRI.WeaponSniperMan);
	`log("LastClass=" @ class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("LastClass") );
	`log("LastTeam=" @ class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("Team") );

	avaPRI.SetPlayerClassID( avaPRI.LastClass );
	avaPRI.FetchCharacterModifier();

	for ( i = 0 ; i < 3 ; ++ i )
	{
		avaPRI.FetchCharacterModifier( i );
		avaPRI.FetchWeaponModifier( i );
	}

	// Level Kismet에 RemoteEvent를 보내자!!
	ActivateRemoteEvent('UpdateChannelCharacter');


	// 값이 없을때만 1번에 한해서 실행해 준다.
	PrecacheCharacters();
}

//! 캐릭터 UIScene에서 모든 처리가 성공적으로 되었으면 호출된다.
function UpdateCreateCharacter()
{
	ActivateSingleEvent(class'avaUIEvent_OnCreateCharacter');
}

/*! @brief 상점에서 무기를 장착한 경우 호출되는 함수.
*/
function InventoryWeaponSet( optional bool bCustom = false )
{
	local PlayerController			PC;
	local avaPlayerController		LocalPC;
	local avaPlayerReplicationInfo	avaPRI;

	ActivateSingleEvent(class'avaUIEvent_OnInventoryWeaponSet');

	// 무기를 새로 설정했다면 StateController에서 받은 정보로 
	// LocalPlayerController의 PRI를 다시 갱신한다.
	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		LocalPC = avaPlayerController(PC);
		break;
	}

	avaPRI = avaPlayerReplicationInfo( LocalPC.PlayerReplicationInfo );

	avaPRI.WeaponPointMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("WeaponP");
	avaPRI.WeaponRifleMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("WeaponR");
	avaPRI.WeaponSniperMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("WeaponS");

	// 테스트.
	`log("WeaponP=" @ avaPRI.WeaponPointMan);
	`log("WeaponR=" @ avaPRI.WeaponRifleMan);
	`log("WeaponS=" @ avaPRI.WeaponSniperMan);

	avaPRI.ResetWeaponModifiers();

	// 각 병과별로 무기정보를 갱신한다.
	avaPRI.FetchWeaponModifier( PCT_PointMan );
	avaPRI.FetchWeaponModifier( PCT_RifleMan );
	avaPRI.FetchWeaponModifier( PCT_Sniper );

	if ( bCustom )
	{
		// 커스텀 창에서는 장착이 아닌 선택된 무기에 대해서 보여줘야 한다.
		ActivateSingleEvent(class'avaUIEvent_OnInventoryCustomWeaponSet');
	}
	else
	{
		// 서버에서 받은 정보로 무기를 갱신하는 이벤트 활성화 (Level Kismet).
		ActivateRemoteEvent('ChangeServerWeapon');
	}
}

function ShopBuy(optional bool bSuccess = true)
{
	if ( bSuccess )
		ActivateSingleEvent(class'avaUIEvent_OnShopBuyWeapon');
	else
		ActivateUIRemoteEvent('OnShopBuyWeapon_EnableButtons');
}

function ConvertRIS( optional bool bSuccess =true )
{
	if( bSuccess )
		ActivateSingleEvent(class'avaUIEvent_OnInventoryConvertRIS');
	else
		ActivateUIRemoteEvent('OnConvertRIS_EnableButtons');
}

function RepairWeapon( optional bool bSuccess = true )
{
	if( bSuccess )
		ActivateSingleEvent(class'avaUIEvent_OnInventoryRepairWeapon');
	else
		ActivateUIRemoteEvent('OnRepairWeapon_EnableButtons');

}

/*! @brief 상점에서 캐릭터 장비를 장착한 경우 호출되는 함수.
*/
function InventoryEquipSet()
{
	local PlayerController			PC;
	local avaPlayerController		LocalPC;
	local avaPlayerReplicationInfo	avaPRI;

	ActivateSingleEvent(class'avaUIEvent_OnInventoryEquipSet');

	// 장비를 새로 설정했다면 StateController에서 받은 정보로 
	// LocalPlayerController의 PRI를 다시 갱신한다.
	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		LocalPC = avaPlayerController(PC);
		break;
	}

	avaPRI = avaPlayerReplicationInfo( LocalPC.PlayerReplicationInfo );

	avaPRI.CharacterItem = class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("ChItem");

	// 테스트.
	`log("ChItem=" @ avaPRI.CharacterItem);

	// 내부적으로 모든 캐릭터 Modifiers정보가 초기화.
	avaPRI.ResetCharacterModifiers();

	// 각 병과별로 장비정보를 갱신한다.
	avaPRI.FetchCharacterModifier( );
	avaPRI.FetchCharacterModifier( PCT_PointMan );
	avaPRI.FetchCharacterModifier( PCT_RifleMan );
	avaPRI.FetchCharacterModifier( PCT_Sniper );

	// 서버에서 받은 정보로 무기를 갱신하는 이벤트 활성화 (Level Kismet).
	ActivateRemoteEvent('ChangeServerCharacter');
}


static function ActivateSingleEvent( class<UIEvent> EventClassToActivate )
{
	Local array<UIEvent> EventsToActivate;

	GetSupportedEvent(EventClassToActivate, EventsToActivate);

	if( EventsToActivate.Length == 0 )
		`warn("There's no event events preparing for the event "$EventClassToActivate);
	else
		ActivateEventByValue(EventsToActivate);
}

final static function GetSupportedEvent( class<UIEvent> EventClassToFind, out array<UIEvent> out_FoundEvents)
{
	Local int i, ChildIndex;
	Local array<UIObject> Child;
	local UIInteraction				UIControl;
	local PlayerController			PC, LocalPC;

	foreach class'avaNetHandler'.static.GetAvaNetHandler().GetWorldInfo().LocalPlayerControllers( PC )
	{
		LocalPC = PC;
	}

	if ( LocalPC == None )
	{
		`warn("Can't find LocalPlayerController to activate an event ");
		return;
	}

	UIControl = LocalPlayer(localPC.Player).ViewportClient.UIController;
	if ( UIControl.SceneClient == None )
	{
		`warn("Can't find SceneClient to activate an event ");
		return;
	}

	//각각의 Active Scene에 대해
	for( i = 0 ; i < UIControl.SceneClient.ActiveScenes.Length ; i++ )
	{
		if( UIControl.SceneClient.ActiveScenes[i] != None )
		{
			// 자신(Scene)에서 먼저 Event를 찾고
			UIControl.SceneClient.ActiveScenes[i].FindEventsOfClass(EventClassToFind, out_FoundEvents);
			// 자식들 재귀적으로 찾고
			Child = UIControl.SceneClient.ActiveScenes[i].GetChildren(TRUE);
			// 찾은 자식들에 대해 EventClass를 찾는다.
			for( ChildIndex = 0 ; ChildIndex < Child.Length ; ChildIndex++ )
				if( Child[ChildIndex] != None )
					Child[ChildIndex].FindEventsOfClass(EventClassToFind, out_FoundEvents);
		}
	}
}

final static function ActivateEventByValue(array<UIEvent> EventsToActivate, optional array<EventDescStrType> VarStr, optional array<EventDescIntType> VarInt, optional array<EventDescFloatType> VarFloat, optional array<EventDescBoolType> VarBool)
{
	Local int i;
	Local int VarLinksIndex;
	Local int LinkedVarIndex;
	Local int VarIndex;
	Local SequenceVariable SeqVar;
	Local SeqVarLink VarLinks;

	for( i = 0 ; i < EventsToActivate.Length ; i++ )
	{
		if( EventsToActivate[i] == None )
			continue;

		EventsToActivate[i].ConditionalActivateUIEvent( -1 , EventsToActivate[i].GetOwner(), None, FALSE);
		for( VarLinksIndex = 0 ; VarLinksIndex < EventsToActivate[i].VariableLinks.Length ; VarLinksIndex++ )
		{
			VarLinks = EventsToActivate[i].VariableLinks[VarLinksIndex];

			for( LinkedVarIndex = 0 ; LinkedVarIndex < VarLinks.LinkedVariables.Length ; LinkedVarIndex++ )
			{
				SeqVar = VarLinks.LinkedVariables[LinkedVarIndex];

				if( VarStr.Length != 0 && SeqVar_String(SeqVar) != None )
				{
					for( VarIndex = 0 ; VarIndex < VarStr.Length ; VarIndex++ )
					{
						if( VarLinks.LinkDesc == VarStr[VarIndex].Desc)
						{
							SeqVar_String(SeqVar).StrValue = VarStr[VarIndex].StringValue;
							VarStr.Remove(VarIndex,1);
							break;
						}
					}
				}
				else if ( VarBool.Length != 0 && SeqVar_Bool(SeqVar) != None )
				{
					for( VarIndex = 0 ; VarIndex < VarBool.Length ; VarIndex++ )
					{
						if( VarLinks.LinkDesc == VarBool[VarIndex].Desc)
						{
							SeqVar_Bool(SeqVar).bValue = int(VarBool[VarIndex].BoolValue);
							VarBool.Remove(VarIndex,1);
							break;
						}
					}
				}
				else if ( VarFloat.Length != 0 && SeqVar_Float(SeqVar) != None )
				{
					for( VarIndex = 0 ; VarIndex < VarFloat.Length ; VarIndex++ )
					{
						if( VarLinks.LinkDesc == VarFloat[VarIndex].Desc)
						{
							SeqVar_Float(SeqVar).FloatValue = VarFloat[VarIndex].FloatValue;
							VarFloat.Remove(VarIndex,1);
							break;
						}
					}
				}
				else if ( VarInt.Length != 0 && SeqVar_Int(SeqVar) != None)
				{
					for( VarIndex = 0 ; VarIndex < VarStr.Length ; VarIndex++ )
					{
						if( VarLinks.LinkDesc == VarInt[VarIndex].Desc)
						{
							SeqVar_Int(SeqVar).IntValue = VarInt[VarIndex].IntValue;
							VarInt.Remove(VarIndex,1);
							break;
						}
					}
				}

			}		// end of LinkedVariables

		}		// end of VariableLinks

	}		// end of EventsToActivate
}

static event LoadDLOs()
{
	DynamicLoadObject( default.TitleSceneName, class'Object' );	
}

function RestartPlayer(Controller NewPlayer)
{

}

defaultproperties
{
	
	
	//LobbySceneName = "avaUIScene.Channel.Lobby"
	//RoomSceneName = "avaUIScene.Channel.ReadyRoom"
	//InventorySceneName = "avaUIScene.Channel.Inventory_Temp"
	//ShopSceneName = "avaUIScene.Channel.Shop_Temp"
	//PopUpMsgSceneName = "avaUIScene.Channel.PopUpMsg"
	//LoadingSceneName = "avaUIScene.Channel.Loading"


	LogoSceneName			=	"avaUISceneEx_Logo.NeowizLogo"
	RedduckLogoSceneName	=	"avaUISceneEx_Logo.Logo"
	AvaLogoSceneName		=	"avaUISceneEx_Logo.avaBI"
	TitleSceneName			=	"avaUISceneEx_Title.Title"
	NewCharacterSceneName	=	"avaUISceneEx_CreateCharacter.CreateCharacter"
	ChannelListSceneName	=	"avaUISceneEx_Channel.ChannelList"
	LobbySceneName			=	"avaUISceneEx_Lobby.Lobby"
	ClanLobbySceneName		=	"avaUISceneEx_ClanLobby.ClanLobby"
	RoomSceneName			=	"avaUISceneEX_ReadyRoom.ReadyRoom"
	ResultSceneName			=	"avaUISceneEx_Result.Result"
	ResultMDSceneName		=	"avaUISceneEx_Result.Result_MilitaryDrill"
	
	ExitSceneName	= "avaUISceneEx_Exit.Exit"

//  PopUpMessage를 Kismet으로 넘기지 않고 기존의 스크립트 팝업을 사용
	PopUpMsgSceneName = "avaUISceneEx_Misc.PopUpWindow"
	WidgetMessageLabel = "UILabel_PopUpMessage"

	GlobalSceneName			=	"avaUISceneEx_Global.GlobalScene"

	DefaultUIPathPrefix		=	"avaUISceneEx"
}
