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
// Admin

void PM::ADMIN::NOTICE_NTF::Send(TID_CHANNEL idChannel, const FString &Msg)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsAdmin() && _StateController->PlayerInfo.GetCurrentChannelMaskLevel() <= _CML_PLAYER)
	{
		PROC_MSG_SEND_ERROR(TEXT("admin only"));
		return;
	}
	//if (Msg.Len() == 0)
	//	return;

	TMSG msg(TMSG::BODY_LENGTH + PM::_GetStringSize((WCHAR*)*Msg));

	msg.Data().idChannel = idChannel;

	PM::_MsgString MsgString(msg, msg.Data().tmsg);
	MsgString.SetOffset(0);
	MsgString = *Msg;

	SEND_MSG_CHANNEL(msg);
}

void PM::ADMIN::SET_VISIBILITY_REQ::Send(UBOOL bVisible)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsAdmin())
	{
		PROC_MSG_SEND_ERROR(TEXT("admin only"));
		return;
	}

	TMSG msg;

	msg.Data().bVisible = (BYTE)bVisible;

	SEND_MSG_CHANNEL(msg);
}

void PM::ADMIN::KICK_REQ::Send(INT type, TID_ACCOUNT idAccount, const FString &name)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsAdmin())
	{
		PROC_MSG_SEND_ERROR(TEXT("admin only"));
		return;
	}

	if (type < 0 || type > 2)
		return;
	if (type == 0 && idAccount == ID_INVALID_ACCOUNT)
		return;
	if (type == 1 && (name.Len() == 0 || name.Len() > SIZE_USER_ID))
		return;
	if (type == 2 && (name.Len() == 0 || name.Len() > SIZE_NICKNAME))
		return;

	TMSG msg;
	DEF &def = msg.Data();

	def.type = type;
	def.user_sn = idAccount;
	appStrcpy(def.name, *name);

	SEND_MSG_AUTO(msg);
}

void PM::ADMIN::CHATOFF_REQ::Send(const FString &Nickname)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsAdmin() && _StateController->PlayerInfo.GetCurrentChannelMaskLevel() <= _CML_PLAYER)
	{
		PROC_MSG_SEND_ERROR(TEXT("admin only"));
		return;
	}
	if (Nickname.Len() == 0 || Nickname.Len() > SIZE_NICKNAME)
	{
		PROC_MSG_SEND_ERROR(TEXT("wrong name length"));
		return;
	}

	TMSG msg;

	appStrcpy(msg.Data().nickname, *Nickname);

	SEND_MSG_AUTO(msg);
}

void PM::ADMIN::CHANGE_ROOMNAME_REQ::Send(TID_ROOM idRoom, const FString &RoomName)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsAdmin())
	{
		PROC_MSG_SEND_ERROR(TEXT("admin only"));
		return;
	}
	if (RoomName.Len() == 0 || RoomName.Len() > SIZE_ROOM_NAME)
	{
		PROC_MSG_SEND_ERROR(TEXT("wrong name length"));
		return;
	}

	TMSG msg;

	msg.Data().idRoom = idRoom;
	appStrcpy(msg.Data().roomname, *RoomName);

	SEND_MSG_AUTO(msg);
}

void PM::ADMIN::SET_MAINNOTICE_REQ::Send(const FString &Notice)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsAdmin())
	{
		PROC_MSG_SEND_ERROR(TEXT("admin only"));
		return;
	}
	if (Notice.Len() == 0 || Notice.Len() > SIZE_NOTICE_MSG)
	{
		PROC_MSG_SEND_ERROR(TEXT("wrong msg length"));
		return;
	}

	TMSG msg;

	appStrcpy(msg.Data().notice, *Notice);

	SEND_MSG_CHANNEL(msg);
}

void PM::ADMIN::WHISPER_NTF::Send(TID_ACCOUNT idAccountTo, const FString &NicknameTo, const FString &ChatMsg)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsAdmin())
	{
		PROC_MSG_SEND_ERROR(TEXT("admin only"));
		return;
	}
	if (idAccountTo == _StateController->PlayerInfo.PlayerInfo.idAccount || NicknameTo == _StateController->PlayerInfo.PlayerInfo.nickname)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid player"));
		return;
	}
	if (ChatMsg.Len() == 0)
	{
		PROC_MSG_SEND_ERROR(TEXT("wrong msg length"));
		return;
	}

	TMSG msg(TMSG::BODY_LENGTH + PM::_GetStringSize((WCHAR*)*ChatMsg));
	DEF &def = msg.Data();

	def.idAccount = idAccountTo;
	appStrncpy(def.nickname, *NicknameTo, SIZE_NICKNAME + 1);

	PM::_MsgString MsgString(msg, msg.Data().chatmsg);
	MsgString.SetOffset(0);
	MsgString = *ChatMsg;

	SEND_MSG_AUTO(msg);
}




/////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcMsgSendErrorAdmin(WORD MsgID, const FString& Err)
{
	BYTE id = (MsgID & 0xff);

	using namespace PM::ADMIN;

	switch (id)
	{
		CASE_MSG_SEND_ERROR(Admin, Notice, NOTICE_NTF, Err)
		CASE_MSG_SEND_ERROR(Admin, SetVisibility, SET_VISIBILITY_REQ, Err)
		CASE_MSG_SEND_ERROR(Admin, Kick, KICK_REQ, Err)
		CASE_MSG_SEND_ERROR(Admin, ChatOff, CHATOFF_REQ, Err)
		CASE_MSG_SEND_ERROR(Admin, ChangeRoomName, CHANGE_ROOMNAME_REQ, Err)
		CASE_MSG_SEND_ERROR(Admin, MainNotice, SET_MAINNOTICE_REQ, Err)
		CASE_MSG_SEND_ERROR(Admin, Whisper, WHISPER_NTF, Err)
	default:
		_LOG(TEXT("Failed to send some ADMIN message. ID = %d"), id);
	}
}
