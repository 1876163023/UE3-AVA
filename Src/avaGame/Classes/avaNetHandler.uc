/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaGame

	Name: avaNetHandler.uc

	Description: Interface class between UE3 and AVA networking

***/
class avaNetHandler extends Object
	native(Net);


`include(avaGame/avaGame.uci)

/////////////////////////////////////////////////////////////////////////////////////////////
// message

enum EMsgCategory
{
	EMsg_Client,
	EMsg_Channel,
	EMsg_Room,
	EMsg_Game,
	EMsg_Inventory,
	EMsg_Guild,
	EMsg_Friend,
	EMsg_Admin,
	EMsg_Buddy,
	EMsg_Option,		// 옵션설정시에 예외 팝업 메세지 (해상도를 즉시 변경할 수 없습니다, 새 셰이더 모델은 ... 등등) 처리
};

enum EMsgClient
{
	EMsg_Client_Connect,
	EMsg_Client_Disconnect,
	EMsg_Client_CheckNick,
	EMsg_Client_PlayerInfo,
	EMsg_Client_MainNotice,
	EMsg_Client_Kick,
	EMsg_Client_GuildConnect,
	EMsg_Client_GuildDisconnect,
	EMsg_Client_HwInfo,
	EMsg_Client_SetConfig,
	EMsg_Client_SetRttTestAddr,
	EMsg_Client_NoMorePremiumPcBang,
};

enum EMsgChannel
{
	EMsg_Channel_List,
	EMsg_Channel_Join,
	EMsg_Channel_Leave,
	EMsg_Channel_RoomList,
	EMsg_Channel_RoomInfo,
	EMsg_Channel_PlayerList,
	EMsg_Channel_PlayerInfo,
	EMsg_Channel_FollowPlayer,
	EMsg_Channel_LobbyJoin,
	EMsg_Channel_LobbyLeave,
	EMsg_Channel_LobbyChat,
	EMsg_Channel_RoomState,
	EMsg_Channel_RoomSetting,
	EMsg_Channel_RoomCreate,
	EMsg_Channel_RoomDelete,
	EMsg_Channel_Whisper,
};

enum EMsgRoom
{
	EMsg_Room_Create,
	EMsg_Room_Join,
	EMsg_Room_Leave,
	EMsg_Room_Info,
	EMsg_Room_PlayerList,
	EMsg_Room_PlayerInfo,
	EMsg_Room_Chat,
	EMsg_Room_ChangeHost,
	EMsg_Room_Kick,
	EMsg_Room_ChangeSetting,
	EMsg_Room_ChangeState,
	EMsg_Room_Ready,
	EMsg_Room_ChangeSlot,
	EMsg_Room_ChangeClass,
	EMsg_Room_ChangeWeapon,
	EMsg_Room_RttRating,
	EMsg_Room_RtttStart,
	EMsg_Room_SwapTeam,
	EMsg_Room_QuickJoin,
	EMsg_Room_ClanInfo,
	EMsg_Room_ClaimHost,
};

enum EMsgGame
{
	EMsg_Game_Start,
	EMsg_Game_StartCount,
	EMsg_Game_CancelCount,
	EMsg_Game_LoadingProgress,
	EMsg_Game_Leave,
	EMsg_Game_ResultUpdate,
	EMsg_Game_End,
	EMsg_Game_Chat,
	EMsg_Game_LevelUpGrats,
};

enum EMsgInventory
{
	EMsg_Inventory_EquipSet,
	EMsg_Inventory_WeaponSet,
	EMsg_Inventory_CustomSet,
	EMsg_Inventory_EffSet,
	EMsg_Inventory_Buy,
	EMsg_Inventory_EffBuy,
	EMsg_Inventory_SendGift,
	EMsg_Inventory_Repair,
	EMsg_Inventory_ConvertRIS,
	EMsg_Inventory_Sell,
	EMsg_Inventory_UpdateGauge,
	EMsg_Inventory_DeleteItem,
	EMsg_Inventory_DeleteCustom,
	EMsg_Inventory_DeleteEffect,
	EMsg_Inventory_Enter,
	EMsg_Inventory_Leave,
	EMsg_Inventory_ChangeClass,
	EMsg_Inventory_GetCash,
	EMsg_Inventory_Charge,
};

enum EMsgAdmin
{
	EMsg_Admin_Notice,
	EMsg_Admin_Kick,
	EMsg_Admin_ChatOff,
	EMsg_Admin_ChangeRoomName,
	EMsg_Admin_SetVisibility,
	EMsg_Admin_MainNotice,
	EMsg_Admin_Whisper,
};

enum EMsgBuddy
{
	EMsg_Buddy_ConsoleCommand,
	EMsg_Buddy_AddReq,
	EMsg_Buddy_AddReqAns,
	EMsg_Buddy_Delete,
	EMsg_Buddy_AddBlock,
	EMsg_Buddy_DeleteBlock,
	EMsg_Buddy_InviteGame,
	EMsg_Buddy_InviteGuild,
	EMsg_Buddy_Chat,
	EMsg_Buddy_Location,
	EMsg_Buddy_StateChanged,
	EMsg_Buddy_List,
	EMsg_Buddy_PlayerInfo,
	EMsg_Buddy_EndFollow,
	
};

enum EMsgGuild
{
	EMsg_Guild_Info,
	EMsg_Guild_MemberList,
	EMsg_Guild_JoinChannel,
	EMsg_Guild_LeaveChannel,
	EMsg_Guild_Motd,
	EMsg_Guild_Chat,
	EMsg_Guild_Notice,
	EMsg_Guild_MemberState,
	EMsg_Guild_PlayerInfo,
	EMsg_Guild_Invite,
	EMsg_Guild_Join,
	EMsg_Guild_Leave,
	EMsg_Guild_Kick,
	EMsg_Guild_GetChannelAddr,
	EMsg_Guild_RankChanged,
	EMsg_Guild_MasterChanged,
	EMsg_Guild_UpdateScore,
};

enum EMsgOption
{
	EMsg_Option_SettingChanged,
};

/////////////////////////////////////////////////////////////////////////////////////////////
// const

enum ENetGameState
{
	EGS_None,
	EGS_GameBegin,
	EGS_GameEnd,
	EGS_RoundBegin,
	EGS_RoundEnd,
};

enum EVoteCommand
{
	EVC_Kick,
};

enum EVoteKickReason
{
	EKR_HackUser,
	EKR_BugUser,
	EKR_Abuser,
	EKR_NoManner,
	EKR_Misc,
};

enum EChannelFlag
{
	EChannelFlag_Normal,
	EChannelFlag_Trainee,
	EChannelFlag_Match,
	EChannelFlag_Reserve2,
	EChannelFlag_Reserve3,
	EChannelFlag_Newbie,
	EChannelFlag_Clan,
	EChannelFlag_PCBang,
	EChannelFlag_Event,
	EChannelFlag_MyClan,
	EChannelFlag_Practice,
	EChannelFlag_Broadcast,
	EChannelFlag_AutoBalance,
	EChannelFlag_Temp,
};

enum EChannelSetting
{
	EChannelSetting_EnableAutoBalance,
	EChannelSetting_EnableSpectator,
	EChannelSetting_EnableInterrupt,
	EChannelSetting_EnableBackView,
	EChannelSetting_EnableGhostChat,
	EChannelSetting_EnableAutoSwapTeam,
	EChannelSetting_EnableMaxPlayers,
	EChannelSetting_EnableTKLevel,

	EChannelSetting_MissionTypeSpecial,
	EChannelSetting_MissionTypeWarfare,
	EChannelSetting_MissionTypeTraining,

	EChannelSetting_DefaultAutoBalance,
	EChannelSetting_DefaultSpectator,
	EChannelSetting_DefaultInterrupt,
	EChannelSetting_DefaultBackView,
	EChannelSetting_DefaultGhostChat,
	EChannelSetting_DefaultAutoSwapTeam,
	EChannelSetting_DefaultTKLevel,
	EChannelSetting_DefaultMaxPlayers,

	EChannelSetting_RoomChangeTeam,
	EChannelSetting_RoomSkipMinPlayerCheck,
	EChannelSetting_RoomSkipBalanceCheck,

	EChannelSetting_GameIdleCheck,
	EChannelSetting_GameFreeCam,
	EChannelSetting_GameCheckAllOut,

	EChannelSetting_UpdatePlayerScore,
	EChannelSetting_UpdateStatLog,

	EChannelSetting_InvenCashItem,
	EChannelSetting_AllowPCBangBonus,
	EChannelSetting_AllowBoostItem,
	EChannelSetting_AllowEventBonus,
};


/////////////////////////////////////////////////////////////////////////////////////////////
// score information

const EWeapon_Pirmary = 3;

struct native avaScoreInfo
{
	var int Attacker;
	var int Defender;
	var int Leader;
	var int Tactic;
};

struct native avaClassScoreInfo
{
	var int PlayTime;			// 플레이한 시간 (초)
	var int PlayRound;			// 플레이한 라운드/스폰 수
	var int SprintTime;			// 스프린트한 시간 (초)
	var int KillCount;			// 킬 횟수
	var int HeadshotCount;		// 주무기로 준 헤드샷 횟수
	var int HeadshotKillCount;	// 주무기 헤드샷으로 달성한 킬 횟수
	var int TakenDamage;		// 받은 데미지

	var int WeaponDamage[4];	// 각 무기별로 준 데미지; EWeapon_Primary 사용
	var int WeaponKillCount[4];	// 각 무기별 킬 수; EWeapon_Primary 사용
};

struct native avaPlayerScoreInfo
{
	// 스킬 관련

	var int idAccount;
	var int GameWin;						// 이긴 게임 수 (1 or 0)
	var int RoundWin;						// 이긴 라운드 수
	var int RoundDefeat;					// 진 라운드 수
	var int DisconnectCount;				// 디스커넥트한 횟수 (1 or 0)
	var int DeathCount;						// 죽은 횟수
	var avaScoreInfo Score;

	var avaClassScoreInfo ClassScoreInfo[3];

	// 훈장 관련

	var int RoundTopKillCount;				// 킬 수 1위 달성한 라운드 횟수
	var int RoundTopHeadshotKillCount;		// 헤드샷 킬 수 1위 달성한 라운드 횟수
	var int TopLevelKillCount;				// 상대팀에서 계급이 제일 높은 유저 킬 수
	var int HigherLevelKillCount;			// 나보다 계급이 높은 유저 킬 수
	var int BulletMultiKillCount;			// 탄환으로 멀티킬 달성한 횟수
	var int GrenadeMultiKillCount;			// 수류탄으로 멀티킬 달성한 횟수
	var int NoDamageWinCount;				// 데미지를 전혀 입지 않고 승리한 라운드 횟수

	var int TopScoreCount;					// 팀 내 점수에서 1등한 라운드 수
	var int NoDeathRoundCount;				// 한번도 킬 당하지 않은 라운드 수

	var int WeaponFireCount[4];				// 발사한 총알 수; 0 : Pistol, 1 : SMG, 2 : RIFLE, 3 : SNIPER,
	var int WeaponHitCount[4];				// 명중한 총알 수; 0 : Pistol, 1 : SMG, 2 : RIFLE, 3 : SNIPER,
	var int WeaponHeadshotCount[4];			// 헤드샷 횟수; 0 : Pistol, 1 : SMG, 2 : RIFLE, 3 : SNIPER,

	var int HelmetDropCount;				// 헬멧 떨어뜨린 횟수

	// 기타

	var int TeamKillCount;					// 팀킬 횟수
	var int CurrentClass;					// 게임 내에서 마지막으로 선택한 병과
	var bool bLeader;						// 분대장이면 true

	structcpptext
	{
		FavaPlayerScoreInfo();
	}
};


/////////////////////////////////////////////////////////////////////////////////////////////
// log information

struct native avaGameScoreLog				// 진영별로 합계를 구한다
{
	var int KillCount;						// 킬 수
	var int SuicideCount;					// 자살 수
	var int HeadshotKillCount;				// 헤드샷 킬 수
	var avaScoreInfo Score;					// 점수

	var int FriendlyFireCount;				// Friendly Fire 수
	var int FriendlyKillCount;				// Friendly Fire로 킬 한 수

	var int SpawnCount[3];					// 스폰 수 (포인트맨, 라이플맨, 스나이퍼)
};

struct native avaWeaponLog					// 무기 데이터
{
	var class<avaWeapon> Weapon;			// 무기

	var int UsedCount;						// 사용 회수/빈도 (스폰하거나, 줍거나)
	var int FireCount;						// 발사 회수
	//var int HitCount;						// 명중 회수
	var int HitCount_Head;					// 머리 부분 명중 회수
	var int HitCount_Body;					// 몸통 부분 명중 회수
	var int HitCount_Stomach;				// 배 부분 명중 회수
	var int HitCount_LeftArm;				// 왼팔 부분 명중 회수
	var int HitCount_RightArm;				// 오른팔 부분 명중 회수
	var int HitCount_LeftLeg;				// 왼쪽 다리 부분 명중 회수
	var int HitCount_RightLeg;				// 오른쪽 다리 부분 명중 회수
	var float HitDistance;					// total 사거리
	var int HitDamage;						// total 데미지

	var int KillCount[3];					// 사살 회수 (포인트맨, 라이플맨, 스나이퍼)
	var int HeadshotKillCount;				// 헤드샷으로 사살 회수
	var int MultiKillCount;					// 멀티킬 회수
};

struct native avaRoundPlayLog
{
	var int Winner;							// 승리 팀 (EU, NRF)
	var int WinType;						// 라운드가 끝난 방식 (Annihilation/Mission)
	var int StartTime;						// 라운드 시작 시간
	var int RoundTime;						// 플레이 타임
	var int PlayerCount[2];					// 라운드가 끝난 시점에 각 진영별 플레이어 수; 0 = EU, 1 = NRF

	var array<avaWeaponLog> WeaponLogs;		// 라운드별 무기 데이터
};

struct native avaKillLog					// 사망 이벤트 로그
{
	var int KillTime;						// 죽인 시간
	var class<avaWeapon> Weapon;			// 죽인 무기
	var vector KillerLocation;				// 사살자의 위치
	var vector VictimLocation;				// 사망자의 위치
};

struct native avaStatLog
{
	var avaGameScoreLog GameScoreLogs[2];					// 스코어 로그, 0 = EU, 1 = NRF
	var array<avaRoundPlayLog> RoundPlayLogs;				// 라운드 플레이 로그
	var array<avaKillLog> KillLogs;							// 킬 이벤트 로그
};



/////////////////////////////////////////////////////////////////////////////////////////////

enum EChatMsgType
{
	EChat_Normal,
	EChat_Notice,
	EChat_GuildChat,
	EChat_GuildNotice,
	EChat_Whisper,
	EChat_InGameSystem,
	EChat_ReadyRoom,
	EChat_PlayerSystem,
	EChat_ServerNotice,
	EChat_ChNotice,
	EChat_GuildSystem,
	EChat_GMWhisper,
	EChat_TeamSay,
	EChat_Say,
};

/////////////////////////////////////////////////////////////////////////////////////////////

enum EPopUpMsgType
{
	EPT_None,
	EPT_Notice,
	EPT_YesNo,
	EPT_Warning,
	EPT_Error,
	EPT_GameInvitation,
	EPT_FriendInvitation,
	EPT_GuildInvitation,
};


struct native avaPopUpMsgInfo
{
	var EPopUpMsgType MsgType;
	var string	PopUpMsg;
	var string	NextScene;
	var Name	NextUIEventName;
};



/////////////////////////////////////////////////////////////////////////////////////////////

struct native avaRoomPlayerInfo
{
	var int AccountID;
	var string Nickname;
	var name NickFName;
	var int Level;
	var int TeamID;
	var int LoadingProgress;
	var int LoadingStepCount;
	var int UpdateCount;
	var double UpdateTime;
};


/////////////////////////////////////////////////////////////////////////////////////////////

struct native avaWeaponIDMap
{
	var class<avaWeapon> Weapon;
	var int ItemID;
};

var array<avaWeaponIDMap>			WeaponIDList;
var array<avaRoomPlayerInfo>		RoomStartingPlayerList;

var float							LoadingCheckTime;
var int								LoadingCutOffPerc;

var transient int					HostLoadingProgress;


/////////////////////////////////////////////////////////////////////////////////////////////

struct native avaNetEventParam
{
	var class<UIEvent>		EventClass;
	var bool				BoolParam;
	var int					IntParam;
	var float				FloatParam;
	var string				StrParam;
	var Object				ObjParam;
};

// OpenScene Managed에서 Scene을 열때 열려야할 조건을 검사하고 필요하다면 펜딩
struct native PendingSceneInfo
{
	var	class<GameInfo>				BaseGameClass;
	var UIScene						UIScene;
	var EScenePriorityType			Priority;
	var array< avaNetEventParam >	EventParams;

	// additional functionalities
	var float						LifeTime;
	var array< Name >				ExclSceneTags;
};

// Event를 발생해야할때 발생해야할 조건을 검사하고 필요하다면 펜딩
struct native PendingEventInfo
{
	var avaNetEventParam			EventParam;
	var class<GameInfo>				BaseGameClass;
	var array< Name >				BaseSceneTags;
	var array< Name >				ExclSceneTags;
};

// Message가 발생했을때 현재 Message를 받아들일 수 있는지 검사하고 필요하다면 펜딩
struct native PendingMsgInfo
{
	var EMsgCategory				MsgCategory;
	var byte						Msg;
	var class<GameInfo>				BaseGameClass;

	var string						Param1;
	var string						Param2;
	var int							Param3;
	var int							Param4;
};

var array<PendingSceneInfo>		PendingSceneList;

var array<PendingEventInfo>		PendingEventList;

var array<PendingMsgInfo>		PendingMsgList;

/////////////////////////////////////////////////////////////////////////////////////////////

var string		PopupSceneName;
var string		HostMigrationSceneName;
var	string		ExitSceneName;

var const class<GameInfo>		NetEntryGameClass;
var const class<GameInfo>		TeamGameClass;
var const class<GameInfo>		DeathMatchGameClass;

var array<INT>	StartPlayerIDList;

var	array< class<avaModifier> >	CharacterModifierList;
var array< class<avaModifier> > WeaponModifierList;

var const array<name> EntrySceneTags;
var const array<name> PersistentEntrySceneTags;

// Option Data Transactor
var /*const*/ avaTransBuffer	Trans;

/////////////////////////////////////////////////////////////////////////////////////////////

enum EavaLeavingReason
{
	ELR_Normal,
	ELR_Idle,
	ELR_DidntReady,
	ELR_HostDidntStart,
	ELR_PackageMismatch,
	ELR_PackageNotFound,
	ELR_FailedToConnectHost,
	ELR_RejectedByHost,
	ELR_MD5Failed,
	ELR_P2PConnectionFailed,
};

/////////////////////////////////////////////////////////////////////////////////////////////

native final function LeaveRoom(EavaLeavingReason Reason = ELR_Normal);


/////////////////////////////////////////////////////////////////////////////////////////////
// Loading time-out check functions

native final function bool CanExitGame(); // [+] 20070531 dEAthcURe|HM
native final function KickSlowLoadingPlayers(int CutOffPerc);
native final function CheckLoadingTimeOut();
native final function StartLoadingCheck();

/////////////////////////////////////////////////////////////////////////////////////////////

function InitWeaponIDList()
{
	local avaGameReplicationInfo GRI;
	local class<avaMod_Weapon> WeaponMod;
	local avaWeaponIDMap Elem;
	local int i;

	`log("avaNetHandler: Initializing weapon id list");

	if (WeaponIDList.Length > 0)
	{
		`warn("already initialized");
		return;
	}
	
	GRI = avaGameReplicationInfo(GetWorldInfo().GRI);
	if (GRI == None)
		return;

	for (i = 0; i < WeaponModifierList.Length; i++)
	{
		WeaponMod = class<avaMod_Weapon>(WeaponModifierList[i]);
		if (WeaponMod != None && WeaponMod.default.WeaponClass != None)
		{
			Elem.Weapon = WeaponMod.default.WeaponClass;
			Elem.ItemID = WeaponMod.default.ID;
			//WeaponIDList.Length = WeaponIDList.Length + 1;
			WeaponIDList[WeaponIDList.Length] = Elem;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////

event OnInit()
{
	local int i;
	local array< class<object> >	Modifiers;
	class'ClassIterator'.static.FindSubClass( class'avaModifier', Modifiers );
	for ( i = 0 ; i < Modifiers.length ; ++ i )
	{
		if ( class<avaCharacterModifier>( Modifiers[i] ) != None   )	CharacterModifierList[ CharacterModifierList.length ] = class<avaModifier>( Modifiers[i] );
		else if ( class<avaMod_Weapon>( Modifiers[i] ) != None  )		WeaponModifierList[ WeaponModifierList.length ] = class<avaModifier>( Modifiers[i] );
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////

cpptext
{
public:
	UavaNetHandler();

	void ProcMessage(EMsgCategory MsgCat, INT MsgID, const FString &Param1, const FString &Param2, int Param3, int Param4);
	FString GetWeaponIconCode(INT idItem)
	{
		return eventGetWeaponIconCode(idItem);
	}

	void CollectLastResultInfo();
}


/////////////////////////////////////////////////////////////////////////////////////////////
// pop up message

// PopupMessage를 PendingSceneList에 걸어준다.
/**
 * 기존의 PopupMessage를 유지하기 위해 만든 PopupMessage. 
 * 
 * @param MsgType			메세지타입 - Notice, Warning, Error ...
 * @param PopupMsg			메세지 내용 
 * @param NextScene			팝업이 끝난뒤 열릴 씬.
 * @param NextUIEventName	팝업이 끝난뒤 발생할 이벤트 이름(UIRemoteEvent)
 * @param ExclusiveScenes	[팝업 속성] 팝업이 열려서는 안되는 씬 리스트
 * @param LifeTime			[팝업 속성] 팝업의 생존시간 // 0.f = INFINITY
 *
 */
event PopupMessage(EPopupMsgType MsgType, string PopupMsg, String NextScene, optional name NextUIEventName, optional array<Name> ExclSceneTags, optional float LifeTime )
{
	Local array< AvaNetEventParam >	EventParams;
	Local string			NextSceneName;
	Local UIInteraction		UIController;
	Local UIScene			NextUIScene;
	
	if( MsgType == EPT_None || Len(PopupMsg) == 0 )
		return;

	// @ NextScene처리 (특정한 이름에 대해선 Scene이름을 반환해 돌려준다. 이외의 것들은 그대로 Scene이름으로 사용 )
	if( Len(NextScene) != 0 )
	{
		switch( NextScene )
		{
			// @TODO : bExit 같은 Parameter를 받아 직접 프로그램 종료
			case "Exit":			NextSceneName		= ExitSceneName;					break;
			default:				NextSceneName		= NextScene;						break;
		}

		UIController = GetUIController();

		if( UIController != None && Len(NextSceneName) != 0 && NextSceneName ~= "none" )
			NextUIScene = UIScene(DynamicLoadObject(class'GameInfo'.static.GetFullUIPath(NextSceneName), class'UIScene'));
	}
	
	// 이벤트를 이름으로 실행 ( 열려있는 모든 씬에 대해서, 씬이 끝날때  )
	if( Len(NextUIEventName) != 0 && Locs(NextUIEventName) != "none" )
	{
		AppendAvaNetEventParam(EventParams, class'avaUIEvent_PopupActivateNextUIEvent', string(NextUIEventName));
	}
	
	// '확인'을 눌렀을때 다음에 열어야할 씬을 지정
	if( NextUIScene != none )
	{
		AppendAvaNetEventParam( EventParams, class'avaUIEvent_PopupOpenNextUIScene' ,/*Str*/,/* Int*/, /*Bool*/, /*Float*/,NextUIScene);
	}

	AppendAvaNetEventParam( EventParams, class'avaUIEvent_PopupSetLabelText', PopupMsg);

	OpenSceneManagedName( PopupSceneName, NetEntryGameClass, EScenePrior_PopUpScene_High, EventParams, ExclSceneTags, LifeTime );
}

/*! @brief PendingEvent를 추가한다.
	@param EventClass
		PendingEvent 클래스.
	@param BaseGameClass
		avaNetEntryGameEx(채널용) or avaSWGame(게임용) 클래스.
	@param BaseSceneTags
		이벤트가 발생할 수 있는 장면의 태그 리스트.
	@param ExclSceneTags
		이벤트가 발생할 수 없는 장면의 태그 리스트.(계속 Pending상태가 된다)
*/
event AddPendingEvent( class<UIEvent> EventClass,optional class<GameInfo> BaseGameClass, optional array<Name> BaseSceneTags, optional array<Name> ExclSceneTags, optional string StrParam, optional int IntParam, optional bool BoolParam, optional float FloatParam, optional Object ObjParam )
{
	Local int Length;

	Length = PendingEventList.Length;
	PendingEventList.Add(1);

	PendingEventList[Length].BaseGameClass = BaseGameClass;
	// Excl 가 Base를 우선한다. (예를 들어 Lobby가 Base와 Excl SceneTags에 모두 포함되어 있으면 배제된다)
	PendingEventList[Length].BaseSceneTags = BaseSceneTags;
	PendingEventList[Length].ExclSceneTags = ExclSceneTags;

	PendingEventList[Length].EventParam.EventClass = EventClass;
	PendingEventList[Length].EventParam.StrParam = StrParam;
	PendingEventList[Length].EventParam.IntParam = IntParam;
	PendingEventList[Length].EventParam.BoolParam = BoolParam;
	PendingEventList[Length].EventParam.FloatParam = FloatParam;
	PendingEventList[Length].EventParam.ObjParam = ObjParam;
	//UpdatePendingEvents();
}

function AppendAvaNetEventParam( out array<AvaNetEventParam> EventParamArray, class<UIEvent> EventClass,  optional string StrParam, optional int IntParam, optional bool BoolParam, optional float FloatParam, optional Object ObjParam)
{
	Local int Length;

	assert( EventClass != None );

	Length = EventParamArray.Length;
	EventParamArray.Add(1);
	EventParamArray[Length].EventClass = EventClass;
	EventParamArray[Length].StrParam = StrParam;
	EventParamArray[Length].IntParam = IntParam;
	EventParamArray[Length].BoolParam = BoolParam;
	EventParamArray[Length].FloatParam = FloatParam;
	EventParamArray[Length].ObjParam = ObjParam;
}

event AddPendingMsg( EMsgCategory MsgCat, byte Msg, class<GameInfo> BaseGameClass, string Param1, string Param2, int Param3, int Param4 )
{
	Local int Length;

	Length = PendingMsgList.Length;

	PendingMsgList.Add(1);
	PendingMsgList[Length].MsgCategory = MsgCat;
	PendingMsgList[Length].Msg = Msg;
	PendingMsgList[Length].BaseGameClass = BaseGameClass;
	PendingMsgList[Length].Param1 = Param1;
	PendingMsgList[Length].Param2 = Param2;
	PendingMsgList[Length].Param3 = Param3;
	PendingMsgList[Length].Param4 = Param4;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// error handler

event ProcCriticalError(string Code)
{
	Local class<GameInfo>	GameClass;

	`log("ProcCriticalError Code = "$Code);
	
	GameClass = GetGameInfoClass();
	if( ClassIsChildOf(GameClass, NetEntryGameClass) )
	{
		switch (Code)
		{
		case "socket failed to connect":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Error_SocketFailedToConnect>", "Exit");		break;
		case "socket failed to receive":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Error_SocketFailedToRecv>", "Exit");		break;
		case "socket failed to send":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Error_SocketFailedToSend>", "Exit");		break;
		case "no connection":				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Error_NoConnection>", "Exit");				break;
		case "session failed to connect":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Error_SessionFailedToConnect>", "Exit");	break;
		case "session failed to create":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Error_SessionFailedToCreate>", "Exit");		break;
		case "session failed to change":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Error_SessionFailedToChange>", "Exit");		break;
		case "client invalid state":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Error_ClientInvalidState>", "Exit");		break;
		}
	}
	else
	{
		`warn("invalid game class"@GameClass@"Code ="@Code);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// message processor

event ProcClientMessage(EMsgClient Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local class<GameInfo>	GameClass;

	GameClass = GetGameInfoClass();
	if( ClassIsChildOf(GameClass, NetEntryGameClass) )
		ProcClientMessageNet(Msg, Param1,Param2,Param3,Param4);
	else if ( ClassIsChildOf(GameClass, TeamGameClass) || ClassIsChildOf(GameClass, DeathMatchGameClass) )
		ProcClientMessageGame(Msg, Param1, Param2, Param3, Param4);

	else
		`warn("invalid game class"@GameClass@" @ ProcAdminMessage("@Msg@Param1@Param2@Param3@Param4@")");

	`Log("ProcClientMessage Msg = "$Msg$", Param1 = "$Param1);
}

event ProcClientMessageNet(EMsgClient Msg, string Param1, string Param2, int Param3, int Param4)
{
	local avaNetEntryGame GameObj;
	Local array<Name> ExclSceneTags;
	Local array<Name> BaseSceneTags;

	GameObj = avaNetEntryGame(GetWorldInfo().Game);

	switch (Msg)
	{
	case EMsg_Client_Connect:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcClientConnect',Param1);
		switch (Param1)
		{
		case "ok":					/*GotoScene("ChannelList");*/	break;
		case "invalid id":			PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Client_BadCharInID>", "Exit");				break;
		case "no nick":				/*Gameobj.GotoScene("NewCharacter");*/	break;
		case "already connected":	PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Client_AlreadyConnectedAccount>", "Exit");	break;
		case "connecting":			break;
		case "loading":				break;
		case "invalid version":		PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Client_InvalidVersion>", "Exit");				break;
		case "time out":			PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Client_TimeOut>", "Exit");					break;
		case "channel full":		PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Channel_FailedToJoinChannelFull>", "None", 'JoinChannelCanceled');				break;
		case "no resource":			PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Client_NoResource>", "Exit");				break;
		case "gameguard error":		PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Client_GameGuardError>", "Exit");				break;
		case "failed":	
		default:					PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Client_FailedToConnect>", "None");
		}
		break;

	case EMsg_Client_Disconnect:
		ExclSceneTags.AddItem('Logo');
		ExclSceneTags.AddItem('NeowizLogo');
		ExclSceneTags.AddItem('avaBI');
		ExclSceneTags.AddItem('Title');

		switch (Param1)
		{
		case "server":				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Client_ServerConnectionLost>", "Exit", ,ExclSceneTags);				break;
		case "already connected":	//Title Scene이 열리면 Get Connect Result에서 "already connected"가 반환되면서 필요한 ReconnectAcction 처리가 이루어짐 
									/*PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Client_AlreadyConnectedAccount>","",'Title_ReconnectAccount');*/	break;
		}
		break;
	
	case EMsg_Client_CheckNick:
		switch (Param1)
		{
		
		case "ok":					GameObj.UpdateCreateCharacter();		break;
		case "already exists":		PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Client_NickNameExists>", "None");					break;
		case "invalid name":		PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Client_BadCharInNickName>", "None");				break;
		case "time out":			PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Client_FailedToCreateCharacterTimeOut>", "None"); break;
		case "failed":
		default:					PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Client_FailedToCreateCharacter>", "None");			break;
		}
		break;

	case EMsg_Client_PlayerInfo:
		BaseSceneTags.AddItem('ChannelList');
		BaseSceneTags.AddItem('Lobby');
		BaseSceneTags.AddItem('ClanLobby');
		// CustomParent장면에서도 갱신되도록 추가.
		BaseSceneTags.AddItem('Award');
		BaseSceneTags.AddItem('Skill');
		BaseSceneTags.AddItem('Stat');
		AddPendingEvent(class'avaUIEvent_ProcClientPlayerInfo', NetEntryGameClass, BaseSceneTags);
		//class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcClientPlayerInfo',Param1);
		GameObj.UpdatePlayerInfo();
		if( GetLocalPC() != None )
		{
			`Log("GetConfigString() = "$GetConfigString());
			GetLocalPC().PlayerInput.SetUserKeyString(GetConfigString());				// 조작키를 DB에서 가져옴
			class'avaOptionSettings'.static.SetGameOptionString( GetConfigString2() );	// 게임옵션을 DB에서 가져옴
		}
		break;
	case EMsg_Client_MainNotice:
		break;
	case EMsg_Client_GuildConnect:
		switch (Param1)
		{
		case "ok":					break;
		case "time out":			break;
		}
		break;
	case EMsg_Client_Kick:
		switch( Param1 )
		{
		case "forced connection":
			PopupMessage(EPT_Warning,"<Strings:avaNet.UIPopUpMsg.Msg_Client_Kicked>","None");
			break;
		}
	break;

	//! 프리미엄 PC방 효력이 종료된 경우.(2007/12/11)
	//! (플레이 하지 않는 경우에만 발생한다고 함)
	case EMsg_Client_NoMorePremiumPcBang:
//		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcRoomKick', "no more premium pcbang");
		PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Client_NoMorePremiumPcBang>", "None");
		break;
	}
}

event ProcClientMessageGame(EMsgClient Msg, string Param1, string Param2, int Param3, int Param4)
{
	switch (Msg)
	{
		//  현재로써는 EMsg_Client_Kick은 한 계정으로 여러명 접속했을때 
		// 먼저 접속한 사람이 튕겨나가야할 경우에만 들어온다
		case EMsg_Client_Kick:
		switch( Param1 )
		{
		case "forced connection":
			ChatMessage(Localize("UIPopUpMsg","Msg_Client_Kicked","avaNet"), EChat_Notice);
			break;
		}
		//Param1 == "forced connection"
		break;

		case EMsg_Client_NoMorePremiumPcBang:
//			AddPendingEvent(class'avaUIEvent_ProcRoomKick',NetEntryGameClass,,,"no more premium pcbang");
			ChatMessage(Localize("UIPopUpMsg","Msg_Client_NoMorePremiumPcBang","avaNet"), EChat_Notice);
			break;
	}	
}

event ProcChannelMessage(EMsgChannel Msg, string Param1, string Param2, int Param3, int Param4)
{
	//local avaNetEntryGame GameObj;
	//GameObj = avaNetEntryGame(GetWorldInfo().Game);
	//if (GameObj == None)
	//	return;
	Local class<GameInfo>	GameClass;
	GameClass = GetGameInfoClass();
	if( ClassIsChildOf( GameClass, NetEntryGameClass ) == false )
		return;

	`log( "ProcChannelMessage(" @Msg @"," @Param1 @"," @Param2 @"," @Param3 @"," @Param4 @")" );

	switch (Msg)
	{
	case EMsg_Channel_List:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelList',Param1);
		switch (Param1)
		{
		case "ok":				break;
		case "time out":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGetChannelListTimeOut>", "None", 'JoinChannelCanceled' );		break;
		case "failed":	
		default:				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGetChannelList>", "None", 'JoinChannelCanceled' );
		}
		break;

	case EMsg_Channel_Join:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelJoin',Param1);
		switch (Param1)
		{
		case "ok":					break;
		case "moving":				break;
		case "full":				PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Channel_FailedToJoinChannelFull>", "None", 'JoinChannelCanceled');				break;
		case "not available":		PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Channel_FailedToJoinChannelNotAvailable>", "None", 'JoinChannelCanceled');		break;
		//case "level limited":		PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Channel_FailedToJoinChannelLevelLimit>", "None", 'JoinChannelCanceled');		break;
		case "trainee only":		PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Channel_FailedToJoinChannelTraineeOnly>", "None", 'JoinChannelCanceled');		break;
		case "newbie only":			PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Channel_FailedToJoinChannelNewbieOnly>", "None", 'JoinChannelCanceled');		break;
		case "guild only":			PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Channel_FailedToJoinChannelGuildOnly>", "None", 'JoinChannelCanceled');		break;
		case "regular guild only":	PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Channel_FailedToGuildJoinChannelNotRegular>", "None", 'JoinChannelCanceled');		break;
		case "pcbang only":			PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Channel_FailedToJoinChannelPCBangOnly>", "None", 'JoinChannelCanceled');		break;
		case "time out":			PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Channel_FailedToJoinChannelTimeOut>", "None", 'JoinChannelCanceled');			break;
		case "quickjoin failed":	PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Channel_FailedToJoinChannelQuickJoinFailed>", "None", 'JoinChannelCanceled');			break;
		case "no priv":				PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Channel_FailedToJoinChannelNoPrivate>", "None", 'JoinChannelCanceled');			break;
		case "failed":
		default:					PopUpMessage(EPT_Error, "<Strings:AVANET.UiPopUpMsg.Msg_Channel_FailedToJoinChannel>", "None", 'JoinChannelCanceled');
		}
		break;

	case EMsg_Channel_Leave:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelLeave',Param1);
		switch (Param1)
		{
		case "ok":				break;
		case "time out":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToLeaveChannelTimeOut>", "Exit");	break;
		case "failed":
		default:				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToLeaveChannel>", "Exit");
		}
		break;

	case EMsg_Channel_RoomList:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelRoomList',Param1);
		switch (Param1)
		{
		case "ok":				break;
		case "done":			/*GameObj.UpdateLobbyRoomList();*/			break;
		case "selected":		/*GameObj.UpdateLobbySelectedRoom();*/		break;
		case "time out":		break;
		case "failed":
		default:				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGetRoomList>", "None");
		}
		break;

	case EMsg_Channel_RoomInfo:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelRoomInfo',Param1);
		switch (Param1)
		{
		case "ok":				break;
		case "failed":
		default:				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGetRoomInfo>", "None");
		}
		break;

	case EMsg_Channel_PlayerList:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelPlayerList',Param1);
		switch (Param1)
		{
		case "ok":				break;
		case "done":			/*GameObj.UpdateLobbyPlayerList();*/			break;
		case "selected":		/*GameObj.UpdateLobbySelectedPlayer();*/		break;
		case "time out":		break;
		case "failed":
		default:				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGetPlayerList>", "None");
		}
		break;

	case EMsg_Channel_PlayerInfo:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelPlayerInfo',Param1);
		switch (Param1)
		{
		case "ok":				/*GameObj.UpdateLobbySelectedPlayer();*/			break;
		case "time out":		break;
		case "failed":
		default:				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGetPlayerInfo>", "None");
		}
		break;

	case EMsg_Channel_LobbyJoin:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelLobbyJoin',Param1);
		/*GameObj.AddLobbyPlayerList(Param3);*/
		break;

	case EMsg_Channel_LobbyLeave:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelLobbyLeave',Param1);
		/*GameObj.RemoveLobbyPlayerList(Param3);*/
		break;

	case EMsg_Channel_LobbyChat:
		//class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_OnChatMessage',Param1);
		//class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelLobbyChat',Param1);
		/*GameObj.Chat(Param1, Param2);*/
		break;

	case EMsg_Channel_RoomState:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelRoomState',Param1);
		/*GameObj.UpdateRoomState(Param3);*/
		break;

	case EMsg_Channel_RoomSetting:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelRoomSetting',Param1);
		/*GameObj.UpdateRoomSetting(Param3);*/
		break;

	case EMsg_Channel_RoomCreate:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelRoomCreate',Param1);
		/*GameObj.AddRoomList(Param3);*/
		break;

	case EMsg_Channel_RoomDelete:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelRoomDelete',Param1);
		/*GameObj.RemoveRoomList(Param3);*/
		break;

	case EMsg_Channel_FollowPlayer:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcChannelFollowPlayer',Param1);
		switch(Param1)
		{
			case "ok": break;
			case "need password": break;	//! 암호 걸린 방인 경우.
			case "failed": break;
		}
		break;
	}
}

event ProcRoomMessage(EMsgRoom Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local class<GameInfo>	GameClass;

	`log("ProcRoomMessage(" @Msg @", " @Param1 @", " @Param2 @", " @Param3 @", " @Param4);

	GameClass = GetGameInfoClass();
	if( ClassIsChildOf(GameClass, NetEntryGameClass) )
		ProcRoomMessageNet(Msg, Param1,Param2,Param3,Param4);
	else if ( ClassIsChildOf(GameClass, TeamGameClass) || ClassIsChildOf(GameClass, DeathMatchGameClass) )
		ProcRoomMessageGame(Msg, Param1, Param2, Param3, Param4);
	else
		`warn("invalid game class"@GameClass@" @ ProcAdminMessage("@Msg@Param1@Param2@Param3@Param4@")");
}

event ProcRoomMessageNet(EMsgRoom Msg, string Param1, string Param2, int Param3, int Param4)
{
	local avaNetEntryGame	GameObj;
	Local PlayerController PC;
	Local avaGameViewportClient ViewportClient;

	GameObj = avaNetEntryGame(GetWorldInfo().Game);

	PC = GetLocalPC();
	if( PC != None )
		ViewportClient = avaGameViewportClient(LocalPlayer(PC.Player).ViewportClient);

	// 대기실, 채널리스트, 로비 등등등
	switch (Msg)
	{
	// 방생성에 관련한 응답...
	case EMsg_Room_Create:
		switch (Param1)
		{
		case "ok":				GameObj.GotoScene("Room");			break;
		case "invalid name":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToCreateRoomInvalidName>",	"None", 'All_LobbyRoomButtons_Enabled' );	break;
		case "invalid map":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToCreateRoomInvalidMap>",	"None", 'All_LobbyRoomButtons_Enabled' );	break;
		case "time out":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToCreateRoomTimeOut>",		"None", 'All_LobbyRoomButtons_Enabled' );	break;
		case "already created":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToCreateRoomAlreadyCreated>","None", 'All_LobbyRoomButtons_Enabled' );	break;
		case "no priv":			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToCreateRoomNoPrivate>",		"None", 'All_LobbyRoomButtons_Enabled' );	break;
		default:				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToCreateRoom>",				"None", 'All_LobbyRoomButtons_Enabled' );	break;
		}
		break;

	// 방참가에 관련한 응답...
	case EMsg_Room_Join:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcRoomJoin', Param1);
		switch (Param1)
		{
		case "ok":				GameObj.GotoScene("Room");	break;
		case "notify":			GameObj.AddRoomPlayerList(Param3);	break;
		case "time out":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToJoinRoomTimeOut>", "None", 'All_LobbyRoomButtons_Enabled' );		break;
		case "full":			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToJoinFull>", "None", 'All_LobbyRoomButtons_Enabled' );	break;
		case "already joined":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToJoinRoomAlreadyJoined>", "None", 'All_LobbyRoomButtons_Enabled' );	break;
		case "invalid level":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToJoinInvalidLevel>", "None", 'All_LobbyRoomButtons_Enabled' );		break;
		case "invalid password":PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToJoinInvalidPassword>", "None", 'All_LobbyRoomButtons_Enabled' );	break;
		case "invalid clan":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToJoinInvalidClan>", "None", 'All_LobbyRoomButtons_Enabled' );	break;
		case "kicked":			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToJoinKicked>", "None", 'All_LobbyRoomButtons_Enabled' );	break;
		case "not found":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToJoinNotFound>", "None", 'All_LobbyRoomButtons_Enabled' );			break;
		default:				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToJoinRoom>", "None", 'All_LobbyRoomButtons_Enabled' );				break;
		}
		break;

	case EMsg_Room_Leave:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcRoomLeave', Param1);
		switch (Param1)
		{
		case "ok":				/*if( IsInClanLobby() )
									GameObj.GotoScene("ClanLobby");
								else
									GameObj.GotoScene("Lobby");*/	break;
		case "notify":			GameObj.RemoveRoomPlayerList(Param3);	break;
		case "time out":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToLeaveRoomTimeOut>", "Exit");	break;
		default:				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToLeaveRoom>", "Exit");
		}
		break;

	case EMsg_Room_Info:
		switch (Param1)
		{
		case "ok":				GameObj.UpdateRoomPlayerList();			break;
		case "room name":		break;
		case "invalid":			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToGetRoomInfoInvalid>", "None");	break;
		default:				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToGetRoomInfo>", "None");
		}
		break;

	case EMsg_Room_PlayerList:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcRoomPlayerList',Param1);
		//if (Param1 == "selected")
		//{
		//	GameObj.UpdateRoomSelectedPlayer();
		//}
		break;

	case EMsg_Room_PlayerInfo:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcRoomPlayerInfo',Param1);
		switch (Param1)
		{
		case "ok":
			//GameObj.UpdateRoomSelectedPlayer();
			break;
		//case "time out":
		//	break;
		//case "failed":
		//default:
		//	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToGetPlayerInfo>", "None");
		}
		break;

	case EMsg_Room_Chat:
		// GameObj.Chat(Param1, Param2);
		break;

	case EMsg_Room_ChangeHost:
		if( GameObj != None )
			GameObj.RoomChangeHost(Param3);
		else if( ViewportClient != None )
			ViewportClient.SetHostMigration(true);
		// 호스트 마이그레이션인지의 판단은 여기와 avaNetEntryGameEx.PostLogin 에서 한다.


		break;

	case EMsg_Room_Kick:

		if( Param1 == "you" )
		{
			switch (Param2)
			{
			//case "banned":				ResolvedMessage = Localize("UIPopUpMsg","Msg_Room_Kicked_Banned","avaNet");	break;
			//case "loading time out":	ResolvedMessage = Localize("UIPopUpMsg","Msg_Room_Kicked_LoadingTimeOut","avaNet");	break;
			//case "setting changed":		ResolvedMessage = Localize("UIPopUpMsg","Msg_Room_Kicked_SettingChanged","avaNet");	break;
			//case "room destroyed":		ResolvedMessage = Localize("UIPopUpMsg","Msg_Room_Kicked_RoomDestroyed","avaNet");	break;
			//case "lag":					ResolvedMessage = Localize("UIPopUpMsg","Msg_Room_Kicked_Lag","avaNet");	break;
			//case "didn't ready":		ResolvedMessage = Localize("UIPopUpMsg","Msg_Room_Kicked_Ready","avaNet");	break;
			//case "invalid level":		ResolvedMessage = Localize("UIPopUpMsg","Msg_Room_Kicked_InvalidLevel","avaNet");	break;
			//case "ready in 30sec":		ChatMessage(Localize("UIPopUpMsg","Msg_Room_Kicked_ReadyIn30","avaNet"), EChat_Notice );	return;
			}
			//if( Len(ResolvedMessage) > 0 )
			//	PopUpMessage(EPT_Notice, ResolvedMessage, "None" );

			// 메세지처리를 키즈멧 안으로 옮김
			class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcRoomKick',Param2);
		}
		else if ( Param1 == "notify" )
		{
			GameObj.RoomKick(Param3);
			class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcRoomKick',Param1);
		}
		else if ( Param1 == "not enough fee" )
			PopUpMessage(EPT_Notice, "<Strings:avaNet.UIPopUpMsg.Msg_Room_Kicked_NotEnoughFee>", "None" );

		break;
		
	case EMsg_Room_ChangeSetting:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcRoomChangeSetting',Param1);
		switch (Param1)
		{
		case "ok":
		case "notify":		/*GameObj.RoomChangeSetting();*/		break;
		case "time out":	break;
		case "failed":
		default:			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToChangeSettings>", "None");
		}
		break;

	case EMsg_Room_ChangeState:
		GameObj.RoomChangeState();
		break;

	case EMsg_Room_Ready:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcRoomReady');
		GameObj.RoomReady(Param3, Param4);
		break;

	case EMsg_Room_ChangeSlot:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcRoomChangeSlot',Param1);
		switch (Param1)
		{
		case "ok":					GameObj.RoomChangeTeam(Param3, Param4);																	break;
		case "invalid slot":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToChangeSlot_InvalidSlot>", "None", 'avaUIEvent_OnReadyRoomButton');	break;
		case "invalid team":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToChangeSlot_InvalidTeam>", "None", 'avaUIEvent_OnReadyRoomButton');	break;
		case "no empty slot":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToChangeSlot_NoEmptySlot>", "None", 'avaUIEvent_OnReadyRoomButton');	break;
		case "slot is not empty":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_SlotToChangeIsNotEmpty>", "None", 'avaUIEvent_OnReadyRoomButton');			break;
		case "no priv":				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToNoPrivate>", "None", 'avaUIEvent_OnReadyRoomButton');				break;	
		default:					PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Room_FailedToChangeSlot>", "None", 'avaUIEvent_OnReadyRoomButton');				break;	
		}
		break;

	case EMsg_Room_ChangeClass:
		GameObj.RoomChangeClass(Param3);
		break;

	case EMsg_Room_ChangeWeapon:
		GameObj.RoomChangeWeapon(Param3);
		break;

	case EMsg_Room_RttRating:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcRoomRttRating',Param1);
		break;

	case EMsg_Room_RtttStart:
		break;

	case EMsg_Room_SwapTeam:
		break;

	case EMsg_Room_QuickJoin:
		switch (Param1)
		{
		case "time out":
			break;
		}
		break;
	case EMsg_Room_ClanInfo:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcRoomClanInfo',Param1);
		break;
	}
}

event ProcRoomMessageGame(EMsgRoom Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local UIInteraction		UIController;
	Local array<name>		ExclSceneTags;
	Local avaGame			GameObj;

	if ( avaNetEntryGame( GetWorldInfo().Game ) == None )
	{
		GameObj = avaGame(GetWorldInfo().Game);
	}
	if( GameObj == none )
	{
		`warn("SWGameObj not found @ ProcGameMessageGame("@Msg@Param1@Param2@Param3@Param4@")");
	}

	UIController = GetUIController();
	switch( Msg )
	{
	case EMsg_Room_ChangeHost:
		if( UIController != none )
		{
			OpenSceneManagedName( HostMigrationSceneName );
			//class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcRoomChangeHost',Param1);
		}
		break;
	case EMsg_Room_Kick:
		if( Param1 == "you" )
		{
			// 씬 이동처리 (Kick당하면 로비로 이동한다) + 메세지처리
			ExclSceneTags.AddItem('Result');
			AddPendingEvent(class'avaUIEvent_ProcRoomKick',NetEntryGameClass,,ExclSceneTags,Param2);
		}
		break;
	case EMsg_Room_SwapTeam:
		if (Param1 == "ok" && GameObj != none)
		{
			GameObj.SwapTeam();
		}
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////
/// ProcInventoryMessage

event ProcInventoryMessage(EMsgInventory Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local class<GameInfo>	GameClass;

	GameClass = GetGameInfoClass();
	if( ClassIsChildOf(GameClass, NetEntryGameClass) )
		ProcInventoryMessageNet(Msg, Param1,Param2,Param3,Param4);
	else if ( ClassIsChildOf(GameClass, TeamGameClass) || ClassIsChildOf(GameClass, DeathMatchGameClass) )
		ProcInventoryMessageGame(Msg, Param1, Param2, Param3, Param4);
	else
		`warn("invalid game class"@GameClass@" @ ProcInventoryMessage("@Msg@Param1@Param2@Param3@Param4@")");

	`log( "ProcNetMessage(" @Msg @"," @Param1 @"," @Param2 @"," @Param3 @"," @Param4 @")" );
}


function ProcInventoryMessageNet(EMsgInventory Msg, string Param1, string Param2, int Param3, int Param4)
{
	local avaNetEntryGame GameObj;
	Local array<Name>	BaseSceneTags;

	GameObj = avaNetEntryGame(GetWorldInfo().Game);
	if (GameObj == None)
		return;

	switch (Msg)
	{
	case EMsg_Inventory_EquipSet:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcInvenEquipSet',Param1);
		switch (Param1)
		{
		case "ok":
			GameObj.InventoryEquipSet();
			break;
		case "invalid item":
			break;
		case "same item":
			break;
		case "set default":
			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_SetDefaultEquip>", "None");
			GameObj.InventoryEquipSet();
			break;
		case "low level":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToSetEquipLowLevel>", "None");
			break;
		case "cannot unset default":
			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_CannotUnsetDefaultEquip>", "None");
			break;
		case "time out":
			break;
		case "failed":
		default:
			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToSetEquip>", "None");
		}
		break;

	case EMsg_Inventory_WeaponSet:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcInvenWeaponSet',Param1);
		switch (Param1)
		{
		case "ok":
			GameObj.InventoryWeaponSet();
			break;
		case "invalid item":
			break;
		case "same item":
			break;
		case "set default":
			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_SetDefaultWeapon>", "None");
			GameObj.InventoryWeaponSet();
			break;
		case "low level":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToSetWeaponLowLevel>", "None");
			break;
		case "cannot unset default":
			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_CannotUnsetDefaultWeapon>", "None");
			break;
		case "time out":
			break;
		case "failed":
		default:
			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToSetWeapon>", "None");
		}
		break;

	case EMsg_Inventory_CustomSet:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcInvenCustomSet',Param1);
		switch (Param1)
		{
		case "ok":
			GameObj.InventoryWeaponSet(true);
			break;
		case "set default":
			GameObj.InventoryWeaponSet(true);
			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_SetDefaultCustom>","",'WeaponCustom_CloseAskBuyingScene');
			break;
		case "empty inventory slot":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToBuyCustomEmptySlot>","",'WeaponCustom_CloseAskBuyingScene');		break;
		case "invalid item":			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToBuyCustomInvalidItem>","",'WeaponCustom_CloseAskBuyingScene');	break;
		case "full":					PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToBuyCustomFull>","",'WeaponCustom_CloseAskBuyingScene');			break;
		case "no money":				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToBuyCustomNoMoney>","",'WeaponCustom_CloseAskBuyingScene');		break;
		case "no cash":					PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToBuyCustomNoMoney>","",'WeaponCustom_CloseAskBuyingScene');		break;
		case "low level":				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Shop_FailedToBuyLowLevel>", "None",'Weaponinventory_CloseAskBuyingScene');			break;
		case "time out":				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Msg_Inventory_FailedToBuyCustomTimeOut>","",'WeaponCustom_CloseAskBuyingScene');	break;
		case "processing":				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICProcessing>","",'WeaponCustom_CloseAskBuyingScene');				break;
		case "failed":
		default:						PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToBuyCustomFailed>", "None", 'WeaponCustom_CloseAskBuyingScene');	break;
		}
		break;

	case EMsg_Inventory_EffSet:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcInvenEffectSet',Param1);
		switch(Param1)
		{
			case "ok":
				break;
			case "set default":
				break;

			case "invalid item":
				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToUseInvalidItem>", "None");
				break;
			case "same item":
				break;

			case "failed":
			default:
				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToUseEffectItem>", "None");
				break;
		}
		break;

	case EMsg_Inventory_Buy:
//		GameObj.ShopBuy( Param1 == "ok" );
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcInvenBuy',Param1);
		switch (Param1)
		{
		case "ok":			break;
		case "full":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Shop_FailedToBuyFull>", "None",'Weaponinventory_CloseAskBuyingScene');			break;
		case "no money":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Shop_NoMoney>", "None",'Weaponinventory_CloseAskBuyingScene');					break;
		case "no cash":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Shop_NoMoney>", "None",'Weaponinventory_CloseAskBuyingScene');					break;
		case "low level":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Shop_FailedToBuyLowLevel>", "None",'Weaponinventory_CloseAskBuyingScene');		break;
		case "invalid":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Shop_InvalidItem>", "None",'Weaponinventory_CloseAskBuyingScene');				break;
		case "time out":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToBuyTimeOut>", "None",'Weaponinventory_CloseAskBuyingScene');	break;
		case "processing":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICProcessing>","",'Weaponinventory_CloseAskBuyingScene');		break;
		case "failed":
		default:			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Shop_FailedToBuy>", "None",'Weaponinventory_CloseAskBuyingScene');
		}
		break;

	case EMsg_Inventory_Repair:
//		GameObj.RepairWeapon( Param1 == "ok" );
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcInvenRepair',Param1);
		switch (Param1)
		{
		case "ok":				break;
		case "empty slot":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToRepairEmptySlot>", "None",'Weaponinventory_CloseAskRepairingScene');			break;
		case "invalid item":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToRepairInvalidItem>", "None",'Weaponinventory_CloseAskRepairingScene');			break;
		case "cannot repair":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToRepairCannotRepair>", "None", 'Weaponinventory_CloseAskRepairingScene');			break;
		case "no money":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToRepairNoMoney>", "None", 'Weaponinventory_CloseAskRepairingScene');			break;
		case "already repaired":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToRepairNoNeedToRepair>", "None", 'Weaponinventory_CloseAskRepairingScene');			break;
		case "time out":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToRepairTimeOut>", "None",'Weaponinventory_CloseAskRepairingScene');			break;
		case "failed":			
		default:				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToRepairFailed>", "None",'Weaponinventory_CloseAskRepairingScene');			break;
		}
		break;

	case EMsg_Inventory_ConvertRIS:
//		GameObj.ConvertRIS( Param1 == "ok" );
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcInvenConvertRIS',Param1);
		switch (Param1)
		{
		case "ok":				break;
		case "invalid slot":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToConvertInvalidSlot>", "None", 'Weaponinventory_CloseAskConvertingScene');			break;
		case "no convertible":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToConvertNoConvertable>", "None", 'Weaponinventory_CloseAskConvertingScene');			break;
		case "no money":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToConvertNoMoney>", "None", 'Weaponinventory_CloseAskConvertingScene');			break;
		case "no money":			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToConvertNoMoney>", "None", 'Weaponinventory_CloseAskConvertingScene');			break;
		case "time out":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToConvertTimeOut>", "None", 'Weaponinventory_CloseAskConvertingScene');			break;
		case "failed":			
		default:				PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToConvertFailed>", "None",'Weaponinventory_CloseAskConvertingScene');			break;
		}
		break;
		
	case EMsg_Inventory_Sell:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcInvenSell',Param1);
		switch (Param1)
		{
		case "ok":				break;
		case "empty slot":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToSellEmptySlot>", "None");	break;
		case "invalid item":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToSellInvalidItem>", "None");	break;
		case "set default":		if (Param2 == "weapon") PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToSellSetDefaultWeapon>", "None");
								else if (Param2 == "equip") PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToSellSetDefaultEquip>", "None");
								break;
		case "cannot sell":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToSellCannotSell>", "None");	break;
		case "failed":			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToSell>", "None");	break;
		}
		break;
	case EMsg_Inventory_DeleteItem:
		BaseSceneTags = PersistentEntrySceneTags;
		AddPendingEvent( class'avaUIEvent_ProcInvenDeleteItem',, BaseSceneTags,, Param1, Param3);
		break;
	case EMsg_Inventory_DeleteCustom:
		BaseSceneTags = PersistentEntrySceneTags;
		AddPendingEvent( class'avaUIEvent_ProcInvenDeleteCustom',, BaseSceneTags,, Param1, Param3);
		break;
	case EMsg_Inventory_DeleteEffect:
		BaseSceneTags = EntrySceneTags;
		AddPendingEvent( class'avaUIEvent_ProcInvenDeleteEffect',, BaseSceneTags,, Param1, Param3);
		break;

	case EMsg_Inventory_UpdateGauge:
		// 인벤토리의 어느 화면이라도 나올 수 있습니다.
		// 말 그대로 아이템의 게이지가 업데이트 되었다는 이벤트이구요. 
		// 기간제 아이템을 중복 구매했을 때, 아이템이 추가되지는 않고 
		// 기존 아이템에 기간이 추가되면서 발생하게 됩니다.
		// 화면에 있는 모든 아이템 슬롯들을 refresh.
		break;

	/*! @brief 캐쉬 아이템 처리로 추가된 부분.(2007/12/06)
		@note Param1
			"wic inactive"
				런처가 죽어 있으니, 게임 다시 실행하라고 메시지를 띄우면 됩니다.
				(요청을 했으나 런처가 죽었거나 해서 WebInClient 요청이 불가능한 경우)
			"wic working"
				잠시 후 다시 해보거나, 아니면 게임을 다시 실행하고 해보라고 메시지를 띄우면 됩니다.
				(이미 다른 어떤 WiC 요청이 진행 중일 때)
			"processing"
				이미 아이템 구매가 진행 중이며, 시간이 오래 걸리면 게임을 다시 실행하고, 
				이전 구매가 잘 처리되었는지 확인해보라고 메시지를 띄우면 됩니다.
				(중복 구매 시)

			유료 결제는 꽤나 민감한 사안인지라, 최대한 팍팍하고 까다롭게 처리해야 할 듯 합니다.(동관님曰)
	*/
	case EMsg_Inventory_Charge:
		`log("EMsg_Inventory_Charge" @Param1);
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcInvenCharge',Param1);
		switch(Param1)
		{
			case "ok": break;	// WIC윈도우가 닫이는 경우 발생하는 이벤트.

			case "wic inactive":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICInvative>", "None");	break;
			case "wic working":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICWorking>", "None");	break;
		}
		break;
	case EMsg_Inventory_GetCash:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcInvenGetCash',Param1);
		`log("EMsg_Inventory_GetCash" @Param1);
		switch(Param1)
		{
			case "ok": break;
			case "failed": break;
//			case "bad web result":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICBadWebResult>", "None");	break;	// 웹 오작동

//			case "wic inactive":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICInvative>", "None");	break;
//			case "wic working":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICWorking>", "None");	break;
		}
		break;
	case EMsg_Inventory_EffBuy:
		class'avaEventTrigger'.static.ActivateNetEventByClass(class'avaUIEvent_ProcInvenEffectBuy',Param1, Param2, Param3, Param4);
		`log("EMsg_Inventory_EffBuy" @Param1);
		switch(Param1)
		{
			case "ok": break; // 결제 후 게임 서버에서 아이템을 지급받고 나면 발생하는 이벤트.
			case "failed":			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToBuyCashItemFailed>", "None");	break;
			case "invalid":			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToBuyCashItemInvalid>", "None");	break;

			case "web ok": break; // cash = param3
			case "web failed":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedToBuyCashItemWebFailed>", "None");	break;

			case "bad web result":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICBadWebResult>", "None");	break;	// 웹 오작동
			case "wic inactive":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICInvative>", "None");	break;
			case "wic working":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICWorking>", "None");	break;
			case "processing":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICProcessing>", "None");	break;
		}
		break;
	// 아직 구현되지 않아서 사용하지 않을 것이다.
	case EMsg_Inventory_SendGift:
		`log("EMsg_Inventory_SendGift" @Param1);
		switch(Param1)
		{
			case "ok": break;

			case "wic inactive":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICInvative>", "None");	break;
			case "wic working":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICWorking>", "None");	break;
			case "processing":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Inventory_FailedWICProcessing>", "None");	break;
		}
		break;
	}
}

function ProcInventoryMessageGame(EMsgInventory Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local array<Name>	BaseSceneTags;
	switch(Msg)
	{
	case EMsg_Inventory_DeleteItem:
		BaseSceneTags = PersistentEntrySceneTags;
		AddPendingEvent( class'avaUIEvent_ProcInvenDeleteItem',, BaseSceneTags,, Param1, Param3);
		break;
	case EMsg_Inventory_DeleteCustom:
		BaseSceneTags = PersistentEntrySceneTags;
		AddPendingEvent( class'avaUIEvent_ProcInvenDeleteCustom',, BaseSceneTags,, Param1, Param3);
		break;
	case EMsg_Inventory_DeleteEffect:
		BaseSceneTags = EntrySceneTags;
		AddPendingEvent( class'avaUIEvent_ProcInvenDeleteEffect',, BaseSceneTags,, Param1, Param3);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
/// ProcGameMessage ( Net, Game )

event ProcGameMessage(EMsgGame Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local class<GameInfo>	GameClass;

	GameClass = GetGameInfoClass();
	if( ClassIsChildOf(GameClass, NetEntryGameClass) )
	{
		`log( "ProcGameMessageNet(" @Msg @"," @Param1 @"," @Param2 @"," @Param3 @"," @Param4 @")" );
		ProcGameMessageNet(Msg, Param1,Param2,Param3,Param4);
	}
	else if ( ClassIsChildOf(GameClass, TeamGameClass) || ClassIsChildOf(GameClass, DeathMatchGameClass) )
	{
		`log( "ProcGameMessageGame(" @Msg @"," @Param1 @"," @Param2 @"," @Param3 @"," @Param4 @")" );
		ProcGameMessageGame(Msg, Param1, Param2, Param3, Param4);
	}
	else
	{
		AddPendingMsg(EMsg_Game, Msg, none, Param1, Param2, Param3, Param4);
		`warn("invalid game class"@GameClass@" @ ProcAdminMessage("@Msg@Param1@Param2@Param3@Param4@")");
	}

	//if ( Parm1 == EMsg_Game_LoadingProgress && Param3 == Host && Param4 == 100 )
	//{
	//	if ( Param3 == Host )
	//	{
	//		
	//		if ( Param4 == 100 )
	//		{
	//			SetTimer( false, 5.0f, "RoomOut" );
	//		}
	//	}
	//}

	if ( Msg == EMsg_Game_LoadingProgress )
	{
		if ( Param4 > 90 )
		{
			`log( "EMsg_GameLoadingProgress" @Param1 @Param3 @GetHostAccountID() @Param4 );
		}
	}
	HostLoadingProgress = Param4;
}

function RoomOut()
{
	class'avaEventTrigger'.static.ActivateEventByName( 'RoomOut' );
}

function ProcGameMessageNet(EMsgGame Msg, string Param1, string Param2, int Param3, int Param4)
{
	local avaNetEntryGame GameObj;
//	local int				i;
	Local array<Name>		BaseSceneTags;

	GameObj = avaNetEntryGame(GetWorldInfo().Game);
	if( GameObj == None )
	{
		`warn("there's no accetable game object @ ProcGameMessageNet("@Msg@Param1@Param2@Param3@Param4@")");
		return;
	}

	switch (Msg)
	{
	case EMsg_Game_Start:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGameStart', Param1);
		if (Param1 == "failed")
		{
			switch( Param2 )
			{
			case "UnbalanceWhenStart":		OpenSceneManagedName("ReadyRoom.Popup_UnbalanceWhenStart", NetEntryGameClass);	break;
			case "UnbalanceWhenInterrupt":	OpenSceneManagedName("ReadyRoom.Popup_UnbalanceWhenInterrupt", NetEntryGameClass);	break;
			case "TotalMinPlayers":
			case "EachTeamMinPlayers":
				if( GetCurrentMapMissionType() != 2/* NMT_MilitaryDrill */ )
					PopUpMessage(EPT_Error, "<strings:avaNET.UIPopUpMsg.Msg_Game_FailedToStartGame_EachTeamMinPlayers>", "None", '');
				else
					PopUpMessage(EPT_Error, "<strings:avaNET.UIPopUpMsg.Msg_Game_FailedToStartGame_EachTeamMinPlayers_MilitaryDrill>", "None", '');
				break;
			case "InvalidRoom":				PopUpMessage(EPT_Error, "<strings:avaNET.UIPopUpMsg.Msg_Game_FailedToStartGame_InvalidRoom>", "None", '');	break;
			case "NeedToReady":				PopUpMessage(EPT_Error, "<strings:avaNET.UIPopUpMsg.Msg_Game_FailedToStartGame_NeedToReady>", "None", '');	break;
			case "DontAllowInterrupt":		PopUpMessage(EPT_Error, "<strings:avaNET.UIPopUpMsg.Msg_Game_FailedToStartGame_DontAllowInterrupt>", "None", '');	break;
			default:						PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Game_FailedToStartGame>", "None", '');	break;
			}
			class'avaEventTrigger'.static.ActivateEventByName('ReadyRoom_EnableRequestButton');
		}
		else
		{
			if ( AmIHost() == false )
				GetRoomStartPlayerList( StartPlayerIDList );

			if (GameObj != None)
				GameObj.GameStarted(Param3);
		}
		break;

	case EMsg_Game_StartCount:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGameStartCount');
		GameObj.GameCountDown(true);		
		ChatMessage("");	/*simply refresh*/	break;
	case EMsg_Game_CancelCount:		
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGameCancelCount');
		GameObj.GameCountDown(false);
		ChatMessage("");	/*simply refresh*/	break;
	case EMsg_Game_LoadingProgress:
		switch (Param1)
		{
		// 이벤트를 생성해도 어차피 avaNetEntryGameEx가 새로 만들어져 로그인 하기때문에 
		// 무슨짓을 해도 Scene이 닫히면서 없어진다.
		//case "client time out":	class'avaEventTrigger'.static.ActivateEventByName('GameLoading_ClientTimeOut');	break;
		//case "client package mismatch": class'avaEventTrigger'.static.ActivateEventByName('GameLoading_ClientPackageMismatch');	break;
		//case "client package not found": class'avaEventTrigger'.static.ActivateEventByName('GameLoading_ClientPackageNotFound');	break;
		case "rejected by host":			PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_RejectedByHost>", "None",);	break;
		case "md5 failed":					PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_Md5Failed>", "None",);	break;
		case "client failed to connect":	PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_ClientFailedToConnect>", "None",);	break;
		case "p2p connection failed":	PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_P2PConnectionFailed>", "None",);	break;
		case "client time out":			PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_LoadingTimeOut>", "None",);	break;
		case "client package mismatch":	PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_PackageVersionMismatch>", "None",);	break;
		case "client package not found": PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_PackageNotFound>", "None",);	break;
		case "skip check":	break;
		case "progress":	break;
		case "complete":
		case "cancel":		/* SWGameObj.PlayerReady( Param3 ); ProcGameMessageGame에서 처리*/
			if ( Param2 == "me" )
			{
				StartPlayerIDList.length = 0;
			}

			`log( "EMsg_GameLoadingProgress" @Param1 @Param3 @GetHostAccountID() );
			// Host 가 Loading 이 끝났으면 RoomOut Event 를 호출 해 준다....
			// 여전히 Loading Scene 이라면 5초후 RoomOut 을 한다....
			if ( Param1 == "complete" && GetHostAccountID() == Param3 )
			{
				RoomOut();
			}
			break;
		}
		//if ( SWGameObj != None )
		//{
		//	if ( Param4 >= 100 )
		//	{
		//		SWGameObj.PlayerReady( Param3 );
		//	}
		//}
		// Param3 == PlayerIndex ; eg. StartingPlayerList[Param3].Nickname
		// Param4 == progress perc.; same as StartingPlayerList[Param3].LoadingProgress
		// if (Param4 == 100) loading completed
		break;

	case EMsg_Game_Leave:
		if (GameObj != None)
			GameObj.UpdateRoomPlayerList();
		break;

	case EMsg_Game_ResultUpdate:
		if ( GameObj != None )
			GameObj.UpdateRoomResult(Param1);

		//class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGameResultUpdate', Param1);
		// 정비 필요 무기창은 결과창에 나와서는 안된다.
		BaseSceneTags = Param1 == "item" ? PersistentEntrySceneTags : EntrySceneTags;
		AddPendingEvent( class'avaUIEvent_ProcGameResultUpdate',,BaseSceneTags,, Param1);
		
		switch (Param1)
		{
		case "level":
			//PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Game_LevelUp>\n" @ GetPlayerLevelName(Param3), "None");
			break;
		case "supply":
			//PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Game_SupplyRecved>\n (" $ Param3 $ ")", "None");
			break;
		case "money":
			//PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Game_MoneyEarned>\n $" $ Param3, "None");
			break;
		case "award":
			//PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Game_AwardRecved>\n" @ GetAwardName(Param3), "None");
			break;
		case "skill":
			//PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Game_SkillUpdate>\n" @ GetSkillName(Param3, Param4), "None");
			break;
		case "player result":
			break;
		case "room result":
			break;

		// 혹시나 보급의 결과로 부스터 아이템이 지급된다면 다음 이벤트가 발생합니다.
		// 물론 현재는 사용하지 않으며, 사용될 가능성이 없어보이긴 합니다.(2007/12/11)
		case "effect item received":
			//! empty (itemName=param2,itemID=param3)
			break;
		}
		break;
		
	case EMsg_Game_Chat:
		ChatMessage(Param1 @ ":" @ Param2);
		break;

	// 훈련병 탈출 축하 아이템 지급.(2007/12/11)
	case EMsg_Game_LevelUpGrats:
		BaseSceneTags = PersistentEntrySceneTags;
		AddPendingEvent( class'avaUIEvent_ProcGameLevelUp',,BaseSceneTags,, Param1);
		`log("GameMessageNet - avaUIEvent_ProcGameLevelUp" @Param1);
//		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGameLevelUp',Param1);
		break;
	}
}

function ProcGameMessageGame(EMsgGame Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local avaGame				GameObj;
	Local array<Name>			BaseSceneTags;
	Local avaPlayerController	PC;

	PC = avaPlayerController(GetLocalPC());

	switch (Msg)
	{
	case EMsg_Game_Start:
		if (Param1 == "failed")
			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Game_FailedToStartGame>", "None", 'ReadyRoom_EnableRequestButton');
		break;

	case EMsg_Game_StartCount:		break;
	case EMsg_Game_CancelCount:		break;
	case EMsg_Game_LoadingProgress:
		switch (Param1)
		{
		case "rejected by host":			PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_RejectedByHost>", "None",);	break;
		case "md5 failed":					PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_Md5Failed>", "None",);	break;
		case "client failed to connect":	PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_ClientFailedToConnect>", "None",);	break;
		case "p2p connection failed":		PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_P2PConnectionFailed>", "None",);	break;
		case "client time out":				break; //PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_LoadingTimeOut>", "None",);	break;
		case "client package mismatch":		PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_PackageVersionMismatch>", "None",);	break;
		case "client package not found":	PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Room_Kicked_PackageNotFound>", "None",);	break;
		case "progress":	break;
		case "skip check":
		case "complete":
		case "cancel":
			GameObj = avaGame(GetWorldInfo().Game);
			if( GameObj != None )
				GameObj.PlayerReady( Param3 );
			break;
		}
		//if ( SWGameObj != None )
		//{
		//	if ( Param4 >= 100 )
		//	{
		//		SWGameObj.PlayerReady( Param3 );
		//	}
		//}
		// Param3 == PlayerIndex ; eg. StartingPlayerList[Param3].Nickname
		// Param4 == progress perc.; same as StartingPlayerList[Param3].LoadingProgress
		// if (Param4 == 100) loading completed
		break;

	case EMsg_Game_Leave:
		break;

	case EMsg_Game_ResultUpdate:
		//class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGameResultUpdate', Param1);
		
		// 정비 필요 무기창은 결과창에 나와서는 안된다.	
		BaseSceneTags = Param1 == "item" ? PersistentEntrySceneTags : EntrySceneTags;
		AddPendingEvent( class'avaUIEvent_ProcGameResultUpdate',,BaseSceneTags,, Param1);

		switch (Param1)
		{
		case "level":	break;
		case "supply":	break;
		case "money":	break;
		case "award":	break;
		case "skill":	break;
		case "player result":		break;
		case "room result":			break;
		case "effect item received": break;
		}
		break;
	case EMsg_Game_Chat:
		avaHUD(PC.MyHUD).AddEmergencyChatMessage(Param2, Param3, bool(Param4));
		break;

	// 훈련병 탈출 축하 아이템 지급.(2007/12/11)
	case EMsg_Game_LevelUpGrats:
		BaseSceneTags = PersistentEntrySceneTags;
		AddPendingEvent( class'avaUIEvent_ProcGameLevelUp',,BaseSceneTags,, Param1);
		`log("GameMessageGame - avaUIEvent_ProcGameLevelUp" @Param1);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
/// ProcAdminMessage ( Net, Game )

event ProcAdminMessage(EMsgAdmin Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local class<GameInfo>		GameClass;

	GameClass = GetGameInfoClass();
	if( ClassIsChildOf(GameClass, NetEntryGameClass) )
		ProcAdminMessageNet(Msg, Param1,Param2,Param3,Param4);
	else if ( ClassIsChildOf(GameClass, TeamGameClass) || ClassIsChildOf(GameClass, DeathMatchGameClass) )
		ProcAdminMessageGame(Msg, Param1, Param2, Param3, Param4);
	else
		`warn("invalid game class"@GameClass@" @ ProcAdminMessage("@Msg@Param1@Param2@Param3@Param4@")");
}

function ProcAdminMessageNet( EMsgAdmin Msg, string Param1, string Param2, int Param3, int Param4 )
{
	local avaNetEntryGame GameObj;
	GameObj = avaNetEntryGame(GetWorldInfo().Game);
	if( GameObj == None )
	{
		`warn("there's no accetable game object @ ProcAdminGame("@Msg@Param1@Param2@Param3@Param4@")");
		return;
	}

	switch (Msg)
	{
	case EMsg_Admin_Notice:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcAdminNotice',Param1);

//		GameObj.UpdateRealTimeNotice();
//		AddChatMsg( Param1, EChat_Notice);
//		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcCommonChat');
//		GameObj.Chat("", Param1);

		ChatMessage( Param1, EChat_Notice );
		break;
	case EMsg_Admin_Kick:
		switch (Param1)
		{
		case "you":
			PopUpMessage(EPT_Notice, "<Strings:AVANET.UIPopUpMsg.Msg_Admin_KickedFromGame>", "Exit");
			break;
		case "player":

			//AddChatMsg( Param2 @ Localize("UIMiscScene", "Text_KickedFromGame_Other", "AVANET"), EChat_ReadyRoom);
			//GameObj.Chat("", Param2);

			ChatMessage( Param2 @ Localize("UIMiscScene", "Text_KickedFromGame_Other", "AVANET"), EChat_ReadyRoom);
			/*Param2 is kiced from game*/
			break;
		}
	case EMsg_Admin_ChatOff:
		switch (Param1)
		{
		case "you":
			//AddChatMsg( Localize("UIMiscScene", "Text_Chat_BlockedByAdmin", "AVANET"), EChat_PlayerSystem);
			ChatMessage( Localize("UIMiscScene", "Text_Chat_BlockedByAdmin", "AVANET"), EChat_PlayerSystem);

			GameObj.Chat(Param1, Param2);
			break;
		case "blocked":
			//AddChatMsg( Localize("UIMiscScene", "Text_Chat_CannotChatBecauseBlocked", "AVANET"), EChat_ReadyRoom);
			//GameObj.Chat(Param1, Param2);
			
			ChatMessage( Localize("UIMiscScene", "Text_Chat_CannotChatBecauseBlocked", "AVANET"), EChat_ReadyRoom);
			break;
		}
		break;
	case EMsg_Admin_ChangeRoomName: break;
	}
}

function ProcAdminMessageGame( EMsgAdmin Msg, String Param1, string Param2, int Param3, int Param4 )
{
	Local avaPlayerController PC;
	PC = avaPlayerController(GetLocalPC());

	if( PC == None )
	{
		`warn("Can't find a PlayerController @ ProcAdminGame("@Msg@Param1@Param2@Param3@Param4@")");
		return;
	}

	switch (Msg)
	{
	case EMsg_Admin_Notice:
			avaHUD(PC.MyHUD).UpdateRTNotice(Param1);
		break;
	case EMsg_Admin_ChatOff:
			PC.NotifyChatOff();
		break;
	case EMsg_Admin_Kick:
		if( Param1 == "you" )
			ChatMessage(Localize("UIPopUpMsg","Msg_Room_Kicked_Admin","avaNet"), EChat_Notice);
		else
			ChatMessage(Param2@Localize("UIPopUpMsg","Msg_Room_Kicked_NotifyOthers","avaNet"), EChat_InGameSystem );
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
/// ProcBuddyMessage

event ProcBuddyMessage(EMsgBuddy Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local class<GameInfo>	GameClass;

	GameClass = GetGameInfoClass();
	if( ClassIsChildOf(GameClass, NetEntryGameClass ) )
		ProcBuddyMessageNet(Msg, Param1, Param2, Param3, Param4);
	else if ( ClassIsChildOf(GameClass, TeamGameClass) || ClassIsChildOf(GameClass, DeathMatchGameClass) )
		ProcBuddyMessageGame(Msg, Param1, Param2, Param3, Param4);
	else
		`warn("invalid game class @ ProcBuddyMessage("@Msg@Param1@Param2@Param3@Param4@")");

	`log( "ProcBuddyMessage(" @Msg @"," @Param1 @"," @Param2 @"," @Param3 @"," @Param4 @")" );
}

function ProcBuddyMessageNet(EMsgBuddy Msg, string Param1, string Param2, int Param3, int Param4)
{
	local avaNetEntryGame GameObj;
	Local array<string> PackedStrParms;

	GameObj = avaNetEntryGame(GetWorldInfo().Game);
	if( GameObj == None )
	{
		`warn("there's no accetable game object @ ProcBuddyMessageNet("@Msg@Param1@Param2@Param3@Param4@")");
		return;
	}

	switch (Msg)
	{
	case EMsg_Buddy_ConsoleCommand:
	case EMsg_Buddy_Chat:
		//switch (CheckMyLocation())
		//{
		//case 1:
		//	ProcChannelMessage(EMsg_Channel_LobbyChat, Param1, Param2, Param3, Param4);
		//	break;
		//case 2:
		//	ProcRoomMessage(EMsg_Room_Chat, Param1, Param2, Param3, Param4);
		//	break;
		//}
		break;
	case EMsg_Buddy_AddReq:
		PackedStrParms.AddItem(Param1);
		PackedStrParms.AddItem(Param2);
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcBuddyAddReq',class'avaStringHelper'.static.PackString(PackedStrParms));
		break;
	case EMsg_Buddy_AddReqAns:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcBuddyAddReqAns',Param2, ,Param1 == "accepted" );
		break;
	case EMsg_Buddy_Delete:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcBuddyDelete',Param2, ,Param1 == "accepted" );
		break;
	case EMsg_Buddy_AddBlock:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcBuddyAddBlock',Param1);
		break;
	case EMsg_Buddy_DeleteBlock:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcBuddyDeleteBlock',Param1);
		break;
	case EMsg_Buddy_InviteGame:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcBuddyInviteGame',Param1);
		break;
	case EMsg_Buddy_InviteGuild:
		PackedStrParms.AddItem(Param1);
		PackedStrParms.AddItem(Param2);
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcBuddyInviteGuild',class'avaStringHelper'.static.PackString(PackedStrParms));
		break;
	case EMsg_Buddy_Location:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcBuddyLocation',Param1);
		break;
	case EMsg_Buddy_StateChanged:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcBuddyStateChanged',Param2, ,Param1 == "connected");
		break;
	case EMsg_Buddy_List:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcBuddyList',Param1);
		switch (Param1)
		{
		case "ok":				break;
		case "selected":		break;
		case "block selected":	break;
		}
		break;
	case EMsg_Buddy_PlayerInfo:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcBuddyPlayerinfo',Param1);
		switch (Param1)
		{
		case "ok":			break;
		case "failed":		break;
		}
		break;
	case EMsg_Buddy_EndFollow:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcBuddyEndFollow',Param1);
		switch (Param1)
		{
		case "channel list":	break;
		case "lobby":			break;
		case "room":			break;
		}
		break;
	}
}

function ProcBuddyMessageGame(EMsgBuddy Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local avaPlayerController PC;
	Local array<Name> InclSceneTags;
	Local array<string> PackedStrParms;
	PC = avaPlayerController(GetLocalPC());

	if( PC == None )
	{
		`warn("Can't find a PlayerController @ ProcBuddyMessageGame("@Msg@Param1@Param2@Param3@Param4@")");
		return;
	}

	switch ( Msg )
	{
	case EMsg_Buddy_ConsoleCommand:
	case EMsg_Buddy_Chat:
		//avaHUD(PC.MyHUD).NotifyBuddyMessage( Param1, Param2 );
		break;
	case EMsg_Buddy_AddReq:
		PackedStrParms.AddItem(Param1);
		PackedStrParms.AddItem(Param2);
		InclSceneTags = PersistentEntrySceneTags;
		AddPendingEvent(class'avaUIEvent_ProcBuddyAddReq',,InclSceneTags,, class'avaStringHelper'.static.PackString(PackedStrParms));
		break;
	case EMsg_Buddy_AddReqAns:
		break;
	case EMsg_Buddy_InviteGuild:
		PackedStrParms.AddItem(Param1);
		PackedStrParms.AddItem(Param2);
		InclSceneTags = PersistentEntrySceneTags;
		AddPendingEvent(class'avaUIEvent_ProcBuddyInviteGuild',,InclSceneTags,, class'avaStringHelper'.static.PackString(PackedStrParms));
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////
/// ProcGuildMessage

event ProcGuildMessage(EMsgGuild Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local class<GameInfo>	GameClass;

	GameClass = GetGameInfoClass();
	if( ClassIsChildOf(GameClass, NetEntryGameClass ) )
		ProcGuildMessageNet(Msg, Param1, Param2, Param3, Param4);
	else if ( ClassIsChildOf(GameClass, TeamGameClass) || ClassIsChildOf(GameClass, DeathMatchGameClass) )
		ProcGuildMessageGame(Msg, Param1, Param2, Param3, Param4);
	else
		`warn("invalid game class @ ProcGuildMessage("@Msg@Param1@Param2@Param3@Param4@")");

	`log( "ProcGuildMessage(" @Msg @"," @Param1 @"," @Param2 @"," @Param3 @"," @Param4 @")" );
}

function ProcGuildMessageNet(EMsgGuild Msg, string Param1, string Param2, int Param3, int Param4)
{
	local avaNetEntryGame GameObj;
	Local string ResolvedMessage;

	GameObj = avaNetEntryGame(GetWorldInfo().Game);
	if( GameObj == None )
	{
		`warn("there's no accetable game object @ ProcGuildMessageNet("@Msg@Param1@Param2@Param3@Param4@")");
		return;
	}

	switch (Msg)
	{
	case EMsg_Guild_Info:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGuildInfo',Param1);
		break;
	case EMsg_Guild_MemberList:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGuildMemberList',Param1);
		switch (Param1)
		{
		case "ok":				break;
		case "selected":		break;
		}
		break;
	case EMsg_Guild_JoinChannel:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGuildJoinChannel',Param1);
		switch (Param1)
		{
		case "ok":				break;
		case "no guild":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGuildJoinChannelNoGuild>", "");	break;
		case "not connected":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGuildJoinChannelNotConnected>", "" );	break;
		case "not regular":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGuildJoinChannelNotRegular>", "" );	break;
		case "failed":			PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGuildJoinChannelFailed>", "" );	break;
		case "time out":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGuildJoinChannelTimeout>","" );	break;
		}
		break;
	case EMsg_Guild_LeaveChannel:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGuildLeaveChannel',Param1);
		switch (Param1)
		{
		case "ok":				break;
		case "time out":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGuildLeaveChannelTimeout>","" );	break;
		}
		break;

	case EMsg_Guild_Motd:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGuildMotd',Param1);		
		switch (Param1)
		{
		case "ok":				break;
		case "no guild":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGuildJoinChannelNoGuild>", "");	break;
		case "not connected":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGuildJoinChannelNotConnected>", "" );	break;
		case "no auth":			break;//PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGuildJoinChannelNotConnected>", "" );	break;
		}
		break;
	case EMsg_Guild_Chat:
		//class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGuildChat',Param1);
		//switch (CheckMyLocation())
		//{
		//case 1:
		//	ProcChannelMessage(EMsg_Channel_LobbyChat, Param1, Param2, Param3, Param4);
		//	break;
		//case 2:
		//	ProcRoomMessage(EMsg_Room_Chat, Param1, Param2, Param3, Param4);
		//	break;
		//}
		break;
	case EMsg_Guild_Notice:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGuildNotice',Param1);
		switch (Param1)
		{
		case "no guild":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGuildJoinChannelNoGuild>", "");	break;
		case "not connected":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGuildJoinChannelNotConnected>", "" );	break;
		case "no auth":			break;//PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Channel_FailedToGuildJoinChannelNotConnected>", "" );	break;
		default:				break;
		}
		break;
	case EMsg_Guild_MemberState:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGuildMemberState',Param1);
		break;
	case EMsg_Guild_PlayerInfo:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGuildPlayerInfo',Param1);
		switch (Param1)
		{
		case "ok":				break;
		case "failed":			break;
		default: break;
		}
		break;
	case EMsg_Guild_Kick:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGuildKick',Param1);
		if( Param1 != "you" )
		{
			switch ( Param1 )
			{
			case "not connected":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Guild_FailedToKickClanNotConnected>", "");	break;
			case "no guild":		PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Guild_FailedToKickClanNoGuild>", "");	break;
			case "member not found":	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Guild_FailedToKickClanMemberNotFound>", "");	break;
			case "failed":
			default:	PopUpMessage(EPT_Error, "<Strings:AVANET.UIPopUpMsg.Msg_Guild_FailedToKickClanMember>", "");	break;
			}
		}
		break;
	case EMsg_Guild_RankChanged:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGuildRankChanged',Param1);
		break;
	case EMsg_Guild_MasterChanged:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGuildMasterChanged',Param1,Param3);
		ResolvedMessage = Localize("UIPopUpMsg","Msg_Channel_GuildMasterChanged","avaNet");
		PopUpMessage(EPT_Notice, class'avaStringHelper'.static.Replace(Param1, "%s", ResolvedMessage), "");
		break;
	case EMsg_Guild_UpdateScore:
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcGuildUpdateScore');
		break;
	}
}

function ProcGuildMessageGame(EMsgGuild Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local avaPlayerController PC;
	Local array<name> ExclSceneTags;
	PC = avaPlayerController(GetLocalPC());

	if( PC == None )
	{
		`warn("Can't find a PlayerController @ ProcGuildMessageGame("@Msg@Param1@Param2@Param3@Param4@")");
		return;
	}

	switch (Msg)
	{
	case EMsg_Guild_Info:
		break;
	case EMsg_Guild_MemberList:
		switch (Param1)
		{
		case "ok":				break;
		case "selected":		break;
		}
		break;
	case EMsg_Guild_JoinChannel:
		switch (Param1)
		{
		case "ok":				break;
		case "no guild":		break;
		case "not connected":	break;
		case "failed":			break;
		case "time out":		break;
		}
		break;
	case EMsg_Guild_LeaveChannel:
		switch (Param1)
		{
		case "ok":				break;
		case "time out":		break;
		}
		break;
	case EMsg_Guild_Motd:
//		avaHUD(PC.MyHUD).NotifyBuddyMessage( Param1, Param2 );
		break;
	case EMsg_Guild_Chat:
//		avaHUD(PC.MyHUD).NotifyBuddyMessage( Param1, Param2 );
		break;
	case EMsg_Guild_Notice:
//		avaHUD(PC.MyHUD).NotifyBuddyMessage( Param1, Param2 );
		break;
	case EMsg_Guild_MemberState:
		break;
	case EMsg_Guild_PlayerInfo:
		switch (Param1)
		{
		case "ok":				break;
		case "failed":			break;
		}
		break;
	case EMsg_Guild_Kick:
		ExclSceneTags.AddItem('Result');
		AddPendingEvent(class'avaUIEvent_ProcGuildKick',NetEntryGameClass,,ExclSceneTags, Param1);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// ProcOptionMessage

event ProcOptionMessage(EMsgOption Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local class<GameInfo>	GameClass;

	GameClass = GetGameInfoClass();
	if( ClassIsChildOf(GameClass, NetEntryGameClass ) )
		ProcOptionMessageNet(Msg, Param1, Param2, Param3, Param4);
	//else if ( ClassIsChildOf(GameClass, TeamGameClass) || ClassIsChildOf(GameClass, DeathMatchGameClass) )
	//	ProcGuildMessageGame(Msg, Param1, Param2, Param3, Param4);
	else
		`warn("invalid game class @ ProcGuildMessage("@Msg@Param1@Param2@Param3@Param4@")");

	`log( "ProcOptionMessage(" @Msg @"," @Param1 @"," @Param2 @"," @Param3 @"," @Param4 @")" );
}

function ProcOptionMessageNet(EMsgOption Msg, string Param1, string Param2, int Param3, int Param4)
{
	Local array<string> PackedString;

	switch( Msg )
	{
	case EMsg_Option_SettingChanged:
		switch( Param1 )
		{
		case "Resolution":
			PackedString.AddItem(Param1);
			PackedString.AddItem(Param2);
			class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcOptionSettingChanged', class'avaStringHelper'.static.PackString(PackedString), ,bool(Param3));
			break;
		case "AudioChannel":	
			PopUpMessage(EPT_Warning, "<Strings:avaNet.UIOptionScene.Text_AudioChannelChanged>", "");
			break;
		case "ShaderModel":
			PopUpMessage(EPT_Warning, "<Strings:avaNet.UIOptionScene.Text_ShaderModelAppliedLater>", "");
			break;
		case "WorldShadow":
			PopUpMessage(EPT_Warning, "<Strings:avaNet.UIOptionScene.Text_WorldShadowAppliedLater>", "");
			break;
		case "LoadMapCache":
			PopUpMessage(EPT_Warning, "<Strings:avaNet.UIOptionScene.Text_LoadMapCacheAppliedLater>", "");
			break;
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Error handling

event ProcSendError(EMsgCategory Category, int id, string Err)
{
	class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcErrorSend', Err);
	if (Err == "no connection")
	{
		PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Send_NoConnection>", "Exit");
		return;
	}
	//else if (Err == "invalid state")
	//{
	//	PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Send_InvalidState>", "Exit");
	//	return;
	//}
	//else if (Err == "host only")
	//{
	//	PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Send_HostOnly>","");
	//	return;
	//}
	//else if (Err == "pending any")
	//{
	//	PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Send_PendingAny>","");
	//}
	//else if (Err == "pending same")
	//{
	//	PopUpMessage(EPT_Warning, "<Strings:AVANET.UIPopUpMsg.Msg_Send_PendingSame>","");
	//}

	switch (Category)
	{
	case EMsg_Client:
		switch (id)
		{
		case EMsg_Client_CheckNick:
			break;
		}
		break;
	case EMsg_Channel:
		switch (id)
		{
		case EMsg_Channel_Whisper:
			switch (Err)
			{
			case "not you":				break;	// 자신에게 보낼 수는 없음
			case "wrong msg length":	break;
			}
			break;
		}
		break;
	case EMsg_Game:
		switch (id)
		{
		case EMsg_Game_ResultUpdate:
			switch (Err)
			{
			case "no player":		break;
			}
			break;
		}
		break;
	case EMsg_Admin:
		if (Err == "admin only")
		{
			return;
		}
		switch (id)
		{
		case EMsg_Admin_ChatOff:
			switch (Err)
			{
			case "wrong name length":		break;
			}
			break;
		case EMsg_Admin_ChangeRoomName:
			switch (Err)
			{
			case "wrong name length":		break;
			}
			break;
		case EMsg_Admin_MainNotice:
			switch (Err)
			{
			case "wrong msg length":		break;
			}
			break;
		case EMsg_Admin_Whisper:
			switch (Err)
			{
			case "wrong msg length":		break;
			}
			break;
		}
		break;
	case EMsg_Guild:
		if (Err == "no guild channel")
		{
			return;
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// items

function class<avaCharacterModifier> GetCharacterModifier(int idItem)
{
	return class<avaCharacterModifier>(class'avaMod_TranslatorBase'.static.GetModifier(CharacterModifierList, idItem));
}

function class<avaMod_Weapon> GetWeaponModifier(int idItem)
{
	return class<avaMod_Weapon>(class'avaMod_TranslatorBase'.static.GetModifier(WeaponModifierList, idItem));
}

/*! @brief 해당 무기 클래스와 장착된 무기 클래스 리스트를 얻어온다.(2007/02/09 고광록)
	@param ItemSN
		아이템의 시리얼 넘버.
	@return
		소지한 무기 클래스와 장착 무기 클래스 리스트.
		[0]은 기본 무기 클래스이고, [1..n]은 장착 무기 클래스이다.
	@note
		avaPlayerModifierInfo.uc에서 참고함.
*/
function array< class<avaMod_Weapon> > GetWeaponModifiers( string ItemSN )
{
	local array< class<avaMod_Weapon> >	Modifiers;
	local class<avaMod_Weapon>			Mod;
	local string						Code;
	local string						Value;
	local string						Remainder;

	// ItemSN에 장착된 BaseWeapon과 CustomWeapon들의 id를 얻어온다.
	// 각 id의 값을 avaMod_Weapon클래스로 변경해서 배열을 만들어 주고 리턴한다.

	// 해당 무기의 ItemID리스트를 문자열로 얻어온다.
	Code = GetMyURLString( "WeaponSN" $ ItemSN );

	`log("### GetWeaponModifiers - Code : " @Code );

	// 문자열로 얻어온 CustomWeapon ID들을 파싱해서 Modifier로 변환하여 리스트에 추가한다.
	while( class'avaMod_TranslatorBase'.static.GrabCode( ";", Code, Value, Remainder ) )
	{
		Mod = class<avaMod_Weapon>(class'avaMod_TranslatorBase'.static.GetModifier( WeaponModifierList, int(Value) ));
		if ( Mod != None )
		{
			`log("### GetWeaponModifiers - FoundModifier[" @Modifiers.Length @"] = " @Mod);
			Modifiers[Modifiers.Length] = Mod;
		}
	}

	return Modifiers;
}

function class<avaWeaponAttachment> GetWeaponAttachment(int idItem)
{
	local class<avaMod_Weapon> ModClass;

	ModClass = class<avaMod_Weapon>(class'avaMod_TranslatorBase'.static.GetModifier(WeaponModifierList, idItem));
	if (ModClass != None)
	{
		return ModClass.default.WeaponClass.default.AttachmentClass;
	}

	return None;
}

event string GetWeaponIconCode(int idItem)
{
	local class<avaWeaponAttachment> AttachClass;

	AttachClass = GetWeaponAttachment(idItem);
	if (AttachClass != None)
	{
		return AttachClass.default.DeathIconStr;
	}

	return "";
}


/////////////////////////////////////////////////////////////////////////////////////////////

//function GetRoomStartingPlayerList(out array<avaRoomPlayerInfo> PlayerList)
//{
//	PlayerList = RoomStartingPlayerList;
//}
native final function GetRoomStartPlayerList(out array<INT> IDList);

final function PlayerController GetLocalPC()
{
	Local WorldInfo WorldInfo;
	Local PlayerController PC, LocalPC;
	WorldInfo = GetWorldInfo();
	if( WorldInfo == None )
		return None;

	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		LocalPC = PC;
	}
	return LocalPC;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// host migration

event ProcHMEndGame(bool bHost)
{
	local WorldInfo WorldInfo;
	local avaGameReplicationInfo GRI;
	local array<avaPlayerScoreInfo>	psiArray;

	`log( "avaNetHandler.ProcHMEndGame()" );

	WorldInfo = GetWorldInfo();
	GRI = avaGameReplicationInfo(WorldInfo.GRI);
	if (GRI == None)
	{
		`log( "avaNetHandler.ProcHMEndGame(): GRI not found!!" );
		psiArray.length = 0;
		EndGame(psiArray);
		return;
	}

	//if (bHost)
	//{
		`log( "avaNetHandler.ProcHMEndGame(): Generating results" );
		GRI.GenerateResult( psiArray );
		EndGame(psiArray);

		GRI.bReportEndGame = true;
		//GRI.ReportGameLog();
	//}
}


/////////////////////////////////////////////////////////////////////////////////////////////
// client information

native final function bool IsPlayerAdult();
native final function string GetConnectResult();

native final function int CheckMyLocation();

native final function bool AmIAdmin();
native final function bool IsStealthMode();
native final function bool IsInClanLobby();

native final function EChannelFlag GetCurrentChannelFlag();
native final function int GetChannelSetting(EChannelSetting Setting);

native final function bool IsInPcBang();

native final function string GetConfigString();
native final function string GetConfigString2();

/////////////////////////////////////////////////////////////////////////////////////////////
// list requests

native final function ListChannel();
native final function ListRoom();

/////////////////////////////////////////////////////////////////////////////////////////////
// in-game requests

native final function UpdatePlayerScore(avaPlayerScoreInfo ScoreInfo);
native final function UpdateGameState(int RoundCount, ENetGameState GameState = EGS_None);
native final function EndGame(array<avaPlayerScoreInfo> ScoreInfoList, int AvgHostPing = -1);
native final function ReportGameResult(array<avaPlayerScoreInfo> ScoreInfoList, int TeamScoreEU = -1, int TeamScoreNRF = -1);
native final function ReportEndGame(int AvgHostPing = -1);
native final function LeaveGame();
native final function DisconnectFromGame(int NextState);
native final function VoteForHostBan();

native final function ReportGameStat(out avaStatLog StatLog);

native final function bool IsGameResultValid(bool bFull = true);

// vote kick --> ReportVote(EVC_Kick, Caller_Account_ID, Victim_Account_ID, EVoteKickReason)
native final function ReportVoteNew(EVoteCommand Command, int idCaller, array<int> VoterList, int Param1, int Param2);
native final function bool IsVoteAvailable();

native final function ProcHostCrash();

native final function SwapTeamInGame();

event SendPlayerResult(int idAccount)
{
	local WorldInfo World;
	local avaGameReplicationInfo GRI;

	if (AmIHost())
	{
		World = GetWorldInfo();
		if (World != None && World.WorldInfo != None && World.WorldInfo.GRI != None)
		{
			GRI = avaGameReplicationInfo(World.WorldInfo.GRI);
			if (GRI != None)
			{
				GRI.GetPlayerResult(idAccount);
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////
// chatting

native final function AddChatMsg(string ChatMsg, EChatMsgType MsgType = EChat_Normal);
native final function bool FilterChatMsg(out string ChatMsg);
native final function bool ParseChatCommand( string cmd );

event ChatMessage(string ChatMsg, EChatMsgType MsgType = EChat_Normal, bool InChannel = true, bool InGame = true)
{
	// compose chatting message using MsgType
	Local class<GameInfo>	GameClass;
	Local bool				bIsInGame;
	Local bool				bIsInNet;
	Local PlayerController	PC;
	Local string			NewMsg;
	Local array<string>		WrappedStr;
	Local string			WrappedUnit;

	GameClass = GetGameInfoClass();
	bIsInNet = ClassIsChildOf(GameClass, NetEntryGameClass );
	bIsInGame = ClassIsChildOf(GameClass, TeamGameClass) || ClassIsChildOf(GameClass, DeathMatchGameClass);

	`log( "ChatMessage(" @MsgType @","@ ChatMsg @"," @ InChannel @"," @ InGame @")" );

	NewMsg = Localize("ChatTypeLabel", "Text_ChatPrefix["$int(MsgType)$"]", "avaNet") $ ChatMsg;

	if (InChannel && bIsInNet)
	{
		// append the message to channel message list
		if( Len(ChatMsg) > 0 )
		{
			GetWrappedString(WrappedStr, NewMsg ,MsgType , 410);
			foreach WrappedStr( WrappedUnit )
				AddChatMsg( WrappedUnit, MsgType);
		}
		// refresh
		class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_ProcCommonChat');
	}

	PC = GetLocalPC();
	if (InGame && bIsInGame && PC != None)
	{
		avaHUD(PC.MyHUD).AddTypedMessage( NewMsg, MsgType );
		// add the message to hud
	}
}

native function GetWrappedString( out array<string> out_strings, string inMsg, int MsgType, int BoundSize = 300);

/////////////////////////////////////////////////////////////////////////////////////////////
// player states

native final function string GetMyNickname();
native final function string GetMyClanName();
native final function int GetMyAccountID();
native final function EChannelFlag GetMyBestChannelFlag();
native final function int GetCurrentRoomState();
native final function bool GetCurrentEquipState(out int MyTeam, out int MyClass, out int MyFace, out int MyWeapon);
native final function int GetHostAccountID();
native final function bool AmIHost();
native final function bool AmISpectator();
native final function bool AmIReady();
native final function bool IsCountingDown();
native final function bool IsGameStartable(bool bCheckHost = true);
native final function bool IsGameStartableEx(out string ErrorTypeStr, bool bCheckHost = true);
native final function bool IsBalancedRoomPlayers( out string ErrorTypeStr );
native final function bool IsSpectatorAllowed();
native final function string GetCurrentMapFileName();
native final function byte GetCurrentMapMissionType();
native final function int GetMyRoomSlot();
native final function int GetPlayerRoomSlot(int idAccount);
native final function bool GetRoomInfo( int RoomListIndex, out int RoomID, out int nPassword, out string RoomName, out string HostName );
native final function bool IsMatchRoom();

native final function int GetCurrentChannelMaskLevel(int idAccount = 0);

native final function string GetURLString(int idAccount, string Option = "");
native final function string GetMyURLString(string Option = "");

native final function bool IsPlayerInGame(int idAccount);

native final function bool GetPlayerInfo(out byte Level, out byte LastClass, out byte LastTeam, out int exp, out int SupplyPoint, out int Cash, out int Money, out int idClanMark);
native final function int GetPlayerTeamIndex(int idAccount);
native final function int GetClanMarkID(int idAccount);

native final function bool DoIHaveItem(int ListIndex);
native final function bool DoIHaveCustomItem(int ListIndex);
native final function bool DoIHaveCustomItemInSlot(int InvenSlot, int SlotIdx);

native final function bool HaveFriendPlayerNamed( string FriendPlayerName, optional out int ListIndex, optional out int BuddyType );
native final function bool HaveBlockedPlayerNamed( string BlockedPlayerName, optional out int ListIndex );
native final function bool HaveClanMemberNamed( string BlockedPlayerName, optional out int ListIndex );
native final function bool IsBIAPlayer( string PlayerName );
native final function bool HasOwnClan( string PlayerName, optional out string ItsOwnClanName);

native final function bool GetLobbyPlayerName( int LobbyPlayerListIndex, out string PlayerName );
native final function bool GetFriendPlayerName( int BuddyListIndex, out string PlayerName );
native final function bool GetBlockedPlayerName( int BlockListIndex, out string PlayerName );

native final function bool GetSelectedLobbyPlayerInfo( out string NickName, out string GuildName, out int Level, out int WinCount, out int DefeatCount, out int DisconnectCount, out int KillCount, out int DeathCount );
native final function bool GetSelectedFriendPlayerInfo( out string NickName, out string GuildName, out int Level, out int WinCount, out int DefeatCount, out int DisconnectCount, out int KillCount, out int DeathCount );
native final function bool GetSelectedBlockedPlayerInfo( out string NickName, out string GuildName, out int Level, out int WinCount, out int DefeatCount, out int DisconnectCount, out int KillCount, out int DeathCount );
native final function bool GetSelectedGuildPlayerInfo( out string NickName, out string GuildName, out int Level, out int WinCount, out int DefeatCount, out int DisconnectCount, out int KillCount, out int DeathCount );

native final function bool CheckWeaponRefundCond( int InvenSlot );
native final function bool CheckEquipRefundCond( int InvenSlot );
native final function bool CheckCustomRefundCond( int InvenSlot, int CustomSlotIndex );

native final function string CheckRoomKickedState();
native final function string GetLastWhisperedPlayerName();
native final function int GetBIAAccountID();


/////////////////////////////////////////////////////////////////////////////////////////////
// item-related utilities

native final function int GetAvailableWeaponsBySlot(int idSlot, out array<int> ItemList);
native final function int GetAvailableEquipsBySlot(int idSlot, out array<int> ItemList);
native final function int GetAvailableEffectsBySlot(int idSlot, out array<int> ItemList);

native final function string GetWeaponName(int idItem);

native final function string GetSkillName(byte PlayerClass, int SkillID);
native final function string GetAwardName(int AwardID);
native final function string GetPlayerLevelName(int PlayerLevel);
native final function bool GetItemDesc(int itemId, out string ItemName, out byte LevelLimit, out byte GaugeType, out int Price, out int RepairPrice, out int RebuildPrice, out string LiteralDesc, out string IconCode, out int Customizable);
native final function bool GetEffectItemDesc( int ItemID, out int GaugeType, out int EffectType, out int EffectValue, out string ItemName, out string ItemDesc, out string IconStr );
native final function bool GetItemId(byte IndexType, int ListIndex, out int ItemID);
native final function bool GetCustomItemID( int InvenSlot, int CustomSlotIndex, out int ItemID );
native final function bool GetWeaponRIS(int InvenSlot, out byte bIsRISConvertible, out int RemodelPrice);
native final function bool GetWeaponRepairInfo(int InvenSlot, out int RepairPrice, out byte bAfford);
native final function bool GetEquipRepairInfo(int InvenSlot, out int RepairPrice, out byte bAfford);
native final function bool GetCustomCompInfo( int InvenSlot, int CustomListIndex, out int AlterSlot,out int AlterItemID, out int CompPrice );	// Get Compensation Info , 무기 커스터마이징 아이템의 보상 관련

/////////////////////////////////////////////////////////////////////////////////////////////
// web-in-client related utilities

native final function bool WICGetCash();
native final function bool WICOpenChargeWindow();
native final function bool WICBuyItem(int idItem);
native final function bool WICSendGift(int idItem, int idAccountTo);
native final function bool WICOpenGiftWindow(int idItem);

/////////////////////////////////////////////////////////////////////////////////////////////
/// Transactions

native final function int BeginTransaction( string SessionName );
native final function bool UndoTransaction();
native final function int EndTransaction();
native final function GetTransactionObjects( out array<Object> OutObjects );

/////////////////////////////////////////////////////////////////////////////////////////////
// misc

native final function bool ProcCountDown();

native final function ClearRTNotice();

native final function int GetCurrentTickerCount();
native final function float GetChatOffDue();

native final function bool CanMyClanJoinSelectedRoom();
native final function GetCurrentRoomsClanNames(out string EUName, out string NRFName);

native final function bool IsPendingPopUpMsg();
//native final function PushPopUpMsg(EPopUpMsgType MsgType, string PopUpMsg, string NextScene, optional name NextUIEventName );
//native final function bool PopFirstPopUpMsg(out avaPopUpMsgInfo Info);
//native final function ClearPopUpMsg();

native final function OptionSaveUserKey( string UserKeyStr, string OptionStr = "" );

native final function WorldInfo GetWorldInfo();
native final function class<GameInfo> GetGameInfoClass();
native final function int GetNetVersion();

//native final function AddDummyPlayer( string DummyPlayerName, bool bReady, byte TeamID );

/////////////////////////////////////////////////////////////////////////////////////////////
// pop up message

final function UIInteraction GetUIController()
{
	Local WorldInfo WorldInfo;
	Local PlayerController PC;
	Local UIInteraction UIController;

	WorldInfo = GetWorldInfo();

	if( WorldInfo == None )
		return none;

	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		if ( LocalPlayer(PC.Player) == None )
			continue;

		if ( LocalPlayer(PC.Player).ViewportClient == None )
			continue;

		UIController = LocalPlayer(PC.Player).ViewportClient.UIController;
		return UIController;
	}
	return none;
}

final function UISceneClient GetSceneClient()
{
	Local WorldInfo WorldInfo;
	Local PlayerController PC;
	Local UIInteraction UIController;

	WorldInfo = GetWorldInfo();

	if( WorldInfo == None )
		return none;

	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		UIController = LocalPlayer(PC.Player).ViewportClient.UIController;
		if( UIController != None && UIController.SceneClient != None )
			return UIController.SceneClient;
	}
	return none;
}

function array<UIScene> GetActiveScenes()
{
	Local GameUISceneClient SceneClient;
	Local array<UIScene> ActiveScenes;

	SceneClient = GameUISceneClient(GetSceneClient());
	if( SceneClient != None )
		ActiveScenes = SceneClient.ActiveScenes;

	return ActiveScenes;
}

final function _OpenSceneManaged( PendingSceneInfo SceneInfo )
{
	PendingSceneList.Add(1);
	PendingSceneList[ PendingSceneList.Length - 1 ] = SceneInfo;
}

final function OpenSceneManagedName( string ScenePathName, class BaseGameClass = class'GameInfo', EScenePriorityType Priority = EScenePrior_UIScene_Normal, optional array<avaNetEventParam> EventParams, optional array<Name> ExclSceneTags, optional float LifeTime )
{
	Local UIScene Scene;

	Scene = UIScene(DynamicLoadObject(class'GameInfo'.static.GetFullUIPath(ScenePathName), class'UIScene'));
	if( Scene == none )
	{
		`Log("avaNetHandler.OpenSceneManagedTag.DynamicLoadObject() - failed");
		return;
	}

	OpenSceneManaged( Scene, BaseGameClass, Priority, EventParams, ExclSceneTags, LifeTime);
}


final function OpenSceneManaged( UIScene SceneToOpen, class BaseGameClass = class'GameInfo', EScenePriorityType Priority = EScenePrior_UIScene_Normal, optional array<avaNetEventParam> EventParams, optional array<Name> ExclSceneTags, optional float LifeTime )
{
	Local int Length;
	Length = PendingSceneList.Length;

	PendingSceneList.Add(1);
	PendingSceneList[Length].UIScene = SceneToOpen;
	PendingSceneList[Length].BaseGameClass = class<GameInfo>(BaseGameClass);
	PendingSceneList[Length].Priority = Priority;
	PendingSceneList[Length].EventParams = EventParams;
	PendingSceneList[Length].LifeTime = LifeTime;
	PendingSceneList[Length].ExclSceneTags = ExclSceneTags;

	UpdatePendingScenes();
}

final function CloseSceneManaged( UIScene SceneToClose)
{
	Local GameUISceneClient SceneClient;
	SceneClient = GameUISceneClient(GetSceneClient());
	if( SceneClient != none )
		SceneClient.CloseScene( SceneToClose );
}

function bool CheckSceneOpenCondition( INT PendingSceneListIndex )
{
	Local WorldInfo WorldInfo;
	Local UIInteraction UIController;
	Local bool bResult;
	Local name SceneTag;
	Local array<Name> ExclSceneTags;

	WorldInfo = GetWorldInfo();
	UIController = GetUIController();
	if( WorldInfo == none || UIController == none || 
		!( 0 <= PendingSceneListIndex && PendingSceneListIndex < PendingSceneList.Length ) )
		return false;

	bResult = true;
	bResult = !bResult || ClassIsChildOf(GetGameInfoClass(), PendingSceneList[PendingSceneListIndex].BaseGameClass);
	bResult = !bResult || (UIController.FindSceneByTag(PendingSceneList[PendingSceneListIndex].UIScene.SceneTag) == None);

	if( bResult && PendingSceneList[ PendingSceneListIndex ].ExclSceneTags.Length > 0 )
	{
		ExclSceneTags = PendingSceneList[ PendingSceneListIndex ].ExclSceneTags;
		foreach ExclSceneTags( SceneTag )
		{
			if( UIController.FindSceneByTag(SceneTag) != none )
				bResult = false;
		}
	}

	return  bResult;
}

function UpdatePendingScenes()
{
	Local int i, EventIndex, ActivatedEventIndex;
	Local UIInteraction UIController;
	Local WorldInfo WorldInfo;
	Local PlayerController PC, LocalPC;
	Local UIScene UISceneInstance;
	Local avaNetEventParam EventParam;
	Local array<UIEvent> ActivatedEvents;

	UIController = GetUIController();
	WorldInfo = GetWorldInfo();
	if( UIController == none || WorldInfo == none)
		return;

	foreach WorldInfo.LocalPlayerControllers(LocalPC)
	{
		PC = LocalPC;
	}

	if( PC == none )
		return;

	// Scene리스트에 들어온 순서대로 처리해줘야한다.
	for( i = 0 ; i < PendingSceneList.Length ; i++ )
	{
		if( PendingSceneList[i].UIScene == None )
		{
			PendingSceneList.Remove(i,1);
			i--;
			continue;
		}

		// GameClass가 Pending상태의 Scene과 맞는 GameClass인지 확인하고 ( eg. avaSWGame에서만 열어야되는 씬, avaNetEntryGame에서만 열어야되는씬 등등)
		// 현재 같은 씬이 열려있지 않을때 새 씬을 연다. ( 같은 팝업 창을 동시에 두개 열 수는 없다. )
		if( CheckSceneOpenCondition( i ) )
		{
			UIController.OpenScene(PendingSceneList[i].UIScene, ,UISceneInstance, PendingSceneList[i].Priority);
			// Scene을 여는데 성공하면 해당하는 이벤트를 활성화 시켜준다.
			if( UISceneInstance != None )
			{
				`Log("OpenScene Instance = "$UISceneInstance @"EventParams.Len =" @PendingSceneList[i].EventParams.Length);
				for( EventIndex = 0 ; EventIndex < PendingSceneList[i].EventParams.Length ; EventIndex++ )
				{
					EventParam = PendingSceneList[i].EventParams[EventIndex];
					
					ActivatedEvents.Length = 0;
					if( EventParam.EventClass != none )
					{
						`log("EventParam.EventClass" @EventParam.EventClass);
						UISceneInstance.ActivateEventByClass( INDEX_NONE, EventParam.EventClass, UISceneInstance, false, , ActivatedEvents);
					}

					//for( ActivatedEventIndex = 0 ; ActivatedEventIndex < ActivatedEvents.Length ; ActivatedEventIndex++ )
					//	`Log( "ActivateEvent "@ActivatedEvents[ActivatedEventIndex]@EventParam.StrParam@EventParam.IntParam@EventParam.BoolParam@EventParam.FloatParam@EventParam.ObjParam );

					for( ActivatedEventIndex = 0 ; ActivatedEventIndex < ActivatedEvents.Length ; ActivatedEventIndex++ )
						class'avaEventTrigger'.static.SetEventParam( ActivatedEvents[ActivatedEventIndex], EventParam.StrParam, EventParam.IntParam, EventParam.BoolParam, EventParam.FloatParam, EventParam.ObjParam );
					
				}
			}
			else
			{
				`Log("Failed to OpenScene"@PendingSceneList[i].UIScene@PendingSceneList[i].UIScene.SceneTag@"is going to be deleted");
			}
			PendingSceneList.Remove(i,1);
			i--;
		}
	}
}

function bool CheckEventTriggerCondition( INT PendingEventListIndex )
{
	Local WorldInfo WorldInfo;
	Local UIInteraction UIController;
	Local bool bResult;
	Local array<name> SceneTags;
	Local name SceneTag;
	
	WorldInfo = GetWorldInfo();
	UIController = GetUIController();
	if( WorldInfo == none || UIController == none || 
		!( 0 <= PendingEventListIndex && PendingEventListIndex < PendingEventList.Length ) )
	{
		return false;
	}

	if( PendingEventList[PendingEventListIndex].BaseGameClass != none 
		&& ! ClassIsChildOf(GetGameInfoClass(), PendingEventList[PendingEventListIndex].BaseGameClass) )
	{
		return false;
	}

	// 이벤트가 발생 가능한 장면이 특별히 설정되지 않았다면 일단 true.
	bResult = (PendingEventList[PendingEventListIndex].BaseSceneTags.Length == 0);

	SceneTags = PendingEventList[PendingEventListIndex].BaseSceneTags;
	foreach SceneTags( SceneTag )
	{
		if( UIController.FindSceneByTag( SceneTag ) != none )
		{
			bResult = true;
			break;
		}
	}
	SceneTags = PendingEventList[PendingEventListIndex].ExclSceneTags;
	foreach SceneTags( SceneTag )
	{
		if( UIController.FindSceneByTag( SceneTag ) != none )
		{
			bResult = false;
			break;
		}
	}
	
	return bResult;
}

function UpdatePendingEvents()
{
	Local int EventIndex;
	Local avaNetEventParam EventParam;
	Local array< class<UIEvent> > EventClasses;

	for( EventIndex = 0 ; EventIndex < PendingEventList.Length ; EventIndex++ )
	{
		EventParam = PendingEventList[EventIndex].EventParam;
		if( CheckEventTriggerCondition( EventIndex ) && EventClasses.Find(EventParam.EventClass) == INDEX_NONE)
		{
			class'avaEventTrigger'.static.ActivateEventByClass( EventParam.EventClass, EventParam.StrParam, EventParam.IntParam, EventParam.BoolParam, EventParam.FloatParam, EventParam.ObjParam );

			EventClasses.AddItem(EventParam.EventClass);
			PendingEventList.Remove(EventIndex, 1);
			EventIndex--;
		}
	}
}

function UpdatePendingMsgs()
{
	Local int MsgIndex;
	Local class<GameInfo> GameInfoClass, MsgGameClass;
	Local EMsgCategory MsgCat;
	Local byte Msg;
	Local string Param1, Param2;
	Local int Param3, Param4;

	GameInfoClass = GetGameInfoClass();

	if( GameInfoClass == none )
		return;

	for( MsgIndex = 0 ; MsgIndex < PendingMsgList.Length ; MsgIndex++ )
	{
		MsgGameClass = PendingMsgList[MsgIndex].BaseGameClass;
		if( MsgGameClass == none || ClassIsChildOf( GameInfoClass, MsgGameClass ) )
		{
			MsgCat = PendingMsgList[MsgIndex].MsgCategory;
			Msg = PendingMsgList[MsgIndex].Msg;
			
			Param1 = PendingMsgList[MsgIndex].Param1;
			Param2 = PendingMsgList[MsgIndex].Param2;
			Param3 = PendingMsgList[MsgIndex].Param3;
			Param4 = PendingMsgList[MsgIndex].Param4;

			switch( MsgCat )
			{
			case EMsg_Client:	ProcClientMessage( EMsgClient(Msg), Param1, Param2, Param3, Param4);	break;
			case EMsg_Channel:	ProcChannelMessage( EMsgChannel(Msg), Param1, Param2, Param3, Param4);	break;
			case EMsg_Room:	ProcRoomMessage(EMsgRoom(Msg), Param1, Param2, Param3, Param4);	break;
			case EMsg_Game:	ProcGameMessage(EMsgGame(Msg), Param1, Param2, Param3, Param4);	break;
			case EMsg_Inventory:	ProcInventoryMessage(EMsgInventory(Msg), Param1, Param2, Param3, Param4);	break;
			case EMsg_Guild:	ProcGuildMessage(EMsgGuild(Msg), Param1, Param2, Param3, Param4);	break;
			case EMsg_Friend:	break;
			case EMsg_Admin:	ProcAdminMessage(EMsgAdmin(Msg), Param1, Param2, Param3, Param4);	break;
			case EMsg_Buddy:	ProcBuddyMessage(EMsgBuddy(Msg), Param1, Param2, Param3, Param4);	break;
			case EMsg_Option:	ProcOptionMessage(EMsgOption(Msg), Param1, Param2, Param3, Param4);	break;
			default:	assert(false);	break;
			}
			PendingMsgList.Remove(MsgIndex, 1);
			MsgIndex--;
		}
	}
}

event Tick( float DeltaTime )
{
	local array< class< object > >	OutList;
	local array< object >			ObjectList;
	local int						i;

	UpdatePendingScenes();
	UpdatePendingEvents();
	UpdatePendingMsgs();
	
	// Precache Player Data...
	if ( StartPlayerIDList.length > 0 )
	{
		if ( LocalPlayer(GetLocalPC().Player) != None )
		{
			class'avaMod_TranslatorBase'.static.GetCharacterModifierList( GetURLString(StartPlayerIDList[0], "ChItem"), CharacterModifierList, OutList );
			class'avaMod_TranslatorBase'.static.GetCharacterModifierList( GetURLString(StartPlayerIDList[0], "SkillP"), CharacterModifierList, OutList );
			class'avaMod_TranslatorBase'.static.GetCharacterModifierList( GetURLString(StartPlayerIDList[0], "SkillR"), CharacterModifierList, OutList );
			class'avaMod_TranslatorBase'.static.GetCharacterModifierList( GetURLString(StartPlayerIDList[0], "SkillS"), CharacterModifierList, OutList );
			class'avaMod_TranslatorBase'.static.GetWeaponModifierList( GetURLString(StartPlayerIDList[0], "WeaponP"), WeaponModifierList, OutList );
			class'avaMod_TranslatorBase'.static.GetWeaponModifierList( GetURLString(StartPlayerIDList[0], "WeaponR"), WeaponModifierList, OutList );
			class'avaMod_TranslatorBase'.static.GetWeaponModifierList( GetURLString(StartPlayerIDList[0], "WeaponS"), WeaponModifierList, OutList );

			for ( i = 0 ; i < OutList.length ; ++ i )
			{
				`log( "Precache Player Data" @StartPlayerIDList[0] @OutList[i] );
				OutList[i].static.PreCache( ObjectList );
			}
			LocalPlayer(GetLocalPC().Player).AddPlayerWorkingSet( ObjectList );
			StartPlayerIDList.Remove( 0, 1 );
		}
	}
}

native static function avaNetHandler GetAvaNetHandler();

/*! @brief
		ClanMarkID 를 해당 Resource 이름으로 변환해 준다....
	@param ID
		클랜마크 ID.
	@param bSmall
		클랜마크 작은 크기용인지 유무.(기본값은 큰 크기이다)
		(bMarkup == false)경우에는 무시되고 무조건 Large로 처리된다.
	@param bMarkup
		Markup String으로 얻을 것인지 유무.
	@note
		ex )	0 convert -> avaClanMarkLarge01.00000
				1 convert -> avaClanMarkLarge01.00001
		ex2)	0 convert -> avaClanMarkSmall01
				1 convert -> <Images:avaClanMarkSmall01; UL=19 VL=19 DimX=26 DimY=26 TileNum=675 Offset=1>
				2 convert -> <Images:avaClanMarkSmall01; UL=19 VL=19 DimX=26 DimY=26 TileNum=675 Offset=2>
*/	
native static function string GetClanMarkPkgNameFromID( int ID, optional bool bSmall = false, optional bool bMarkup = false );

/*
{
	local int PageNum;
	local int Index;
	local int IndexX;
	local int IndexY;
	PageNum	= ID / 676;			// 676 means 26 * 26
	Index	= ID % 676;
	IndexX	= Index / 26;		// 26 means "A" to "Z"
	IndexY	= Index % 26;
	if ( PageNum < 10 )	return "avaClanMarkLarge"$"0"$(PageNum+1)$"."$Chr(65+IndexX)$Chr(65+IndexY);	// 65 means "A"
	else				return "avaClanMarkLarge"$PageNum$"."$Chr(65+IndexX)$Chr(65+IndexY);


	//local int		PageNum;
	//local int		ZeroCnt;
	//local int		i;
	//local string	result;
	//PageNum	=	ID / 676 + 1;	// 676 means 26 * 26
	//if ( PageNum < 10 )		result	=	"avaClanMarkLarge"$"0"$PageNum$".";
	//else					result	=	"avaClanMarkLarge"$PageNum$".";	
	//ZeroCnt	=	5 - Len( ID );	// 00000,00001... format
	//for ( i = 0 ; i < ZeroCnt ; ++i )
	//	result	=	result$"0";
	//return result$ID;
}
*/

defaultproperties
{
	PopupSceneName			=	"Misc.PopUpWindow"
	HostMigrationSceneName	=	"HostMigration.HostMigration"
	ExitSceneName			=	"Exit.Exit"

	NetEntryGameClass		=	class'avaNetEntryGame'
	TeamGameClass			=	class'avaTeamGame'
	DeathMatchGameClass		=	class'avaDeathMatch'

	EntrySceneTags.Add(ReadyRoom)
	EntrySceneTags.Add(FrClanRoom)
	EntrySceneTags.Add(Result)
	EntrySceneTags.Add(Result_MilitaryDrill)
	EntrySceneTags.Add(Title)
	EntrySceneTags.Add(Lobby)
	EntrySceneTags.Add(ClanLobby)

	PersistentEntrySceneTags.Add(ReadyRoom)
	PersistentEntrySceneTags.Add(FrClanRoom)
	PersistentEntrySceneTags.Add(Title)
	PersistentEntrySceneTags.Add(Lobby)
	PersistentEntrySceneTags.Add(ClanLobby)
}
