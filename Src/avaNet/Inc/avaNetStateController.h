/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaNetStateController.h

	Description: Declarations of CavaNetStateController; it controls and manages server commands

***/
#ifndef __AVANETSTATECONTROLLER_H__
#define __AVANETSTATECONTROLLER_H__


#include "avaStaticData.h"
#include "ComDef/Def.h"
#include "ComDef/Inventory.h"




struct FGuildInfo;
struct FGuildPlayerInfo;
struct FavaNetChannelSettingInfo;

// my player information
struct FPlayerInfo
{
	Def::PLAYER_INFO PlayerInfo;
	Def::PLAYER_ITEM_INFO OldItemInfo;	// current item info is backed up before game results are updated, so we can check out condition change of items.
	BYTE PrivLevel;						// 0 = normal user, 1 = GM; more privilege levels can be added later.
	CInventory Inven;					// inventory reference to PlayerInfo.itemInfo

	DWORD Cash;							// cash info is retrived via WebInClient

	//Stat index
	INT Region;		// connectiing region 01 - 17
	INT AgeCode;	// age band 0 = ~13, 1 = ~16, 2 = ~19, 3 = ~24, 4 = ~29, 5 = ~39, 6 = 40 ~
	INT Gender;		// 0 = male, 1 = female
	INT Birth;		// birth year
	INT Age;		// age

	UBOOL PcBangFlag;
	DWORD PcBangServiceType;

	__int64 RxGuildAddress;	// if my player's guild info is valid, this address would be received from channel server

	UBOOL bSkillInfo, bScoreInfo, bItemInfo, bAwardInfo, bGuildInfo;	// is proper information is updated?

	BYTE RoomKickedReason;	// updated when my player is kicked from the room; used by UI scenes.

	BYTE OldCurrentClass;

	FPlayerInfo() : PrivLevel(0), Cash(0), Region(0), AgeCode(0), Gender(0), Birth(0), Age(20), PcBangFlag(FALSE), PcBangServiceType(0),
					RxGuildAddress(0), bSkillInfo(FALSE), bScoreInfo(FALSE), bItemInfo(FALSE), bAwardInfo(FALSE), bGuildInfo(FALSE),
					RoomKickedReason(255), OldCurrentClass(0)
	{
		PlayerInfo.idAccount = Def::ID_INVALID_ACCOUNT;
		PlayerInfo.guildInfo.idGuild = Def::ID_INVALID_GUILD;

		Inven.Init(&_ItemDesc(), &PlayerInfo.itemInfo);
	}

	UBOOL IsValid() const
	{
		return PlayerInfo.idAccount != Def::ID_INVALID_ACCOUNT;
	}
	UBOOL IsLoaded() const
	{
		return IsValid() && bSkillInfo && bScoreInfo && bItemInfo && bAwardInfo && bGuildInfo;
	}
	UBOOL IsAdmin() const
	{
		return PrivLevel > 0;
	}
	UBOOL IsPcBang() const
	{
		return /*PcBangFlag || */PcBangServiceType > 0;
	}

	void SetItemInfo(Def::ITEM_INFO *WeaponInven, WORD WeaponCount,
					Def::ITEM_INFO *EquipInven, WORD EquipCount,
					Def::CUSTOM_ITEM_INFO *CustomInven, WORD CustomCount,
					Def::EFFECT_ITEM_INFO *EffectInven, WORD EffectCount,
					Def::TSN_ITEM *WeaponSet, Def::TSN_ITEM *EquipSet, Def::TSN_ITEM *EffectSet);
	void SetSkillInfo(Def::PLAYER_SKILL_INFO &skill);
	void SetScoreInfo(Def::PLAYER_SCORE_INFO &score);
	void SetAwardInfo(Def::PLAYER_AWARD_INFO &award);
	void SetGuildInfo(Def::PLAYER_GUILD_INFO &guild);

	INT GetAwardProgress(BYTE idAward);

	void BackupItemInfo();		// back up current item information to OldItemInfo
	void CheckGrenadeSlots();	// recalculate count of available grenade slots, based from current equipments.

	void SetRxGuildAddress(__int64 addr)
	{
		RxGuildAddress = addr;
	}

	void UpdateCurrentClass();

	INT GetBoostXPPerc();
	INT GetBonusXPPerc();
	INT GetPcBangBonusXPPerc();
	INT GetBoostSupplyPerc();
	INT GetBonusSupplyPerc();
	INT GetPcBangBonusMoneyPerc();
	INT GetBoostMoneyPerc();
	INT GetBonusMoneyPerc();
	INT GetBonusBIAPerc();

	INT GetClanMarkID();

	INT GetCurrentChannelMaskLevel();

	void DumpPlayerInfo();
	void DumpItemInfo();
	void DumpGuildInfo();
};

// other player's displayed information
// retrived from channel/guild server via CHANNEL::PLAYER_INFO_REQ
struct FPlayerDispInfo
{
	Def::PLAYER_DISP_INFO PlayerInfo;
	DOUBLE InfoTimeStamp;

	FPlayerDispInfo()
	{
		Clear();
	}

	UBOOL IsValid()
	{
		return PlayerInfo.idAccount != Def::ID_INVALID_ACCOUNT;
	}
	UBOOL IsFullInfo()
	{
		return InfoTimeStamp > 0 && appSeconds() < InfoTimeStamp + 60;
	}
	void SetFullInfo(UBOOL bFull)
	{
		InfoTimeStamp = (bFull ? appSeconds() : 0);
	}

	virtual void DumpPlayerInfo();

	virtual void Clear()
	{
		PlayerInfo.idAccount = Def::ID_INVALID_ACCOUNT;
		PlayerInfo.nickname[0] = 0;
		//appMemset(PlayerInfo.equipItem, 0, Def::MAX_EQUIPSET_SIZE * sizeof(Def::TID_ITEM));
		//appMemset(PlayerInfo.weaponItem, 0, Def::MAX_WEAPONSET_SIZE * sizeof(Def::TID_ITEM));
		InfoTimeStamp = 0;
	}

	virtual void Set(Def::PLAYER_INFO &Info);
	virtual void Set(Def::PLAYER_DISP_INFO &Info);
	virtual void Set(Def::LOBBY_PLAYER_INFO &Info) {}
	virtual void Set(Def::ROOM_PLAYER_INFO &Info) {}

	virtual INT GetClanMarkID();

	// if input information is newer than current information, update it.
	void SyncPlayerInfo(FGuildPlayerInfo &Player);
};

// information displayed in lobby player list
struct FLobbyPlayerInfo : public FPlayerDispInfo
{
	Def::LOBBY_PLAYER_INFO LobbyPlayerInfo;

	FLobbyPlayerInfo() : FPlayerDispInfo() {}

	void Clear()
	{
		FPlayerDispInfo::Clear();

		LobbyPlayerInfo.idAccount = Def::ID_INVALID_ACCOUNT;
	}

	virtual void Set(Def::PLAYER_INFO &player);
	virtual void Set(Def::PLAYER_DISP_INFO &Info);
	virtual void Set(Def::LOBBY_PLAYER_INFO &Info);
};

// information displayed in room player list
struct FRoomPlayerInfo : public FPlayerDispInfo
{
	Def::ROOM_PLAYER_INFO RoomPlayerInfo;
	FLOAT RttValue;

	FRoomPlayerInfo() : FPlayerDispInfo(), RttValue(-1) {}

	virtual void DumpPlayerInfo();

	void Clear()
	{
		FPlayerDispInfo::Clear();

		RoomPlayerInfo.idAccount = Def::ID_INVALID_ACCOUNT;
		RttValue = -1;
	}

	virtual void Set(Def::PLAYER_INFO &player);
	virtual void Set(Def::PLAYER_DISP_INFO &Info);
	virtual void Set(Def::ROOM_PLAYER_INFO &Info);

	virtual INT GetClanMarkID();

	FString GetPrimaryWeaponIconCode(INT idClass = -1);	// idClass = -1 -> current class
	DWORD GetPrimaryWeaponSlotIdx(INT idClass = -1);
	DWORD GetPrimaryWeaponID(INT idClass = -1);
	FString GetCurrentWeaponIconCode();
	BYTE GetTeamID();

	UBOOL IsReady()
	{
		return RoomPlayerInfo.bReady == Def::_READY_WAIT;
	}
	UBOOL IsPlaying()
	{
		return RoomPlayerInfo.bReady == Def::_READY_PLAYING || RoomPlayerInfo.bReady == Def::_READY_LOADING;
	}
};

struct FLobbyPlayerList
{
	TArray<FLobbyPlayerInfo> PlayerList;
	INT ListIndex;	// UIList's selected index

	FLobbyPlayerList() : ListIndex(-1) {}

	FLobbyPlayerInfo* Add(Def::LOBBY_PLAYER_INFO &Player);
	FLobbyPlayerInfo* Add(Def::PLAYER_DISP_INFO &Player);
	FLobbyPlayerInfo* Add(Def::PLAYER_INFO &Player);
	UBOOL Remove(LONG idAccount);
	FLobbyPlayerInfo* Find(LONG idAccount);
	FLobbyPlayerInfo* Find(const FString &PlayerName);

	FLobbyPlayerInfo* GetSelected()
	{
		if (PlayerList.IsValidIndex(ListIndex))
			return &(PlayerList(ListIndex));
		else
			return NULL;
	}

	void Clear()
	{
		ListIndex = -1;
		PlayerList.Empty();
	}

	void DumpPlayerList()
	{
		for (INT i = 0; i < PlayerList.Num(); ++i)
			PlayerList(i).DumpPlayerInfo();
	}
};

struct FRoomPlayerList
{
	FRoomPlayerInfo PlayerList[Def::MAX_ALL_PLAYER_PER_ROOM];
	INT ListIndex;	// UIList's selected index

	TArray<INT> StartPlayerList;	// obsolete

	FRoomPlayerList() : ListIndex(-1)
	{
		for (INT i = 0; i < Def::MAX_ALL_PLAYER_PER_ROOM; ++i)
		{
			PlayerList[i].RoomPlayerInfo.idSlot = i;
			//PlayerList[i].RoomPlayerInfo.idTeam = (INT)(i / Def::MAX_PLAYER_PER_TEAM);
		}
	}

	FRoomPlayerInfo* Add(Def::ROOM_PLAYER_INFO &Player);
	FRoomPlayerInfo* Add(Def::PLAYER_DISP_INFO &Player);
	FRoomPlayerInfo* Add(Def::PLAYER_INFO &Player);
	FRoomPlayerInfo* Find(LONG idAccount);
	FRoomPlayerInfo* Find(const FString &PlayerName);

	UBOOL Empty(BYTE idSlot);
	UBOOL IsEmpty(BYTE idSlot)
	{
		if (idSlot >= 0 && idSlot < Def::MAX_ALL_PLAYER_PER_ROOM)
			return (PlayerList[idSlot].PlayerInfo.idAccount == Def::ID_INVALID_ACCOUNT);

		return TRUE;
	}
	INT FindEmptySlot(BYTE MaxNum)
	{
		if (MaxNum > Def::MAX_ALL_PLAYER_PER_ROOM)
			MaxNum = Def::MAX_ALL_PLAYER_PER_ROOM;
		for (INT i = 0; i < MaxNum; ++i)
			if (IsEmpty(i))
				return i;
		return -1;
	}
	/*! @brief 해당 팀에서 첫번째로 유효한 슬롯 번호를 찾아준다.
		@param eRoomTeam
			RT_EU
			RT_NRF
			RT_SPECTATOR
	*/
	INT FindFirstValidSlot(_ROOM_TEAM eRoomTeam)
	{
		INT first, last;
		if ( eRoomTeam == RT_EU )
		{
			first = 0;
			last = MAX_PLAYER_PER_TEAM;
		}
		else if( eRoomTeam == RT_NRF )
		{
			first = MAX_PLAYER_PER_TEAM;
			last  = MAX_PLAYER_PER_TEAM * 2;
		}
		else if( eRoomTeam == RT_NRF )
		{
			first = MAX_PLAYER_PER_ROOM;
			last  = MAX_ALL_PLAYER_PER_ROOM;
		}
		else
		{
			return -1;
		}

		for ( INT i = first; i < last; i++ )
			if ( PlayerList[i].IsValid() )
				return i;
		return -1;
	}

	FRoomPlayerInfo* GetSelected()
	{
		if (ListIndex >= 0 && ListIndex < Def::MAX_ALL_PLAYER_PER_ROOM && PlayerList[ListIndex].IsValid())
			return &(PlayerList[ListIndex]);
		else
			return NULL;
	}

	void Clear()
	{
		ListIndex = -1;

		for (INT i = 0; i < Def::MAX_ALL_PLAYER_PER_ROOM; ++i)
		{
			PlayerList[i].Clear();
		}

		StartPlayerList.Empty();
		//StartingPlayerList.Empty();
	}

	void DumpPlayerList(BYTE MaxNum)
	{
		if (MaxNum > Def::MAX_ALL_PLAYER_PER_ROOM)
			MaxNum = Def::MAX_ALL_PLAYER_PER_ROOM;
		for (INT i = 0; i < MaxNum; ++i)
			PlayerList[i].DumpPlayerInfo();
	}
};

struct FChannelInfo
{
	enum _ChannelType
	{
		CT_UNKNOWN = 0,
		CT_NORMAL,	// normal/trainee/friendly clan match channel
		CT_GUILD,	// my clan home
	};

	Def::TID_CHANNEL idChannel;
	FString ChannelName;
	FString ChannelNameShort;
	INT Flag;		// refer to EChannelFlag
	INT MaxPlayers;
	INT Count;		// -1 = invalidated channel; so it isn't displayed in the channel list
	INT Mask;

	FChannelInfo() : idChannel(Def::ID_INVALID_CHANNEL), Flag(Def::_CF_NORMAL), MaxPlayers(1000), Count(-1), Mask(-1) {}
	FChannelInfo(Def::TID_CHANNEL InChannelID, INT InFlag, INT InMask, INT Idx) : idChannel(InChannelID), Flag(InFlag), Mask(InMask), MaxPlayers(1000), Count(-1)
	{
		SetChannelName(TEXT(""), Idx);
	}

	// channel peer's listen address
	FString GetRxAddress()
	{
		FString Addr(TEXT("CHL"));
		//Addr += appItoa(Idx);
		Addr += appItoa(idChannel);
		return Addr;
	}

	UBOOL IsValid()
	{
		//return (Idx != Def::ID_INVALID_CHANNEL && Count >= 0);
		return (idChannel != Def::ID_INVALID_CHANNEL && Count >= 0);
	}
	void Invalidate()
	{
		Count = -1;
	}

	UBOOL IsMaskTurnedOn() const { return Mask >= 0 && Mask < Def::MAX_CHANNEL_MASK; }

	static FString IsJoinableLevel(BYTE ChannelFlag, BYTE Level);
	static FString IsJoinable(BYTE ChannelFlag);
	FString IsJoinable();
	UBOOL IsJoinableMask();

	UBOOL IsNormalChannel() const { return Flag != EChannelFlag_MyClan; }
	UBOOL IsMyClanChannel() const { return Flag == EChannelFlag_MyClan; }		// 내클랜홈
	UBOOL IsFriendlyGuildChannel() const { return Flag == EChannelFlag_Clan; }	// 친선 클랜전 채널
	UBOOL IsMatchChannel() const { return Flag == EChannelFlag_Match; }			// 대회 채널
	UBOOL IsPracticeChannel() const { return Flag == EChannelFlag_Practice; }	// 연습 채널
	INT GetChannelType() const { return IsMyClanChannel() ? CT_GUILD :CT_NORMAL; }

	void SetFromGuild(const FGuildInfo &Guild);
	void SetChannelName(const TCHAR *InName, INT Idx);

	void DumpChannelInfo();
};

struct FChannelList
{
	TArray<FChannelInfo> ChannelList;

	INT Set();		// initial set up
	FChannelInfo* Update(const Def::CHANNEL_DESC &Desc, const FString &InName);
	FChannelInfo* Find(TID_CHANNEL idChannel);
	FChannelInfo* RandomSelect();

	void InvalidateAll()
	{
		for (INT i = 0; i < ChannelList.Num(); ++i)
			ChannelList(i).Invalidate();
	}

	void DumpChannelList()
	{
		for (INT i = 0; i < ChannelList.Num(); ++i)
			if (ChannelList(i).IsValid())
				ChannelList(i).DumpChannelInfo();
	}
};



// information displayed in lobby room list
struct FRoomDispInfo
{
	Def::ROOM_INFO RoomInfo;

	FRoomDispInfo()
	{
		RoomInfo.idRoom = Def::ID_INVALID_ROOM;
	}

	UBOOL IsValid() const
	{
		return RoomInfo.idRoom != Def::ID_INVALID_ROOM;
	}
	UBOOL IsPlaying() const
	{
		return IsValid() && RoomInfo.state.playing == Def::RIP_PLAYING;
	}
	UBOOL IsFull() const
	{
		return RoomInfo.state.numCurr >= RoomInfo.setting.numMax;
	}

	virtual void Clear()
	{
		RoomInfo.idRoom = Def::ID_INVALID_ROOM;
	}

	virtual void DumpRoomInfo();
};


// information about currently joined room
struct FRoomInfo : public FRoomDispInfo
{
	//Def::ROOM_CLAN_INFO RoomClanInfo[2]; --> deprecated
	FRoomPlayerList PlayerList;
	BYTE HostIdx;
	DWORD StartTime;

	FRoomInfo() : FRoomDispInfo(), HostIdx(Def::ID_INVALID_ROOM_SLOT), StartTime(0)
	{
		//appMemzero(RoomClanInfo, 2 * sizeof(Def::ROOM_CLAN_INFO));
	}

	static INT SlotToTeam(INT Slot)
	{
		return (INT)(Slot / (Def::MAX_PLAYER_PER_TEAM));
	}
	static UBOOL IsSlotIDValid(BYTE idSlot, UBOOL bExceptSpectator = TRUE)
	{
		return idSlot >= 0 && idSlot < (bExceptSpectator ? Def::MAX_PLAYER_PER_ROOM : Def::MAX_ALL_PLAYER_PER_ROOM);
	}
	static UBOOL IsSpectatorSlot(BYTE idSlot)
	{
		return idSlot >= Def::MAX_PLAYER_PER_ROOM && idSlot < Def::MAX_ALL_PLAYER_PER_ROOM;
	}
	UBOOL IsOpenSlot(BYTE idSlot) const
	{
		return (idSlot >= 0 && idSlot < RoomInfo.setting.numMax / 2) ||
			(idSlot >= Def::MAX_PLAYER_PER_TEAM && idSlot < Def::MAX_PLAYER_PER_TEAM + RoomInfo.setting.numMax / 2);
	}

	FRoomPlayerInfo* AddPlayer(Def::ROOM_PLAYER_INFO &Player);
	FRoomPlayerInfo* AddPlayer(Def::PLAYER_DISP_INFO &Player);
	FRoomPlayerInfo* AddPlayer(Def::PLAYER_INFO &Player);
	void Remove(Def::TID_ACCOUNT idAccount);
	void RemoveSlot(BYTE idSlot);

	void RefreshPlayerCount();
	void RefreshClanInfo();

	INT FindEmptySlot(BYTE idTeam = RT_NONE);

	virtual void Clear()
	{
		FRoomDispInfo::Clear();

		//appMemzero(RoomClanInfo, 2 * sizeof(Def::ROOM_CLAN_INFO));
		HostIdx = Def::ID_INVALID_ROOM_SLOT;
		StartTime = 0;
		PlayerList.Clear();
	}

	void DumpClanInfo();
};


struct FRoomList
{
	enum _RoomListConst
	{
		MAX_ROOMS_PER_CHANNEL = UCHAR_MAX + 1,
		BUF_MAX_SIZE = MAX_ROOMS_PER_CHANNEL * sizeof(Def::ROOM_INFO)
	};

	TArray<FRoomDispInfo> RoomList;
	INT ListIndex;		// UIList's selected index

	DWORD DiffIndex;	// diff index received from channel server; if new diff index is not DiffIndex + 1, full room list is requested to channel server
	Def::ROOM_INFO ListBuf[MAX_ROOMS_PER_CHANNEL];	// current full room list; used for diff patching of room list

	FRoomList() : DiffIndex(0), ListIndex(-1)
	{
		Clear();
	}

	FRoomDispInfo* Add(Def::ROOM_INFO &Room);
	UBOOL Remove(WORD idRoom);
	void Clear();
	FRoomDispInfo* Find(WORD idRoom);
	void MergeFromDiff(BYTE *DiffBuf, INT BufSize);	// diff patch DiffBuf to ListBuf, and update RoomList from patched ListBuf

	FRoomDispInfo* GetSelected()
	{
		if (RoomList.IsValidIndex(ListIndex))
			return &(RoomList(ListIndex));
		else
			return NULL;
	}

	void DumpRoomList()
	{
		for (INT i = 0; i < RoomList.Num(); ++i)
			RoomList(i).DumpRoomInfo();
	}
};

enum ELastResultMsgType
{
	LastResultMsgType_PCBangXP,
	LastResultMsgType_PCBangMoney,
	LastResultMsgType_PCBangClanPoint,
	LastResultMsgType_Level,
	LastResultMsgType_SupplyMoney,
	LastResultMsgType_SupplyItem,
	LastResultMsgType_XP,
	LastResultMsgType_Supply,
	LastResultMsgType_Money,
	LastResultMsgType_MoneyBoost,
	LastResultMsgType_Brother,
	LastResultMsgType_EventXP,
	LastResultMsgType_EventCoin,
	LastResultMsgType_MAX,
};

// stores game result information
struct FLastResultInfo
{
	struct FLastResultMsgInfo
	{
		ELastResultMsgType MsgType;
		INT Variation;

		FLastResultMsgInfo( ELastResultMsgType InMsgType, INT InVariation = 0 ) : MsgType(InMsgType), Variation(InVariation) {}
	};

	struct FPlayerResultInfo
	{
		//INT idAccount;
		INT idSlot;
		INT LastLevel;
		INT Level;
		FString Nickname;
		INT Score;
		INT Death;
		INT xp;
		INT SupplyPoint;
		UBOOL bLeader;
		UBOOL bPcBang;
		INT biaXPFlag;

		FPlayerResultInfo()
		{
			Clear();
		}
		UBOOL IsValid()
		{
			return idSlot >= 0 && idSlot < Def::MAX_PLAYER_PER_ROOM;
		}
		void Clear();

		TArray<Def::TID_ITEM> EffectList;

		void AddEffect(Def::TID_ITEM idItem);
		INT GetBonusXPPerc();
		INT GetPcBangBonusXPPerc();
		INT GetBonusSupplyPerc();
	};


	INT InfoLevel;		// 0 = no info, 1 = my player score info is updated, 2 = other player's results are filled

	// InfoLevel = 1
	Def::PLAYER_RESULT_INFO PlayerResultInfo;
	INT Level;
	INT xp;
	INT SupplyPoint;
	INT SupplyCount;				// 1 = new supply arrived
	INT Money;
	INT BIAFlag;
	TArray<INT> SkillList[3];		// received skill list per class
	TArray<INT> AwardList;			// received award list
	TArray<FLastResultMsgInfo> LastResultMsgData;	// update message data

	// InfoLevel = 2
	INT TeamScore[2];
	TArray<FPlayerResultInfo> RoomResultInfo;

	FLastResultInfo()
	{
		Clear();
	}

	FPlayerResultInfo* FindPlayer(INT Slot);
	void Clear();

	void AddMsgInfo( ELastResultMsgType MsgType, INT Variation = 0 )
	{
		new(LastResultMsgData) FLastResultMsgInfo(MsgType, Variation);
	}
	
	FLastResultMsgInfo* GetLastResultMsgInfo( ELastResultMsgType MsgType )
	{
		FLastResultMsgInfo* MsgInfo = NULL;
		for( INT i = 0 ; i < LastResultMsgData.Num() ; i++ )
			if( LastResultMsgData(i).MsgType == MsgType )
				MsgInfo = &LastResultMsgData(i);
		return MsgInfo;
	}

	FLastResultMsgInfo& GetMsgInfo( INT MsgInfoIndex )
	{
		check(LastResultMsgData.IsValidIndex(MsgInfoIndex));
		return LastResultMsgData(MsgInfoIndex);
	}

	UBOOL IsValid()
	{
		return InfoLevel > 0;
	}
	UBOOL IsFullResult()
	{
		return InfoLevel > 1;
	}
	void Sort();					// sort player results, ordered by score
	void Dump();
};


struct FChatMsg
{
	FString Msg;
	DOUBLE AddedTime;

	const TCHAR* operator*()
	{
		return *Msg;
	}
	FString GetAddedTime(UBOOL bAddDate = FALSE);
};

struct FChatMsgList
{
	typedef TDoubleLinkedList<FChatMsg> TStringList;
	typedef TStringList::TIterator TIterator;
	typedef TStringList::TDoubleLinkedListNode TDoubleLinkedListNode;

	TStringList ChatList;

	void Add(const FString& Msg, const INT MsgType = 0);
	void Add(const TCHAR *Msg, const INT MsgType = 0);
	void AddWithPrefix(const FString& Msg, const INT MsgType = 0);
	void AddWithPrefix(const TCHAR *Msg, const INT MsgType = 0);
	void Clear();
	void Dump();

	TDoubleLinkedListNode* GetHead() const
	{
		return ChatList.GetHead();
	}
	TDoubleLinkedListNode* GetTail() const
	{
		return ChatList.GetTail();
	}

	FString operator[](INT Idx)
	{
		if (Idx >= ChatList.Num())
			return TEXT("");

		INT MsgIdx = ChatList.Num() - Idx - 1;

		TIterator It(GetHead());
		for (INT i = 0; i < MsgIdx; ++i)
		{
			++It;
		}
		return (*It).Msg;
	}

	FChatMsg* At(INT Idx)
	{
		if (Idx >= ChatList.Num())
			return NULL;

		INT MsgIdx = ChatList.Num() - Idx - 1;

		TIterator It(GetHead());
		for (INT i = 0; i < MsgIdx; ++i)
		{
			++It;
		}
		return &(*It);
	}

	INT Num()
	{
		return ChatList.Num();
	}
};


struct FGuildPlayerInfo
{
	Def::GUILD_PLAYER_INFO GuildPlayerInfo;

	DWORD GameWin;
	DWORD GameDefeat;
	DWORD DisconnectCount;
	Def::PLAYER_SCORE_INFO::SCORE_INFO Score;
	DWORD KillCount;
	DWORD DeathCount;
	DOUBLE InfoTimeStamp;
	DOUBLE LocationUpdatedTime;

	// location info
	Def::TID_CHANNEL idChannel;
	Def::TID_ROOM idRoom;

	FGuildPlayerInfo() : GameWin(0), GameDefeat(0), DisconnectCount(0), KillCount(0), DeathCount(0), InfoTimeStamp(0), LocationUpdatedTime(0)
	{
		appMemzero(&Score, sizeof(Score));
	}

	UBOOL IsValid() const
	{
		return GuildPlayerInfo.idAccount != Def::ID_INVALID_ACCOUNT;
	}
	UBOOL IsFullInfo() const
	{
		return InfoTimeStamp > 0 && appSeconds() < InfoTimeStamp + 60;
	}
	void SetFullInfo(UBOOL bFull)
	{
		InfoTimeStamp = (bFull ? appSeconds() : 0);
	}
	UBOOL IsLocationOutdated() const
	{
		return appSeconds() - LocationUpdatedTime > 1.0;
	}
	void SetLocationUpdated()
	{
		LocationUpdatedTime = appSeconds();
	}
	void Clear()
	{
		GuildPlayerInfo.idAccount = Def::ID_INVALID_ACCOUNT;
		InfoTimeStamp = 0.0;
		LocationUpdatedTime = 0.0;
	}

	UBOOL IsOnline() const
	{
		return GuildPlayerInfo.idChannel != Def::ID_INVALID_CHANNEL;
	}
	void SetOffline()
	{
		GuildPlayerInfo.idChannel = Def::ID_INVALID_CHANNEL;
		SetFullInfo(FALSE);
	}

	void Set(Def::GUILD_PLAYER_INFO &Player);
	void Set(Def::PLAYER_INFO &Player);
	void Set(Def::PLAYER_DISP_INFO &Player);

	//Def::GUILD_RANK_INFO& GetAuth() const;
	FLOAT GetSDRatio()
	{
		return ((FLOAT)(Score.Sum() + KillCount)) / (DeathCount > 0 ? ((FLOAT)DeathCount) : 1.0);
	}

	void UpdateNickname(const FString &Nickname, UBOOL bShowMsg = TRUE);
	FString GetLocation();

	void DumpPlayerInfo();
};

struct FGuildPlayerList
{
	enum _DumpFlag
	{
		_DF_ALL,
		_DF_ON,
		_DF_OFF
	};

	TArray<FGuildPlayerInfo> PlayerList;
	INT ListIndex;

	FGuildPlayerList() : ListIndex(-1) {}

	FGuildPlayerInfo* Add(Def::GUILD_PLAYER_INFO &Player);
	FGuildPlayerInfo* Add(Def::PLAYER_INFO &Player);
	FGuildPlayerInfo* Add(Def::PLAYER_DISP_INFO &Player);
	UBOOL Remove(LONG idAccount);
	FGuildPlayerInfo* Find(LONG idAccount);
	FGuildPlayerInfo* Find(const FString &PlayerName);

	FGuildPlayerInfo* GetSelected()
	{
		if (PlayerList.IsValidIndex(ListIndex))
			return &(PlayerList(ListIndex));
		else
			return NULL;
	}

	void Clear()
	{
		ListIndex = -1;
		PlayerList.Empty();
	}

	void DumpPlayerList()
	{
		for (INT i = 0; i < PlayerList.Num(); ++i)
			PlayerList(i).DumpPlayerInfo();
	}
	void DumpToConsole(INT DumpFlag = _DF_ALL);
};


struct FGuildInfo
{
	//static Def::GUILD_RANK_INFO AuthInfo[Def::SIZE_GUILD_RANK];

	Def::GUILD_INFO GuildInfo;
	DOUBLE InfoTimeStamp;
	FGuildPlayerList PlayerList;

	FGuildInfo() : InfoTimeStamp(0)
	{
	}

	UBOOL IsValid()
	{
		//_LOG(TEXT("=== Guild channel is %s ==="), GuildInfo.idGuild != Def::ID_INVALID_GUILD ? TEXT("valid") : TEXT("invalid"));
		return GuildInfo.idGuild != Def::ID_INVALID_GUILD;
	}
	UBOOL IsFullInfo()
	{
		return InfoTimeStamp > 0;
	}
	void SetFullInfo(UBOOL bFull)
	{
		InfoTimeStamp = (bFull ? appSeconds() : 0.0);
	}
	UBOOL IsChannelConnected()
	{
		//_LOG(TEXT("=== Guild channel is %s ==="), IsValid() && GavaNetClient->SessionKeyGuild != 0xff ? TEXT("connected") : TEXT("disconnected"));
		return IsValid() && GavaNetClient->SessionKeyGuild != 0xff;
	}
	UBOOL IsRegularGuild()
	{
		return GuildInfo.level > 0;
	}

	FString GetMasterName()
	{
		FGuildPlayerInfo *Info = PlayerList.Find(GuildInfo.idMaster);
		return Info ? Info->GuildPlayerInfo.nickname : TEXT("");
	}

	void Clear();
	void Set(Def::PLAYER_GUILD_INFO &Info);
	void SetMotd(const TCHAR *Motd);
	void RefreshMemberCount()
	{
		GuildInfo.cntMember = PlayerList.PlayerList.Num();
	}

	void SyncPlayerInfo(FPlayerDispInfo &Player);
	void SyncPlayerInfo(Def::PLAYER_DISP_INFO &Player);
	void SyncPlayerInfo(Def::TID_ACCOUNT idAccount, Def::TID_CHANNEL idChannel);
	void SyncPlayerInfo(Def::TID_ACCOUNT idAccount, Def::TID_CHANNEL idChannel, Def::TID_ROOM idRoom);

	INT GetClanMarkID();

	void Dump();
};

struct FavaNetMapInfo;
struct FavaNetMissionInfo;

struct FMapInfo
{
	INT idMap;
	FString MapName;
	FString ImagePathName;
	FString Description;
	FString FileName;
	TArray<INT> MaxPlayerList;
	TArray<INT> WinCondList;
	INT DefaultMaxPlayer;
	INT DefaultWinCond;
	FString WinCondType;
	BYTE MissionType;
	TArray<BYTE> ExclChannelGroups;
	UBOOL bStatLog;
	UBOOL AllowMapOption;
	UBOOL bHidden;

	FMapInfo() : idMap(0), DefaultMaxPlayer(0), DefaultWinCond(0), MissionType(0), bStatLog(FALSE), AllowMapOption(FALSE), bHidden(FALSE) {}
	FMapInfo( FavaNetMapInfo* MapInfo, FavaNetMissionInfo* MissionInfo );

	void Dump();
};

struct FMapList
{
	TArray<FMapInfo> MapList;

	INT Set();
	//FMapInfo* Add(INT idMap, const FString &MapName, const FString &ImagePathName, const FString &Description,
	//			const FString &FileName, const FString &MaxPlayers, const FString &WinCondition, const BYTE MissionType,const FString &WinConditionType,
	//			const TArray<BYTE>& ExclChannelGroups, UBOOL bStatLog, UBOOL AllowMapOption, UBOOL bHidden = FALSE);
	FMapInfo* Add( FavaNetMapInfo* MapInfo, FavaNetMissionInfo* MissionInfo );

	FMapInfo* Find(INT idMap)
	{
		for (INT i = 0; i < MapList.Num(); ++i)
		{
			if (idMap == MapList(i).idMap)
				return &(MapList(i));
		}
		return NULL;
	}
	FMapInfo* Find(FString &MapName)
	{
		for (INT i = 0; i < MapList.Num(); ++i)
		{
			if (MapName == MapList(i).MapName)
				return &(MapList(i));
		}
		return NULL;
	}
	void Dump()
	{
		for (INT i = 0; i < MapList.Num(); ++i)
		{
			MapList(i).Dump();
		}
	}
};


//struct FPopUpMsgInfo
//{
//	BYTE MsgType;
//	TCHAR PopUpMsg[128];
//	TCHAR NextScene[32];
//	FName NextUIEventName;	
//};

//struct FPopUpMsgList
//{
//	TArray<FPopUpMsgInfo> Msgs;
//
//	INT Num()
//	{
//		return Msgs.Num();
//	}
//
//	void PushPopUpMsg(BYTE MsgType, const FString& PopUpMsg, const FString& NextScene, FName NextUIEventName );
//	UBOOL PopFirstPopUpMsg(FPopUpMsgInfo& Info);
//	UBOOL PopFirstPopUpMsg(BYTE& MsgType, FString& PopUpMsg, FString& NextScene, FName& NextUIEventName );
//	void ClearPopUpMsg()
//	{
//		Msgs.Empty();
//	}
//};

struct FBuyingItemInfo
{
	Def::TID_ITEM idItem;
	Def::TID_ACCOUNT idAccount;

	FBuyingItemInfo()
	{
		Clear();
	}
	void SetProcess(Def::TID_ITEM InItem, Def::TID_ACCOUNT InAccount = Def::ID_INVALID_ACCOUNT)
	{
		idItem = InItem;
		idAccount = InAccount;
	}
	UBOOL IsProcessing() const
	{
		return idItem != Def::ID_INVALID_ITEM || idAccount != Def::ID_INVALID_ACCOUNT;
	}
	void Clear()
	{
		idItem = Def::ID_INVALID_ITEM;
		idAccount = Def::ID_INVALID_ACCOUNT;
	}
};

struct FAutoMoveDest
{
	UBOOL bMove;
	Def::TID_CHANNEL idChannel;
	Def::TID_ROOM idRoom;
	FString RoomName;
	FString RoomPassword;
	Def::TID_ACCOUNT idAccount;
	FString Nickname;

	FAutoMoveDest()
	{
		Clear();
	}

	UBOOL IsMoving() const { return bMove; }
	UBOOL IsFollowing() const { return idAccount != Def::ID_INVALID_ACCOUNT; }
	UBOOL IsNormalChannel() const { return idChannel != Def::ID_INVALID_CHANNEL && idChannel != Def::ID_MY_CLAN_HOME; }
	UBOOL IsGuildChannel() const { return idChannel == Def::ID_MY_CLAN_HOME; }

	void Clear()
	{
		bMove = FALSE;
		idChannel = Def::ID_INVALID_CHANNEL;
		idRoom = Def::ID_INVALID_ROOM;
		RoomName = TEXT("");
		RoomPassword = TEXT("");
		idAccount = Def::ID_INVALID_ACCOUNT;
		Nickname = TEXT("");
	}

	void Pause()
	{
		bMove = FALSE;
	}
	void Continue(const FString &Password)
	{
		if (Password.Len() > 0)
			RoomPassword = Password;
		bMove = TRUE;
	}

	void SetMoveDestTo(Def::TID_CHANNEL Chan, Def::TID_ROOM RoomID = Def::ID_INVALID_ROOM, const FString &InRoomName = TEXT(""),
					Def::TID_ACCOUNT Account = Def::ID_INVALID_ACCOUNT)
	{
		idChannel = Chan;
		idRoom = RoomID;
		RoomName = InRoomName;
		idAccount = Account;
		bMove = TRUE;
	}
	void SetFollowTarget(Def::TID_ACCOUNT Account, const FString &InNickname)
	{
		idAccount = Account;
		Nickname = InNickname;
	}
	UBOOL IsPasswordSet() const { return RoomPassword.Len() > 0; }
};


enum EavaNetControlState
{
	_AN_NONE = -1,
	_AN_DISCONNECTED = 0,
	_AN_CONNECTING,
	_AN_NEWCHARACTER,
	_AN_CHANNELLIST,
	_AN_LOBBY,
	_AN_ROOM,
	_AN_INGAME,
	_AN_INVENTORY,
	_AN_MAX
};



class CavaNetStateController
{
private:
	INT NetState;

	DOUBLE BaseSec;
	__time32_t BaseTime;

public:
	FMapList MapList;

	FPlayerInfo PlayerInfo;
	FGuildInfo GuildInfo;
	FChannelInfo ChannelInfo;
	FRoomInfo RoomInfo;
	BYTE MyRoomSlotIdx;

	FLastResultInfo LastResultInfo;

	TArray<INT> QuickJoinChannelList;
	FChannelList ChannelList;
	FRoomList RoomList;
	FLobbyPlayerList LobbyPlayerList;
	FChatMsgList ChatMsgList;
	FChatMsgList SentChatMsgList;

	//FPopUpMsgList PendingPopUpMsgs;
	FString MainNotice;
	FString RTNotice;

	FString LastConnectResult;
	INT CountDown;

	INT RoomHostKickCount;
	DOUBLE RoomReadyDue;
	INT RoomReadyKickCount;

	DOUBLE ChatOffDue;

	UBOOL StealthMode;

	FAutoMoveDest AutoMoveDest;

	FBuyingItemInfo BuyingItemInfo;

public:
	CavaNetStateController() : NetState(_AN_NONE), MyRoomSlotIdx(Def::ID_INVALID_ROOM_SLOT),
							CountDown(-1), RoomHostKickCount(-1), RoomReadyDue(0.0), RoomReadyKickCount(0), ChatOffDue(0.0),
							StealthMode(FALSE)
	{
	}
	~CavaNetStateController() {}

public:
	void Init();
	void Final();

	void Tick();

	INT GetNetState() const
	{
		return NetState;
	}
	UBOOL IsStatePlaying() const
	{
		return NetState == _AN_INGAME;
	}
	UBOOL IsStateInRoom() const
	{
		return NetState == _AN_ROOM || IsStatePlaying();
	}
	UBOOL IsStateInLobby() const
	{
		return NetState == _AN_LOBBY || NetState == _AN_INVENTORY;
	}

	FString GetNetStateString( BYTE State )
	{
		switch(State)
		{
		case _AN_INGAME:	return TEXT("InGame");
		case _AN_INVENTORY:	return TEXT("Inventory");
		case _AN_LOBBY:		return TEXT("Lobby");
		case _AN_ROOM:		return TEXT("Room");
		case _AN_NEWCHARACTER:	return TEXT("NewCharacter");
		case _AN_CHANNELLIST:	return TEXT("ChannelList");
		}
		return TEXT("None");
	}

	INT GoToState(INT NewState)
	{
		if (NewState <= _AN_NONE || NewState >= _AN_MAX)
			return _AN_NONE;
		if (NewState == NetState)
			return NetState;

		if (!LeavingState(NewState))
			return _AN_NONE;
		if (!EnteringState(NewState))
			return _AN_NONE;

		INT Temp = NetState;
		NetState = NewState;

		return Temp;
	}

	UBOOL LeavingState(INT NewState);
	UBOOL EnteringState(INT NewState);

	FString GetTimeFromAppSec(DOUBLE Sec, UBOOL bAddDate = FALSE);
	INT GetNow();

	void SetLastConnectResult(const FString &Result);

	void CheckRoomHostKickCondition();
	void SetRoomReadyDue();

	void StartCountDown();
	UBOOL ProcCountDown();
	void CancelCountDown();
	UBOOL IsCountingDown() const
	{
		return CountDown > -1;
	}

	UBOOL IsVoteAvailable();

	FChannelInfo* SelectChannel(UBOOL bPreferLastChannel);

	void ClearPlayerInfo();
	void ClearChannelInfo();
	void ClearLobbyInfo();
	void ClearRoomInfo();

	void RefreshCurrentRoom();
	void SetCurrentHostUnreachable();
	UBOOL IsHostUnreachable(TCHAR *HostName);
	UBOOL IsHostUnreachable(INT RoomIndex);

	INT CheckMyLocation();

	FString GetURLString(INT idAccount, const FString &Option);
	void GetOpenURL(FURL &OpenURL);

	UBOOL AmIHost()
	{
		return (RoomInfo.IsValid() && RoomInfo.HostIdx == MyRoomSlotIdx && MyRoomSlotIdx != Def::ID_INVALID_ROOM_SLOT /* Added by deif */);
	}
	UBOOL AmISpectator()
	{
		return (FRoomInfo::IsSlotIDValid(MyRoomSlotIdx, FALSE) && RoomInfo.IsValid()) ? (RoomInfo.PlayerList.PlayerList[MyRoomSlotIdx].GetTeamID() == Def::RT_SPECTATOR) : FALSE;
	}
	UBOOL AmIAdmin()
	{
		return PlayerInfo.IsValid() && PlayerInfo.PrivLevel > 0;
	}
	UBOOL AmIStealthMode()
	{
		return AmIAdmin() && StealthMode;
	}
	UBOOL DoIHaveGuildPriv(Def::_GUILD_PRIV Priv)
	{
		return PlayerInfo.IsValid() && PlayerInfo.PlayerInfo.guildInfo.idGuild != Def::ID_INVALID_GUILD && PlayerInfo.PlayerInfo.guildInfo.HasPriv(Priv);
	}
	UBOOL IsWhisperBlockedByClanMatch()
	{
		return (ChannelInfo.IsFriendlyGuildChannel() && IsStatePlaying() &&
				RoomInfo.IsValid() && RoomInfo.RoomInfo.setting.allowGhostChat == 0);
	}
	UBOOL IsMatchProcessing()
	{
		return (ChannelInfo.IsMatchChannel() && RoomInfo.RoomInfo.state.bMatch > 0 && IsStatePlaying());
	}

	FRoomPlayerInfo* GetMyRoomPlayerInfo()
	{
		return (FRoomInfo::IsSlotIDValid(MyRoomSlotIdx, FALSE) && RoomInfo.IsValid()) ? &(RoomInfo.PlayerList.PlayerList[MyRoomSlotIdx]) : NULL;
	}
	TCHAR* GetMyNickname()
	{
		return PlayerInfo.PlayerInfo.nickname;
	}

	FMapInfo* GetCurrentMap()
	{
		return (RoomInfo.IsValid() ? MapList.Find(RoomInfo.RoomInfo.setting.idMap) : NULL);
	}

	INT GetChannelSetting(BYTE Setting);
	FavaNetChannelSettingInfo& GetChannelSettingInfo();

	FPlayerDispInfo* FindPlayerFromList(const FString &Nickname);
	FPlayerDispInfo* FindPlayerFromList(Def::TID_ACCOUNT idAccount);

	UBOOL ProcAutoMove();
	void StopAutoMove();

	void ProcQuickJoinChannel();

	void ParseConsoleCommand(const TCHAR *Cmd);
	UBOOL ParseChatCommand(const TCHAR *Cmd, UBOOL Team = TRUE);
	void LogChatConsole(const TCHAR *Msg, INT MsgType = 0);
	void LogChatConsoleNoMatch(const TCHAR *Msg, INT MsgType = 0);

	UBOOL CheckChatBlocked(const TCHAR *ChatMsg);
	UBOOL CheckChatProhibition();

	UBOOL ChatSave();

	void SyncPlayerLevel(Def::TID_ACCOUNT idAccount, INT Level);

	FString GetLocationString(UBOOL bOnline, Def::TID_CHANNEL idChannel, Def::TID_ROOM idRoom = Def::ID_INVALID_ROOM);

private:
	void ParseChannelCommand(const TCHAR *Cmd);
	void ParseRoomCommand(const TCHAR *Cmd);
	void ParseGameCommand(const TCHAR *Cmd);
	void ParseGuildCommand(const TCHAR *Cmd);
	void ParseInventoryCommand(const TCHAR *Cmd);
};



#define _StateController	GavaNetClient->StateController


#endif

