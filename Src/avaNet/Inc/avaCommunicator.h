#pragma once


#include "RxGateTranslator/Def.h"
#include "ComDef/Def.h"
#include "CMSMessage.h"


enum _BuddyType
{
	BT_BUDDY_ONESIDE = 1,
	BT_BUDDY_BOTH = 2,
	BT_BUDDY_BIA = 5,
	BT_BUDDY_OTHER = 1,
	BT_BLOCK = 9,
};

struct FPlayerDispInfo;
struct FGuildPlayerInfo;

struct FBuddyInfo
{
	Def::TID_ACCOUNT idAccount;
	FString Nickname;
	WORD BuddyType;
	//Def::TID_SERVER idServer;
	Def::TID_CHANNEL idChannel;
	Def::TID_ROOM idRoom;
	BYTE ServerType;
	BYTE Level;
	Def::TID_GUILD idGuild;
	FString GuildName;
	Def::TID_GUILDMARK idGuildMark;
	UBOOL bOnline;

	DWORD GameWin;						// ÀÌ±ä °ÔÀÓ ¼ö
	DWORD GameDefeat;					// Áø °ÔÀÓ ¼ö
	DWORD DisconnectCount;				// µð½ºÄ¿³ØÆ®ÇÑ È½¼ö
	Def::PLAYER_SCORE_INFO::SCORE_INFO Score;	// ½ºÄÚ¾î
	DWORD KillCount;					// Kill È½¼ö
	DWORD DeathCount;					// Á×Àº È½¼ö

	DOUBLE InfoTimeStamp;
	DOUBLE LocationUpdatedTime;

	FBuddyInfo(WORD _BuddyType, Def::TID_ACCOUNT _idAccount, FString _Nickname) :
			BuddyType(_BuddyType), idAccount(_idAccount), Nickname(_Nickname), bOnline(FALSE), Level(0),
			idChannel(Def::ID_INVALID_CHANNEL), idRoom(Def::ID_INVALID_ROOM), ServerType(0), idGuild(Def::ID_INVALID_GUILD), idGuildMark(Def::ID_INVALID_GUILDMARK),
			GameWin(0), GameDefeat(0), DisconnectCount(0), KillCount(0), DeathCount(0), InfoTimeStamp(0), LocationUpdatedTime(0)
	{
		appMemzero(&Score, sizeof(Score));
	}

	FBuddyInfo(const RXCLIENTCMSPROTOCOL::_BUDDYINFO &Buddy) :
			idGuild(Def::ID_INVALID_GUILD), GameWin(0), GameDefeat(0), DisconnectCount(0), KillCount(0), DeathCount(0), InfoTimeStamp(0)
	{
		appMemzero(&Score, sizeof(Score));
		operator=(Buddy);
	}

	FBuddyInfo& operator=(const RXCLIENTCMSPROTOCOL::_BUDDYINFO &Buddy)
	{
		idAccount = Buddy.userid;
		Nickname = Buddy.beforenickname;
		BuddyType = Buddy.friendtype;
		GuildName = Buddy.clanname;
		ServerType = Buddy.loinfo.servertype;
		idChannel = Buddy.loinfo.channelid;
		idGuild = Buddy.loinfo.clanid;
		Level = Buddy.level;
		bOnline = (Buddy.bOnlineState ? TRUE : FALSE);
		return *this;
	}
	FBuddyInfo& operator=(const RXCLIENTCMSPROTOCOL::_PAIRBUDDYINFO &Buddy)
	{
		BuddyType = Buddy.ftype;
		Level = Buddy.level;
		ServerType = Buddy.loinfo.servertype;
		idChannel = Buddy.loinfo.channelid;
		idGuild = Buddy.loinfo.clanid;
		return *this;
	}
	FBuddyInfo& operator=(const RXCLIENTCMSPROTOCOL::LOCATIONINFO &Location)
	{
		ServerType = Location.servertype;
		idChannel = Location.channelid;
		idGuild = Location.clanid;
		return *this;
	}
	UBOOL IsOnline() const { return IsBuddyBoth() && bOnline; }
	UBOOL IsFullInfo() const { return IsOnline() && InfoTimeStamp > 0.0; }
	UBOOL IsLocationOutdated() const { return appSeconds() - LocationUpdatedTime > 1.0; }
	void SetLocationUpdated()
	{
		LocationUpdatedTime = appSeconds();
	}
	void SetOffline()
	{
		bOnline = FALSE;
		Level = 0;
		ServerType = 0;
		idChannel = Def::ID_INVALID_CHANNEL;
		idGuild = Def::ID_INVALID_GUILD;
		idRoom = Def::ID_INVALID_ROOM;
		LocationUpdatedTime = 0.0;
		SetFullInfo(FALSE);
	}
	void SetFullInfo(UBOOL bFull)
	{
		InfoTimeStamp = (bFull ? appSeconds() : 0.0);
	}
	FString GetLocation();

	FLOAT GetSDRatio()
	{
		return ((FLOAT)(Score.Sum() + KillCount)) / (DeathCount > 0 ? (FLOAT)DeathCount : 1.0);
	}

	INT GetClanMarkID()
	{
		return idGuild != Def::ID_INVALID_GUILD ? idGuildMark : -1;
	}

	UBOOL IsBuddy() const { return IsBuddyOneSide() || IsBuddyBoth(); }
	UBOOL IsBlock() const { return BuddyType == BT_BLOCK; }
	UBOOL IsBuddyOneSide() const { return BuddyType == BT_BUDDY_ONESIDE; }
	UBOOL IsBuddyBoth() const { return BuddyType == BT_BUDDY_BOTH || IsBuddyBIA(); }
	UBOOL IsBuddyBIA() const { return BuddyType == BT_BUDDY_BIA; }

	void UpdateNickname(const FString &Nickname, UBOOL bShowMsg = TRUE);
};

struct FBuddyList
{
	TArray<FBuddyInfo> BuddyList;
	INT ListIndex;

	FBuddyList() : ListIndex(-1) {}

	INT Num() const { return BuddyList.Num(); }
	FBuddyInfo* Add(WORD BuddyType, Def::TID_ACCOUNT idAccount, FString Nickname)
	{
		FBuddyInfo *Info = Find(idAccount);
		if (!Info)
		{
			return new(BuddyList) FBuddyInfo(BuddyType, idAccount, Nickname);
		}
		return Info;
	}
	FBuddyInfo* Add(const RXCLIENTCMSPROTOCOL::_BUDDYINFO &Buddy)
	{
		FBuddyInfo *Info = Find(Buddy.userid);
		if (!Info)
		{
			return new(BuddyList) FBuddyInfo(Buddy);
		}
		return Info;
	}
	FBuddyInfo* Add(const FBuddyInfo &Buddy)
	{
		FBuddyInfo *Info = Find(Buddy.idAccount);
		if (!Info)
		{
			return new(BuddyList) FBuddyInfo(Buddy);
		}
		return Info;
	}
	FBuddyInfo* Find(Def::TID_ACCOUNT idAccount)
	{
		for (INT i = 0; i < BuddyList.Num(); ++i)
		{
			if (BuddyList(i).idAccount == idAccount)
			{
				return &BuddyList(i);
			}
		}
		return NULL;
	}
	FBuddyInfo* Find(const FString &Nickname)
	{
		if (Nickname.Len() < 3)
			return NULL;
		for (INT i = 0; i < BuddyList.Num(); ++i)
		{
			if (BuddyList(i).Nickname == Nickname)
			{
				return &BuddyList(i);
			}
		}
		return NULL;
	}
	FBuddyInfo* Find(const FString &Nickname, INT& ListIndex)
	{
		if (Nickname.Len() < 3)
			return NULL;
		for (INT i = 0; i < BuddyList.Num(); ++i)
		{
			if (BuddyList(i).Nickname == Nickname)
			{
				ListIndex = i;
				return &BuddyList(i);
			}
		}
		return NULL;
	}
	void Remove(Def::TID_ACCOUNT idAccount)
	{
		for (INT i = 0; i < BuddyList.Num(); ++i)
		{
			if (BuddyList(i).idAccount == idAccount)
			{
				BuddyList.Remove(i);
				return;
			}
		}
	}
	void Clear()
	{
		BuddyList.Empty();
	}
	UBOOL IsOnline(Def::TID_ACCOUNT idAccount)
	{
		FBuddyInfo *Buddy = Find(idAccount);
		return Buddy ? Buddy->IsOnline() : FALSE;
	}

	FBuddyInfo* GetSelected()
	{
		if (BuddyList.IsValidIndex(ListIndex))
			return &(BuddyList(ListIndex));
		else
			return NULL;
	}

	FBuddyInfo& operator()(INT i)
	{
		checkSlow(i >= 0);
		checkSlow(i < BuddyList.Num() || (i==0 && BuddyList.Num() ==0));
		return BuddyList(i);
	}
};


class CavaCommunicator
{
public:
	enum { MaxListSize = 100 };

	enum _DumpFlag
	{
		_DF_ALL,
		_DF_ON,
		_DF_OFF
	};

	enum _BuddyConst
	{
		BUDDY_ACCEPT = 0,
		BUDDY_REJECT = 110,
	};

public:
	CavaCommunicator();
	virtual ~CavaCommunicator();

	UBOOL Init();
	void Clear();
	void SetGateAddress(RxGate::LPRXNERVE_ADDRESS pAddress) { GateAddr = *pAddress; }
	RxGate::LPRXNERVE_ADDRESS GetGateAddress() { return &GateAddr; }
	RxGate::LPRXNERVE_ADDRESS GetCMSAddress() { return &CMSAddr; }

	void DumpBuddyListToConsole(INT DumpFlag = _DF_ALL);
	void DumpBlockListToConsole();

	INT GetListCount() const { return BuddyList.Num() + BlockList.Num(); }
	INT GetListMaxCount() const { return MaxListSize; }
	UBOOL IsListFull() const { return GetListCount() >= MaxListSize; }

	void SyncPlayerInfo(FPlayerDispInfo &Player);
	void SyncPlayerInfo(Def::PLAYER_DISP_INFO &Player);
	void SyncPlayerInfo(FGuildPlayerInfo &Player);
	void SyncPlayerInfo(Def::TID_ACCOUNT idAccount, Def::TID_CHANNEL idChannel);
	void SyncPlayerInfo(Def::TID_ACCOUNT idAccount, Def::TID_CHANNEL idChannel, Def::TID_ROOM idRoom);

	UBOOL IsMsgCoded(const TCHAR *Msg);
	INT ParseCodedMsgType(const TCHAR *Msg, INT DefaultType);
	FString ParseCodedMsg(const TCHAR *Msg);

	UBOOL CheckFollowable(Def::TID_ACCOUNT idAccount, UBOOL bShowMsg = TRUE);

	FBuddyInfo *GetBIA();

	////////////////////////////////////////////////////////////////////
	// send
	void AddBuddy(UBOOL bForce, Def::TID_ACCOUNT idAccount, FString Nickname = TEXT(""));
	void AddBuddyAns(UBOOL bYes, Def::TID_ACCOUNT idAccount = Def::ID_INVALID_ACCOUNT);
	void AddBuddyAns(UBOOL bYes, const FString &Nickname);
	void AddBlock(UBOOL bForce, Def::TID_ACCOUNT idAccount, const FString &Nickname);
	void DeleteBuddy(Def::TID_ACCOUNT idAccount);
	void DeleteBuddy(const FString &Nickname);
	void DeleteBlock(Def::TID_ACCOUNT idAccount);
	void DeleteBlock(const FString &Nickname);
	void InviteGame(Def::TID_ACCOUNT idAccount);
	void InviteGame(const FString &Nickname);
	void InviteGameAns(UBOOL bYes, Def::TID_ACCOUNT idAccount = Def::ID_INVALID_ACCOUNT);
	void InviteGameAns(UBOOL bYes, const FString &Nickname);
	void MoveToPlayer(Def::TID_ACCOUNT idAccount, Def::TID_ROOM idRoom);
	void InviteGuild(Def::TID_ACCOUNT idAccount, const FString &Nickname);
	void InviteGuild(const FString &Nickname);
	void InviteGuildAns(UBOOL bYes, Def::TID_ACCOUNT idAccount = Def::ID_INVALID_ACCOUNT);
	void InviteGuildAns(UBOOL bYes, const FString &Nickname);
	void JoinGuild(Def::TID_ACCOUNT idAccount, RxGate::LPRXNERVE_ADDRESS GCSAddr, Def::TID_GUILD idGuild);
	void BuddyChat(Def::TID_ACCOUNT idAccount, const FString &ChatMsg, UBOOL bShowMsg = TRUE);
	void BuddyChat(const FString &Nickname, const FString &ChatMsg, UBOOL bShowMsg = TRUE);
	void Whisper(const FString &Nickname, const FString &ChatMsg, UBOOL bAutoReply = FALSE);
	void GMWhisper(const FString &Nickname, const FString &ChatMsg);
	void Reply(const FString &ChatMsg);
	void GetLocationInfo(Def::TID_ACCOUNT idAccount);
	void GetLocationInfo(const FString &Nickname);
	void GetBuddyList();
	void BeginBuddyList();
	void EndBuddyList();

	void FollowPlayer(Def::TID_ACCOUNT idAccount, const FString &Nickname, Def::TID_GUILD idGuild, Def::TID_CHANNEL idChannel);

	////////////////////////////////////////////////////////////////////
	// recv
	UBOOL ProcMsg(BYTE *buf, DWORD bufLen);

public:
	void ProcAddBuddyReq(BYTE BuddyType, Def::TID_ACCOUNT idAccount, const FString &Nickname);
	void ProcAddBuddyAns(DWORD errCode, Def::TID_ACCOUNT idAccount, const FString &Nickname);
	void ProcInviteGame(Def::TID_ACCOUNT idInvitor, const FString &InvitorName, RXCLIENTCMSPROTOCOL::LOCATIONINFO &Location, const FString &RoomName, Def::TID_ROOM idRoom);
	void ProcInviteGuild(Def::TID_ACCOUNT idInvitor, const FString &InvitorName, Def::TID_GUILD idGuild, const FString &GuildName);
	void ProcInviteGuildAns(DWORD errCode, Def::TID_ACCOUNT idAccount);
	void ProcJoinGuild(RxGate::LPRXNERVE_ADDRESS GCSAddr, Def::TID_GUILD idGuild);
	void ProcBuddyChat(Def::TID_ACCOUNT idSender, const FString &SenderName, const FString &ChatMsg);
	void ProcLocationInfo(DWORD errCode, Def::TID_ACCOUNT idAccount, RXCLIENTCMSPROTOCOL::LOCATIONINFO &Location);
	void ProcBuddyStateChanged(RXCLIENTCMSPROTOCOL::_BUDDYINFO &BuddyInfo);
	void ProcBuddyInfo(BYTE Count, TArray<RXCLIENTCMSPROTOCOL::_BUDDYINFO> &InfoList);
	void ProcPairBuddyInfo(BYTE Err, BYTE Count, TArray<RXCLIENTCMSPROTOCOL::_PAIRBUDDYINFO> &InfoList);
	void ProcUpdateNickName(Def::TID_ACCOUNT idAccount, const FString &Nickname);
	void ProcPCBUserTimeExpire(BYTE Ret);
	void ProcPCBRemainTime(const FString &StrTime);

public:
	FBuddyList BuddyList;
	FBuddyList BlockList;
	//FBuddyList RequestedList;
	FBuddyList ToAnswerList;
	FBuddyList GameInvitorList;
	FBuddyList GuildInvitorList;
	FBuddyList GuildInviteList;

	UBOOL bListingBuddy;

	FString LastWhisperedPlayer;

private:
	RxGate::RXNERVE_ADDRESS CMSAddr;
	RxGate::RXNERVE_ADDRESS GateAddr;

};


inline CavaCommunicator& _Communicator()
{
	static CavaCommunicator _comm;
	return _comm;
}
