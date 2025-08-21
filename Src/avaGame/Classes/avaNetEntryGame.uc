/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaGame

	Name: avaNetEntryGame.uc

	Description: Game class for interacting with AVA Servers via avaNet.

***/
class avaNetEntryGame extends avaGame;


`include(avaGame/avaGame.uci)

`define AVANET

var UIInteraction UIController;
var string	LogoSceneName, 
			RedduckLogoSceneName,
			AvaLogoSceneName,
			TitleSceneName, 
			ChannelListSceneName, 
			LobbySceneName,
			ClanLobbySceneName,
			RoomSceneName, 
			InventorySceneName, 
			ShopSceneName, 
			NewCharacterSceneName, 
			PopUpMsgSceneName, 
			LoadingSceneName, 
			ExitSceneName,
			ResultSceneName,
			ResultMDSceneName;	// ResultSceneMD(MilitaryDrill)

var UIScene LogoScene, TitleScene, ChannelListScene, LobbyScene, RoomScene, InventoryScene, ShopScene, NewCharacterScene, PopUpMsgScene, LoadingScene;
var UIScene CurrentScene;//, NextScene;
//var EScenes /*CurrentScene, */NextScene;
var bool bPopUpMsgOpen;
var string	NextScene;
var name	NextUIEventName;

var name WidgetMessageLabel, WidgetChannelListGeneric, WidgetLobbyChatMsg, WidgetLobbyPlayerList, WidgetLobbyRoomList;
var name WidgetLobbyRoomName, WidgetLobbyRoomMapPreview, WidgetLobbyRoomSettings;
var name WidgetLobbyRoomMapDesc, WidgetLobbyRoomWinCondition, WidgetLobbyRoomPlayers, WidgetLobbyRoomCurrentRound;
var name WidgetLobbyRoomHost, WidgetLobbyRoomPlayersEU, WidgetLobbyRoomPlayersNRF;
var name WidgetRoomStartButton, WidgetRoomReadyButton, WidgetRoomSettingButton, WidgetRoomSpectateButton, WidgetRoomCharPIP/*, WidgetRoomWeaponSet, WidgetRoomPlayerWeaponName*/;
var name WidgetRoomMapName, WidgetRoomSettings, WidgetRoomWinCondition;
var name WidgetRoomMapPreview, WidgetRoomMaxPlayers, WidgetRoomMapDesc, WidgetRoomChatMsg, WidgetRoomPlayersEU,WidgetRoomPlayersNRF/*, WidgetRoomPlayersSpectator*/;
var name WidgetRoomSpectator1, WidgetRoomSpectator2, WidgetRoomSpectator3, WidgetRoomSpectator4;
var name WidgetRoomPlayerLevel, WidgetRoomPlayerName, WidgetRoomPlayerGuildName, WidgetRoomPlayerWinCount, WidgetRoomPlayerKDRatio, WidgetRoomPlayerDisconnect;
var name WidgetRoomPlayerClassP, WidgetRoomPlayerClassR, WidgetRoomPlayerClassS/*, WidgetRoomPointmanWeapon, WidgetRoomRiflemanWeapon, WidgetRoomSniperWeapon*/;
var name WidgetRoomWeaponName[6], WidgetRoomWeaponIcon[6];
var name WidgetInventoryEquipName, WidgetInventoryEquipIcon, WidgetInventoryListEquip, WidgetInventoryListWeapon;
var name WidgetShopWeaponInven, WidgetShopEquipInven, WidgetShopLabelMoney;

//var string ChannelTItle;
//var string RoomTitle;


`if( `isdefined(AVANET) )

event PostLogin( PlayerController NewPlayer )
{
	//super.PostLogin(NewPlayer);

	`log( "avaNetEntryGame.PostLogin()" );	
	NewPlayer.WorldInfo.SetNetEntryGameClass( Self );

}

function bool CheckEndGame(PlayerReplicationInfo Winner, string Reason)
{
	return false;
}

// utilities

function InitScenes()
{
	if ( UIController != NONE )
	{
		`log( "Initializing scenes." );

		LogoScene = None;
		TitleScene = None;
		ChannelListScene = None;
		LobbyScene = None;
		RoomScene = None;
		NewCharacterScene = None;
		PopUpMsgScene = None;
		LoadingScene = None;

		CurrentScene = None;
		//NextScene = "None";
		//CurrentScene = EScene_None;
		NextScene = "None";

		bPopUpMsgOpen = false;
	}
	else
	{
		`warn( "No controller to create scenes." );
	}
}

`endif





exec function GotoScene( string InScene, optional bool bDirect, optional name InNextUIEventName, optional bool bPreload )
{
	//if (NextScene != EScene_None || CurrentScene == InScene)
	//	return;

	//NextScene = InScene;

	switch (InScene)
	{
	case "Logo":
		GotoState('Logo');
		break;
	case "Title":
		GotoState('Title');
		break;
	case "ChannelList":
		GotoState('ChannelList');
		break;
	case "Lobby":
		GotoState('Lobby');
		break;
	case "Room":
		GotoState('Room');
		break;
	case "Inventory":
		GotoState('Inventory');
		break;
	case "Shop":
		GotoState('Shop');
		break;
	case "NewCharacter":
		GotoState('NewCharacter');
		break;
	default:
		//NextScene = "None";
		break;
	}
}

function GotoNextScene()
{
	GotoScene(NextScene,,NextUIEventName);
	NextScene = "None";
	NextUIEventName = '';
}

function PopUpMessage(string Msg, string InNextScene, optional name InNextUIEventName )
{
	local UILabel MsgText;
	local UIScene LoadedScene;

	`log( "PopUpMessage" @Msg @InNextScene );

	if (bPopUpMsgOpen)
		return;

	LoadedScene = LoadScene( PopUpMsgSceneName );
	if ( LoadedScene == None || UIController == None)
	{
		`warn( "avaNetEntryGame: Cannot find the scene.!" $ LoadedScene $ UIController );
		return;
	}

	UIController.SceneClient.OpenScene(LoadedScene, , PopUpMsgScene);

	MsgText = UILabel(PopUpMsgScene.FindChild(WidgetMessageLabel, true));
	if (MsgText == None)
	{
		`warn( "Cannot find message control." );
		return;
	}

	//MsgText.SetValue(Msg);
	MsgText.SetDataStoreBinding(Msg);
	MsgText.RefreshSubscriberValue();

	//PopUpMsgScene.OnSceneDeactivated = OnPopUpMsgSceneDeactivated;

	bPopUpMsgOpen = true;

	NextScene		=	InNextScene;
	NextUIEventName =	InNextUIEventName;

}

function UpdatePopUpMessage(string Msg, string InNextScene)
{
	local UILabel MsgText;
	//local UIScene LoadedScene;

	if (!bPopUpMsgOpen)
		return;

	MsgText = UILabel(PopUpMsgScene.FindChild(WidgetMessageLabel, true));
	if (MsgText == None)
	{
		`warn( "Cannot find message control." );
		return;
	}

	//MsgText.SetValue(Msg);
	MsgText.SetDataStoreBinding(Msg);
	MsgText.RefreshSubscriberValue();

	if (InNextScene != "None" && InNextScene != "")
		NextScene = InNextScene;
}

function ClosePopUpMessage()
{
	if (!bPopUpMsgOpen)
		return;

	if (PopUpMsgScene == None)
	{
		bPopUpMsgOpen = false;
		return;
	}

	`log( "avaNetEntryGame.ClosePopUpMessage" );
	UIController.CloseScene(PopUpMsgScene);

	PopUpMsgScene = None;
	bPopUpMsgOpen = false;
	GotoNextScene();
}

// state-specific update functions

function UpdateChannelList()
{
}

function Chat(string Nickname, string Msg)
{
}

/*! @brief EMsg_Client_PlayerInfo메시지에 대한 처리 함수.
	@note
		avaNetEntryGameEx.uc에서 사용하기 위한 함수 선언.
*/
function UpdatePlayerInfo()
{
}

function UpdateCreateCharacter()
{
}

/** Lobby Messages*/
function UpdateLobbyPlayerList()
{
}

function AddLobbyPlayerList(int idAccount)
{
}

function RemoveLobbyPlayerList(int idAccount)
{
}

function UpdateLobbyRoomList()
{
}

function UpdateLobbySelectedRoom()
{
}

function UpdateLobbySelectedPlayer()
{
}

// 채널과 로비에서 요청(REQ)에 대한 응답(ACK/NAK)이 왔을때 불린다.
function ResponseChannelLobbyMessage()
{
}

/** ReadyRoom Messages */
function AddRoomList(int idRoom)
{
}

function RemoveRoomList(int idRoom)
{
}

function UpdateRoomState(int idRoom)
{
}

function UpdateRoomSetting(int idRoom)
{
}

//function UpdateRoomTitle(string Text)
//{
//	RoomTitle = Text;
//}

function UpdateRoomPlayerList()
{
}

function UpdateRoomSelectedPlayer()
{
}

function AddRoomPlayerList(int idPlayer)
{
}

function RemoveRoomPlayerList(int idPlayer)
{
}

function RoomChangeHost(int idPlayer)
{
}

function RoomChangeSetting()
{
}

function RoomChangeState()
{
}

function RoomKick(int idPlayer)
{
}

function RoomReady(int idPlayer, int Ready)
{
}

function RoomChangeClass(int idPlayer)
{
}

function RoomChangeWeapon(int idPlayer)
{
}

function RoomChangeTeam(int idPlayer, int idTeam)
{
}

function RoomNameChanged()
{
}

function UpdateRoomResult(string param)
{
}

function GameStarted(int bStart)
{
}

function GameCountDown(bool bStart)
{
}

function ResponseRoomMessage()
{
}

/** Inventory / Room Messages*/
function InventoryWeaponSet( optional bool bCustom = false )
{
}

function InventoryEquipSet()
{
}

function ShopBuy(optional bool bSuccess = true)
{
}

function ConvertRIS( optional bool bSuccess = true )
{

}

function RepairWeapon( optional bool bSuccess = true )
{
}

function AddTypedMessage( Name MsgType , string Msg )
{
}

function UpdateRealTimeNotice()
{
	
}

function Timer_CountDown()
{
	if ( class'avaNetHandler'.static.GetAvaNetHandler().ProcCountDown() )
	{
		Chat("", "");
	}
	else
	{
		ClearTimer('Timer_CountDown');
	}
}

function ResponseInventoryMessage()
{
}


auto State MatchPending
{

    function BeginState(Name PreviousStateName)
    {
		bWaitingToStartMatch = false;
		avaGameReplicationInfo(GameReplicationInfo).bWarmupRound = false;
    }

	function EndState(Name NextStateName)
	{
		avaGameReplicationInfo(GameReplicationInfo).bWarmupRound = false;
	}

}

simulated function UIScene LoadScene( string SceneName )
{
	local UIScene ret;
	
	`Log("LoadScene("$SceneName$")");

	ret = UIScene(DynamicLoadObject( SceneName, class'UIScene' ));

	if (ret == None)
	{
		`warn( "avaNetEntryGame: No such UIScene'" @ SceneName@ "'" );
	}

	return ret;
}

`if( `isdefined(AVANET) )


State Logo
{
	function BeginState(Name PreviousStateName)
	{
		local UIScene LoadedScene;
		LoadedScene = LoadScene( LogoSceneName );
		if ( LoadedScene != None )
		{
			UIController.OpenScene(LoadedScene, , LogoScene);
			CurrentScene = LogoScene;
		}
		else
		{
			`warn( "avaNetEntryGame: No LogoScene to open" );
		}
		NextScene = "None";
	}

	function EndState(Name NextStateName)
	{
		if ( LogoScene != None )
		{
			UIController.CloseScene(LogoScene);
			LogoScene = None;
		}
		CurrentScene = None;
	}
}

State Title
{
	function BeginState(Name PreviousStateName)
	{
		local UIScene LoadedScene;
		LoadedScene = LoadScene( TitleSceneName );
		if ( LoadedScene != None )
		{
			UIController.OpenScene(LoadedScene, , TitleScene);
			CurrentScene = TitleScene;
		}
		else
		{
			`warn( "avaNetEntryGame: No TitleScene to open" );
		}
		NextScene = "None";
	}

	function EndState(Name NextStateName)
	{
		if ( TitleScene != None )
		{
			UIController.CloseScene(TitleScene);
			TitleScene = None;
		}
		CurrentScene = None;
	}
}

State ChannelList
{
	function BeginState(Name PreviousStateName)
	{
		local UIScene LoadedScene;
		LoadedScene = LoadScene( ChannelListSceneName );
		if ( LoadedScene != None )
		{			
			UIController.OpenScene(LoadedScene, , ChannelListScene);
			CurrentScene = ChannelListScene;

			UpdateChannelList();
		}
		else
		{
			`warn( "avaNetEntryGame: No ChannelListScene to open" );
		}
		NextScene = "None";
	}

	function EndState(Name NextStateName)
	{
		if ( ChannelListScene != None )
		{
			UIController.CloseScene(ChannelListScene);
			ChannelListScene = None;
		}
		CurrentScene = None;
	}

	function UpdateChannelList()
	{
		local UIList ChannelListCtrl;
		local int ListIndex;

		`log( "avaNetEntryGame: Updating channel list." );

		if (ChannelListScene == None)
		{
			`warn( "avaNetEntryGame: Cannot find the scene." );
			return;
		}

		ChannelListCtrl = UIList(ChannelListScene.FindChild(WidgetChannelListGeneric, true));
		if (ChannelListCtrl == None)
		{
			`warn("avaNetEntryGame: Cannot find channel list widget.");
			return;
		}

		ListIndex = ChannelListCtrl.Index;
		ChannelListCtrl.RefreshSubscriberValue();
		ChannelListCtrl.SetIndex(ListIndex);
	}
}

State Lobby
{
	function BeginState(Name PreviousStateName)
	{
		local UIScene LoadedScene;
		LoadedScene = LoadScene( LobbySceneName );
		if ( LoadedScene != None )
		{			
			UIController.OpenScene(LoadedScene, , LobbyScene);
			CurrentScene = LobbyScene;

			//UpdateChannelTitle(ChannelTitle);
			UpdateLobbyPlayerList();
			UpdateLobbyRoomList();
			UpdateLobbySelectedRoom();
			UpdateLobbySelectedPlayer();
		}
		else
		{
			`warn(self @ "No LobbyScene to open" );
		}
		NextScene = "None";
	}

	function EndState(Name NextStateName)
	{
		if ( LobbyScene != None )
		{
			UIController.CloseScene(LobbyScene);
			LobbyScene = None;
		}
		CurrentScene = None;

		if (NextStateName != 'Room')
		{
			//ChannelTitle = "";
		}
	}

	function Chat(string Nickname, string Msg)
	{
		local UIList ChatMsg;

		if (LobbyScene == None)
		{
			`warn( "avaNetEntryGame: Cannot find the scene." );
			return;
		}

		ChatMsg = UIList(LobbyScene.FindChild(WidgetLobbyChatMsg, true));
		if (ChatMsg == None)
		{
			`warn("avaNetEntryGame: Cannot find chatting message list widget.");
			return;
		}

		ChatMsg.RefreshSubscriberValue();
		if (ChatMsg.Items.Length > ChatMsg.MaxVisibleItems)
		{
			ChatMsg.SetTopIndex(ChatMsg.Items.Length - ChatMsg.MaxVisibleItems + 1);
		}
	}

	function UpdateLobbyPlayerList()
	{
		local UIList PlayerList;
		local int ListIndex;

		`log( "avaNetEntryGame: Updating lobby player list." );

		if (LobbyScene == None)
		{
			`warn( "avaNetEntryGame: Cannot find the scene." );
			return;
		}

		PlayerList = UIList(LobbyScene.FindChild(WidgetLobbyPlayerList, true));
		if (PlayerList == None)
		{
			`warn("avaNetEntryGame: Cannot find lobby player list widget.");
			return;
		}

		ListIndex = PlayerList.Index;
		PlayerList.RefreshSubscriberValue();
		PlayerList.SetIndex(ListIndex);
	}

	function AddLobbyPlayerList(int idAccount)
	{
		UpdateLobbyPlayerList();
	}

	function RemoveLobbyPlayerList(int idAccount)
	{
		UpdateLobbyPlayerList();
	}

	function UpdateLobbyRoomList()
	{
		local UIList RoomList;
		local int ListIndex;

		`log( "avaNetEntryGame: Updating lobby room list." );

		if (LobbyScene == None)
		{
			`warn( "avaNetEntryGame: Cannot find the scene." );
			return;
		}

		RoomList = UIList(LobbyScene.FindChild(WidgetLobbyRoomList, true));
		if (RoomList == None)
		{
			`warn("avaNetEntryGame: Cannot find lobby room list widget.");
			return;
		}

		ListIndex = RoomList.Index;
		RoomList.RefreshSubscriberValue();
		RoomList.SetIndex(ListIndex);
	}

	function UpdateLobbySelectedRoom()
	{
		local UILabel LabelInfo;
		local UIList ListPlayers;

		LabelInfo = UILabel(LobbyScene.FindChild(WidgetLobbyRoomName, true));
		if (LabelInfo != None)
			LabelInfo.RefreshSubscriberValue();
		else
			`warn("avaNetEntryGame: Cannot find lobby RoomName widget.");

		LabelInfo = UILabel(LobbyScene.FindChild(WidgetLobbyRoomMapPreview, true));
		if (LabelInfo != None)
			LabelInfo.RefreshSubscriberValue();
		else
			`warn("avaNetEntryGame: Cannot find lobby RoomMapPreview widget.");

		LabelInfo = UILabel(LobbyScene.FindChild(WidgetLobbyRoomSettings, true));
		if (LabelInfo != None)
			LabelInfo.RefreshSubscriberValue();
		else
			`warn("avaNetEntryGame: Cannot find lobby RoomSettings widget.");

		LabelInfo = UILabel(LobbyScene.FindChild(WidgetLobbyRoomMapDesc, true));
		if (LabelInfo != None)
			LabelInfo.RefreshSubscriberValue();
		else
			`warn("avaNetEntryGame: Cannot find lobby RoomMapDesc widget.");

		LabelInfo = UILabel(LobbyScene.FindChild(WidgetLobbyRoomWinCondition, true));
		if (LabelInfo != None)
			LabelInfo.RefreshSubscriberValue();
		else
			`warn("avaNetEntryGame: Cannot find lobby RoomWinCondition widget.");

		LabelInfo = UILabel(LobbyScene.FindChild(WidgetLobbyRoomPlayers, true));
		if (LabelInfo != None)
			LabelInfo.RefreshSubscriberValue();
		else
			`warn("avaNetEntryGame: Cannot find lobby RoomPlayers widget.");

		LabelInfo = UILabel(LobbyScene.FindChild(WidgetLobbyRoomCurrentRound, true));
		if (LabelInfo != None)
			LabelInfo.RefreshSubscriberValue();
		else
			`warn("avaNetEntryGame: Cannot find lobby RoomCurrentRound widget.");

		LabelInfo = UILabel(LobbyScene.FindChild(WidgetLobbyRoomHost, true));
		if (LabelInfo != None)
			LabelInfo.RefreshSubscriberValue();
		else
			`warn("avaNetEntryGame: Cannot find lobby RoomHost widget.");

		ListPlayers = UIList(LobbyScene.FindChild(WidgetLobbyRoomPlayersEU, true));
		if (ListPlayers != None)
			ListPlayers.RefreshSubscriberValue();
		else
			`warn("avaNetEntryGame: Cannot find lobby RoomPlayersEU widget.");

		ListPlayers = UIList(LobbyScene.FindChild(WidgetLobbyRoomPlayersNRF, true));
		if (ListPlayers != None)
			ListPlayers.RefreshSubscriberValue();
		else
			`warn("avaNetEntryGame: Cannot find lobby RoomPlayersNRF widget.");
	}

	function UpdateLobbySelectedPlayer()
	{
	}

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

}

State Room
{
	function BeginState(Name PreviousStateName)
	{
		local UIScene LoadedScene;
		LoadedScene = LoadScene( RoomSceneName );
		if ( LoadedScene != None )
		{			
			UIController.OpenScene(LoadedScene, , RoomScene);
			CurrentScene = RoomScene;

			//UpdateRoomTitle(RoomTitle);
			UpdateRoomPlayerList();
			UpdateRoomSelectedPlayer();
			UpdateRoomButtons();
			UpdateCharacterPIP(true, true, true);
			UpdateEquipWeapon();
			UpdateRoomSettings();
		}
		else
		{
			`warn( "avaNetEntryGame: No RoomScene to open" );
		}
		NextScene = "None";
	}

	function EndState(Name NextStateName)
	{
		if ( RoomScene != None )
		{
			UIController.CloseScene(RoomScene);
			RoomScene = None;
		}
		CurrentScene = None;

		//if (NextStateName != ...)
		//RoomTitle = "";
	}

	function UpdateRoomButtons()
	{
		local UIObject ButtonStart, ButtonReady, ButtonSetting, ButtonSpectator, RadioPointman, RadioRifleman, RadioSniper;
		//local UIObject ButtonWeaponP, ButtonWeaponR, ButtonWeaponS;
		local int MyTeam, MyClass, MyFace, MyWeapon;

		ButtonStart = RoomScene.FindChild(WidgetRoomStartButton, true);
		ButtonReady = RoomScene.FindChild(WidgetRoomReadyButton, true);
		ButtonSetting = RoomScene.FindChild(WidgetRoomSettingButton, true);
		if (ButtonStart != None && ButtonReady != None && ButtonSetting != None)
		{
			if ( class'avaNetHandler'.static.GetAvaNetHandler().AmIHost() )
			{
				ButtonReady.SetVisibility(false);
				ButtonStart.SetVisibility(true);
				ButtonSetting.EnableWidget(0);

				if ( class'avaNetHandler'.static.GetAvaNetHandler().IsGameStartable() )
				{
					if (ButtonStart.GetCurrentState() == class'UIState_Disabled')
						ButtonStart.EnableWidget(0);
				}
				else
				{
					ButtonStart.DisableWidget(0);
				}
			}
			else
			{
				if ( class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentRoomState() == 2 )
				{
					ButtonReady.SetVisibility(false);
					ButtonStart.SetVisibility(true);
					if (ButtonStart.GetCurrentState() == class'UIState_Disabled')
						ButtonStart.EnableWidget(0);
				}
				else
				{
					ButtonStart.SetVisibility(false);
					ButtonReady.SetVisibility(true);
				}

				ButtonSetting.DisableWidget(0);
			}
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room button widget." );
		}

		ButtonSpectator = RoomScene.FindChild(WidgetRoomSpectateButton, true);
		if (ButtonSpectator != None)
		{
			if ( class'avaNetHandler'.static.GetAvaNetHandler().IsSpectatorAllowed() )
			{
				ButtonSpectator.EnableWidget(0);
			}
			else
			{
				ButtonSpectator.DisableWidget(0);
			}
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room spectate button." );
		}

		class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentEquipState(MyTeam, MyClass, MyFace, MyWeapon);

		RadioPointman = RoomScene.FindChild(WidgetRoomPlayerClassP, true);
		RadioRifleman = RoomScene.FindChild(WidgetRoomPlayerClassR, true);
		RadioSniper = RoomScene.FindChild(WidgetRoomPlayerClassS, true);
		if (RadioPointman != None && RadioRifleman != None && RadioSniper != None)
		{
			if (class'avaNetHandler'.static.GetAvaNetHandler().AmISpectator())
			{
				RadioPointman.DisableWidget(0);
				RadioRifleman.DisableWidget(0);
				RadioSniper.DisableWidget(0);
			}
			else
			{
				switch (MyClass)
				{
				case 0:
					RadioPointman.EnableWidget(0);
					RadioRifleman.DisableWidget(0);
					RadioSniper.DisableWidget(0);
					break;
				case 1:
					RadioPointman.DisableWidget(0);
					RadioRifleman.EnableWidget(0);
					RadioSniper.DisableWidget(0);
					break;
				case 2:
					RadioPointman.DisableWidget(0);
					RadioRifleman.DisableWidget(0);
					RadioSniper.EnableWidget(0);
					break;
				}
			}
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room player class radio widget." );
		}

		//ButtonWeaponP = RoomScene.FindChild(WidgetRoomPointmanWeapon, true);
		//ButtonWeaponR = RoomScene.FindChild(WidgetRoomRiflemanWeapon, true);
		//ButtonWeaponS = RoomScene.FindChild(WidgetRoomSniperWeapon, true);
		//if (ButtonWeaponP != None && ButtonWeaponR != None && ButtonWeaponS != None)
		//{
		//	ButtonWeaponP.SetVisibility(MyClass == 0);
		//	ButtonWeaponR.SetVisibility(MyClass == 1);
		//	ButtonWeaponS.SetVisibility(MyClass == 2);
		//}
		//else
		//{
		//	`warn( "avaNetEntryGame: Cannot find room class weapon button widget." );
		//}
	}

	function UpdateCharacterPIP(bool bBody, bool bWeapon, bool bFace)
	{
		local avaUICharacterPIP PIP;
		local int MyTeam, MyClass, MyFace, MyWeapon;
		local class<avaCharacter> CharClass;
		local class<avaWeaponAttachment> AttachClass;
		local class<avaCharacterModifier> ModClass;
		local bool bRefresh;

		PIP = avaUICharacterPIP(RoomScene.FindChild(WidgetRoomCharPIP, true));
		if (PIP == None)
		{
			`warn( "avaNetEntryGame: Cannot find room character PIP widget." );
			return;
		}

		if ( !class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentEquipState(MyTeam, MyClass, MyFace, MyWeapon) )
		{
			`warn( "Failed to get current equip state" );
			return;
		}

		bRefresh = false;

		if (bBody)
		{
			if (MyClass >= 0 && MyClass < 3)
			{
				if (MyTeam == 0)
					CharClass = class<avaCharacter>(DynamicLoadObject(EUPawnClassName[MyClass],class'class'));
				else if (MyTeam == 1)
					CharClass = class<avaCharacter>(DynamicLoadObject(NRFPawnClassName[MyClass],class'class'));

				if (CharClass != None && CharClass != PIP.Template)
				{
					PIP.Template = CharClass;
					bRefresh = true;
				}
			}
		}

		if (bWeapon)
		{
			AttachClass = class'avaNetHandler'.static.GetAvaNetHandler().GetWeaponAttachment(MyWeapon);

			if (AttachClass != None && AttachClass != PIP.WeaponTemplate)
			{
				PIP.WeaponTemplate = AttachClass;
				bRefresh = true;
			}
		}

		if (bFace)
		{
			ModClass = class'avaNetHandler'.static.GetAvaNetHandler().GetCharacterModifier(MyFace);

			if (ModClass != None)
			{
				PIP.ApplyCharacterModifier(ModClass);
				bRefresh = true;
			}
		}

		if (bRefresh)
		{
			PIP.Refresh();
		}
	}

	function UpdateEquipWeapon()
	{
		local int Index, MyTeam, MyClass, MyFace, MyWeapon;
		local UILabel LabelWeapon, LabelIcon;

		if ( !class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentEquipState(MyTeam, MyClass, MyFace, MyWeapon) )
		{
			`warn( "Failed to get current equip state" );
			return;
		}

		for (Index = 0; Index < 6; Index++)
		{
			LabelWeapon = UILabel(RoomScene.FindChild(WidgetRoomWeaponName[Index], true));
			LabelIcon = UILabel(RoomScene.FindChild(WidgetRoomWeaponIcon[Index], true));

			if (LabelWeapon != None && LabelIcon != None)
			{
				if (Index <= 2)
				{
					LabelWeapon.SetDataStoreBinding("<AVANET:PlayerWeapons;" $ (Index * 3 + MyClass) $ ".ItemName>");
					LabelIcon.SetDataStoreBinding("<AVANET:PlayerWeapons;" $ (Index * 3 + MyClass) $ ".IconCode>");
				}
				else
				{
					LabelWeapon.SetDataStoreBinding("<AVANET:PlayerWeapons;" $ (6 + MyClass * 3 + Index) $ ".ItemName>");
					LabelIcon.SetDataStoreBinding("<AVANET:PlayerWeapons;" $ (6 + MyClass * 3 + Index) $ ".IconCode>");
				}

				LabelWeapon.RefreshSubscriberValue();
				LabelIcon.RefreshSubscriberValue();
			}
			else
			{
				`warn( "avaNetEntryGame: Cannot find room weapon name or weapon icon widget" );
			}
		}
	}

	function UpdateRoomSettings()
	{
		local UILabel LabelMapName, LabelSettings, LabelWinCondition, LabelMapPreview, LabelMaxPlayers, LabelMapDesc;

		LabelMapName = UILabel(RoomScene.FindChild(WidgetRoomMapName, true));
		LabelSettings = UILabel(RoomScene.FindChild(WidgetRoomSettings, true));
		LabelWinCondition = UILabel(RoomScene.FindChild(WidgetRoomWinCondition, true));
		LabelMapPreview = UILabel(RoomScene.FindChild(WidgetRoomMapPreview, true));
		LabelMaxPlayers = UILabel(RoomScene.FindChild(WidgetRoomMaxPlayers, true));
		LabelMapDesc = UILabel(RoomScene.FindChild(WidgetRoomMapDesc, true));

		if (LabelMapName != None)
		{
			LabelMapName.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room MapName widget." );
		}

		if (LabelSettings != None)
		{
			LabelSettings.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room Settings widget." );
		}

		if (LabelWinCondition != None)
		{
			LabelWinCondition.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room WinCondition widget." );
		}

		if (LabelMapPreview != None)
		{
			LabelMapPreview.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room MapPreview widget." );
		}

		if (LabelMaxPlayers != None)
		{
			LabelMaxPlayers.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room MaxPlayers widget." );
		}

		if (LabelMapDesc != None)
		{
			LabelMapDesc.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room MapDesc widget." );
		}
	}

	function Chat(string Nickname, string Msg)
	{
		local UIList ChatMsg;

		if (RoomScene == None || UIController == None)
		{
			`warn( "Cannot find the scene." );
			return;
		}

		ChatMsg = UIList(RoomScene.FindChild(WidgetRoomChatMsg, true));
		if (ChatMsg == None)
		{
			`warn( "avaNetEntryGame: Cannot find room chatting message list widget.");
			return;
		}

		ChatMsg.RefreshSubscriberValue();
		if (ChatMsg.Items.Length > ChatMsg.MaxVisibleItems)
		{
			ChatMsg.SetTopIndex(ChatMsg.Items.Length - ChatMsg.MaxVisibleItems + 1);
		}
	}

	function UpdateRoomPlayerList()
	{
		local UIList EUPlayerList, NRFPlayerList/*, SpectatorList*/;
		local UILabel LabelSpectator;
		local int ListIndex;

		if (RoomScene == None || UIController == None)
		{
			`warn( "Cannot find the scene." );
			return;
		}

		EUPlayerList = UIList(RoomScene.FindChild(WIdgetRoomPlayersEU, true));
		NRFPlayerList = UIList(RoomScene.FindChild(WidgetRoomPlayersNRF, true));
		//SpectatorList = UIList(RoomScene.FindChild(WidgetRoomPlayersSpectator, true));

		if (EUPlayerList != None)
		{
			ListIndex = EUPlayerList.Index;
			EUPlayerList.RefreshSubscriberValue();
			EUPlayerList.SetIndex(ListIndex);
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find EUPlayerList widget.");
		}

		if (NRFPlayerList != None)
		{
			ListIndex = NRFPlayerList.Index;
			NRFPlayerList.RefreshSubscriberValue();
			NRFPlayerList.SetIndex(ListIndex);
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find NRFPlayerList widget.");
		}

		//if (SpectatorList != None)
		//{
		//	ListIndex = SpectatorList.Index;
		//	SpectatorList.RefreshSubscriberValue();
		//	SpectatorList.SetIndex(ListIndex);
		//}
		//else
		//{
		//	`warn( "avaNetEntryGame: Cannot find SpectatorList widget.");
		//}

		LabelSpectator = UILabel(RoomScene.FindChild(WidgetRoomSpectator1, true));
		if (LabelSpectator != None)
		{
			LabelSpectator.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find Spectator1 widget.");
		}
		LabelSpectator = UILabel(RoomScene.FindChild(WidgetRoomSpectator2, true));
		if (LabelSpectator != None)
		{
			LabelSpectator.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find Spectator2 widget.");
		}
		LabelSpectator = UILabel(RoomScene.FindChild(WidgetRoomSpectator3, true));
		if (LabelSpectator != None)
		{
			LabelSpectator.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find Spectator3 widget.");
		}
		LabelSpectator = UILabel(RoomScene.FindChild(WidgetRoomSpectator4, true));
		if (LabelSpectator != None)
		{
			LabelSpectator.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find Spectator4 widget.");
		}
	}

	function UpdateRoomSelectedPlayer()
	{
		local UILabel LabelInfo;

		if (RoomScene == None || UIController == None)
		{
			`warn( "Cannot find the scene." );
			return;
		}

		LabelInfo = UILabel(RoomScene.FindChild(WidgetRoomPlayerLevel, true));
		if (LabelInfo != None)
		{
			LabelInfo.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room PlayerLevel widget.");
		}

		LabelInfo = UILabel(RoomScene.FindChild(WidgetRoomPlayerName, true));
		if (LabelInfo != None)
		{
			LabelInfo.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room PlayerName widget.");
		}

		LabelInfo = UILabel(RoomScene.FindChild(WidgetRoomPlayerGuildName, true));
		if (LabelInfo != None)
		{
			LabelInfo.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room PlayerGuildName widget.");
		}

		LabelInfo = UILabel(RoomScene.FindChild(WidgetRoomPlayerWinCount, true));
		if (LabelInfo != None)
		{
			LabelInfo.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room PlayerWinCount widget.");
		}

		LabelInfo = UILabel(RoomScene.FindChild(WidgetRoomPlayerKDRatio, true));
		if (LabelInfo != None)
		{
			LabelInfo.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room PlayerKDRatio widget.");
		}

		LabelInfo = UILabel(RoomScene.FindChild(WidgetRoomPlayerDisconnect, true));
		if (LabelInfo != None)
		{
			LabelInfo.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find room PlayerDisconnect widget.");
		}
	}

	function AddRoomPlayerList(int idPlayer)
	{
		UpdateRoomPlayerList();
	}

	function RemoveRoomPlayerList(int idPlayer)
	{
		UpdateRoomPlayerList();
	}

	function RoomChangeHost(int idPlayer)
	{
		UpdateRoomPlayerList();
		UpdateRoomButtons();
	}

	function RoomChangeSetting()
	{
		UpdateRoomSettings();
		UpdateRoomButtons();
		UpdateRoomPlayerList();
	}

	function RoomChangeState()
	{
		UpdateRoomPlayerList();
		UpdateRoomButtons();
	}

	function RoomKick(int idPlayer)
	{
		UpdateRoomPlayerList();
	}

	function RoomReady(int idPlayer, int Ready)
	{
		UpdateRoomPlayerList();
		UpdateRoomButtons();
	}

	function RoomChangeClass(int idPlayer)
	{
		UpdateRoomPlayerList();
		UpdateCharacterPIP(true, true, false);
		UpdateEquipWeapon();
		UpdateRoomButtons();
	}

	function RoomChangeWeapon(int idPlayer)
	{
		UpdateRoomPlayerList();
		UpdateCharacterPIP(false, true, false);
		UpdateEquipWeapon();
	}

	function RoomChangeTeam(int idPlayer, int idTeam)
	{
		UpdateRoomPlayerList();
		UpdateCharacterPIP(true, false, false);
		UpdateRoomButtons();
	}

	function GameStarted(int bStart)
	{
		local UIScene LoadedScene;

		if (bStart > 0)
		{			
			LoadedScene = LoadScene( LoadingSceneName );
			if ( LoadedScene == None || UIController == None)
			{
				`warn( "Cannot find the scene." );
				return;
			}

			UIController.SceneClient.OpenScene(LoadedScene, , LoadingScene);
		}
		else
		{
			UpdateRoomButtons();
		}
	}

	function GameCountDown(bool bStart)
	{
		`log("Starting count down...");
		if (bStart)
		{
			if ( !IsTimerActive('Timer_CountDown') )
			{
				SetTImer(1, true, 'Timer_CountDown');
			}
		}
		else
		{
			if ( IsTimerActive('Timer_CountDown') )
			{
				ClearTimer('Timer_CountDown');
			}
		}
	}

	function InventoryWeaponSet( optional bool bCustom = false )
	{
		UpdateEquipWeapon();
	}

	function InventoryEquipSet()
	{
	}
}

State Inventory
{
	function BeginState(Name PreviousStateName)
	{
		local UIScene LoadedScene;
		LoadedScene = LoadScene( InventorySceneName );
		if ( LoadedScene != None )
		{
			UIController.OpenScene(LoadedScene, , InventoryScene);
			CurrentScene = InventoryScene;
		}
		else
		{
			`warn( "avaNetEntryGame: No InventoryScene to open" );
		}
		NextScene = "None";
	}

	function EndState(Name NextStateName)
	{
		if ( InventoryScene != None )
		{
			UIController.CloseScene(InventoryScene);
			InventoryScene = None;
		}
		CurrentScene = None;
	}

	function InventoryWeaponSet( optional bool bCustom = false )
	{
		local int i;
		local string PanelName;
		local UIPanel PanelItem;
		local UILabel LabelName, LabelIcon;
		local UIList InvenList;

		if (InventoryScene == None || UIController == None)
		{
			`warn( "Cannot find the scene." );
			return;
		}

		for (i = 0; i < 18; i++)
		{
			if (i < 10)
				PanelName = "PanelWeapon0" $ i;
			else
				PanelName = "PanelWeapon" $ i;

			PanelItem = UIPanel(InventoryScene.FindChild(Name(PanelName), true));
			if (PanelItem != None)
			{
				LabelName = UILabel(PanelItem.FindChild('LabelName'));
				LabelIcon = UILabel(PanelItem.FindChild('LabelItem'));
				if (LabelName != None && LabelIcon != None)
				{
					LabelName.RefreshSubscriberValue();
					LabelIcon.RefreshSubscriberValue();
				}
				else
				{
					`warn( "avaNetEntryGame: Cannot find inventory LabelName and LabelIcon widget. Panel =" @ PanelItem);
				}
			}
			else
			{
				`warn( "avaNetEntryGame: Cannot find inventory PanelWeapon" $ i @ "widget." );
			}
		}

		InvenList = UIList(InventoryScene.FindChild(WidgetInventoryListWeapon, true));
		if (InvenList != None)
		{
			InvenList.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find inventory ListEquip widget.");
		}

		//UpdateEquipWeapon();
	}

	function InventoryEquipSet()
	{
		local int i;
		local string PanelName;
		local UIPanel PanelItem;
		local UILabel LabelName, LabelIcon;
		local UIList InvenList;

		if (InventoryScene == None || UIController == None)
		{
			`warn( "Cannot find the scene." );
			return;
		}

		for (i = 0; i < 21; i++)
		{
			if (i < 10)
				PanelName = "PanelEquip0" $ i;
			else
				PanelName = "PanelEquip" $ i;

			PanelItem = UIPanel(InventoryScene.FindChild(Name(PanelName), true));
			if (PanelItem != None)
			{
				LabelName = UILabel(PanelItem.FindChild(WidgetInventoryEquipName));
				LabelIcon = UILabel(PanelItem.FindChild(WidgetInventoryEquipIcon));
				if (LabelName != None && LabelIcon != None)
				{
					LabelName.RefreshSubscriberValue();
					LabelIcon.RefreshSubscriberValue();
				}
				else
				{
					`warn( "avaNetEntryGame: Cannot find inventory LabelName and LabelIcon widget. Panel =" @ PanelItem);
				}
			}
			else
			{
				`warn( "avaNetEntryGame: Cannot find inventory PanelEquip" $ i @ "widget." );
			}
		}

		InvenList = UIList(InventoryScene.FindChild(WidgetInventoryListEquip, true));
		if (InvenList != None)
		{
			InvenList.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find inventory ListEquip widget.");
		}

		//UpdateEquipWeapon();
	}
}

State Shop
{
	function BeginState(Name PreviousStateName)
	{
		local UIScene LoadedScene;
		LoadedScene = LoadScene( ShopSceneName );
		if ( LoadedScene != None )
		{
			UIController.OpenScene(LoadedScene, , ShopScene);
			CurrentScene = ShopScene;
		}
		else
		{
			`warn( "avaNetEntryGame: No InventoryScene to open" );
		}
		NextScene = "None";
	}

	function EndState(Name NextStateName)
	{
		if ( ShopScene != None )
		{
			UIController.CloseScene(ShopScene);
			ShopScene = None;
		}
		CurrentScene = None;
	}

	function ShopBuy(optional bool bSuccess = true)
	{
		local UIList InvenList;
		local UILabel LabelMoney;

		if (ShopScene == None || UIController == None)
		{
			`warn( "Cannot find the scene." );
			return;
		}

		InvenList = UIList(ShopScene.FindChild(WidgetShopWeaponInven, true));
		if (InvenList != None)
		{
			InvenList.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find shop WeaponInven widget.");
		}

		InvenList = UIList(ShopScene.FindChild(WidgetShopEquipInven, true));
		if (InvenList != None)
		{
			InvenList.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find shop EquipInven widget.");
		}

		LabelMoney = UILabel(ShopScene.FindChild(WidgetShopLabelMoney, true));
		if (LabelMoney != None)
		{
			LabelMoney.RefreshSubscriberValue();
		}
		else
		{
			`warn( "avaNetEntryGame: Cannot find shop LabelMoney widget.");
		}
	}
}

State NewCharacter
{
	function BeginState(Name PreviousStateName)
	{
		local UIScene LoadedScene;
		LoadedScene = LoadScene( NewCharacterSceneName );

		if ( LoadedScene != None )
		{		
			UIController.OpenScene(LoadedScene, , NewCharacterScene);
			CurrentScene = NewCharacterScene;
		}
		else
		{
			`warn( "No NewCharacterScene to open" );
		}
		NextScene = "None";
	}

	function EndState(Name NextStateName)
	{
		if ( NewCharacterScene != None )
		{
			UIController.CloseScene(NewCharacterScene);
			NewCharacterScene = None;
		}
		CurrentScene = None;
	}
}


`endif


static function DLO( string resource )
{
	if (resource != "")
	{		
		DynamicLoadObject( resource, class'Object' );
	}
}

static event LoadDLOs()
{
	DLO( default.LogoSceneName );	
	DLO( default.TitleSceneName );
	DLO( default.ChannelListSceneName );
	DLO( default.LobbySceneName );
	DLO( default.RoomSceneName );
	DLO( default.InventorySceneName  );
	DLO( default.ShopSceneName );
	DLO( default.NewCharacterSceneName );
	DLO( default.PopUpMsgSceneName );
	DLO( default.LoadingSceneName );	
	DLO( default.ExitSceneName );	
	DLO( default.ResultSceneName );	
}

function bool IsValidPlayer( int AccountID )
{
	return true;
}

//static function DLO( string resource )
//{
//
//}

defaultproperties
{
	LogoSceneName = "avaUIScene.Channel.Logo"
	TitleSceneName = "avaUIScene.Channel.Title"
	ChannelListSceneName = "avaUIScene.Channel.ServerSelection"
	LobbySceneName = "avaUIScene.Channel.Lobby"
	RoomSceneName = "avaUIScene.Channel.ReadyRoom"
	InventorySceneName = "avaUIScene.Channel.Inventory_Temp"
	ShopSceneName = "avaUIScene.Channel.Shop_Temp"
	NewCharacterSceneName = "avaUIScene.Channel.CharacterMake"
	PopUpMsgSceneName = "avaUIScene.Channel.PopUpMsg"
	LoadingSceneName = "avaUIScene.Channel.Loading"

	WidgetMessageLabel = LabelMessage
	WidgetChannelListGeneric = ListGenericChannels

	WidgetLobbyChatMsg = ListChatMsg
	WIdgetLobbyPlayerList = ListPlayers
	WidgetLobbyRoomList = ListRooms
	WidgetLobbyRoomName = LabelRoomName
	WidgetLobbyRoomMapPreview = LabelRoomMapPreview
	WidgetLobbyRoomSettings = LabelRoomSettings
	WidgetLobbyRoomMapDesc = LabelRoomMapDesc
	WidgetLobbyRoomWinCondition = LabelRoomWinCondition
	WidgetLobbyRoomPlayers = LabelRoomPlayers
	WidgetLobbyRoomCurrentRound = LabelRoomCurrentRound
	WidgetLobbyRoomHost = LabelRoomHost
	WidgetLobbyRoomPlayersEU = ListRoomPlayersEU
	WidgetLobbyRoomPlayersNRF = ListRoomPlayersNRF

	WidgetRoomStartButton = LButtonStart
	WidgetRoomReadyButton = LButtonReady
	WidgetRoomSettingButton = MapSetting
	WidgetRoomSpectateButton = LButtonSpectate
	WidgetRoomCharPIP = CharPIP
	WidgetRoomWeaponName(0) = LabelWeaponName1
	WidgetRoomWeaponIcon(0) = LabelWeaponIcon1
	WidgetRoomWeaponName(1) = LabelWeaponName2
	WidgetRoomWeaponIcon(1) = LabelWeaponIcon2
	WidgetRoomWeaponName(2) = LabelWeaponName3
	WidgetRoomWeaponIcon(2) = LabelWeaponIcon3
	WidgetRoomWeaponName(3) = LabelWeaponName4
	WidgetRoomWeaponIcon(3) = LabelWeaponIcon4
	WidgetRoomWeaponName(4) = LabelWeaponName5
	WidgetRoomWeaponIcon(4) = LabelWeaponIcon5
	WidgetRoomWeaponName(5) = LabelWeaponName6
	WidgetRoomWeaponIcon(5) = LabelWeaponIcon6
	//WidgetRoomWeaponSet = WeaponSet
	//WidgetRoomPlayerWeaponName = LabelWeaponName
	//WidgetRoomPointmanWeapon = PW
	//WidgetRoomRiflemanWeapon = RW
	//WidgetRoomSniperWeapon = SW
	WidgetRoomMapName = LabelMapName
	WidgetRoomSettings = LabelSettings
	WidgetRoomWinCondition = LabelWinCondition
	WidgetRoomMapPreview = LabelMapPreview
	WidgetRoomMaxPlayers = LabelMaxPlayers
	WidgetRoomMapDesc = LabelMapDesc
	WidgetRoomChatMsg = ListChatMsg
	WidgetRoomPlayerClassP = ps
	WidgetRoomPlayerClassR = rs
	WidgetRoomPlayerClassS = ss
	WidgetRoomPlayersEU = ListEUPlayers
	WidgetRoomPlayersNRF = ListNRFPlayers
	//WidgetRoomPlayersSpectator = ListSpectators
	WidgetRoomSpectator1 = Spectator1
	WidgetRoomSpectator2 = Spectator2
	WidgetRoomSpectator3 = Spectator3
	WidgetRoomSpectator4 = Spectator4
	WidgetRoomPlayerLevel = LabelPlayerLevel
	WidgetRoomPlayerName = LabelPlayerName
	WidgetRoomPlayerGuildName = LabelPlayerGuildName
	WidgetRoomPlayerWinCount = LabelPlayerWinCount
	WidgetRoomPlayerKDRatio = LabelPlayerKDRatio
	WidgetRoomPlayerDisconnect = LabelPlayerDisconnect
	WidgetInventoryEquipName = LabelName
	WidgetInventoryEquipIcon = LabelItem
	WidgetInventoryListEquip = ListEquip
	WidgetInventoryListWeapon = ListWeapon
	WidgetShopWeaponInven = ListWeapon
	WidgetShopEquipInven = ListEquip
	WidgetShopLabelMoney = LabelMoney
}
