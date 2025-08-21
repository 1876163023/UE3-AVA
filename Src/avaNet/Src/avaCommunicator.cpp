#include "avaNet.h"
#include "avaCommunicator.h"
#include "avaNetStateController.h"
#include "avaConnection.h"
#include "avaMsgSend.h"
#include "avaWebInClient.h"

#include "RxGateTranslator/RxGateTranslator.h"
#include "RxGateTranslator/Stream.h"
#include "ComDef/WordCensor.h"

using namespace Def;
using namespace RxGate;
using namespace RXCLIENTCMSPROTOCOL;



#define CHECK_GUILD_PRIV(_p)																										\
	if (!_StateController->DoIHaveGuildPriv(_p))																					\
	{																																\
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoPriv"), TEXT("AVANET")),			\
										EChat_GuildSystem);																			\
		return;																														\
	}



FString FBuddyInfo::GetLocation()
{
	if (!IsBuddyBoth())
		return TEXT("-");
	return _StateController->GetLocationString(IsOnline(), (ServerType == FChannelInfo::CT_GUILD ? ID_MY_CLAN_HOME : idChannel), idRoom);
}

void FBuddyInfo::UpdateNickname(const FString &InNickname, UBOOL bShowMsg)
{
	if (Nickname != InNickname)
	{
		_LOG(TEXT("Nickname changed; [%d]%s -> %s"), idAccount, *Nickname, *InNickname);

		if (bShowMsg)
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_UpdateNickname"), TEXT("AVANET")), *Nickname, *InNickname),
											EChat_PlayerSystem);
		Nickname = InNickname;

		GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_List, TEXT("ok"), TEXT(""), _Communicator().BuddyList.Num(), 0);
	}

	// 가능하면 클랜원 목록에서도 업데이트
	if (_StateController->GuildInfo.IsValid())
	{
		FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(idAccount);
		if (Info && InNickname != Info->GuildPlayerInfo.nickname)
		{
			appStrncpy(Info->GuildPlayerInfo.nickname, *InNickname, SIZE_NICKNAME+1);
			GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MemberList, TEXT("ok"), TEXT(""), 0, 0);

			// 다른 클랜원들에게 알려줌
			PM::GUILD::NICKNAME_UPDATE_NTF::Send(idAccount, InNickname);
		}
	}
}



CavaCommunicator::CavaCommunicator() : bListingBuddy(FALSE)
{
	if ( !Init() )
	{
	}
}

CavaCommunicator::~CavaCommunicator()
{
}

UBOOL CavaCommunicator::Init()
{
	FString CMSAddress;
	if ( !GConfig->GetString(CFG_SECTION, CFG_CMSADDRESS, CMSAddress, GNetIni) )
	{
		_LOG(TEXT("CMSAddress configuration is not found; selecting default configuration."));
		CMSAddress = TEXT("CMSMULTI");
	}

	_LOG(TEXT("CMS Address is %s"), *CMSAddress);

	CMSAddr.addrType = 0;
	WideCharToMultiByte(CP_ACP, 0, *CMSAddress, -1, (LPSTR)CMSAddr.address, RXNERVE_ADDRESS_SIZE, NULL, NULL);

	return TRUE;
}

void CavaCommunicator::Clear()
{
	BuddyList.Clear();
	BlockList.Clear();
	//RequestedList.Clear();
	ToAnswerList.Clear();
	GameInvitorList.Clear();
	GuildInvitorList.Clear();
	GuildInviteList.Clear();
}

void CavaCommunicator::DumpBuddyListToConsole(INT DumpFlag)
{
	INT Cnt = 0;
	for (INT i = 0; i < _Communicator().BuddyList.Num(); ++i)
	{
		FBuddyInfo &Buddy = _Communicator().BuddyList(i);

		if ( (DumpFlag == _DF_ON && !Buddy.IsOnline()) ||
			(DumpFlag == _DF_OFF && Buddy.IsOnline()) )
			continue;

		_StateController->LogChatConsole(*FString::Printf(TEXT("%s - %s"), *Buddy.Nickname, *Buddy.GetLocation()/*, Buddy.ServerType, Buddy.idChannel, Buddy.idGuild*/), EChat_PlayerSystem);

		++Cnt;
	}

	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_List"), TEXT("AVANET")), Cnt),
									EChat_PlayerSystem);
}

void CavaCommunicator::DumpBlockListToConsole()
{
	FString Msg = FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Block_List"), TEXT("AVANET")), BlockList.Num());
	_StateController->LogChatConsole(*Msg, EChat_PlayerSystem);
	for (INT i = 0; i < _Communicator().BlockList.Num(); ++i)
	{
		_StateController->LogChatConsole(*_Communicator().BlockList(i).Nickname, EChat_PlayerSystem);
	}
}


void CavaCommunicator::SyncPlayerInfo(FPlayerDispInfo &Player)
{
	FBuddyInfo *Info = BuddyList.Find(Player.PlayerInfo.idAccount);
	if (Info)
	{
		if (Info->InfoTimeStamp > 0.0 && Info->InfoTimeStamp >= Player.InfoTimeStamp)
			return;

		Info->idRoom = Player.PlayerInfo.idRoom;
		Info->GuildName = Player.PlayerInfo.guildName;
		Info->idGuildMark = Player.PlayerInfo.idGuildMark;
		Info->GameWin = Player.PlayerInfo.gameWin;
		Info->GameDefeat = Player.PlayerInfo.gameDefeat;
		Info->Score = Player.PlayerInfo.scoreInfo;
		Info->KillCount = Player.PlayerInfo.killCount;
		Info->DeathCount = Player.PlayerInfo.deathCount;
		Info->DisconnectCount = Player.PlayerInfo.disconnectCount;
		Info->Level = Player.PlayerInfo.level;
		Info->SetFullInfo(TRUE);
	}
}

void CavaCommunicator::SyncPlayerInfo(Def::PLAYER_DISP_INFO &Player)
{
	_LOG(TEXT("idAccount = %d"), Player.idAccount);
	FBuddyInfo *Info = BuddyList.Find(Player.idAccount);
	if (Info)
	{
		Info->idRoom = Player.idRoom;
		Info->GuildName = Player.guildName;
		Info->idGuildMark = Player.idGuildMark;
		Info->GameWin = Player.gameWin;
		Info->GameDefeat = Player.gameDefeat;
		Info->Score = Player.scoreInfo;
		Info->KillCount = Player.killCount;
		Info->DeathCount = Player.deathCount;
		Info->DisconnectCount = Player.disconnectCount;
		Info->Level = Player.level;
		Info->SetFullInfo(TRUE);
		_LOG(TEXT("idGuildMark = %d"), Info->idGuildMark);
	}
}

void CavaCommunicator::SyncPlayerInfo(FGuildPlayerInfo &Player)
{
	FBuddyInfo *Info = BuddyList.Find(Player.GuildPlayerInfo.idAccount);
	if (Info)
	{
		if (Info->InfoTimeStamp > 0.0 && Info->InfoTimeStamp >= Player.InfoTimeStamp)
			return;

		Info->idChannel = Player.idChannel;
		Info->idRoom = Player.idRoom;
		if (Info->idGuild == _StateController->GuildInfo.GuildInfo.idGuild)
			Info->idGuildMark = _StateController->GuildInfo.GuildInfo.idGuildMark;
		Info->GameWin = Player.GameWin;
		Info->GameDefeat = Player.GameDefeat;
		Info->Score = Player.Score;
		Info->KillCount = Player.KillCount;
		Info->DeathCount = Player.DeathCount;
		Info->DisconnectCount = Player.DisconnectCount;
		Info->Level = Player.GuildPlayerInfo.level;
		Info->SetFullInfo(TRUE);
	}
}

void CavaCommunicator::SyncPlayerInfo(Def::TID_ACCOUNT idAccount, Def::TID_CHANNEL idChannel)
{
	if (idAccount == ID_INVALID_ACCOUNT)
		return;

	FBuddyInfo *Info = BuddyList.Find(idAccount);
	if (Info)
	{
		Info->ServerType = (idChannel == ID_INVALID_CHANNEL ? FChannelInfo::CT_UNKNOWN : idChannel == ID_MY_CLAN_HOME ? FChannelInfo::CT_GUILD : FChannelInfo::CT_NORMAL);
		if (Info->ServerType == FChannelInfo::CT_NORMAL)
			Info->idChannel = idChannel;
		Info->bOnline = TRUE;
	}
}

void CavaCommunicator::SyncPlayerInfo(Def::TID_ACCOUNT idAccount, Def::TID_CHANNEL idChannel, Def::TID_ROOM idRoom)
{
	if (idAccount == ID_INVALID_ACCOUNT)
		return;

	FBuddyInfo *Info = BuddyList.Find(idAccount);
	if (Info)
	{
		Info->ServerType = (idChannel == ID_INVALID_CHANNEL ? FChannelInfo::CT_UNKNOWN : idChannel == ID_MY_CLAN_HOME ? FChannelInfo::CT_GUILD : FChannelInfo::CT_NORMAL);
		if (Info->ServerType == FChannelInfo::CT_NORMAL)
			Info->idChannel = idChannel;
		Info->idRoom = idRoom;
		Info->SetLocationUpdated();
		Info->bOnline = TRUE;
	}
}

UBOOL CavaCommunicator::IsMsgCoded(const TCHAR *Msg)
{
	return Msg[0] == TEXT('\n');
}

INT CavaCommunicator::ParseCodedMsgType(const TCHAR *Msg, INT DefaultType)
{
	if (IsMsgCoded(Msg))
	{
		FString Str = Msg + 1;
		INT Pos = Str.InStr(TEXT("\n"));
		if (Pos <= 0)
			return DefaultType;
		INT MsgType = appAtoi(*Str.Left(Pos));
		return (MsgType >= 0 && MsgType < EChat_MAX) ? MsgType : DefaultType;
	}

	return DefaultType;
}

FString CavaCommunicator::ParseCodedMsg(const TCHAR *InMsg)
{
	TCHAR *Msg = const_cast<TCHAR*>(InMsg);
	if (IsMsgCoded(Msg))
	{
		++Msg;

		BREAK_SECTION_BEGIN()
		{
			TArray<FString> ArgList;
			if (FString(Msg).ParseIntoArray(&ArgList, TEXT("\n"), FALSE) < 2)
				break;
			if (ArgList.Num() == 2)
			{
				// MsgType 지시자만 있음
				return ArgList(1);
			}

			FString Str = Localize(TEXT("ChatConsoleMessage"), *ArgList(1), TEXT("AVANET"));
			for (INT i = 2; i < ArgList.Num(); ++i)
			{
				INT Pos = Str.InStr(TEXT("%s"));
				if (Pos < 0)
					break;
				Str = Str.Left(Pos) + ArgList(i) + Str.Right(Str.Len() - Pos - 2);
			}

			return Str;
		}
		BREAK_SECTION_END()
	}

	return Msg;
}

UBOOL CavaCommunicator::CheckFollowable(TID_ACCOUNT idAccount, UBOOL bShowMsg)
{
	FBuddyInfo *Buddy = BuddyList.Find(idAccount);

	if (Buddy)
	{
		if (!Buddy->IsOnline())
		{
			if (bShowMsg)
				_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerOffline"), TEXT("AVANET")), *Buddy->Nickname),
												EChat_PlayerSystem);
			return FALSE;
		}

		if (Buddy->idChannel == ID_INVALID_CHANNEL || Buddy->ServerType == FChannelInfo::CT_UNKNOWN)
		{
			if (bShowMsg)
				_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowToChannelList"), TEXT("AVANET")), *Buddy->Nickname),
												EChat_PlayerSystem);
			return FALSE;
		}

		if (Buddy->ServerType == FChannelInfo::CT_GUILD)
		{
			if (Buddy->idGuild == ID_INVALID_GUILD)
				return FALSE;
			if (Buddy->idGuild == _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild)
			{
				return TRUE;
			}
			else
			{
				// 상대방이 나와 다른 클랜이면서 클랜홈에 있으면
				if (bShowMsg)
					_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowToClanHome"), TEXT("AVANET")), *Buddy->Nickname),
													EChat_PlayerSystem);
				return FALSE;
			}
		}
		else
		{
			FChannelInfo *Info = _StateController->ChannelList.Find(Buddy->idChannel);
			if (Info)
			{
				FString Res = Info->IsJoinable();
				if (Res == TEXT("ok"))
				{
					return TRUE;
				}
				//else if (Res == TEXT("regular guild only"))
				//{
				//	// 상대방이 친선 클랜전 채널에 있고 내가 정규 클랜원이 아니면
				//	if (bShowMsg)
				//		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowToFriendlyClanMatch"), TEXT("AVANET")), *Buddy->Nickname),
				//										EChat_PlayerSystem);
				//	return FALSE;
				//}
				//else if (Res == TEXT("trainee only"))
				//{
				//	// 상대방이 계급 제한 때문에 들어갈 수 없는 채널에 있으면
				//	if (bShowMsg)
				//		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowToChannelLevel"), TEXT("AVANET")), *Buddy->Nickname),
				//										EChat_PlayerSystem);
				//	return FALSE;
				//}
				//else if (Res == TEXT("newbie only"))
				//{
				//	// 상대방이 S/D 제한 때문에 들어갈 수 없는 채널에 있으면
				//	if (bShowMsg)
				//		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowToChannelLevel"), TEXT("AVANET")), *Buddy->Nickname),
				//										EChat_PlayerSystem);
				//	return FALSE;
				//}
				//else if (Res == TEXT("pcbang only"))
				//{
				//	// 상대방이 PC방 채널에 있고 내가 들어갈 수 없으면
				//	if (bShowMsg)
				//		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowToChannelPCBang"), TEXT("AVANET")), *Buddy->Nickname),
				//										EChat_PlayerSystem);
				//	return FALSE;
				//}
				//else if (Res == TEXT("no priv"))
				//{
				//	// 상대방이 접근 제한 걸린 채널(대회 채널)에 있고 내가 들어갈 권한이 없으면
				//	if (bShowMsg)
				//		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowToChannelSDRatio"), TEXT("AVANET")), *Buddy->Nickname),
				//										EChat_PlayerSystem);
				//	return FALSE;
				//}
				else
				{
					// 상대방이 내가 들어갈 수 없는 채널에 있으면
					if (bShowMsg)
						_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowToChannel"), TEXT("AVANET")), *Buddy->Nickname),
														EChat_PlayerSystem);
					return FALSE;
				}
			}

			return FALSE;
		}
	}

	return FALSE;
}

FBuddyInfo* CavaCommunicator::GetBIA()
{
	for (INT i = 0; i < BuddyList.Num(); ++i)
	{
		if (BuddyList(i).IsBuddyBIA())
			return &BuddyList(i);
	}
	return NULL;
}


////////////////////////////////////////////////////////////////////
// send

inline DWORD OdlStrLen(const FString &str)
{
	return sizeof(ODL_STRLEN) + str.Len() * sizeof(TCHAR);
}

extern void WriteHeader(CRxWriteStream &stream, RXMSGID id, DWORD len);

//void WriteHeader(CRxWriteStream &stream, RXMSGID id, DWORD len)
//{
//	_MSGHDR hdr;
//	hdr.id = id;
//	hdr.len = len;
//	stream.Write((LPVOID)&hdr, sizeof(_MSGHDR));
//}

#define WRITE_HEADER()	WriteHeader(stream, msg.GetID(), msg.GetBodyLength())


#define SEND_TO_CMS()																	\
	{																					\
		ScopedMsgBufPtr MsgBuf = CreateMsgBufN(1024);									\
		check(MsgBuf);																	\
		if ( RxGateTranslator::MsgData(MsgBuf, GavaNetClient->ClientKey, &GavaNetClient->CurrentClientAddress, &CMSAddr, buf) )	\
		{																				\
			GavaNetClient->CurrentConnection()->Send(MsgBuf);							\
			_LOG(TEXT("[%s] Sent."), msg.GetName());									\
		}																				\
	}



void CavaCommunicator::AddBuddy(UBOOL bForce, TID_ACCOUNT InAccountID, FString InNickname)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;

	TID_ACCOUNT idAccount = InAccountID;

	if (idAccount == ID_INVALID_ACCOUNT)
	{
		FPlayerDispInfo *Player = _StateController->FindPlayerFromList(Nickname);
		if (Player)
		{
			idAccount = Player->PlayerInfo.idAccount;
		}
		else if (_StateController->GuildInfo.IsValid())
		{
			FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(Nickname);
			if (Info)
				idAccount = Info->GuildPlayerInfo.idAccount;
		}
	}

	if (idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotAddYourself"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	if ( IsListFull() )
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_ListFull"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	if (idAccount == ID_INVALID_ACCOUNT)
	{
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Nickname),
										EChat_PlayerSystem);
		return;
	}

	if (BuddyList.Find(idAccount))
	{
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_AlreadyExists"), TEXT("AVANET")), *Nickname),
										EChat_PlayerSystem);
		return;
	}

	if (BlockList.Find(idAccount))
	{
		// this player is currently blocked
		if (!bForce)
		{
			// "This player will be removed from your block list. Are you sure?"
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerInBlockList"), TEXT("AVANET")), *Nickname),
											EChat_PlayerSystem);
			//_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_ForceAdd"), TEXT("AVANET")), *Nickname),
			//								EChat_PlayerSystem);
			//GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_AddReq, TEXT("blocked"), TEXT(""), 0, 0);
			return;
		}

		BlockList.Remove(idAccount);
		GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_DeleteBlock, TEXT("deleted"), *Nickname, idAccount, 0);
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Block_Removed"), TEXT("AVANET")), *Nickname),
										EChat_PlayerSystem);
	}

	{
		ScopedMsgBufPtr buf = CreateMsgBufN(1024);
		check(buf);
		CRxWriteStream stream(buf);
		DAT_MsgAddBuddyReq msg;

		msg.BuddyType = BT_BUDDY_ONESIDE;
		msg.FromidAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
		msg.ToidAccount = idAccount;
		msg.FromNick = _StateController->PlayerInfo.PlayerInfo.nickname;

		WRITE_HEADER();
		if ( !msg.Write(&stream) )
		{
			// error
			return;
		}

		SEND_TO_CMS();
	}

	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_Added"), TEXT("AVANET")), *Nickname),
									EChat_PlayerSystem);
	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_AddRequestSent"), TEXT("AVANET")), *Nickname),
									EChat_PlayerSystem);

	//RequestedList.Add(msg.BuddyType, idAccount, Nickname);
	BuddyList.Add(BT_BUDDY_ONESIDE, idAccount, Nickname);

	GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_AddReq, TEXT("sent"), *Nickname, idAccount, 0);
}

void CavaCommunicator::AddBuddyAns(UBOOL bYes, TID_ACCOUNT idAccount)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	if ( IsListFull() )
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_ListFull"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	if (idAccount == ID_INVALID_ACCOUNT)
	{
		if (ToAnswerList.Num() > 0)
			idAccount = ToAnswerList.BuddyList(0).idAccount;
	}

	FBuddyInfo *Buddy = ToAnswerList.Find(idAccount);
	if (!Buddy)
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_NoPlayerToReply"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	{
		ScopedMsgBufPtr buf = CreateMsgBufN(1024);
		check(buf);
		CRxWriteStream stream(buf);
		DAT_MsgAddBuddyAns msg;

		msg.errCode = (bYes ? BUDDY_ACCEPT : BUDDY_REJECT);
		msg.FromidAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
		msg.ToidAccount = idAccount;
		msg.FromNick = _StateController->PlayerInfo.PlayerInfo.nickname;
		//msg.Toloinfo.channelid = _StateController->ChannelInfo.idChannel;
		//msg.Toloinfo.servertype = _StateController->ChannelInfo.GetChannelType();
		//msg.Toloinfo.clanid = 0;

		WRITE_HEADER();
		if ( !msg.Write(&stream) )
		{
			// error
			return;
		}

		SEND_TO_CMS();
	}

	if (bYes)
	{
		Buddy->BuddyType = BT_BUDDY_BOTH;
		Buddy->bOnline = TRUE;
		BuddyList.Add(*Buddy);
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_Added"), TEXT("AVANET")), *Buddy->Nickname),
										EChat_PlayerSystem);

		GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_AddReq, TEXT("accepted"), *Buddy->Nickname, idAccount, 0);

		//GetLocationInfo(idAccount);
	}
	else
	{
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_RejectAddRequest"), TEXT("AVANET")), *Buddy->Nickname),
										EChat_PlayerSystem);
	}

	ToAnswerList.Remove(idAccount);
}

void CavaCommunicator::AddBuddyAns(UBOOL bYes, const FString &InNickname)
{
	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
	{
		AddBuddyAns(bYes);
	}
	else
	{
		FBuddyInfo *Buddy = ToAnswerList.Find(Nickname);
		if (!Buddy)
		{
			_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_NoPlayerToReply"), TEXT("AVANET")),
											EChat_PlayerSystem);
			return;
		}

		AddBuddyAns(bYes, Buddy->idAccount);
	}
}

void CavaCommunicator::AddBlock(UBOOL bForce, TID_ACCOUNT InAccountID, const FString &InNickname)
{
	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;

	if (!_StateController->PlayerInfo.IsValid())
		return;

	TID_ACCOUNT idAccount = InAccountID;

	if (idAccount == ID_INVALID_ACCOUNT)
	{
		FPlayerDispInfo *Player = _StateController->FindPlayerFromList(Nickname);
		if (Player)
		{
			idAccount = Player->PlayerInfo.idAccount;
		}
		else if (_StateController->GuildInfo.IsValid())
		{
			FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(Nickname);
			if (Info)
				idAccount = Info->GuildPlayerInfo.idAccount;
		}
	}

	if (idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Block_CannotAddYourself"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	if ( IsListFull() )
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_ListFull"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	if (idAccount == ID_INVALID_ACCOUNT)
	{
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Nickname),
										EChat_PlayerSystem);
		return;
	}

	if (BlockList.Find(idAccount))
	{
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Block_AlreadyExists"), TEXT("AVANET")), *Nickname),
										EChat_PlayerSystem);
		return;
	}

	FBuddyInfo *Buddy = BuddyList.Find(idAccount);
	if (Buddy)
	{
		// this player is buddy
		if (!bForce)
		{
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Block_PlayerInBuddyList"), TEXT("AVANET")), *Nickname),
											EChat_PlayerSystem);
			return;
		}
		if (Buddy->IsBuddyBIA())
		{
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Block_CannotAddBIA"), TEXT("AVANET")), *Nickname),
											EChat_PlayerSystem);
			return;
		}

		BuddyList.Remove(idAccount);
		GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_Delete, TEXT("deleted"), *Nickname, idAccount, 0);
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_Removed"), TEXT("AVANET")), *Nickname),
										EChat_PlayerSystem);
	}

	{
		ScopedMsgBufPtr buf = CreateMsgBufN(1024);
		check(buf);
		CRxWriteStream stream(buf);
		DAT_MsgAddBuddyReq msg;

		msg.BuddyType = BT_BLOCK;
		msg.FromidAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
		msg.ToidAccount = idAccount;
		msg.FromNick = _StateController->PlayerInfo.PlayerInfo.nickname;

		WRITE_HEADER();
		if ( !msg.Write(&stream) )
		{
			// error
			return;
		}

		SEND_TO_CMS();
	}

	BlockList.Add(BT_BLOCK, idAccount, Nickname);

	GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_AddBlock, TEXT("added"), *Nickname, idAccount, 0);
	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Block_Added"), TEXT("AVANET")), *Nickname),
									EChat_PlayerSystem);
}

void CavaCommunicator::DeleteBuddy(TID_ACCOUNT idAccount)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	if (idAccount == ID_INVALID_ACCOUNT)
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFoundUnknown"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	FBuddyInfo *Buddy = BuddyList.Find(idAccount);
	if (!Buddy)
	{
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Buddy->Nickname),
										EChat_PlayerSystem);
		return;
	}

	if (Buddy->IsBuddyBIA())
	{
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotDeleteBIA"), TEXT("AVANET")), *Buddy->Nickname),
										EChat_PlayerSystem);
		return;
	}

	{
		ScopedMsgBufPtr buf = CreateMsgBufN(1024);
		check(buf);
		CRxWriteStream stream(buf);
		DAT_MsgDeleteBuddyNtf msg;

		msg.sourceidAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
		msg.destidAccount = idAccount;

		WRITE_HEADER();
		if ( !msg.Write(&stream) )
		{
			// error
			return;
		}

		SEND_TO_CMS();
	}

	GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_Delete, TEXT("deleted"), *Buddy->Nickname, idAccount, 0);
	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_Removed"), TEXT("AVANET")), *Buddy->Nickname),
									EChat_PlayerSystem);

	BuddyList.Remove(idAccount);
}

void CavaCommunicator::DeleteBuddy(const FString &InNickname)
{
	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;

	if (!_StateController->PlayerInfo.IsValid())
		return;

	FBuddyInfo *Buddy = BuddyList.Find(Nickname);
	if (Buddy)
		DeleteBuddy(Buddy->idAccount);
	else
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Nickname),
										EChat_PlayerSystem);
}

void CavaCommunicator::DeleteBlock(TID_ACCOUNT idAccount)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	if (idAccount == ID_INVALID_ACCOUNT)
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFoundUnknown"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	FBuddyInfo *Buddy = BlockList.Find(idAccount);
	if (!Buddy)
	{
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Buddy->Nickname),
										EChat_PlayerSystem);
		return;
	}

	{
		ScopedMsgBufPtr buf = CreateMsgBufN(1024);
		check(buf);
		CRxWriteStream stream(buf);
		DAT_MsgDeleteBuddyNtf msg;

		msg.sourceidAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
		msg.destidAccount = idAccount;

		WRITE_HEADER();
		if ( !msg.Write(&stream) )
		{
			// error
			return;
		}

		SEND_TO_CMS();
	}

	GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_DeleteBlock, TEXT("deleted"), *Buddy->Nickname, idAccount, 0);
	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Block_Removed"), TEXT("AVANET")), *Buddy->Nickname),
									EChat_PlayerSystem);

	BlockList.Remove(idAccount);
}

void CavaCommunicator::DeleteBlock(const FString &InNickname)
{
	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;

	if (!_StateController->PlayerInfo.IsValid())
		return;

	FBuddyInfo *Buddy = BlockList.Find(Nickname);
	if (Buddy)
		DeleteBlock(Buddy->idAccount);
	else
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Nickname),
										EChat_PlayerSystem);
}

void CavaCommunicator::InviteGame(TID_ACCOUNT idAccount)
{
	if (!_StateController->PlayerInfo.IsValid() || !_StateController->ChannelInfo.IsValid())
		return;
	if (idAccount == ID_INVALID_ACCOUNT)
		return;

	FBuddyInfo *Buddy = BuddyList.Find(idAccount);
	if (!Buddy)
		return;

	//if (bListingBuddy)
	//{
	//	// 친구 탭을 보고 있을 때에만 정보가 정확하므로, 친구 탭을 보는 동안에만 체크

	//	if (!Buddy->IsOnline())
	//	{
	//		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerOffline"), TEXT("AVANET")), *Buddy->Nickname),
	//										EChat_PlayerSystem);
	//		return;
	//	}

	//	if (_StateController->ChannelInfo.IsMyClanChannel() && Buddy->idGuild != _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild)
	//	{
	//		// I am in my clan home and the invitee is not a member of my clan
	//		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotInviteToMyClanHome"), TEXT("AVANET")), *Buddy->Nickname),
	//										EChat_PlayerSystem);
	//		return;
	//	}

	//	if (_StateController->ChannelInfo.IsFriendlyGuildChannel() && Buddy->idGuild == ID_INVALID_GUILD)
	//	{
	//		// I am in friendly clan match channel and the invitee is not a member of clan
	//		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotInviteToFriendlyClanMatch"), TEXT("AVANET")), *Buddy->Nickname),
	//										EChat_PlayerSystem);
	//		return;
	//	}

	//	if (FChannelInfo::IsJoinableLevel(_StateController->ChannelInfo.Flag, Buddy->Level) != TEXT("ok"))
	//	{
	//		// The invitee cannot join my channel because of his level is not allowed
	//		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotInviteToMyChannelLevel"), TEXT("AVANET")), *Buddy->Nickname),
	//										EChat_PlayerSystem);
	//		return;
	//	}
	//}

	{
		ScopedMsgBufPtr buf = CreateMsgBufN(1024);
		check(buf);
		CRxWriteStream stream(buf);
		DAT_MsgGameInviteNtf msg;

		LOCATIONINFO LocationInfo;
		//LocationInfo.userid = _StateController->PlayerInfo.PlayerInfo.idAccount;
		//LocationInfo.serverid = 0;
		LocationInfo.channelid = _StateController->ChannelInfo.idChannel;
		LocationInfo.servertype = _StateController->ChannelInfo.GetChannelType();
		LocationInfo.clanid = 0;

		msg.FromidAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
		msg.Fromnickname = _StateController->PlayerInfo.PlayerInfo.nickname;
		msg.ToidAccount = idAccount;
		msg.Fromlocationinfo = LocationInfo;
		msg.roomname = (_StateController->RoomInfo.IsValid() ? _StateController->RoomInfo.RoomInfo.roomName : TEXT(""));
		msg.roomid = (_StateController->RoomInfo.IsValid() ? _StateController->RoomInfo.RoomInfo.idRoom : ID_INVALID_ROOM);

		_LOG(TEXT("idChannel = %d, ChannelType = %d, idRoom = %d, RoomName = %s"), LocationInfo.channelid, LocationInfo.servertype, msg.roomid, *msg.roomname);

		WRITE_HEADER();
		if ( !msg.Write(&stream) )
		{
			// error
			return;
		}

		SEND_TO_CMS();
	}

	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_GameInvitationSentTo"), TEXT("AVANET")), *Buddy->Nickname),
									EChat_PlayerSystem);
}

void CavaCommunicator::InviteGame(const FString &InNickname)
{
	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;
	if (!_StateController->PlayerInfo.IsValid())
		return;

	if (!_StateController->IsStateInRoom())
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_GameInvitationNotAllowed"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	//FPlayerDispInfo *Player = _StateController->FindPlayerFromList(Nickname);
	//if (Player)
	//{
	//	// 이미 같은 위치에 있음
	//	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_SameLocationWithInvitee"), TEXT("AVANET")), *Nickname),
	//									EChat_PlayerSystem);
	//}
	//else
	{
		// 현재 친구만 게임 초대 가능
		FBuddyInfo *Buddy = BuddyList.Find(Nickname);
		if (Buddy)
			InviteGame(Buddy->idAccount);
		else
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Nickname),
											EChat_PlayerSystem);
	}
}

void CavaCommunicator::InviteGameAns(UBOOL bYes, TID_ACCOUNT idAccount)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	if (idAccount == ID_INVALID_ACCOUNT)
	{
		if (GameInvitorList.Num() > 0)
			idAccount = GameInvitorList.BuddyList(0).idAccount;
	}

	FBuddyInfo *Invitor = GameInvitorList.Find(idAccount);
	if (!Invitor)
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_NoPlayerToReply"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	if (bYes)
	{
		//FBuddyInfo *Buddy = BuddyList.Find(Invitor->idAccount);
		//if (!Buddy)
		//{
		//	_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_NoPlayerToReply"), TEXT("AVANET")),
		//									EChat_PlayerSystem);
		//	return;
		//}

		if (Invitor->idChannel == _StateController->ChannelInfo.idChannel && Invitor->ServerType == _StateController->ChannelInfo.GetChannelType())
		{
			// 같은 채널에 있으므로 바로 이동
			MoveToPlayer(Invitor->idAccount, Invitor->idRoom);
		}
		else
		{
			// 다른 채널에 있으므로, 우선 그 채널로 들어갈 수 있는지 체크
			BREAK_SECTION_BEGIN()
			{
				// 대상의 위치 체크
				// 대상이 접속해 있는 채널의 multicast address로 위치 정보 요청 메시지를 보냄
				RxGate::RXNERVE_ADDRESS Addr;
				Addr.addrType = 0;
				char addrbuf[RXNERVE_ADDRESS_SIZE + 1];
				appMemzero(addrbuf, RXNERVE_ADDRESS_SIZE + 1);
				if (Invitor->ServerType == FChannelInfo::CT_NORMAL && Invitor->idChannel != ID_INVALID_CHANNEL)
				{
					_LOG(TEXT("Requesting information of %s to channel server."), *Invitor->Nickname);
					sprintf(addrbuf, "CHM%d", Invitor->idChannel);
				}
				else if (Invitor->ServerType == FChannelInfo::CT_GUILD && Invitor->idGuild != ID_INVALID_GUILD)
				{
					_LOG(TEXT("Requesting information of %s to guild server."), *Invitor->Nickname);
					sprintf(addrbuf, "GCS%d", Invitor->idGuild);
				}
				else
				{
					_LOG(TEXT("Error! Cannot find the player."));
					_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Invitor->Nickname),
													EChat_PlayerSystem);
					break;
				}
				appMemcpy(Addr.address, addrbuf, RXNERVE_ADDRESS_SIZE);
				PM::CHANNEL::FOLLOW_PLAYER_REQ::Send(Invitor->idAccount, &Addr);
			}
			BREAK_SECTION_END()
		}

		GameInvitorList.Clear();

		//_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_GameInvitationAccepted"), TEXT("AVANET")),
		//								EChat_PlayerSystem);
	}
	else
	{
		GameInvitorList.Remove(idAccount);

		//_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_GameInvitationRejected"), TEXT("AVANET")),
		//								EChat_PlayerSystem);
	}
}

void CavaCommunicator::InviteGameAns(UBOOL bYes, const FString &InNickname)
{
	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
	{
		InviteGameAns(bYes);
	}
	else
	{
		FBuddyInfo *Buddy = GameInvitorList.Find(Nickname);
		if (!Buddy)
		{
			_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_NoPlayerToReply"), TEXT("AVANET")),
											EChat_PlayerSystem);
			return;
		}

		InviteGameAns(bYes, Buddy->idAccount);
	}
}

void CavaCommunicator::MoveToPlayer(Def::TID_ACCOUNT idAccount, Def::TID_ROOM idRoom)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	if (_StateController->GetNetState() < _AN_CHANNELLIST)
		return;

	if (_StateController->IsStatePlaying())
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowInGame"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	if (idAccount == ID_INVALID_ACCOUNT)
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFoundUnknown"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	FPlayerDispInfo *Player = _StateController->FindPlayerFromList(idAccount);
	if (Player)
	{
		// 이미 같은 위치에 있음
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_SameLocationWithPlayer"), TEXT("AVANET")), Player->PlayerInfo.nickname),
										EChat_PlayerSystem);
		return;
	}

	FBuddyInfo *Buddy = BuddyList.Find(idAccount);
	if (!Buddy)
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFoundUnknown"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	if ( !CheckFollowable(idAccount) )
		return;

	_StateController->AutoMoveDest.SetMoveDestTo(Buddy->idChannel, idRoom, TEXT(""), idAccount);
	_StateController->ProcAutoMove();
}

void CavaCommunicator::InviteGuild(TID_ACCOUNT idAccount, const FString &InNickname)
{
	if (!_StateController->PlayerInfo.IsValid() || _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild == ID_INVALID_GUILD)
		return;

	CHECK_GUILD_PRIV(PRIV_INVITE);

	if (_StateController->GuildInfo.PlayerList.PlayerList.Num() >= 100)
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_MemberFull"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0 || idAccount == ID_INVALID_ACCOUNT)
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFoundUnknown"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	FGuildPlayerInfo *GuildPlayer = _StateController->GuildInfo.PlayerList.Find(idAccount);
	if (GuildPlayer)
	{
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_AlreadyMyClanMember"), TEXT("AVANET")), GuildPlayer->GuildPlayerInfo.nickname),
									EChat_PlayerSystem);
		return;
	}

	FBuddyInfo *Buddy = BuddyList.Find(idAccount);
	if (Buddy)
	{
		if (bListingBuddy)
		{
			// 친구 탭을 보고 있을 때에만 체크
			if (Buddy->IsBuddyBoth() && !Buddy->IsOnline())
			{
				// 양방향 친구여서 상태를 알 수 있다. 오프라인 상태라면 초대 못함.
				_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerOffline"), TEXT("AVANET")), *Buddy->Nickname),
												EChat_PlayerSystem);
				return;
			}
		}

		// 클랜 탈퇴하고 재가입하는 경우 애매함
		//if (Buddy->idGuild != ID_INVALID_GUILD)
		//{
		//	// 이미 클랜에 가입한 친구
		//	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_AlreadyClanMember"), TEXT("AVANET")), *Buddy->Nickname),
		//								EChat_PlayerSystem);
		//	return;
		//}
	}

	// 클랜 탈퇴하고 재가입하는 경우 애매함.
	//FPlayerDispInfo *Info = _StateController->FindPlayerFromList(idAccount);
	//if (Info)
	//{
	//	if (appStrlen(Info->PlayerInfo.guildName) > 0)
	//	{
	//		// 이미 클랜에 가입한 친구
	//		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_AlreadyClanMember"), TEXT("AVANET")), Info->PlayerInfo.nickname),
	//									EChat_PlayerSystem);
	//		return;
	//	}
	//}

	{
		ScopedMsgBufPtr buf = CreateMsgBufN(1024);
		check(buf);
		CRxWriteStream stream(buf);
		DAT_MsgClanInviteReq msg;

		msg.FromidAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
		msg.Fromnickname = _StateController->PlayerInfo.PlayerInfo.nickname;
		msg.ToidAccount = idAccount;
		msg.clanid = _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild;
		msg.clanname = _StateController->PlayerInfo.PlayerInfo.guildInfo.guildName;

		WRITE_HEADER();
		if ( !msg.Write(&stream) )
		{
			// error
			return;
		}

		SEND_TO_CMS();
	}

	if (!GuildInviteList.Find(idAccount))
	{
		GuildInviteList.Add(BT_BUDDY_OTHER, idAccount, Nickname);
	}

	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_InvitationSentTo"), TEXT("AVANET")), *Nickname),
									EChat_PlayerSystem);
}

void CavaCommunicator::InviteGuild(const FString &InNickname)
{
	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;

	if (!_StateController->PlayerInfo.IsValid())
		return;

	if (!_StateController->GuildInfo.IsValid())
	{
		_LOG(TEXT("Error! You have no guild information."));
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoClan"), TEXT("AVANET")), EChat_GuildSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Kick, TEXT("no guild"), TEXT(""), 0, 0);
		return;
	}
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		_LOG(TEXT("Error! Guild channel is not connected."));
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoConnection"), TEXT("AVANET")), EChat_GuildSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Kick, TEXT("not connected"), TEXT(""), 0, 0);
		return;
	}

	TID_ACCOUNT idAccount = ID_INVALID_ACCOUNT;

	FPlayerDispInfo *Player = _StateController->FindPlayerFromList(Nickname);
	if (Player)
	{
		//InviteGuild(Player->PlayerInfo.idAccount);
		idAccount = Player->PlayerInfo.idAccount;
	}
	else
	{
		FBuddyInfo *Buddy = BuddyList.Find(Nickname);
		if (Buddy)
		{
			//InviteGuild(Buddy->idAccount, Nickname);
			idAccount = Buddy->idAccount;
		}
	}

	if (idAccount == ID_INVALID_ACCOUNT)
	{
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Nickname),
										EChat_PlayerSystem);
		return;
	}

	InviteGuild(idAccount, Nickname);
}

void CavaCommunicator::InviteGuildAns(UBOOL bYes, TID_ACCOUNT idAccount)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	if (idAccount == ID_INVALID_ACCOUNT)
	{
		if (GuildInvitorList.Num() > 0)
			idAccount = GuildInvitorList.BuddyList(0).idAccount;
	}

	FBuddyInfo *Buddy = GuildInvitorList.Find(idAccount);
	if (!Buddy)
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_NoPlayerToReply"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	{
		ScopedMsgBufPtr buf = CreateMsgBufN(1024);
		check(buf);
		CRxWriteStream stream(buf);
		DAT_MsgClanInviteAns msg;

		msg.errCode = (bYes ? BUDDY_ACCEPT : BUDDY_REJECT);
		msg.FromidAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
		msg.ToidAccount = idAccount;

		WRITE_HEADER();
		if ( !msg.Write(&stream) )
		{
			// error
			return;
		}

		SEND_TO_CMS();
	}

	if (bYes)
	{
		GuildInvitorList.Clear();
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_InvitationAnsYes"), TEXT("AVANET")),
										EChat_PlayerSystem);
	}
	else
	{
		GuildInvitorList.Remove(idAccount);
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_InvitationAnsNo"), TEXT("AVANET")),
										EChat_PlayerSystem);
	}
}

void CavaCommunicator::InviteGuildAns(UBOOL bYes, const FString &InNickname)
{
	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
	{
		InviteGuildAns(bYes);
	}
	else
	{
		FBuddyInfo *Buddy = GuildInvitorList.Find(Nickname);
		if (!Buddy)
		{
			_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_NoPlayerToReply"), TEXT("AVANET")),
											EChat_PlayerSystem);
			return;
		}

		InviteGuildAns(bYes, Buddy->idAccount);
	}
}

void CavaCommunicator::JoinGuild(Def::TID_ACCOUNT idAccount, RxGate::LPRXNERVE_ADDRESS GCSAddr, Def::TID_GUILD idGuild)
{
	ScopedMsgBufPtr buf = CreateMsgBufN(1024);
	check(buf);
	CRxWriteStream stream(buf);
	DAT_MsgClanJoinNtf msg;

	msg.ToidAccount = idAccount;
	msg.gcsaddr = *GCSAddr;
	msg.clanid = idGuild;

	WRITE_HEADER();
	if ( !msg.Write(&stream) )
	{
		// error
		return;
	}

	SEND_TO_CMS();
}

void CavaCommunicator::BuddyChat(TID_ACCOUNT idAccount, const FString &InChatMsg, UBOOL bShowMsg)
{
	FString ChatMsg = ((FString)InChatMsg).Trim().TrimTrailing();
	if (!_StateController->PlayerInfo.IsValid() || ChatMsg.Len() == 0)
		return;

	FBuddyInfo *Buddy = BuddyList.Find(idAccount);
	if (!Buddy)
	{
		if (bShowMsg)
			_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFoundUnknown"), TEXT("AVANET")),
											EChat_PlayerSystem);
		return;
	}

	//if (!Buddy->IsOnline())
	//{
	//	_StateController->LogChatConsole(*FString::Printf(TEXT("%s is not online."), *Buddy->Nickname), EChat_PlayerSystem);
	//	return;
	//}

	{
		ScopedMsgBufPtr buf = CreateMsgBufN(1024);
		check(buf);
		CRxWriteStream stream(buf);
		DAT_MsgPrivateChatData msg;

		msg.FromidAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
		msg.Fromnickname = _StateController->PlayerInfo.PlayerInfo.nickname;
		msg.ToidAccount = idAccount;
		msg.chatdata = *ChatMsg;

		WRITE_HEADER();
		if ( !msg.Write(&stream) )
		{
			// error
			return;
		}

		SEND_TO_CMS();
	}

	if (bShowMsg)
		_StateController->LogChatConsole(*FString::Printf(TEXT("[To %s] %s"), *Buddy->Nickname, *ChatMsg), EChat_Whisper);
}

void CavaCommunicator::BuddyChat(const FString &InNickname, const FString &InChatMsg, UBOOL bShowMsg)
{
	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;

	FString ChatMsg = ((FString)InChatMsg).Trim().TrimTrailing();
	if (!_StateController->PlayerInfo.IsValid() || ChatMsg.Len() == 0)
		return;

	FBuddyInfo *Buddy = BuddyList.Find(Nickname);
	if (Buddy)
		BuddyChat(Buddy->idAccount, ChatMsg, bShowMsg);
	else if (bShowMsg)
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Nickname),
										EChat_PlayerSystem);
}

void CavaCommunicator::Whisper(const FString &InNickname, const FString &InChatMsg, UBOOL bAutoReply)
{
	if (!bAutoReply && _StateController->IsWhisperBlockedByClanMatch())
	{
		// 클랜전 진행 중이고 사망자 챗이 꺼져 있으면 귓말 불가
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Chat_CannotChatWhileClanMatch"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}
	if (!bAutoReply && _StateController->IsMatchProcessing())
	{
		// 대회 진행 중이면 귓말 불가
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Chat_CannotChatWhileMatch"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;

	FString ChatMsg = ((FString)InChatMsg).Trim().TrimTrailing();
	if (!_StateController->PlayerInfo.IsValid() || ChatMsg.Len() == 0)
		return;
	if (Nickname == _StateController->PlayerInfo.PlayerInfo.nickname)
		return;

	TCHAR *Msg = (TCHAR*)*ChatMsg;
	_WordCensor().ReplaceChatMsg(Msg);
	if (appStrlen(Msg) == 0)
		return;

#define _WHISPER_TO_UNKNOWN

#ifndef _WHISPER_TO_UNKNOWN
	{
		FBuddyInfo *Buddy = BuddyList.Find(InNickname);
		if ((!Buddy || !Buddy->IsBuddyBoth()) &&
			(!_StateController->GuildInfo.IsValid() || !_StateController->GuildInfo.PlayerList.Find(InNickname)))
		{
			// 양방향 친구나 클랜원에게만 귓속말을 보낼 수 있다.
			if (!bAutoReply)
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_WhisperToBuddyOrClanMemberOnly"), TEXT("AVANET")),
												EChat_PlayerSystem);
			return;
		}
	}
#endif

	UBOOL bSent = FALSE;
	UBOOL bFound = FALSE;
	FPlayerDispInfo *Player = _StateController->FindPlayerFromList(Nickname);
	if (Player)
	{
		// 상대방을 내가 접속한 로비나 방에서 찾을 수 있으면, 해당 채널 서버를 통해 귓속말을 보낸다.
		PM::CHANNEL::WHISPER_NTF::Send(Player->PlayerInfo.idAccount, Nickname, Msg);
		bFound = TRUE;
		bSent = TRUE;
	}
	else
	{
		if (_StateController->GuildInfo.IsChannelConnected())
		{
			// 상대방이 나와 같은 길드라면, 길드 서버를 통해 귓속말을 보낸다.
			FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(Nickname);
			if (Info)
			{
				bFound = TRUE;

				if (Info->IsOnline())
				{
					PM::CHANNEL::WHISPER_NTF::Send(Info->GuildPlayerInfo.idAccount, Nickname, Msg, TRUE);
					bSent = TRUE;
				}
			}
		}

		if (!bSent)
		{
			// search for the buddy list
			FBuddyInfo *Buddy = BuddyList.Find(Nickname);
			if (Buddy)
			{
				if (Buddy->IsBuddyBoth())
				{
					// 친구 목록에 있고 서로 친구라면
					bFound = TRUE;

					if (!bListingBuddy || Buddy->IsOnline())
					//if (Buddy->IsOnline())
					{
						// 현재 친구 탭을 보고 있지 않으면 일단 무조건 메시지 보냄
						// 친구 탭을 보고 있다면 온라인일 때만 메시지 보냄

						// the player is online
						//if (_StateController->ChannelInfo.IsNormalChannel() &&
						//	Buddy->ServerType == FChannelInfo::CT_NORMAL && Buddy->idChannel == _StateController->ChannelInfo.idChannel)
						//{
						//	// 상대방의 위치가 나와 같은 채널이라면, 해당 채널 서버를 통해 귓속말을 보낸다.
						//	// idAccount를 알고 있으므로, 서버 입장에서는 좀 싸게 먹힘.
						//	PM::CHANNEL::WHISPER_NTF::Send(Buddy->idAccount, Nickname, Msg);
						//	bSent = TRUE;
						//}
						//else if (_StateController->ChannelInfo.IsMyClanChannel() &&
						//	_StateController->GuildInfo.IsChannelConnected() && Buddy->idGuild == _StateController->GuildInfo.GuildInfo.idGuild)
						//{
						//	// 상대방이 나와 같은 길드라면, 길드 서버를 통해 귓속말을 보낸다.
						//	PM::CHANNEL::WHISPER_NTF::Send(Buddy->idAccount, Nickname, Msg, TRUE);
						//	bSent = TRUE;
						//}
						//else
						{
							// 이도 저도 아니면 커뮤니케이션 서버를 통해 귓속말을 보낸다.
							BuddyChat(Buddy->idAccount, Msg, FALSE);
							bSent = TRUE;
						}
					}
				}
				else
				{
					// 서로 친구가 아니면 안내 메시지만 보여주고 종료
					if (!bAutoReply)
						_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_NotBothType"), TEXT("AVANET")), *Nickname),
														EChat_PlayerSystem);

					return;
				}
			}
		}
	}

	if (!bFound)
	{
		if (!bAutoReply)
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Nickname),
											EChat_PlayerSystem);

		return;
	}

	// 자동 보냄 메시지가 아닌 경우에만 채팅창에 결과를 보여줌
	if (!bAutoReply)
	{
		if (bSent)
		{
			_StateController->LogChatConsole(*FString::Printf(TEXT("[To %s] %s"), *Nickname, Msg), EChat_Whisper);
			LastWhisperedPlayer = Nickname;
		}
		else
		{
			// the player is offline
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerOffline"), TEXT("AVANET")), *Nickname),
											EChat_PlayerSystem);
		}
	}
}

void CavaCommunicator::Reply( const FString &ChatMsg )
{
	if (LastWhisperedPlayer.Len() > 0)
		Whisper(LastWhisperedPlayer, ChatMsg);
}

void CavaCommunicator::GMWhisper(const FString &InNickname, const FString &InChatMsg)
{
	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;

	FString ChatMsg = ((FString)InChatMsg).Trim().TrimTrailing();
	if (!_StateController->PlayerInfo.IsValid() || ChatMsg.Len() == 0)
		return;
	if (!_StateController->AmIAdmin())
		return;
	if (Nickname == _StateController->PlayerInfo.PlayerInfo.nickname)
		return;

	TCHAR *Msg = (TCHAR*)*ChatMsg;
	_WordCensor().ReplaceChatMsg(Msg);
	if (appStrlen(Msg) == 0)
		return;

	UBOOL bSent = FALSE;
	FPlayerDispInfo *Player = _StateController->FindPlayerFromList(Nickname);
	if (Player)
	{
		PM::ADMIN::WHISPER_NTF::Send(Player->PlayerInfo.idAccount, Nickname, Msg);
		bSent = TRUE;
	}
	else
	{
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Nickname),
										EChat_PlayerSystem);
	}

	if (bSent)
		_StateController->LogChatConsole(*FString::Printf(TEXT("[To %s] %s"), *Nickname, Msg), EChat_GMWhisper);
}

void CavaCommunicator::GetLocationInfo(TID_ACCOUNT idAccount)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	if (idAccount == ID_INVALID_ACCOUNT)
		return;

	//if (!BuddyList.IsOnline(idAccount))
	//{
	//	return;
	//}

	ScopedMsgBufPtr buf = CreateMsgBufN(1024);
	check(buf);
	CRxWriteStream stream(buf);
	DAT_MsgLocationInfoReq msg;

	msg.srcidAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
	msg.destidAccount = idAccount;

	WRITE_HEADER();
	if ( !msg.Write(&stream) )
	{
		// error
		return;
	}

	SEND_TO_CMS();
}

void CavaCommunicator::GetLocationInfo(const FString &InNickname)
{
	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;

	if (!_StateController->PlayerInfo.IsValid())
		return;

	FBuddyInfo *Buddy = BuddyList.Find(Nickname);
	if (Buddy)
		GetLocationInfo(Buddy->idAccount);
	else
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Nickname),
										EChat_PlayerSystem);
}

void CavaCommunicator::GetBuddyList()
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	ScopedMsgBufPtr buf = CreateMsgBufN(1024);
	check(buf);
	CRxWriteStream stream(buf);
	DAT_MsgBuddyInfoReq msg;

	msg.bconnectAlram = 0;
	msg.idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;

	WRITE_HEADER();
	if ( !msg.Write(&stream) )
	{
		// error
		return;
	}

	SEND_TO_CMS();
}

void CavaCommunicator::BeginBuddyList()
{
	if (!_StateController->PlayerInfo.IsValid() || bListingBuddy)
		return;

	ScopedMsgBufPtr buf = CreateMsgBufN(1024);
	check(buf);
	CRxWriteStream stream(buf);
	DAT_MsgPairBuddyinfoReq msg;

	msg.idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;

	WRITE_HEADER();
	if ( !msg.Write(&stream) )
	{
		// error
		return;
	}

	SEND_TO_CMS();

	bListingBuddy = TRUE;
}

void CavaCommunicator::EndBuddyList()
{
	if (!_StateController->PlayerInfo.IsValid() || !bListingBuddy)
		return;

	ScopedMsgBufPtr buf = CreateMsgBufN(1024);
	check(buf);
	CRxWriteStream stream(buf);
	DAT_MsgBuddyInfoEndNtf msg;

	msg.idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;

	WRITE_HEADER();
	if ( !msg.Write(&stream) )
	{
		// error
		return;
	}

	SEND_TO_CMS();

	bListingBuddy = FALSE;
}

void CavaCommunicator::FollowPlayer( Def::TID_ACCOUNT idAccount, const FString &Nickname, Def::TID_GUILD idGuild, Def::TID_CHANNEL idChannel )
{
	if (idAccount == ID_INVALID_ACCOUNT || idChannel == ID_INVALID_CHANNEL)
		return;

	if (idChannel == ID_MY_CLAN_HOME)
	{
		// 대상이 클랜 홈에 들어가 있음
		if (_StateController->GuildInfo.IsValid() && _StateController->GuildInfo.GuildInfo.idGuild == idGuild)
		{
			// 대상이 나와 같은 클랜이면 클랜 홈으로 이동
			if (!_StateController->ChannelInfo.IsMyClanChannel())
				GetAvaNetRequest()->GuildJoinChannel();
		}
		else
		{
			// 이동 불가
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowToClanHome"), TEXT("AVANET")), *Nickname),
											EChat_PlayerSystem);
		}
		return;
	}
	else
	{
		FChannelInfo *Channel = _StateController->ChannelList.Find(idChannel);
		if (!Channel || Channel->IsJoinable() != TEXT("ok"))
		{
			// 대상이 따라갈 수 없는 채널에 있음
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowToChannel"), TEXT("AVANET")), *Nickname),
											EChat_PlayerSystem);
			return;
		}

		// 대상의 위치 체크
		// 대상이 접속해 있는 채널의 multicast address로 위치 정보 요청 메시지를 보냄
		RxGate::RXNERVE_ADDRESS Addr;
		CreateCHMAddress(Addr, idChannel);
		PM::CHANNEL::FOLLOW_PLAYER_REQ::Send(idAccount, &Addr);

		_StateController->AutoMoveDest.SetFollowTarget(idAccount, Nickname);
		return;
	}
}


////////////////////////////////////////////////////////////////////
// recv

UBOOL CavaCommunicator::ProcMsg(BYTE *buf, DWORD bufLen)
{
	_MSGHDR *pHdr = (_MSGHDR*)buf;
	BOOL ret = FALSE;

	//_DumpMemory(buf, bufLen);

	if (pHdr->len + sizeof(_MSGHDR) == (unsigned int)bufLen)
	{
		CRxReadStream stream(pHdr);

		switch (pHdr->id)
		{
		case DID_MsgAddBuddyReq:
			{
				DAT_MsgAddBuddyReq msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcAddBuddyReq(msg.BuddyType, msg.FromidAccount, msg.FromNick);
				}
			}
			break;
		case DID_MsgAddBuddyAns:
			{
				DAT_MsgAddBuddyAns msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcAddBuddyAns(msg.errCode, msg.FromidAccount, msg.FromNick);
				}
			}
			break;
		case DID_MsgGameInviteNtf:
			{
				DAT_MsgGameInviteNtf msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					_LOG(TEXT("idChannel = %d, ChannelType = %d, idRoom = %d, RoomName = %s"),
							msg.Fromlocationinfo.channelid, msg.Fromlocationinfo.servertype, msg.roomid, *msg.roomname);
					ProcInviteGame(msg.FromidAccount, msg.Fromnickname, msg.Fromlocationinfo, msg.roomname, msg.roomid);
				}
				else
				{
					_LOG(TEXT("Failed"));
				}
			}
			break;
		case DID_MsgClanInviteReq:
			{
				DAT_MsgClanInviteReq msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcInviteGuild(msg.FromidAccount, msg.Fromnickname, msg.clanid, msg.clanname);
				}
			}
			break;
		case DID_MsgClanInviteAns:
			{
				DAT_MsgClanInviteAns msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcInviteGuildAns(msg.errCode, msg.FromidAccount);
				}
			}
			break;
		case DID_MsgClanJoinNtf:
			{
				DAT_MsgClanJoinNtf msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcJoinGuild(&msg.gcsaddr, msg.clanid);
				}
			}
			break;
		case DID_MsgPrivateChatData:
			{
				DAT_MsgPrivateChatData msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcBuddyChat(msg.FromidAccount, msg.Fromnickname, msg.chatdata);
				}
			}
			break;
		case DID_MsgLocationInfoAns:
			{
				DAT_MsgLocationInfoAns msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcLocationInfo(msg.errCode, msg.idAccount, msg.loinfo);
				}
			}
			break;
		case DID_MsgBuddyAlarmNtf:
			{
				DAT_MsgBuddyAlarmNtf msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcBuddyStateChanged(msg.buddyinfo);
				}
			}
			break;
		case DID_MsgBuddyInfoAns:
			{
				DAT_MsgBuddyInfoAns msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcBuddyInfo(msg.count, msg.ava_friend);
				}
			}
			break;
		case DID_MsgPairBuddyinfoAns:
			{
				DAT_MsgPairBuddyinfoAns msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcPairBuddyInfo(msg.err, msg.cnt, msg.pairbuddyinfo);
				}
			}
			break;
		case DID_MsgUpdateNickNameNtf:
			{
				DAT_MsgUpdateNickNameNtf msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcUpdateNickName(msg.idAccount, msg.nickname);
				}
			}
			break;
		case DID_MsgPCBUserTimeExpireNtf:
			{
				DAT_MsgPCBUserTimeExpireNtf msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcPCBUserTimeExpire(msg.ret);
				}
			}
			break;
		case DID_MsgPCBRemainTimeAns:
			{
				DAT_MsgPCBRemainTimeAns msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcPCBRemainTime(msg.strtime);
				}
			}
			break;
		case DID_MsgPCBRemainTimeNtf:
			{
				DAT_MsgPCBRemainTimeNtf msg;
				ret = msg.Read(stream);
				if (ret)
				{
					_LOG(TEXT("[%s] Received."), msg.GetName());
					ProcPCBRemainTime(msg.strtime);
				}
			}
		default:
			break;
		}
	}

	return ret;
}

void CavaCommunicator::ProcAddBuddyReq(BYTE BuddyType, Def::TID_ACCOUNT idAccount, const FString &InNickname)
{
	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;

	//_LOG(TEXT("BuddyType = %d, idAccount = %d, Nickname = %s, ServerType= %d, idChannel = %d, idGuild = %d"),
	//		BuddyType, idAccount, *InNickname, Location.servertype, Location.channelid, Location.clanid);

	if (!_StateController->PlayerInfo.IsValid())
		return;
	if (BuddyType != BT_BUDDY_ONESIDE || BlockList.Find(idAccount))
		return;

	FBuddyInfo *Buddy = BuddyList.Find(idAccount);
	if (Buddy)
	{
		// he is already on buddy list
		//ToAnswerList.Add(BuddyType, idAccount, Nickname);
		//AddBuddyAns(0, idAccount);

		//Buddy->BuddyType = BT_BUDDY_BOTH;
		return;
	}

	Buddy = ToAnswerList.Add(BuddyType, idAccount, Nickname);
	if (!Buddy)
		return;
	Buddy->bOnline = TRUE;

	GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_AddReq, TEXT("received"), *Nickname, idAccount, 0);

	_StateController->LogChatConsoleNoMatch(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_AddRequestRecved"), TEXT("AVANET")), *Nickname),
									EChat_PlayerSystem);
	if (_StateController->IsStatePlaying() && !_StateController->IsMatchProcessing())
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_AddRequestRecvedInGame"), TEXT("AVANET")), EChat_PlayerSystem);
	}
}

void CavaCommunicator::ProcAddBuddyAns(DWORD errCode, Def::TID_ACCOUNT idAccount, const FString &InNickname)
{
	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;

	if (!_StateController->PlayerInfo.IsValid())
		return;

	_LOG(TEXT("errCode = %d, idAccount = %d, Nickname = %s"), errCode, idAccount, *Nickname);
	//_LOG(TEXT("errCode = %d, idAccount = %d, Nickname = %s, ServerType= %d, idChannel = %d, idGuild = %d"),
	//		errCode, idAccount, *InNickname, Location.servertype, Location.channelid, Location.clanid);

	FBuddyInfo *Buddy = BuddyList.Find(idAccount);
	if (!Buddy)
		return;

	if (errCode == BUDDY_ACCEPT)
	{

		Buddy->BuddyType = BT_BUDDY_BOTH;
		Buddy->bOnline = TRUE;

		_StateController->LogChatConsoleNoMatch(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_AddRequestAccepted"), TEXT("AVANET")), *Buddy->Nickname),
										EChat_PlayerSystem);

		//GetLocationInfo(idAccount);

		GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_AddReqAns, TEXT("accepted"), *Buddy->Nickname, idAccount, 0);
	}
	else if (errCode == BUDDY_REJECT)
	{
		_StateController->LogChatConsoleNoMatch(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_AddRequestRejected"), TEXT("AVANET")), *Buddy->Nickname),
										EChat_PlayerSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_AddReqAns, TEXT("rejected"), *Buddy->Nickname, idAccount, 0);
	}
}

void CavaCommunicator::ProcInviteGame(Def::TID_ACCOUNT idInvitor, const FString &InvitorName, RXCLIENTCMSPROTOCOL::LOCATIONINFO &Location, const FString &RoomName, Def::TID_ROOM idRoom)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	if (BlockList.Find(idInvitor))
		return;
	if (GameInvitorList.Find(idInvitor))
		return;

	FBuddyInfo *Buddy = BuddyList.Find(idInvitor);
	if (Buddy)
	{
		Buddy->idChannel = Location.channelid;
		Buddy->ServerType = Location.servertype;
		Buddy->idGuild = Location.clanid;
		Buddy->bOnline = TRUE;
	}

	FBuddyInfo *Invitor = GameInvitorList.Add(BT_BUDDY_OTHER, idInvitor, InvitorName);
	if (Invitor)
	{
		Invitor->operator=(*Buddy);
		Invitor->idRoom = idRoom;
	}

	FString InvInfo = TEXT("???");
	if (Buddy)
	{
		InvInfo = Buddy->GetLocation();
	}
	else
	{
		if (Location.servertype == FChannelInfo::CT_NORMAL)
		{
			FChannelInfo *Channel = _StateController->ChannelList.Find(Location.channelid);
			if (Channel)
				InvInfo = Channel->ChannelName;
		}
		else if (Location.servertype == FChannelInfo::CT_GUILD)
		{
			FString NameList = Localize(TEXT("Channel"), *FString::Printf(TEXT("Text_ChannelName[%d]"), (INT)EChannelFlag_MyClan), TEXT("AVANET"));
			FString LongName;
			NameList.Split(TEXT("|"), &InvInfo, &LongName);
		}
	}

	if (idRoom != ID_INVALID_ROOM)
		InvInfo += FString::Printf(TEXT(" / [%d]%s"), idRoom, *RoomName);

	GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_InviteGame, *InvitorName, TEXT(""), idInvitor, 0);

	_StateController->LogChatConsoleNoMatch(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_GameInvitationRecved"), TEXT("AVANET")), *InvitorName, *InvInfo),
									EChat_PlayerSystem);
}

void CavaCommunicator::ProcInviteGuild(Def::TID_ACCOUNT idInvitor, const FString &InvitorName, Def::TID_GUILD idGuild, const FString &GuildName)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	if (BlockList.Find(idInvitor))
		return;
	if (GuildInvitorList.Find(idInvitor))
		return;

	if (_StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild != ID_INVALID_GUILD)
	{
		// I am guild member already; Reject the invitation
		ScopedMsgBufPtr buf = CreateMsgBufN(1024);
		check(buf);
		CRxWriteStream stream(buf);
		DAT_MsgClanInviteAns msg;

		msg.errCode = _StateController->GuildInfo.IsRegularGuild() ? 11 : 12;
		msg.FromidAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
		msg.ToidAccount = idInvitor;

		WRITE_HEADER();
		if ( !msg.Write(&stream) )
		{
			// error
			return;
		}

		SEND_TO_CMS();

		return;
	}

	FBuddyInfo *Buddy = GuildInvitorList.Add(BT_BUDDY_OTHER, idInvitor, InvitorName);
	Buddy->idGuild = idGuild;
	Buddy->GuildName = GuildName;

	GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_InviteGuild, *InvitorName, *GuildName, idInvitor, idGuild);

	_StateController->LogChatConsoleNoMatch(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_InvitationRecved"), TEXT("AVANET")), *InvitorName, *GuildName),
									EChat_PlayerSystem);
	if (_StateController->IsStatePlaying() && !_StateController->IsMatchProcessing())
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_InvitationRecvedInGame"), TEXT("AVANET")), EChat_PlayerSystem);
	}
}

void CavaCommunicator::ProcInviteGuildAns(DWORD errCode, Def::TID_ACCOUNT idAccount)
{
	if (!_StateController->PlayerInfo.IsValid() || _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild == ID_INVALID_GUILD)
		return;

	FBuddyInfo *Info = GuildInviteList.Find(idAccount);
	if (!Info)
		return;

	switch (errCode)
	{
	case BUDDY_ACCEPT:
		_StateController->LogChatConsoleNoMatch(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_InvitationAccepted"), TEXT("AVANET")), *Info->Nickname),
									EChat_PlayerSystem);

		if ( !_WebInClient().GuildJoin(_StateController->PlayerInfo.PlayerInfo.guildInfo.strGuildID, _StateController->PlayerInfo.PlayerInfo.idAccount, idAccount) )
		{
			_StateController->LogChatConsoleNoMatch(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_RequestFailed"), TEXT("AVANET")), EChat_PlayerSystem);
		}
		break;
	case BUDDY_REJECT:
		_StateController->LogChatConsoleNoMatch(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_InvitationRejected"), TEXT("AVANET")), *Info->Nickname),
									EChat_PlayerSystem);
		break;
	case 11:
		_StateController->LogChatConsoleNoMatch(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_AlreadyClanMember"), TEXT("AVANET")), *Info->Nickname),
									EChat_PlayerSystem);
		break;
	case 12:
		_StateController->LogChatConsoleNoMatch(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_AlreadyIrregularClanMember"), TEXT("AVANET")), *Info->Nickname),
									EChat_PlayerSystem);
		break;
	}

	GuildInviteList.Remove(idAccount);
}

void CavaCommunicator::ProcJoinGuild(RxGate::LPRXNERVE_ADDRESS GCSAddr, Def::TID_GUILD idGuild)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	// connect to guild channel
	_LOG(TEXT("Connecting to the guild channel; id = %d"), idGuild);

	_StateController->PlayerInfo.SetRxGuildAddress(GCSAddr->address64);
	_StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild = idGuild;

	GavaNetClient->CurrentGuildAddress = *GCSAddr;

	GavaNetClient->CreateRxGateSession(GavaNetClient->CurrentGuildAddress);
}

void CavaCommunicator::ProcBuddyChat(Def::TID_ACCOUNT idSender, const FString &SenderName, const FString &ChatMsg)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	if (BlockList.Find(idSender))
		return;

	TCHAR *Msg = (TCHAR*)*ChatMsg;
	UBOOL bCoded = _Communicator().IsMsgCoded(Msg);
	if (!bCoded)
	{
		_WordCensor().ReplaceChatMsg(Msg);
		if (appStrlen(Msg) == 0)
			return;
	}

	FString ParsedMsg = (bCoded ? *_Communicator().ParseCodedMsg(*ChatMsg) : FString::Printf(TEXT("[From %s] %s"), *SenderName, *ChatMsg));
	INT MsgType = _Communicator().ParseCodedMsgType(*ChatMsg, EChat_Whisper);
	_StateController->LogChatConsoleNoMatch(*ParsedMsg, MsgType);

	if (!_StateController->IsMatchProcessing())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_Chat, *SenderName, *ParsedMsg, idSender, 0);
	}

	LastWhisperedPlayer = SenderName;

	if (!bCoded)
	{
		if (_StateController->IsWhisperBlockedByClanMatch())
		{
			// 클랜전 진행 중이라 응답할 수 없다는 메시지를 보냄
			// 자동 응답은 \n 문자로 인코딩해서 보냄; 받는 쪽에서 파싱
			FString CodedMsg = FString::Printf(TEXT("\n%d\nText_AutoReply_ClanMatch\n%s"), (INT)EChat_PlayerSystem, _StateController->PlayerInfo.PlayerInfo.nickname);
			_Communicator().Whisper(*SenderName, *CodedMsg, TRUE);
		}
		else if (_StateController->IsMatchProcessing())
		{
			// 대회 진행 중이라 응답할 수 없다는 메시지를 보냄
			FString CodedMsg = FString::Printf(TEXT("\n%d\nText_AutoReply_Match\n%s"), (INT)EChat_PlayerSystem, _StateController->PlayerInfo.PlayerInfo.nickname);
			_Communicator().Whisper(*SenderName, *CodedMsg, TRUE);
		}
	}
}

void CavaCommunicator::ProcLocationInfo(DWORD errCode, Def::TID_ACCOUNT idAccount, RXCLIENTCMSPROTOCOL::LOCATIONINFO &Location)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	FBuddyInfo *Buddy = BuddyList.Find(idAccount);
	if (!Buddy)
		return;

	if (errCode > 0 || Location.servertype == FChannelInfo::CT_UNKNOWN)
	{
		if (!_StateController->AutoMoveDest.IsMoving() && _StateController->AutoMoveDest.idAccount == idAccount)
		{
			// 따라가기를 하는 중이었음
			_StateController->StopAutoMove();
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerOffline"), TEXT("AVANET")), *Buddy->Nickname),
											EChat_PlayerSystem);
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_Location, TEXT("failed"), TEXT(""), idAccount, 0);
		return;
	}

	Buddy->operator=(Location);

	GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_Location, TEXT(""), TEXT(""), idAccount, 0);

	if (Location.servertype == FChannelInfo::CT_GUILD)
	{
		if (!_StateController->AutoMoveDest.IsMoving() && _StateController->AutoMoveDest.idAccount == idAccount &&
			!_StateController->ChannelInfo.IsMyClanChannel())
			GetAvaNetRequest()->GuildJoinChannel();
	}
	else
	{
		RxGate::RXNERVE_ADDRESS Addr;
		CreateCHMAddress(Addr, Location.channelid);
		if (!_StateController->AutoMoveDest.IsMoving() && _StateController->AutoMoveDest.idAccount == idAccount)
		{
			// 따라가기를 시도하고 있었음; 따라가기 마저 처리
			PM::CHANNEL::FOLLOW_PLAYER_REQ::Send(idAccount, &Addr);
		}
		//else
		//{
		//	// 플레이어가 CHS에 있다면, 상세 위치까지 확인
		//	PM::CHANNEL::PLAYER_LOCATION_REQ::Send(def.idAccount, &Addr);
		//}
	}
}

void CavaCommunicator::ProcBuddyStateChanged(RXCLIENTCMSPROTOCOL::_BUDDYINFO &BuddyInfo)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	FBuddyInfo *Buddy = BuddyList.Find(BuddyInfo.userid);
	if (!Buddy)
		return;

	_LOG(TEXT("(%s) [%d]%s [%s] <%d/%d/%d>"),
				BuddyInfo.friendtype == BT_BUDDY_BIA ? TEXT("BIA") : BuddyInfo.friendtype == BT_BUDDY_BOTH ? TEXT("BuddyBoth") : BuddyInfo.friendtype == BT_BUDDY_ONESIDE ? TEXT("BuddyOneSide") : TEXT("Block"),
				BuddyInfo.userid, *BuddyInfo.beforenickname, *BuddyInfo.clanname,
				BuddyInfo.loinfo.servertype, BuddyInfo.loinfo.channelid, BuddyInfo.loinfo.clanid);

	BYTE OldType = Buddy->BuddyType;
	UBOOL bWasOnline = Buddy->IsOnline();

	Buddy->operator=(BuddyInfo);

	GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_StateChanged, Buddy->IsOnline() ? TEXT("connected") : TEXT("disconnected"), *Buddy->Nickname, Buddy->idAccount, 0);

	if (BuddyInfo.afternickname.Len() > 0)
	{
		// 닉네임이 바뀌었음
		Buddy->UpdateNickname(BuddyInfo.afternickname);
	}

	if (Buddy->IsBuddyBIA())
	{
		// 오프라인에서 온라인으로 바뀌었을 때, 또는 상태가 일반 친구에서 전우로 바뀌었을 때 한번만 알림.
		if ((!bWasOnline && Buddy->IsOnline()) || OldType != BT_BUDDY_BIA)
		{
			FString Msg = FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_BIAConnected"), TEXT("AVANET")), *Buddy->Nickname);
			_StateController->LogChatConsoleNoMatch(*Msg, EChat_PlayerSystem);
		}
	}
	else if (OldType == BT_BUDDY_BIA)
	{
		// 전우가 해제되었음
		// 전우 게이지 리셋
		_StateController->PlayerInfo.PlayerInfo.biaXP = 0;
	}
	if (!Buddy->IsOnline())
	{
		Buddy->SetOffline();
	}

#if !FINAL_RELEASE
	//if (!bWasOnline && Buddy->IsOnline())
	//{
	//	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_Connected"), TEXT("AVANET")), *Buddy->Nickname),
	//									EChat_PlayerSystem);
	//}
	//else if (bWasOnline && !Buddy->IsOnline())
	//{
	//	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_Disconnected"), TEXT("AVANET")), *Buddy->Nickname),
	//									EChat_PlayerSystem);
	//}
#endif
}

void CavaCommunicator::ProcBuddyInfo(BYTE Count, TArray<RXCLIENTCMSPROTOCOL::_BUDDYINFO> &InfoList)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	BuddyList.Clear();
	BlockList.Clear();

	_LOG(TEXT("%d players in the list"), InfoList.Num());

	for (INT i = 0; i < InfoList.Num(); ++i)
	{
		RXCLIENTCMSPROTOCOL::_BUDDYINFO &Info = InfoList(i);

		if (Info.beforenickname.Len() == 0 && Info.afternickname.Len() == 0)
		{
			// 에러! 닉네임이 없음
			_LOG(TEXT("BuddyInfo[%d] has no nickname!"), Info.userid);
			continue;
		}

		_LOG(TEXT("(%s) [%d]%s [%s] <%d/%d/%d>"),
					Info.friendtype == BT_BUDDY_BIA ? TEXT("BIA") : Info.friendtype == BT_BUDDY_BOTH ? TEXT("BuddyBoth") : Info.friendtype == BT_BUDDY_ONESIDE ? TEXT("BuddyOneSide") : TEXT("Block"),
					Info.userid, *Info.beforenickname, *Info.clanname,
					Info.loinfo.servertype, Info.loinfo.channelid, Info.loinfo.clanid);

		FBuddyInfo *Buddy = (Info.friendtype != BT_BLOCK ? BuddyList.Add(Info) : BlockList.Add(Info));
		if (Buddy && Info.afternickname.Len() > 0)
		{
			// 바뀐 닉네임이 있음
			if (Buddy->Nickname.Len() == 0)
			{
				// 이전 닉네임을 받지 못했음
				// 닉네임 업데이트는 하지만 채팅창에 메시지를 찍지 않는다.
				Buddy->UpdateNickname(Info.afternickname, FALSE);
			}
			else
			{
				// 닉네임 업데이트
				// 양방향 친구의 닉네임을 업데이트할 때에만 메시지를 찍는다.
				Buddy->UpdateNickname(Info.afternickname, Buddy->IsBuddyBoth());
			}
		}
	}

#if !FINAL_RELEASE
	_LOG(TEXT("%d player(s) in the friend list."), BuddyList.Num());
	for (INT i = 0; i < BuddyList.Num(); ++i)
	{
		_LOG(TEXT("%s [%d](%d) [%s]%s (%d/%d)%s"), (BuddyList(i).IsBuddyBIA() ? TEXT("==") : BuddyList(i).IsBuddyBoth() ? TEXT("<>") : TEXT(" >")),
								BuddyList(i).idAccount, BuddyList(i).Level, *BuddyList(i).Nickname, *BuddyList(i).GuildName,
								BuddyList(i).ServerType, BuddyList(i).idChannel, BuddyList(i).IsOnline() ? *BuddyList(i).GetLocation() : TEXT("Offline"));
	}

	_LOG(TEXT("%d player(s) in the block list."), BlockList.Num());
	for (INT i = 0; i < BlockList.Num(); ++i)
	{
		_LOG(TEXT("[%d] %s"), BlockList(i).idAccount, *BlockList(i).Nickname);
	}
#endif

	GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_List, TEXT("ok"), TEXT(""), Count, 0);

	// 아바 PC방인 경우, 여기서 MsgPCBUserStartNtf를 보낸다.
	if (_StateController->PlayerInfo.PcBangServiceType > 0)
	{
		FString *UserID = GavaNetClient->Settings.Find(CFG_USERID);
		if (UserID)
		{
			//FInternetIpAddr Addr = getlocalbindaddr(*GLog);

			_LOG(TEXT("Sending [MsgPCBUserStartNtf] ServiceType = %d, IP = %d, idAccount = %d, pmangID = %s"),
						_StateController->PlayerInfo.PcBangServiceType,
						//*Addr.ToString(FALSE),
						GavaNetClient->ClientIP,
						_StateController->PlayerInfo.PlayerInfo.idAccount,
						**UserID);

			ScopedMsgBufPtr buf = CreateMsgBufN(1024);
			check(buf);
			CRxWriteStream stream(buf);
			DAT_MsgPCBUserStartNtf msg;

			msg.service_type = _StateController->PlayerInfo.PcBangServiceType;
			//Addr.GetIp(msg.IP);
			msg.IP = GavaNetClient->ClientIP;
			msg.idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
			msg.pmangID = *UserID;

			WRITE_HEADER();
			if ( !msg.Write(&stream) )
			{
				// error
				return;
			}

			SEND_TO_CMS();
		}
		else
		{
			_LOG(TEXT("Error! Pmang ID not found!!"));
		}
	}
}

void CavaCommunicator::ProcPairBuddyInfo(BYTE Err, BYTE Count, TArray<RXCLIENTCMSPROTOCOL::_PAIRBUDDYINFO> &InfoList)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	if (Err == 0)
	{
		for (INT i = 0; i < BuddyList.Num(); ++i)
		{
			BuddyList(i).bOnline = 0;
		}

		_LOG(TEXT("%d players in the list"), InfoList.Num());

		for (INT i = 0; i < InfoList.Num(); ++i)
		{
			RXCLIENTCMSPROTOCOL::_PAIRBUDDYINFO &Info = InfoList(i);
			FBuddyInfo *Buddy = BuddyList.Find(Info.userid);
			if (Buddy)
			{
				_LOG(TEXT("(%s) [%d]%s <%d/%d/%d>"),
							Info.ftype == BT_BUDDY_BIA ? TEXT("BuddyBIA") : Info.ftype == BT_BUDDY_BOTH ? TEXT("BuddyBoth") : Info.ftype == BT_BUDDY_ONESIDE ? TEXT("BuddyOneSide") : TEXT("Block"),
							Buddy->idAccount, *Buddy->Nickname,
							Info.loinfo.servertype, Info.loinfo.channelid, Info.loinfo.clanid);
				WORD OldBuddyType = Buddy->BuddyType;
				Buddy->bOnline = 1;
				Buddy->operator=(Info);
				if (Info.nickname.Len() > 0)
				{
					// 닉네임이 변경 되었음
					Buddy->UpdateNickname(Info.nickname);
				}
				if (OldBuddyType == BT_BUDDY_BIA && !Buddy->IsBuddyBIA())
				{
					// 전우가 해제되었음
					// 전우 게이지 리셋
					_StateController->PlayerInfo.PlayerInfo.biaXP = 0;
				}
			}
			else
			{
				_LOG(TEXT("Unknown buddy info received; idAccount = %d"), InfoList(i).userid);
			}
		}

		for (INT i = 0; i < BuddyList.Num(); ++i)
		{
			if (!BuddyList(i).IsOnline())
			{
				BuddyList(i).SetFullInfo(FALSE);
				BuddyList(i).SetOffline();
			}
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_List, TEXT("ok"), TEXT(""), Count, 0);
	}
	else
	{
		_LOG(TEXT("Err = %d"), Err);
	}
}

void CavaCommunicator::ProcUpdateNickName(Def::TID_ACCOUNT idAccount, const FString &Nickname)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	FBuddyInfo *Buddy = BuddyList.Find(idAccount);
	if (Buddy)
		Buddy->UpdateNickname(Nickname);
}

void CavaCommunicator::ProcPCBUserTimeExpire(BYTE Ret)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	if (Ret > 0)
	{
		_StateController->PlayerInfo.PcBangServiceType = 0;
		if (_StateController->IsStateInRoom())
		{
			FRoomPlayerInfo *Info = _StateController->GetMyRoomPlayerInfo();
			if (Info)
			{
				Info->RoomPlayerInfo.pcBang = 0;
				GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_PlayerList, TEXT("ok"), TEXT(""), 0, 0);
			}
		}

		if (_StateController->IsStatePlaying())
		{
			_StateController->LogChatConsoleNoMatch(*Localize(TEXT("UIPopUpMsg"), TEXT("Msg_Client_NoMorePremiumPcBang"), TEXT("AVANET")),
											EChat_PlayerSystem);
		}
		else
		{
			if (_StateController->ChannelInfo.Flag == EChannelFlag_PCBang)
			{
				// 즉시 채널 목록 화면으로 넘어감
				_StateController->GoToState(_AN_CHANNELLIST);
			}
			GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_NoMorePremiumPcBang, TEXT(""), TEXT(""), 0, 0);
		}
	}
}

void CavaCommunicator::ProcPCBRemainTime( const FString &StrTime )
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	DWORD Remain = appAtoi(*StrTime);
	//if (Remain > 0)
	{
		Remain /= 60; // 초 -> 분
		_StateController->LogChatConsoleNoMatch(*FString::Printf(*Localize(TEXT("UIPopUpMsg"), TEXT("Msg_Client_PremiumPcBangRemainTime"), TEXT("AVANET")), Remain / 60, Remain % 60),
											EChat_PlayerSystem);
	}
}
