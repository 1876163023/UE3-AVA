/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaMsgProcAdmin.cpp

	Description: Implementation of message processors

***/
#include "avaNet.h"

#define ASSERT check

#include "ComDef/Def.h"

using namespace Def;

#include "ComDef/MsgDef.h"
#include "ComDef/MsgDefAdmin.h"

#include "avaMsgProc.h"
#include "avaNetStateController.h"



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Admin


void PM::ADMIN::SET_VISIBILITY_ANS::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);
	DEF &def = msg.Data();

	if (msg.Data().result == RC_OK)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Admin, EMsg_Admin_SetVisibility, TEXT("off"), TEXT(""), 0, 0);
	}
	else
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Admin, EMsg_Admin_SetVisibility, TEXT("on"), TEXT(""), 0, 0);
	}
}

void PM::ADMIN::NOTICE_NTF::Proc(_LPMSGBUF pData)
{
	//if (!_StateController->ChannelInfo.IsValid())
	//	return;

	TMSG msg(pData);

	PM::_MsgString MsgStr(msg, msg.Data().tmsg);

	if (MsgStr.GetLength() > 0)
	{
		_StateController->RTNotice = MsgStr.GetString();
	}
	else
	{
		_StateController->RTNotice = TEXT("");
	}

	GetAvaNetHandler()->ProcMessage(EMsg_Admin, EMsg_Admin_Notice, MsgStr.GetString(), TEXT(""), 0, 0);
}

void PM::ADMIN::KICK_ANS::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);

	if (msg.Data().result == RC_OK)
	{
		_StateController->LogChatConsole(TEXT("(Admin) Successfully kicked the player(s)."), EChat_GMWhisper);
	}
	else
	{
		_StateController->LogChatConsole(TEXT("(Admin) Failed to kick the player(s)."), EChat_PlayerSystem);
	}
}

void PM::ADMIN::KICK_NTF::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);

	if (_StateController->PlayerInfo.PlayerInfo.idAccount == msg.Data().idAccount)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Admin, EMsg_Admin_Kick, TEXT("you"), TEXT(""), 0, 0);
	}
	else
	{
		if (_StateController->GetNetState() == _AN_LOBBY)
		{
			FLobbyPlayerInfo *pPlayer = _StateController->LobbyPlayerList.Find(msg.Data().idAccount);
			if (pPlayer)
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Admin, EMsg_Admin_Kick, TEXT("player"), pPlayer->LobbyPlayerInfo.nickname, 0, 0);
			}
		}
		else if (_StateController->GetNetState() == _AN_ROOM || _StateController->GetNetState() == _AN_INGAME)
		{
			FRoomPlayerInfo *pPlayer = _StateController->RoomInfo.PlayerList.Find(msg.Data().idAccount);
			if (pPlayer)
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Admin, EMsg_Admin_Kick, TEXT("player"), pPlayer->RoomPlayerInfo.nickname, 0, 0);
			}
		}
	}
}

void PM::ADMIN::CHATOFF_ANS::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);

	if (msg.Data().result == RC_OK)
	{
		_StateController->LogChatConsole(TEXT("(Admin) Successfully blocked the player(s) from chatting."), EChat_PlayerSystem);
	}
	else
	{
		_StateController->LogChatConsole(TEXT("(Admin) Failed to block the player(s) from chatting."), EChat_PlayerSystem);
	}
}

void PM::ADMIN::CHATOFF_NTF::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);

	if (_StateController->PlayerInfo.PlayerInfo.idAccount == msg.Data().idAccount)
	{
		_StateController->ChatOffDue = appSeconds() + 300.0;
		GetAvaNetHandler()->ProcMessage(EMsg_Admin, EMsg_Admin_ChatOff, TEXT("you"), TEXT(""), 0, 0);
	}
}

void PM::ADMIN::CHANGE_ROOMNAME_ANS::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);

	if (msg.Data().result == RC_OK)
	{
		_StateController->LogChatConsole(TEXT("(Admin) Successfully changed the room name."), EChat_PlayerSystem);
	}
	else
	{
		_StateController->LogChatConsole(TEXT("(Admin) Failed to change the room name."), EChat_PlayerSystem);
	}
}

void PM::ADMIN::CHANGE_ROOMNAME_NTF::Proc(_LPMSGBUF pData)
{
	if (_StateController->GetNetState() < _AN_LOBBY || _StateController->GetNetState() > _AN_INGAME)
		return;

	TMSG msg(pData);

	if (_StateController->GetNetState() == _AN_LOBBY)
	{
		FRoomDispInfo *pRoom = _StateController->RoomList.Find(msg.Data().idRoom);
		if (pRoom)
		{
			appStrcpy(pRoom->RoomInfo.roomName, msg.Data().roomname);
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomState, TEXT(""), TEXT(""), msg.Data().idRoom, 0);
	}
	else
	{
		if (_StateController->RoomInfo.IsValid() && _StateController->RoomInfo.RoomInfo.idRoom == msg.Data().idRoom)
		{
			appStrcpy(_StateController->RoomInfo.RoomInfo.roomName, msg.Data().roomname);
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Info, TEXT("room name"), TEXT(""), 0, 0);
	}
}

void PM::ADMIN::SET_TICKER_ANS::Proc(_LPMSGBUF pData)
{
}

void PM::ADMIN::SET_MAINNOTICE_ANS::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);

	if (msg.Data().result == RC_OK)
	{
		_StateController->LogChatConsole(TEXT("(Admin) Successfully set the main notice message."), EChat_PlayerSystem);
	}
	else
	{
		_StateController->LogChatConsole(TEXT("(Admin) Failed to set the main notice message."), EChat_PlayerSystem);
	}
}

void PM::ADMIN::WHISPER_NTF::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);
	DEF &def = msg.Data();

	PM::_MsgString ChatMsg(msg, def.chatmsg);
	FString ChatStr = FString::Printf(TEXT("[From GM] %s"), (LPCWSTR)ChatMsg);

	_StateController->LogChatConsole(*ChatStr, EChat_GMWhisper);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void PM::ADMIN::Proc(_LPMSGBUF pData)
{
	MSG_HEADER *pHeader = (MSG_HEADER*)pData->GetData();

	switch (pHeader->msg_id)
	{
		CASE_MSG_PROC(SET_VISIBILITY_ANS)
		CASE_MSG_PROC(NOTICE_NTF)
		CASE_MSG_PROC(KICK_ANS)
		CASE_MSG_PROC(KICK_NTF)
		CASE_MSG_PROC(CHATOFF_ANS)
		CASE_MSG_PROC(CHATOFF_NTF)
		CASE_MSG_PROC(CHANGE_ROOMNAME_ANS)
		CASE_MSG_PROC(CHANGE_ROOMNAME_NTF)
		CASE_MSG_PROC(SET_TICKER_ANS)
		CASE_MSG_PROC(SET_MAINNOTICE_ANS)
		CASE_MSG_PROC(WHISPER_NTF)

	default:
		_LOG(TEXT("Invalid ADMIN message received. ID = %d"), pHeader->msg_id);
	}
}
