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

//! UI���� �̹����� ��ġ���ִ� ��ü.
var avaUINoticePatcher	NoticePatcher;

var UIScene				PreloadedScene;		//!< �̸� �ε��� ���.
var string				PreloadedSceneName;	//!< �̸� �ε��� ��� �̸�.

// �׻� ���ִ� ��. (ģ�� �߰� ����� ���� ��鿡 ������� ������ϴ� ���� �̺�Ʈ�� �޴´�.)
var string				GlobalSceneName;
var UIScene				GlobalScene;

/*! @brief Main Entry Point(??)
	@note
		ó�� �����, ���� �÷��� ���� ��(or �� �� ���� ������) ä�η� ���ƿ� ���
		���� �ո� ó���Ǿ� �ʱ�ȭ �Ǵ� �κ�.

		avaGame(ä��) -> avaSWGame(����) -> avaGame(ä��)
*/
event PostLogin( PlayerController NewPlayer )
{
	Local avaGameViewportClient ViewportClient;
	
	Super.PostLogin( NewPlayer );
	`log( "avaNetEntryGame.PostLogin" @class'avaNetHandler'.static.GetAvaNetHandler().CheckMyLocation() );

	// UIController �ʱ�ȭ
	UIController = LocalPlayer(NewPlayer.Player).ViewportClient.UIController;
	if (UIController == NONE)
	{
		`warn( "Failed to set UIController" );
		return;
	}

	// ȣ��Ʈ���̱׷��̼� �÷��� �ʱ�ȭ ( �÷��״� �ε�ȭ�鿡 ������ 'ȣ��Ʈ�� �ű�� ���Դϴ�'�� ������� ���� ����)
	ViewportClient = avaGameViewportClient(LocalPlayer(NewPlayer.Player).ViewportClient);
	if( ViewportClient != None )
		ViewportClient.SetHostMigration(false);

	// GlobalScene �ʱ�ȭ , GlobalScene�� �����ִ� Scene�� ������� Event�� ���� �� �ִ� Scene�� 
	// ���� ��� ģ���߰� ��û�� ���� ���¿� ������� ó��������ϴ� �޼����� ó��.
	GlobalScene = LoadScene( GlobalSceneName );
	if( GlobalScene == None )
		`warn("Can't open global scene, some popup in general purpose is going to be disabled");
	else
		UIController.OpenScene(GlobalScene);

	if ( IsInState('MatchPending') )
	{
		class'avaNetHandler'.static.GetAvaNetHandler().InitWeaponIDList();
		InitScenes();

		// ���� ��ġ�� ���� 'ī�޶� ��ġ'�� 'UI���'�� �����Ѵ�.
		switch ( class'avaNetHandler'.static.GetAvaNetHandler().CheckMyLocation() )
		{
		case 0:
			LoadScene(TitleSceneName);
			
			GotoScene("Title");
			//GotoScene("ChannelList");
			break;

		case 1:	// lobby
			// NetState�� �̹� avaMsgProc���� LOBBY�� �������ֹǷ� Ư���� ó���� ���� ����.
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

//! UI���� ���������� �ٿ�ε� �޴´�.
function PatchNotice()
{
	if ( NoticePatcher != None)
		return ;

//	NoticePatcher = new(self) class'avaUINoticePatcher';

//	NoticePatcher = spawn(class'avaUINoticePatcher', self);
//	NoticePatcher.Download();
}

//! ������ ���� �̹������� �ٿ���� �� ���ŵ� ��� ȣ��Ǵ� �Լ�.
function OnDownloaded()
{
//	`log("avaNetEntryGameEx.OnDownloaded()");

	// UIScene�� ���� �̺�Ʈ�� Ȱ���� �����ش�.
	ActivateUIRemoteEvent('RefreshNoticeEvent');
}

//! ����ϰ� ���̴� ĳ���� ������ �̸� �ε��Ѵ�.
function PrecacheCharacters()
{
	local avaUICharacterCache	CharCache;
	local Vector				loc;
	local Rotator				rot;

	// precache�ȵǰ�...
	return ;

	`log("### PrecacheCharacters() - Count:" @class'avaCache'.static.GetCount());

	// ��ϵ� ������Ʈ�� �ϳ��� ���� ���.
	if ( class'avaCache'.static.GetCount() == 0 )
	{
		// ���� �浹���� �ʰ�...?
		loc.x = 0;
		loc.y = 0;
		loc.z = 20;

		// 1ȸ �����ؼ� ����Ѵ�.
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

	// Popup �� ���ִ°� Check �Ѵ�...
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

	// Military Drill (������)
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

	// �̸� �ε��� �ʿ��ϴٸ� ���� �� ����.
	if ( bPreload )
	{
		if ( NextSceneName != "" )
		{
			// �̸� ����.
			PreloadedSceneName = NextSceneName;

			PreloadedScene = LoadScene( PreloadedSceneName );
			if ( PreloadedScene == None )
				`log("Cannot preload UIScene." @PreloadedSceneName);
			else
				`log("Preloaded UIScene:" @PreloadedSceneName);
		}

		return ;
	}

	// �ٷ� �ε��ؾ� �ϰų�, �̺�Ʈ Ŭ������ ���� ��쿡��.
	if ( bDirect == true || NextUIEventClass == None )
	{
		if ( NextSceneName != "" )
		{
			// �̹� �ε��� ����� �ִٸ� ���´�.
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
		// �̺�Ʈ Ŭ������ �ִ� ��쿡�� �ش� �̺�Ʈ���� Ȱ��ȭ �����ش�.
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
				// �̺�Ʈ Ŭ������ ���� ����(�ڱ��ڽ� or ��ӵ� Ŭ����)�� �ƹ��͵� ã�� ���� ���.
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
	// EU/NRF ����Ʈ�� �������� �÷��̾� ������ �����Ѵ�.
	ActivateSingleEvent(class'avaUIEvent_OnReadyRoomPlayer');
}

function UpdateRoomSelectedPlayer()
{
	// EU/NRF ����Ʈ���� ���õ� �÷��̾��� ������ �����Ѵ�.
	ActivateSingleEvent(class'avaUIEvent_OnReadyRoomSelectedPlayer');
}

function UpdateRoomButtons()
{
	// ��ư���� Visible�� Enable���¸� �缳�� ���ش�.
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

// @deprecated	avaNetHandler���� ��� ó��
//function RoomChangeSetting()
//{
//	// �������� ���� ������ �����Ѵ�.
//	ActivateSingleEvent(class'avaUIEvent_OnReadyRoomSetting');
//
//	// ���� ��ư�� Visible������ ���� �Ǿ�� �Ѵ�.
//	UpdateRoomButtons();
//}

function RoomChangeState()
{
	UpdateRoomPlayerList();
	// Playing������ Host�� ������ �����ϰ� Lobby�� ���ƿ� ���
	// '����'�� '�غ�'�� ���� ����� �Ѵ�.
	UpdateRoomButtons();
}

/*
	@question
		�ڽŰ� �ڽ� �̿��� ��� ���� ���ؼ� ����ǳ�?
		���� �ڽ��� ������� ��쿡�� RoomLeave�� �ڵ����� ���� �� ������...
*/
function RoomKick(int idPlayer)
{
	UpdateRoomPlayerList();

	// ������ϰ� ����Ʈ�� ���ŵǸ� Client�� ������ ������ �� �ִ�.
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
//	// ���̸��� �ٲ�����Ƿ� �漳���� �������ش�.
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

/*! @brief ���� ������ ���� �޽��� �ڵ鷯.
	@param bStart
		Host, Ready������ Client, Spectator�� ��� �����̸�,
		Ready���°� �ƴ� Client�� ���� ��ư�� �����ϰ� �ȴ�.
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

/*! @brief ī��Ʈ�ٿ�.
	@param bStart
		true	ī��Ʈ�ٿ��� �����ϰ� �Ѵ�.
		false	ī��Ʈ�ٿ��� ����ϰ� ���߰� �Ѵ�.
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
	// AutoTeamBalance�� ������ �濡��
	// Client�� ���۹�ư�� ������ EU���� NRF�� �� �Ŀ� '�������'�� ������
	// Ready���·� �״�� ���� ���� �����ϱ� ���ؼ� ȣ��.(2007/02/26)
	UpdateRoomPlayerList();
}

function Timer_CountDown()
{
	// Ȥ�� ���� ����â�� ����� �ִٸ� �ݾ��ش�.
	// ���� ����â�� Ȱ��ȭ ���̸� ������ ��Ȱ��ȭ �Ǽ� �� �ʿ䰡 ����.
//	ActivateUIRemoteEvent('CloseWeaponInventoryScene');

	// ī��Ʈ�ٿ��� ������ ������ true�� �����Ѵ�.
	if ( class'avaNetHandler'.static.GetAvaNetHandler().ProcCountDown() )
	{
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGameCountDown',,,true);
		Chat("", "");
	}
	else
	{
		// ī��Ʈ�ٿ��� ������ ���� ���� Ÿ�̸� �Լ��� ȣ����� �ʵ��� �Ѵ�.
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGameCountDown',,,false);
		ClearTimer('Timer_CountDown');
	}
}

/** �� ���� (ä�ηκ�, �����, �κ��丮) ���� ��û(REQ)�� ���� ����(ACK/NAK)�� ���ƿ����� �Ҹ���*/
function ResponseChannelLobbyMessage()
{
	ActivateSingleEvent(class'avaUIEvent_ResponseChannelLobby');
}

function ResponseRoomMessage()
{
	// ��� ��ư�� ���¸� Enable�� ����� �ش�.
	ActivateSingleEvent(class'avaUIEvent_ResponseReadyRoom');
}

function ResponceInventoryMessage()
{
	ActivateSingleEvent(class'avaUIEvent_ResponseInventory');
}


/*! @brief Level Kismet�� RemoteEvent�� �߻����� �ش�.
	@param EventName
		RemoteEvent�߿� �߻����Ѿ� �� �̺�Ʈ �̸�.
	@note
		avaNetHandler.uc���� ��ġ�� �ű�.
*/
function ActivateRemoteEvent( name EventName )
{
	local Sequence					GameSeq;
	local int						i;
	local array<SequenceObject>		AllSeqEvents;
	local SeqEvent_RemoteEvent		RemoteEvent;

	// World���� Level Kismet�� Sequence�� ��´�.
	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != None )
	{
		// RemoteEvent�� ��� ã�Ƽ�.
		GameSeq.FindSeqObjectsByClass( class'SeqEvent_RemoteEvent', true, AllSeqEvents );

		for ( i = 0; i < AllSeqEvents.Length; ++ i )
		{
			RemoteEvent = SeqEvent_RemoteEvent( AllSeqEvents[i] );

			if ( RemoteEvent == None )
				continue;

			if ( RemoteEvent.bEnabled != true )
				continue;

			// �߻��ϰ� ���� �̺�Ʈ �̸��� RemoteEvent�� �߻���Ų��.
			if( RemoteEvent.EventName == EventName )
			{
				`log("### RemoteEvent - " @EventName @"###");
				RemoteEvent.CheckActivate( WorldInfo, None );
			}
		}
	}

}

/*! @brief UI Kismet�� RemoteEvent�� �߻����� �ش�.
	@param EventName
		RemoteEvent�߿� �߻����Ѿ� �� �̺�Ʈ �̸�.
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

	// ��� Ȱ��ȭ�� ��鿡 ���ؼ� ã�´�.
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

			// ���� �̸��� UI RemoteEvent�� Ȱ��ȭ �����ش�.
			if ( Event.EventName == EventName )
			{
				Event.ActivateUIEvent(0, ActiveScene);
				`log("### Event.ActivateUIEvent(" @EventName @")  ###");

				numEvents++;
			}
		}
	}

	// ���� �ش��ϴ� �̺�Ư�� �ϳ��� ���� ��쿡 ���ؼ� �α׸� �����ش�.
	if ( numEvents == 0 )
		`log("no match an event -" @EventName);
}

/*! @brief �α��� �� �������� ���� �÷��̾� ������ (avaPlayerModifierInfo��) �����Ѵ�.
	@note
		3D World�� ĳ������ ��� ���� ������ �ش�.
*/
function UpdatePlayerInfo()
{
	local PlayerController			PC;
	local avaPlayerController		LocalPC;
	local avaPlayerReplicationInfo	avaPRI;
	local int						i;

	// ���� �������� 1���� ���ؼ� ������ �ش�.
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

	// Level Kismet�� RemoteEvent�� ������!!
	ActivateRemoteEvent('UpdateChannelCharacter');


	// ���� �������� 1���� ���ؼ� ������ �ش�.
	PrecacheCharacters();
}

//! ĳ���� UIScene���� ��� ó���� ���������� �Ǿ����� ȣ��ȴ�.
function UpdateCreateCharacter()
{
	ActivateSingleEvent(class'avaUIEvent_OnCreateCharacter');
}

/*! @brief �������� ���⸦ ������ ��� ȣ��Ǵ� �Լ�.
*/
function InventoryWeaponSet( optional bool bCustom = false )
{
	local PlayerController			PC;
	local avaPlayerController		LocalPC;
	local avaPlayerReplicationInfo	avaPRI;

	ActivateSingleEvent(class'avaUIEvent_OnInventoryWeaponSet');

	// ���⸦ ���� �����ߴٸ� StateController���� ���� ������ 
	// LocalPlayerController�� PRI�� �ٽ� �����Ѵ�.
	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		LocalPC = avaPlayerController(PC);
		break;
	}

	avaPRI = avaPlayerReplicationInfo( LocalPC.PlayerReplicationInfo );

	avaPRI.WeaponPointMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("WeaponP");
	avaPRI.WeaponRifleMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("WeaponR");
	avaPRI.WeaponSniperMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("WeaponS");

	// �׽�Ʈ.
	`log("WeaponP=" @ avaPRI.WeaponPointMan);
	`log("WeaponR=" @ avaPRI.WeaponRifleMan);
	`log("WeaponS=" @ avaPRI.WeaponSniperMan);

	avaPRI.ResetWeaponModifiers();

	// �� �������� ���������� �����Ѵ�.
	avaPRI.FetchWeaponModifier( PCT_PointMan );
	avaPRI.FetchWeaponModifier( PCT_RifleMan );
	avaPRI.FetchWeaponModifier( PCT_Sniper );

	if ( bCustom )
	{
		// Ŀ���� â������ ������ �ƴ� ���õ� ���⿡ ���ؼ� ������� �Ѵ�.
		ActivateSingleEvent(class'avaUIEvent_OnInventoryCustomWeaponSet');
	}
	else
	{
		// �������� ���� ������ ���⸦ �����ϴ� �̺�Ʈ Ȱ��ȭ (Level Kismet).
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

/*! @brief �������� ĳ���� ��� ������ ��� ȣ��Ǵ� �Լ�.
*/
function InventoryEquipSet()
{
	local PlayerController			PC;
	local avaPlayerController		LocalPC;
	local avaPlayerReplicationInfo	avaPRI;

	ActivateSingleEvent(class'avaUIEvent_OnInventoryEquipSet');

	// ��� ���� �����ߴٸ� StateController���� ���� ������ 
	// LocalPlayerController�� PRI�� �ٽ� �����Ѵ�.
	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		LocalPC = avaPlayerController(PC);
		break;
	}

	avaPRI = avaPlayerReplicationInfo( LocalPC.PlayerReplicationInfo );

	avaPRI.CharacterItem = class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("ChItem");

	// �׽�Ʈ.
	`log("ChItem=" @ avaPRI.CharacterItem);

	// ���������� ��� ĳ���� Modifiers������ �ʱ�ȭ.
	avaPRI.ResetCharacterModifiers();

	// �� �������� ��������� �����Ѵ�.
	avaPRI.FetchCharacterModifier( );
	avaPRI.FetchCharacterModifier( PCT_PointMan );
	avaPRI.FetchCharacterModifier( PCT_RifleMan );
	avaPRI.FetchCharacterModifier( PCT_Sniper );

	// �������� ���� ������ ���⸦ �����ϴ� �̺�Ʈ Ȱ��ȭ (Level Kismet).
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

	//������ Active Scene�� ����
	for( i = 0 ; i < UIControl.SceneClient.ActiveScenes.Length ; i++ )
	{
		if( UIControl.SceneClient.ActiveScenes[i] != None )
		{
			// �ڽ�(Scene)���� ���� Event�� ã��
			UIControl.SceneClient.ActiveScenes[i].FindEventsOfClass(EventClassToFind, out_FoundEvents);
			// �ڽĵ� ��������� ã��
			Child = UIControl.SceneClient.ActiveScenes[i].GetChildren(TRUE);
			// ã�� �ڽĵ鿡 ���� EventClass�� ã�´�.
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

//  PopUpMessage�� Kismet���� �ѱ��� �ʰ� ������ ��ũ��Ʈ �˾��� ���
	PopUpMsgSceneName = "avaUISceneEx_Misc.PopUpWindow"
	WidgetMessageLabel = "UILabel_PopUpMessage"

	GlobalSceneName			=	"avaUISceneEx_Global.GlobalScene"

	DefaultUIPathPrefix		=	"avaUISceneEx"
}
