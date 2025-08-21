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
// Room

void PM::ROOM::CREATE_REQ::Send(const FString &RoomName, const FString &Password, ROOM_SETTING &Setting)
{
	CHECK_ANY_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	appStrncpy(def.roomName, *RoomName, SIZE_ROOM_NAME + 1);
	appStrncpy(def.password, *Password, SIZE_ROOM_PWD + 1);
	def.setting = Setting;

	SEND_MSG_AUTO_P(msg);
}

void PM::ROOM::JOIN_REQ::Send(TID_ROOM idRoom, const FString &Password)
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	def.idRoom = idRoom;
	appStrncpy(def.password, *Password, SIZE_ROOM_PWD + 1);

	SEND_MSG_AUTO_P(msg);
}

void PM::ROOM::INFO_REQ::Send()
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_ROOM)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	SEND_MSG_AUTO_P(msg);
}

void PM::ROOM::LEAVE_REQ::Send(BYTE Reason)
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_ROOM && _StateController->GetNetState() != _AN_INGAME)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().reason = Reason;

	SEND_MSG_AUTO_P(msg);
}

void PM::ROOM::CHAT_NTF::Send(const FString &ChatMsg, TID_ACCOUNT idAccount)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsValid())
		return;

	if (_StateController->GetNetState() != _AN_ROOM && _StateController->GetNetState() != _AN_INGAME)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg(TMSG::BODY_LENGTH + PM::_GetStringSize((WCHAR*)*ChatMsg));

	msg.Data().idAccount = idAccount;

	PM::_MsgString MsgString(msg, msg.Data().chatmsg);
	MsgString.SetOffset(0);
	MsgString = *ChatMsg;

	SEND_MSG_AUTO(msg);
}

void PM::ROOM::PLAYER_INFO_REQ::Send(TID_ACCOUNT idAccount)
{
	CHECK_NO_CONNECTION()

	if (_StateController->PlayerInfo.PlayerInfo.idAccount == idAccount)
		return;
	if (_StateController->GetNetState() != _AN_ROOM)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().idAccount = idAccount;

	SEND_MSG_AUTO(msg);
}

void PM::ROOM::KICK_PLAYER_NTF::Send(BYTE idSlot, TID_ACCOUNT idAccount, BYTE reason)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_ROOM && _StateController->GetNetState() != _AN_INGAME)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	def.idSlot = idSlot;
	def.idAccount = idAccount;
	def.reason = reason;

	SEND_MSG_AUTO(msg);
}

void PM::ROOM::CHANGE_SETTING_REQ::Send(ROOM_SETTING Setting)
{
	CHECK_ANY_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_ROOM)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().setting = Setting;

	SEND_MSG_AUTO_P(msg);
}

void PM::ROOM::READY_NTF::Send(BYTE Ready)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_ROOM || _StateController->MyRoomSlotIdx == ID_INVALID_ROOM_SLOT)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	def.idSlot = _StateController->MyRoomSlotIdx;
	def.bReady = Ready;

	SEND_MSG_AUTO(msg);
}

void PM::ROOM::CHANGE_SLOT_NTF::Send(BYTE newSlot)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_ROOM || _StateController->MyRoomSlotIdx == ID_INVALID_ROOM_SLOT)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	if (newSlot >= MAX_ALL_PLAYER_PER_ROOM)
	{
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	def.idSlot = _StateController->MyRoomSlotIdx;
	def.idTeam = FRoomInfo::SlotToTeam(newSlot);
	def.newSlot = newSlot;

	SEND_MSG_AUTO(msg);
}

void PM::ROOM::CHANGE_CLASS_NTF::Send(BYTE idClass)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_ROOM || _StateController->MyRoomSlotIdx == ID_INVALID_ROOM_SLOT)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	if (_StateController->RoomInfo.PlayerList.PlayerList[_StateController->MyRoomSlotIdx].RoomPlayerInfo.currentClass == idClass)
		return;

	TMSG msg;
	DEF &def = msg.Data();

	def.idSlot = _StateController->MyRoomSlotIdx;
	def.idClass = idClass;
	//def.equipWeapon = 0;

	SEND_MSG_AUTO(msg);
}

void PM::ROOM::CLAIM_HOST_NTF::Send()
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_ROOM)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	SEND_MSG_AUTO(msg);
}

void PM::ROOM::SET_HOSTADDR_NTF::Send(IPADDR_INFO &hostAddr)
{
	CHECK_NO_CONNECTION()

	if ( !_StateController->IsStateInRoom() || _StateController->MyRoomSlotIdx == ID_INVALID_ROOM_SLOT )
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	msg.Data().hostInfo.extAddr = hostAddr;	

	SEND_MSG_AUTO(msg);
}

// {{ 20070302 dEAthcURe|HP
void PM::ROOM::SET_HOSTADDR_NTF::Send(IPADDR_INFO &intAddr, IPADDR_INFO &extAddr)
{
	CHECK_NO_CONNECTION()

	if ( !_StateController->IsStateInRoom() || _StateController->MyRoomSlotIdx == ID_INVALID_ROOM_SLOT )
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	msg.Data().hostInfo.intAddr = intAddr;
	msg.Data().hostInfo.extAddr = extAddr;

	SEND_MSG_AUTO(msg);
}
// }} 20070302 dEAthcURe|HP

void PM::ROOM::SWAP_TEAM_NTF::Send(INT Reason)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->IsStateInRoom())
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}
	if (!_StateController->AmIHost() && !_StateController->AmIAdmin())
	{
		PROC_MSG_SEND_ERROR(TEXT("host only"));
		return;
	}

	TMSG msg;

	msg.Data().reason = Reason;

	SEND_MSG_AUTO(msg);
}


void PM::ROOM::RTTT_START_REQ::Send(TID_ACCOUNT idAccount, UDP_HOST_INFO &HostInfo)
{
#ifdef EnableRttTest
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_ROOM && _StateController->GetNetState() != _AN_INGAME)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	def.idAccount = idAccount;
	def.hostInfo = HostInfo;

	SEND_MSG_AUTO_F_P(msg);
#endif
}


void PM::ROOM::RTT_UPDATE_NTF::Send(FLOAT Rating)
{
#ifdef EnableRttTest
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_ROOM && _StateController->GetNetState() != _AN_INGAME)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	msg.Data().idSlot = _StateController->MyRoomSlotIdx;
	msg.Data().rttScore = Rating;

	SEND_MSG_AUTO(msg);
#endif
}


void PM::ROOM::QUICK_JOIN_REQ::Send(TID_MAP idMap)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	msg.Data().idMap = idMap;

	SEND_MSG_AUTO_P(msg);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcMsgSendErrorRoom(WORD MsgID, const FString& Err)
{
	BYTE id = (MsgID & 0xff);

	using namespace PM::ROOM;

	switch (id)
	{
		CASE_MSG_SEND_ERROR(Room, Create, CREATE_REQ, Err)
		CASE_MSG_SEND_ERROR(Room, Join, JOIN_REQ, Err)
		CASE_MSG_SEND_ERROR(Room, Info, INFO_REQ, Err)
		CASE_MSG_SEND_ERROR(Room, Leave, LEAVE_REQ, Err)
		CASE_MSG_SEND_ERROR(Room, Chat, CHAT_NTF, Err)
		CASE_MSG_SEND_ERROR(Room, PlayerInfo, PLAYER_INFO_REQ, Err)
		CASE_MSG_SEND_ERROR(Room, Kick, KICK_PLAYER_NTF, Err)
		CASE_MSG_SEND_ERROR(Room, ChangeSetting, CHANGE_SETTING_REQ, Err)
		CASE_MSG_SEND_ERROR(Room, Ready, READY_NTF, Err)
		CASE_MSG_SEND_ERROR(Room, ChangeSlot, CHANGE_SLOT_NTF, Err)
		CASE_MSG_SEND_ERROR(Room, ChangeClass, CHANGE_CLASS_NTF, Err)
		CASE_MSG_SEND_ERROR(Room, ClaimHost, CLAIM_HOST_NTF, Err)
		//CASE_MSG_SEND_ERROR(Room, SetHostAddr, SET_HOSTADDR_NTF, Err)
		CASE_MSG_SEND_ERROR(Room, SwapTeam, SWAP_TEAM_NTF, Err)
		CASE_MSG_SEND_ERROR(Room, RtttStart, RTTT_START_REQ, Err)
		CASE_MSG_SEND_ERROR(Room, RttRating, RTT_UPDATE_NTF, Err)
	default:
		_LOG(TEXT("Failed to send some ROOM message. ID = %d"), id);
	}
}
