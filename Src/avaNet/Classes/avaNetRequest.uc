/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaNetRequest.uc

	Description: Interface class between UE3 and AVA networking

***/
class avaNetRequest extends Object
	config(net)
	native;



enum EPlayerClass
{
	EClass_Pointman,
	EClass_Rifleman,
	EClass_Sniper
};


enum EItemEffectType
{
	IET_None,
	IET_GR,
	IET_EXP_BOOST,
	IET_SP_BOOST,
	IET_MONEY_BOOST,
	IET_ACCESSORY,
};


enum ENetMissionType
{
	NMT_Mission,
	NMT_Warfare,
	NMT_MilitaryDrill,
};


struct native avaRoomSetting
{
	var int idMap;
	var int tkLevel;
	var bool autoBalance;
	var bool allowSpectator;
	var bool allowInterrupt;
	var bool allowBackView;
	var bool allowGameGhostChat;
	var bool autoSwapTeam;
	var int roundToWin;
	var int MaxPlayer;
	var int mapOption;
	var bool bPassword;
};

struct native avaNetChannelInfo
{
	var int idChannel;
	var EChannelFlag Flag;
	var int ChIdx;
};

struct native avaNetChannelSettingInfo
{
	var EChannelFlag Flag;

	var bool EnableAutoBalance;
	var bool EnableSpectator;
	var bool EnableInterrupt;
	var bool EnableBackView;
	var bool EnableGhostChat;
	var bool EnableAutoSwapTeam;
	var bool EnableMaxPlayers;
	var bool EnableTKLevel;

	var bool MissionTypeSpecial;
	var bool MissionTypeWarfare;
	var bool MissionTypeTraining;

	var bool DefaultAutoBalance;
	var bool DefaultSpectator;
	var bool DefaultInterrupt;
	var bool DefaultBackView;
	var bool DefaultGhostChat;
	var bool DefaultAutoSwapTeam;
	var byte DefaultTKLevel;
	var byte DefaultMaxPlayers;

	var bool RoomChangeTeam;
	var bool RoomSkipMinPlayerCheck;
	var bool RoomSkipBalanceCheck;

	var bool GameIdleCheck;
	var bool GameFreeCam;
	var bool GameCheckAllOut;

	var bool UpdatePlayerScore;
	var bool UpdateStatLog;

	var bool InvenCashItem;
	var bool AllowPCBangBonus;
	var bool AllowBoostItem;
	var bool AllowEventBonus;

	structdefaultproperties
	{
		EnableAutoBalance = true
		EnableSpectator = true
		EnableInterrupt = true
		EnableBackView = true
		EnableGhostChat = true
		EnableAutoSwapTeam = true
		EnableMaxPlayers = true
		EnableTKLevel = true

		MissionTypeSpecial = true
		MissionTypeWarfare = true
		MissionTypeTraining = true

		DefaultAutoBalance = false
		DefaultSpectator = false
		DefaultInterrupt = true
		DefaultBackView = true
		DefaultGhostChat = true
		DefaultAutoSwapTeam = false
		DefaultMaxPlayers = 0;
		DefaultTKLevel = 0;

		RoomChangeTeam = true
		RoomSkipMinPlayerCheck = false
		RoomSkipBalanceCheck = false

		GameIdleCheck = true
		GameFreeCam = false
		GameCheckAllOut = true

		UpdatePlayerScore = true
		UpdateStatLog = true

		InvenCashItem = true
		AllowPCBangBonus = true	
		AllowBoostItem = true
		AllowEventBonus = true
	}
};


struct native avaNetMapInfo
{
	var int MapID;
	var ENetMissionType MissionType;
	var string MapName;
	var string ImagePathName;
	var string Description;
	var string MapFileName;
	var string MaxPlayers<ToolTip=eg. "18 20 22 *24 26">;
	var string WinCondition<ToolTip=eg. "3 5 *7 9 10">;
	var string WinConditionType<ToolTip=eg. "Round","TargetTeamScore">;
	var bool bRedduckInternalOnly;
	var array<EChannelFlag>	ExclChannelGroups;
	var bool bStatLog;
	var bool AllowMapOption;
};

struct native avaNetMissionInfo
{
	var ENetMissionType			MissionType;
	// 개별 맵중에 WinCondition, MaxPlayer가 없을때 디폴트 정보
	var string MaxPlayers;
	var string WinCondition;
};


var globalconfig array<avaNetChannelInfo> ChannelList;
var globalconfig array<avaNetChannelSettingInfo> ChannelSettingList;
var globalconfig array<avaNetMapInfo> MapList;
var globalconfig array<avaNetMissionInfo> MissionList;

var private{private} array<avaRoomSetting>	RoomSettingTyped;

cpptext
{
public:
	void NPGameSendAuth(DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3);

	/** 미션타입에 따른 방설정을 반환 */
	FavaRoomSetting* GetCurrentRoomSetting( INT CurrentMissionType );
	void ResetRoomSettingsAsDefault();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Connection
native final function bool AutoConnect(bool bForce, string USN = "", string UserID = "", string Key = "");
native final function CloseConnection(int ExitCode);
native final function CreateCharacter(string Nickname, int FaceIndex);
native final function Quit(bool bGraceful = true);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Channel list
native final function ListChannel();
native final function JoinChannel(int ListIndex);
native final function JoinChannelByID(int ID, bool bFollowing = false);
native final function QuickJoinChannel(string Flags);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lobby
native final function LeaveChannel();
native final function ListRoom();
native final function ListLobbyPlayer();
native final function LobbySelectRoom(int ListIndex);

native final function CreateRoom(string RoomName, string RoomPassword, avaRoomSetting Setting);
native final function bool JoinRoom(int ListIndex, string RoomPassword);
native final function bool QuickJoinRoom(int idMap = 0);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Room
native final function LeaveRoom(EavaLeavingReason LeavingReason = ELR_Normal);
native final function RoomKickPlayer(string Nickname, int reason = 1);	// 1 = ban, 2 = loading time out, 3 = setting changed
native final function RoomSetting(avaRoomSetting Setting);
native final function RoomReady(bool Ready);
native final function RoomReadyToggle();
native final function RoomChangeTeam(int idTeam);
native final function RoomChangeTeamToggle();
native final function RoomChangeSlot(int Slot);
native final function RoomChangeClass(int idClass);
native final function RoomStart();
native final function RoomCancelStart();
native final function RoomSwapTeam(int Reason = 0);
native final function RoomClaimHost();
native final function SetMyRttValue(float RttValue);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lobby & Room
native final function Chat(string ChatMsg);
native final function bool ParseChatCommand(string Cmd);
native final function FilterChatMsg(out string ChatMsg);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// In-game
native final function LeaveGame();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Communication
native final function BuddyListBegin();
native final function BuddyListEnd();
native final function BuddyAdd(bool bForce, string Nickname);
native final function BuddyAddAns(bool bAccept, string Nickname);
native final function BuddyDelete(string Nickname);

native final function BlockAdd(bool bForce, string Nickname);
native final function BlockDelete(string Nickname);

native final function Whisper(string Nickname, string ChatMsg);
native final function InviteGame(string Nickname);
native final function InviteGameAns(bool bAccept, string Nickname);
native final function FollowPlayer(string Nickname);
native final function SetFollowPlayerPwd(string Password);

native final function InviteGuild(string Nickname);
native final function InviteGuildAns(bool bAccept, string Nickname);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Inventory
native final function EnterInven();
native final function LeaveInven();
native final function InvenSetWeapon(int EquipSlot, int InvenSlot);
native final function InvenSetEquip(int EquipSlot, int InvenSlot);
native final function InvenSetCustom(int InvenSlot, int CustomIndex, int OptionIndex = 0);
native final function InvenSetEffect(int EquipSlot, int InvenSlot);
native final function InvenUnsetWeapon(int EquipSlot, int InvenSlot);
native final function InvenUnsetEquip(int EquipSlot, int InvenSlot);
native final function InvenUnsetCustom(int InvenSlot, int CustomSlot);
native final function InvenUnsetEffect(int EquipSlot, int InvenSlot);
native final function InvenRepairWeapon(int InvenSlot);
native final function InvenRepairEquip(int InvenSlot);
native final function InvenConvertRIS(int InvenSlot);
native final function bool InvenGetWeaponRefundPrice(int InvenSlot, out int Money, out int Cash);
native final function bool InvenGetEquipRefundPrice(int InvenSlot, out int Money, out int Cash);
native final function bool InvenGetCustomRefundPrice(int InvenSlot, int CustomSlot, out int Money, out int Cash);

native final function int GetUsedItemEffect(EItemEffectType ItemEffect);

native final function ShopBuyWeapon(int ListIndex, int OptionIndex = 0);
native final function ShopBuyEquip(int ListIndex, int OptionIndex = 0);
native final function ShopSellWeapon(int InvenSlot);
native final function ShopSellEquip(int InvenSlot);

native final function bool WICGetCash();
native final function bool WICOpenChargeWindow();
native final function bool WICBuyItem(int idItem);
native final function bool WICSendGift(int idItem, int idAccountTo);
native final function bool WICOpenGiftWindow(int idItem);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Guild
native final function GuildJoinChannel();
native final function GuildSetMotd(string Motd);
native final function GuildChat(string ChatMsg, bool bParse = true);
native final function GuildNotice(string NoticeMsg);
native final function GuildLeave();
native final function GuildKick(string Nickname);
native final function bool GuildDoIHavePriv(int Priv);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Misc
native final function OptionSaveUserKey( string UserKeyStr, string OptionStr );

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Information
native final function SelectPlayer(int ListIndex);
native final function SelectGuildPlayer(int ListIndex);
native final function SelectBuddy(int ListIndex);
native final function SelectBlock(int ListIndex);

native final function string GetCurrentChannelName();
//native final function string GetCurrentRoomName();
native final function int GetCurrentRoomIndex();
native final function int GetCurrentRoomState();
native final function bool GetCurrentRoomSetting(out avaRoomSetting Setting);
native final function bool GetCurrentEquipState(out int MyTeam, out int MyClass, out int MyFace, out int MyWeapon);
native final function int GetMyRoomSlot();

native final function string GetRandomRoomName();
native final function string GetMapName(int ListIndex);
native final function string GetWeaponIconCode(int idItem);
native final function string GetWeaponName(int idItem);

native final function string GetSkillName(EPlayerClass PlayerClass, int SkillID);
native final function string GetAwardName(int AwardID);
native final function string GetPlayerLevelName(int PlayerLevel);

native final function bool AmIHost();
native final function bool AmISpectator();


native static function avaNetRequest GetAvaNetRequest();


defaultproperties
{
}