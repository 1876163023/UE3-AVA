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
// Guild

void PM::GUILD::LOBBY_JOIN_REQ::Send(__int64 ChAddr)
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_CHANNELLIST)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().idGuild = _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild;
	msg.Data().idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
	msg.Data().addr = ChAddr;

	SEND_MSG_GUILD_P(msg);
}

void PM::GUILD::LOBBY_LEAVE_REQ::Send()
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().idGuild = _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild;
	msg.Data().idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;

	SEND_MSG_GUILD_P(msg);
}

void PM::GUILD::LOBBY_CHAT_NTF::Send(const FString &ChatMsg)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg(TMSG::BODY_LENGTH + PM::_GetStringSize((WCHAR*)*ChatMsg));

	msg.Data().idGuild = _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild;
	msg.Data().idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
	appStrcpy(msg.Data().nickname, _StateController->PlayerInfo.PlayerInfo.nickname);

	PM::_MsgString MsgString(msg, msg.Data().chatmsg);
	MsgString.SetOffset(0);
	MsgString = *ChatMsg;

	SEND_MSG_GUILD(msg);
}

void PM::GUILD::SET_MOTD_REQ::Send(const FString &Motd)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		PROC_MSG_SEND_ERROR(TEXT("no guild channel"));
		return;
	}

	TMSG msg(TMSG::BODY_LENGTH + PM::_GetStringSize((WCHAR*)*Motd));

	PM::_MsgString MsgString(msg, msg.Data().motd);
	MsgString.SetOffset(0);
	MsgString = *Motd;

	SEND_MSG_GUILD(msg);
}

void PM::GUILD::NOTICE_NTF::Send(const FString &NoticeMsg)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsValid())
	{
		PROC_MSG_SEND_ERROR(TEXT("inavlid state"));
		return;
	}
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		PROC_MSG_SEND_ERROR(TEXT("no guild channel"));
		return;
	}

	TMSG msg(TMSG::BODY_LENGTH + PM::_GetStringSize((WCHAR*)*NoticeMsg));

	msg.Data().idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;

	PM::_MsgString MsgString(msg, msg.Data().chatmsg);
	MsgString.SetOffset(0);
	MsgString = *NoticeMsg;

	SEND_MSG_GUILD(msg);
}

void PM::GUILD::CHAT_NTF::Send(const FString &ChatMsg)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsValid())
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		PROC_MSG_SEND_ERROR(TEXT("no guild channel"));
		return;
	}

	TMSG msg(TMSG::BODY_LENGTH + PM::_GetStringSize((WCHAR*)*ChatMsg));

	msg.Data().idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;

	PM::_MsgString MsgString(msg, msg.Data().chatmsg);
	MsgString.SetOffset(0);
	MsgString = *ChatMsg;

	SEND_MSG_GUILD(msg);
}

void PM::GUILD::WHISPER_NTF::Send(TID_ACCOUNT idAccountTo, const FString &NicknameTo, const FString &ChatMsg)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsValid())
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		PROC_MSG_SEND_ERROR(TEXT("no guild channel"));
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

	SEND_MSG_GUILD(msg);
}

void PM::GUILD::PLAYER_INFO_REQ::Send(TID_ACCOUNT idAccount)
{
	CHECK_NO_CONNECTION()

	if (_StateController->PlayerInfo.PlayerInfo.idAccount == idAccount)
		return;
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		PROC_MSG_SEND_ERROR(TEXT("no guild channel"));
		return;
	}

	TMSG msg;

	msg.Data().idAccount = idAccount;
	msg.Data().idGuild = _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild;

	SEND_MSG_GUILD(msg);
}

void PM::GUILD::PLAYER_INFO_REQ::Send(TID_ACCOUNT idAccount, TID_GUILD idGuild, RxGate::RXNERVE_ADDRESS *ServerAddr)
{
	CHECK_NO_CONNECTION()

	if (_StateController->PlayerInfo.PlayerInfo.idAccount == idAccount)
		return;
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		PROC_MSG_SEND_ERROR(TEXT("no guild channel"));
		return;
	}

	TMSG msg;

	msg.Data().idAccount = idAccount;
	msg.Data().idGuild = idGuild;

	if (ServerAddr)
	{
		_SendMsgToAddr(msg.GetBuf(), GavaNetClient->ClientKey, ID, DEF::MsgName(), ServerAddr, FALSE, 0.0);
	}
	else
	{
		SEND_MSG_GUILD(msg);
	}
}

void PM::GUILD::LEAVE_NTF::Send()
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsValid())
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		PROC_MSG_SEND_ERROR(TEXT("no guild channel"));
		return;
	}

	TMSG msg;

	SEND_MSG_GUILD(msg);

	msg.Data().idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
}

void PM::GUILD::KICK_NTF::Send( Def::TID_ACCOUNT idAccount )
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsValid())
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		PROC_MSG_SEND_ERROR(TEXT("no guild channel"));
		return;
	}
	if (idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid player"));
		return;
	}
	// check priv

	TMSG msg;

	msg.Data().idAccount = idAccount;

	SEND_MSG_GUILD(msg);
}

void PM::GUILD::GET_CHANNEL_ADDR_REQ::Send()
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsValid())
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}
	if (!_StateController->GuildInfo.IsValid())
	{
		PROC_MSG_SEND_ERROR(TEXT("no guild"));
		return;
	}
	if (_StateController->GuildInfo.IsChannelConnected())
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	SEND_MSG_CHANNEL_P(msg);
}

void PM::GUILD::NICKNAME_UPDATE_NTF::Send( Def::TID_ACCOUNT idAccount, const FString Nickname )
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsValid())
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}
	if (!_StateController->GuildInfo.IsValid())
	{
		PROC_MSG_SEND_ERROR(TEXT("no guild"));
		return;
	}
	if (_StateController->GuildInfo.IsChannelConnected())
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().idAccount = idAccount;
	appStrncpy(msg.Data().nickname, *Nickname, SIZE_NICKNAME+1);

	SEND_MSG_GUILD(msg);
}

void PM::GUILD::PLAYER_LOCATION_REQ::Send(TID_ACCOUNT idAccount)
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
	SEND_MSG_GUILD(msg);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcMsgSendErrorGuild(WORD MsgID, const FString& Err)
{
	BYTE id = (MsgID & 0xff);

	using namespace PM::GUILD;

	switch (id)
	{
		CASE_MSG_SEND_ERROR(Guild, JoinChannel, LOBBY_JOIN_REQ, Err)
		CASE_MSG_SEND_ERROR(Guild, LeaveChannel, LOBBY_LEAVE_REQ, Err)
		CASE_MSG_SEND_ERROR(Guild, Motd, SET_MOTD_REQ, Err)
		CASE_MSG_SEND_ERROR(Guild, Notice, NOTICE_NTF, Err)
		CASE_MSG_SEND_ERROR(Guild, Chat, CHAT_NTF, Err)
		CASE_MSG_SEND_ERROR(Guild, PlayerInfo, PLAYER_INFO_REQ, Err)
		CASE_MSG_SEND_ERROR(Guild, Join, JOIN_NTF, Err)
		CASE_MSG_SEND_ERROR(Guild, Leave, LEAVE_NTF, Err)
		CASE_MSG_SEND_ERROR(Guild, Kick, KICK_NTF, Err)
		CASE_MSG_SEND_ERROR(Guild, GetChannelAddr, GET_CHANNEL_ADDR_REQ, Err)
		CASE_MSG_SEND_ERROR(Guild, PlayerInfo, PLAYER_LOCATION_REQ, Err)
	default:
		_LOG(TEXT("Failed to send some GUILD message. ID = %d"), id);
	}
}
