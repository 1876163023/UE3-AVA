/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaMsgSend.cpp

	Description: Implementation of message senders

***/
#include "avaNet.h"

using namespace Def;


#define ASSERT check

#include "avaMsgSend.h"
#include "avaConnection.h"
#include "avaNetStateController.h"

#include "ComDef/MsgDef.h"
#include "ComDef/MsgDefClient.h"
#include "ComDef/MsgDefChannel.h"
#include "ComDef/MsgDefRoom.h"
#include "ComDef/MsgDefGame.h"
#include "ComDef/MsgDefInventory.h"
#include "ComDef/MsgDefAdmin.h"
#include "ComDef/MsgDefGuild.h"
#include "RxGateTranslator/RxGateTranslator.h"




/////////////////////////////////////////////////////////////////////////////////////////////////////
// Channel

void PM::CHANNEL::CHANNEL_LIST_REQ::Send()
{
	CHECK_ANY_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_CHANNELLIST)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	SEND_MSG_CHANNEL_P(msg);
}

void PM::CHANNEL::CHANNEL_JOIN_REQ::Send(TID_CHANNEL idChannel, UBOOL bFollowing)
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	//if (GavaNetClient->CLState != CLS_Connected)
	//{
	//	PROC_MSG_SEND_ERROR(TEXT("invalid state"));
	//	return;
	//}
	if (_StateController->GetNetState() != _AN_CHANNELLIST)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().idChannel = idChannel;
	msg.Data().bFollowing = bFollowing;

	SEND_MSG_CHANNEL_P(msg);
}

void PM::CHANNEL::CHANNEL_LEAVE_REQ::Send()
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	SEND_MSG_CHANNEL_P(msg);
}

void PM::CHANNEL::PLAYER_LIST_REQ::Send()
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	SEND_MSG_AUTO_P(msg);
}

void PM::CHANNEL::ROOM_LIST_REQ::Send()
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	SEND_MSG_AUTO_P(msg);
}

#ifndef _SERVER_PUSH

void PM::CHANNEL::ROOM_LIST_VIEW_NTF::Send(TArray<TID_ROOM> &RoomList)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	if (RoomList.Num() > 0)
	{
		def.count = ::Min<BYTE>(RoomList.Num(), MAX_ROOM_PER_PAGE);
		//appMemcpy(def.roomView, RoomList.GetTypedData(), def.count);
		for (INT i = 0; i < def.count; ++i)
		{
			def.roomView[i] = RoomList(i);
		}

		SEND_MSG_CHANNEL(msg);
	}
}

void PM::CHANNEL::ROOM_LIST_VIEW_ADD_NTF::Send(TID_ROOM idRoom)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_LOBBY || idRoom == ID_INVALID_ROOM)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	def.room_id = idRoom;

	SEND_MSG_CHANNEL(msg);
}

#endif

void PM::CHANNEL::ROOM_INFO_REQ::Send(TID_ROOM idRoom)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_LOBBY || idRoom == ID_INVALID_ROOM)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	msg.Data().room_id = idRoom;

	SEND_MSG_AUTO(msg);
}

void PM::CHANNEL::LOBBY_CHAT_NTF::Send(const FString &ChatMsg)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsValid())
		return;

	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg(TMSG::BODY_LENGTH + PM::_GetStringSize((WCHAR*)*ChatMsg));

	msg.Data().idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;

	PM::_MsgString MsgString(msg, msg.Data().chatmsg);
	MsgString.SetOffset(0);
	MsgString = *ChatMsg;

	SEND_MSG_AUTO(msg);
}

void PM::CHANNEL::PLAYER_INFO_REQ::Send(TID_ACCOUNT idAccount, RxGate::RXNERVE_ADDRESS *ServerAddr)
{
	CHECK_NO_CONNECTION()

	if (_StateController->PlayerInfo.PlayerInfo.idAccount == idAccount)
		return;
	if (_StateController->GetNetState() != _AN_LOBBY && !_StateController->IsStateInRoom())
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().idAccount = idAccount;

	if (ServerAddr)
	{
		_SendMsgToAddr(msg.GetBuf(), GavaNetClient->ClientKey, ID, DEF::MsgName(), ServerAddr, FALSE, 0.0);
	}
	else
	{
		SEND_MSG_AUTO(msg);
	}
}

void PM::CHANNEL::PLAYER_LOCATION_REQ::Send(TID_ACCOUNT idAccount, RxGate::RXNERVE_ADDRESS *ServerAddr)
{
	CHECK_NO_CONNECTION()

	if (_StateController->PlayerInfo.PlayerInfo.idAccount == idAccount)
		return;
	if (_StateController->GetNetState() != _AN_LOBBY && !_StateController->IsStateInRoom())
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().idAccount = idAccount;

	if (ServerAddr)
	{
		_SendMsgToAddr(msg.GetBuf(), GavaNetClient->ClientKey, ID, DEF::MsgName(), ServerAddr, FALSE, 0.0);
	}
	else
	{
		SEND_MSG_AUTO(msg);
	}
}

void PM::CHANNEL::FOLLOW_PLAYER_REQ::Send( Def::TID_ACCOUNT idAccount,  RxGate::RXNERVE_ADDRESS *ServerAddr )
{
	CHECK_NO_CONNECTION()

	if (_StateController->PlayerInfo.PlayerInfo.idAccount == idAccount)
		return;
	if (ServerAddr == NULL || _StateController->GetNetState() < _AN_CHANNELLIST)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	def.idTarget = idAccount;
	def.idSender = _StateController->PlayerInfo.PlayerInfo.idAccount;
	def.level = _StateController->PlayerInfo.PlayerInfo.level;
	def.idGuild = _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild;
	def.pcBangFlag = _StateController->PlayerInfo.IsPcBang() ? 1 : 0;
	def.sdRatio = _StateController->PlayerInfo.PlayerInfo.scoreInfo.GetSDRatio();

	_SendMsgToAddr(msg.GetBuf(), GavaNetClient->ClientKey, ID, DEF::MsgName(), ServerAddr, FALSE, GavaNetClient->TimeOutSec);
}

void PM::CHANNEL::WHISPER_NTF::Send(TID_ACCOUNT idAccountTo, const FString &NicknameTo, const FString &ChatMsg, UBOOL ToGuild)
{
	CHECK_NO_CONNECTION()

	if (idAccountTo == _StateController->PlayerInfo.PlayerInfo.idAccount || NicknameTo == _StateController->PlayerInfo.PlayerInfo.nickname)
	{
		PROC_MSG_SEND_ERROR(TEXT("not you"));
		return;
	}
	if (ChatMsg.Len() == 0)
	{
		PROC_MSG_SEND_ERROR(TEXT("no message"));
		return;
	}

	TMSG msg(TMSG::BODY_LENGTH + PM::_GetStringSize((WCHAR*)*ChatMsg));
	DEF &def = msg.Data();

	def.idAccount = idAccountTo;
	appStrncpy(def.nickname, *NicknameTo, SIZE_NICKNAME + 1);

	PM::_MsgString MsgString(msg, msg.Data().chatmsg);
	MsgString.SetOffset(0);
	MsgString = *ChatMsg;

	if (ToGuild)
	{
		SEND_MSG_GUILD(msg);
	}
	else
	{
		SEND_MSG_AUTO(msg);
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcMsgSendErrorChannel(WORD MsgID, const FString& Err)
{
	BYTE id = (MsgID & 0xff);

	using namespace PM::CHANNEL;

	switch (id)
	{
		CASE_MSG_SEND_ERROR(Channel, List, CHANNEL_LIST_REQ, Err)
		CASE_MSG_SEND_ERROR(Channel, Join, CHANNEL_JOIN_REQ, Err)
		CASE_MSG_SEND_ERROR(Channel, Leave, CHANNEL_LEAVE_REQ, Err)
		CASE_MSG_SEND_ERROR(Channel, PlayerList, PLAYER_LIST_REQ, Err)
		CASE_MSG_SEND_ERROR(Channel, RoomList, ROOM_LIST_REQ, Err)
		CASE_MSG_SEND_ERROR(Channel, RoomList, ROOM_LIST_VIEW_NTF, Err)
		CASE_MSG_SEND_ERROR(Channel, RoomList, ROOM_LIST_VIEW_ADD_NTF, Err)
		CASE_MSG_SEND_ERROR(Channel, RoomInfo, ROOM_INFO_REQ, Err)
		CASE_MSG_SEND_ERROR(Channel, LobbyChat, LOBBY_CHAT_NTF, Err)
		CASE_MSG_SEND_ERROR(Channel, PlayerInfo, PLAYER_INFO_REQ, Err)
		CASE_MSG_SEND_ERROR(Channel, PlayerInfo, PLAYER_LOCATION_REQ, Err)
		CASE_MSG_SEND_ERROR(Channel, Whisper, WHISPER_NTF, Err)
	default:
		_LOG(TEXT("Failed to send some CHANNEL message. ID = %d"), id);
	}
}
