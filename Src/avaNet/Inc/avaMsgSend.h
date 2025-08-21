/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaMsgSend.h

	Description: Message senders.

***/

#ifndef __AVAMSGSEND_H__
#define __AVAMSGSEND_H__


#include "ComDef/Def.h"
using namespace Def;

class CavaConnection;

namespace PM
{
	namespace CLIENT
	{
		namespace CONNECT_REQ
		{
			void Send(WORD versionProtocol, WORD versionClient, const FString& key, BYTE reconnect = 0, __int64 addr = 0);
		}

		//namespace CREATE_ACCOUNT_REQ
		//{
		//	void Send(FString &UserID, FString &Password);
		//}

		namespace CHECK_NICK_REQ
		{
			void Send(const FString &Nickname, BYTE idFace);
		}

		namespace HWINFO_NTF
		{
			void Send(const FString &cpu, const FString &gpu, DWORD mem, BYTE *adapterAddress);
		}

		namespace SET_CONFIG_NTF
		{
			void Send(const FString &ConfigStr1, const FString &ConfigStr2);
		}

		namespace SET_RTTTEST_ADDR_NTF
		{
			void Send(UDP_HOST_INFO &HostInfo);
		}

		namespace GUILD_CONNECT_REQ
		{
			void Send(WORD versionProtocol, WORD versionClient, TID_ACCOUNT idAccount, TID_GUILD idGuild, const FString& key, BYTE reconnect = 0, __int64 addr = 0);
		}
	}

	namespace CHANNEL
	{
		namespace CHANNEL_LIST_REQ
		{
			void Send();
		}

		namespace CHANNEL_JOIN_REQ
		{
			void Send(TID_CHANNEL idChannel, UBOOL bFollowing = FALSE);
		}

		namespace CHANNEL_LEAVE_REQ
		{
			void Send();
		}

		namespace PLAYER_LIST_REQ
		{
			void Send();
		}

		namespace ROOM_LIST_REQ
		{
			void Send();
		}

#ifndef _SERVER_PUSH
		namespace ROOM_LIST_VIEW_NTF
		{
			void Send(TArray<TID_ROOM> &RoomList);
		}

		namespace ROOM_LIST_VIEW_ADD_NTF
		{
			void Send(TID_ROOM idRoom);
		}
#endif

		namespace ROOM_INFO_REQ
		{
			void Send(TID_ROOM idRoom);
		}

		namespace LOBBY_CHAT_NTF
		{
			void Send(const FString &ChatMsg);
		}

		namespace PLAYER_INFO_REQ
		{
			void Send(TID_ACCOUNT idAccount, RxGate::RXNERVE_ADDRESS *ServerAddr = NULL);
		}

		namespace PLAYER_LOCATION_REQ
		{
			void Send(TID_ACCOUNT idAccount, RxGate::RXNERVE_ADDRESS *ServerAddr = NULL);
		}

		namespace FOLLOW_PLAYER_REQ
		{
			void Send(TID_ACCOUNT idAccount, RxGate::RXNERVE_ADDRESS *ServerAddr);
		}

		namespace WHISPER_NTF
		{
			void Send(TID_ACCOUNT idAccountTo, const FString &NicknameTo, const FString &ChatMsg, UBOOL ToGuild = FALSE);
		}
	}

	namespace ROOM
	{
		namespace CREATE_REQ
		{
			void Send(const FString &RoomName, const FString &Password, ROOM_SETTING &Setting);
		}

		namespace JOIN_REQ
		{
			void Send(TID_ROOM idRoom, const FString &Password);
		}

		namespace INFO_REQ
		{
			void Send();
		}

		namespace LEAVE_REQ
		{
			void Send(BYTE Reason);
		}

		namespace CHAT_NTF
		{
			void Send(const FString &ChatMsg, TID_ACCOUNT idAccount = ID_INVALID_ACCOUNT);
		}

		namespace PLAYER_INFO_REQ
		{
			void Send(TID_ACCOUNT idAccount);
		}

		namespace KICK_PLAYER_NTF
		{
			void Send(BYTE idSlot, TID_ACCOUNT idAccount, BYTE reason = 0);	// 0 = ban, 1 = loading time out, 2 = setting changed
		}

		namespace CHANGE_SETTING_REQ
		{
			void Send(ROOM_SETTING Setting);
		}

		namespace READY_NTF
		{
			void Send(BYTE Ready);
		}

		namespace CHANGE_SLOT_NTF
		{
			void Send(BYTE newSlot);
		}

		namespace CHANGE_CLASS_NTF
		{
			void Send(BYTE idClass);
		}

		namespace CLAIM_HOST_NTF
		{
			void Send();
		}

		namespace SET_HOSTADDR_NTF
		{
			void Send(IPADDR_INFO &hostAddr);
			void Send(IPADDR_INFO &intAddr, IPADDR_INFO &extAddr);
		}

		namespace SWAP_TEAM_NTF
		{
			void Send(INT Reason);
		}

		namespace RTTT_START_REQ
		{
			void Send(TID_ACCOUNT idAccount, UDP_HOST_INFO &HostInfo);
		}

		namespace RTT_UPDATE_NTF
		{
			void Send(FLOAT Rating);
		}

		namespace QUICK_JOIN_REQ
		{
			void Send(TID_MAP idMap = ID_INVALID_MAP);
		}
	}

	namespace GAME
	{
		namespace START_NTF
		{
			void Send();
		}

		namespace READY_NTF
		{
			void Send();
		}

		namespace JOIN_NTF
		{
			void Send(UDP_HOST_INFO &addrInfo);
		}

		namespace UPDATE_STATE_NTF
		{
			void Send(BYTE RoundCount);
		}

		namespace LEAVE_NTF
		{
			void Send();
		}

		namespace END_NTF
		{
			void Send(INT AvgHostPing);
		}

		namespace UPDATE_SCORE_NTF
		{
			void Send(PLAYER_RESULT_INFO &PlayerScore);
			void Send(const TArray<PLAYER_RESULT_INFO> &PlayerScore, INT TeamScore[2], UBOOL bGameEnd = TRUE);
		}

		namespace HOST_BAN_NTF
		{
			void Send();
		}

		namespace START_COUNT_NTF
		{
			void Send();
		}

		namespace CANCEL_COUNT_NTF
		{
			void Send();
		}

		namespace REPORT_STAT_NTF
		{
			void Send(STAT_GAME_SCORE_LOG *GameScoreLogs, TArray<STAT_ROUND_PLAY_LOG> &RoundPlayLogs, TArray<STAT_WEAPON_LOG> &WeaponLogs, TArray<STAT_KILL_LOG> &KillLogs);
		}

		namespace LOADING_PROGRESS_NTF
		{
			void Send(BYTE Progress, INT Step);
		}

		namespace REPORT_VOTE_NTF
		{
			void Send(DWORD Command, DWORD idCaller, const TArray<INT> &VoterList, DWORD param1, DWORD param2);
		}

		namespace CHAT_NTF
		{
			void Send(const FString &ChatMsg, UBOOL Team = TRUE);
		}
	}

	namespace INVENTORY
	{
		namespace ENTER_NTF
		{
			void Send();
		}

		namespace LEAVE_NTF
		{
			void Send();
		}

		namespace EQUIPSET_REQ
		{
			void Send(TID_EQUIP_SLOT EquipSlot, TSN_ITEM ItemSN);
		}

		namespace WEAPONSET_REQ
		{
			void Send(TID_EQUIP_SLOT EquipSlot, TSN_ITEM ItemSN);
		}

		namespace CUSTOMSET_REQ
		{
			void Send(TSN_ITEM ItemSN, TID_ITEM idCustomPart, BYTE Pos);
		}

		namespace ITEM_BUY_REQ
		{
			void Send(TID_ITEM idItem);
		}

		namespace ITEM_GIFT_REQ
		{
			void Send(TID_ITEM idItem, DWORD Expire, TID_ACCOUNT idAccount);
		}

		namespace REPAIR_REQ
		{
			void Send(TSN_ITEM ItemSN);
		}

		namespace CHANGE_CLASS_NTF
		{
			void Send(BYTE idClass);
		}

		namespace CONVERT_RIS_REQ
		{
			void Send(TSN_ITEM ItemSN);
		}

		namespace ITEM_REFUND_REQ
		{
			void Send(ITEM_INFO &Item);
		}

		namespace CASHITEM_BUY_REQ
		{
			void Send(TID_ITEM idItem, TSN_ITEM ItemSN);
		}

		namespace EFFSET_REQ
		{
			void Send(TID_EQUIP_SLOT EquipSlot, TSN_ITEM ItemSN);
		}
	}

	namespace ADMIN
	{
		namespace SET_VISIBILITY_REQ
		{
			void Send(UBOOL bVisible);
		}

		namespace NOTICE_NTF
		{
			void Send(TID_CHANNEL idChannel, const FString &Msg);
		}

		namespace KICK_REQ
		{
			void Send(INT type, TID_ACCOUNT idAccount, const FString &name);
		}

		namespace CHATOFF_REQ
		{
			void Send(const FString &Nickname);
		}

		namespace CHANGE_ROOMNAME_REQ
		{
			void Send(TID_ROOM idRoom, const FString &RoomName);
		}

		namespace SET_MAINNOTICE_REQ
		{
			void Send(const FString &Notice);
		}

		namespace WHISPER_NTF
		{
			void Send(TID_ACCOUNT idAccountTo, const FString &NicknameTo, const FString &ChatMsg);
		}
	}

	namespace GUILD
	{
		namespace LOBBY_JOIN_REQ
		{
			void Send(__int64 ChAddr);
		}

		namespace LOBBY_LEAVE_REQ
		{
			void Send();
		}

		namespace LOBBY_CHAT_NTF
		{
			void Send(const FString &ChatMsg);
		}

		namespace SET_MOTD_REQ
		{
			void Send(const FString &Motd);
		}

		namespace NOTICE_NTF
		{
			void Send(const FString &NoticeMsg);
		}

		namespace CHAT_NTF
		{
			void Send(const FString &ChatMsg);
		}

		namespace WHISPER_NTF
		{
			void Send(TID_ACCOUNT idAccountTo, const FString &NicknameTo, const FString &ChatMsg);
		}

		namespace PLAYER_INFO_REQ
		{
			void Send(TID_ACCOUNT idAccount);
			void Send(TID_ACCOUNT idAccount, TID_GUILD idGuild, RxGate::RXNERVE_ADDRESS *ServerAddr);
		}

		namespace LEAVE_NTF
		{
			void Send();
		}

		namespace KICK_NTF
		{
			void Send(TID_ACCOUNT idAccount);
		}

		namespace GET_CHANNEL_ADDR_REQ
		{
			void Send();
		}

		namespace NICKNAME_UPDATE_NTF
		{
			void Send(TID_ACCOUNT idAccount, const FString Nickname);
		}

		namespace PLAYER_LOCATION_REQ
		{
			void Send(TID_ACCOUNT idAccount);
		}

	}
}


void _SendMsgToAddr(_LPMSGBUF pBuf, DWORD Key, WORD id, const TCHAR *MsgName, RxGate::LPRXNERVE_ADDRESS pAddr, UBOOL bForceDirect, DOUBLE PendingTime = 0.0);
void _SendMsgTo(_LPMSGBUF pBuf, WORD Key, WORD id, const TCHAR *MsgName, UBOOL bForceDirect, DOUBLE PendingTime = 0.0);



#define SEND_MSG_CHANNEL(_m)		_SendMsgTo(_m.GetBuf(), GavaNetClient->SessionKeyChannel, ID, DEF::MsgName(), FALSE, 0.0)
#define SEND_MSG_CHANNEL_P(_m)		_SendMsgTo(_m.GetBuf(), GavaNetClient->SessionKeyChannel, ID, DEF::MsgName(), FALSE, GavaNetClient->TimeOutSec)

#define SEND_MSG_CHANNEL_F(_m)		_SendMsgTo(_m.GetBuf(), GavaNetClient->SessionKeyChannel, ID, DEF::MsgName(), TRUE, 0.0)
#define SEND_MSG_CHANNEL_F_P(_m)	_SendMsgTo(_m.GetBuf(), GavaNetClient->SessionKeyChannel, ID, DEF::MsgName(), TRUE, GavaNetClient->TimeOutSec)

#define SEND_MSG_GUILD(_m)			_SendMsgTo(_m.GetBuf(), GavaNetClient->SessionKeyGuild, ID, DEF::MsgName(), FALSE, 0.0)
#define SEND_MSG_GUILD_P(_m)		_SendMsgTo(_m.GetBuf(), GavaNetClient->SessionKeyGuild, ID, DEF::MsgName(), FALSE, GavaNetClient->TimeOutSec)

#define SEND_MSG_GUILD_F(_m)		_SendMsgTo(_m.GetBuf(), GavaNetClient->SessionKeyGuild, ID, DEF::MsgName(), TRUE, 0.0)
#define SEND_MSG_GUILD_F_P(_m)		_SendMsgTo(_m.GetBuf(), GavaNetClient->SessionKeyGuild, ID, DEF::MsgName(), TRUE, GavaNetClient->TimeOutSec)

#define SEND_MSG_AUTO(_m)										\
	if (_StateController->ChannelInfo.IsNormalChannel())		\
		SEND_MSG_CHANNEL(_m);									\
	else if (_StateController->ChannelInfo.IsMyClanChannel())	\
		SEND_MSG_GUILD(_m);

#define SEND_MSG_AUTO_F(_m)										\
	if (_StateController->ChannelInfo.IsNormalChannel())		\
		SEND_MSG_CHANNEL_F(_m);									\
	else if (_StateController->ChannelInfo.IsMyClanChannel())	\
		SEND_MSG_GUILD_F(_m);

#define SEND_MSG_AUTO_P(_m)										\
	if (_StateController->ChannelInfo.IsNormalChannel())		\
		SEND_MSG_CHANNEL_P(_m);									\
	else if (_StateController->ChannelInfo.IsMyClanChannel())	\
		SEND_MSG_GUILD_P(_m);

#define SEND_MSG_AUTO_F_P(_m)									\
	if (_StateController->ChannelInfo.IsNormalChannel())		\
		SEND_MSG_CHANNEL_F_P(_m);								\
	else if (_StateController->ChannelInfo.IsMyClanChannel())	\
		SEND_MSG_GUILD_F_P(_m);



void ProcMsgSendError(WORD MsgID, const FString Err);
void ProcMsgSendErrorClient(WORD MsgID, const FString& Err);
void ProcMsgSendErrorChannel(WORD MsgID, const FString& Err);
void ProcMsgSendErrorRoom(WORD MsgID, const FString& Err);
void ProcMsgSendErrorGame(WORD MsgID, const FString& Err);
void ProcMsgSendErrorInventory(WORD MsgID, const FString& Err);
void ProcMsgSendErrorGuild(WORD MsgID, const FString& Err);
void ProcMsgSendErrorAdmin(WORD MsgID, const FString& Err);


#define PROC_MSG_SEND_ERROR(str)	ProcMsgSendError(ID, str)

#define CHECK_NO_CONNECTION()											\
	if (!GavaNetClient || !GavaNetClient->CurrentConnection())			\
	{																	\
		PROC_MSG_SEND_ERROR(TEXT("no connection"));						\
		return;															\
	}


#define CHECK_PENDING_SEND_MSG()													\
	if (GavaNetClient->PendingMsgs.Find(ID) != NULL)								\
	{																				\
		_LOG(TEXT("[%s] is already sent and not processed yet."), DEF::MsgName());	\
		ProcMsgSendError(ID, TEXT("pending same"));									\
		return;																		\
	}

#define CHECK_ANY_PENDING_SEND_MSG()												\
	if (GavaNetClient->PendingMsgs.Num() > 0)										\
	{																				\
		_LOG(TEXT("[%s] Some message is pending."), DEF::MsgName());				\
		for (TDynamicMap<WORD,FPendingMsg>::TIterator it(GavaNetClient->PendingMsgs); it; ++it)	\
			it.Value().Dump();														\
		ProcMsgSendError(ID, TEXT("pending any"));									\
		return;																		\
	}




//#if FINAL_RELEASE

#define CASE_MSG_SEND_ERROR(cat, id, name, err)										\
	case ID_##name:																	\
		_LOG(TEXT("[%s] Send ERROR. (%s)"), name::DEF::MsgName(), *err);			\
		GetAvaNetHandler()->eventProcSendError(EMsg_##cat, EMsg_##cat##_##id, err);	\
		break;

//#else
//
//#define CASE_MSG_SEND_ERROR(cat, id, name, err)										\
//	case ID_##name:																	\
//		_LOG(TEXT("[%s] Send ERROR."), name::DEF::MsgName());						\
//		break;

//#endif

#endif

