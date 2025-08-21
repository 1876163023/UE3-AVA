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
// Client

void PM::CLIENT::CONNECT_REQ::Send(WORD versionProtocol, WORD versionClient, const FString &key, BYTE reconnect, __int64 addr)
{
	//CHECK_ANY_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_CONNECTING && _StateController->GetNetState() != _AN_CHANNELLIST)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg(TMSG::BODY_LENGTH + PM::_GetStringSize((WCHAR*)*key));
	DEF &def = msg.Data();

	def.versionProtocol = versionProtocol;
	def.versionClient = versionClient;
	def.reconnect = reconnect;
	def.addr = addr;

	PM::_MsgString KeyString(msg, msg.Data().key);
	KeyString.SetOffset(0);
	KeyString = *key;

	//SEND_MSG_CHANNEL_P(msg);
	SEND_MSG_CHANNEL(msg);
}

void PM::CLIENT::CHECK_NICK_REQ::Send(const FString &Nickname, BYTE idFace)
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_NEWCHARACTER)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	appStrncpy(def.nickname, *Nickname, SIZE_NICKNAME + 1);
	def.idFace = idFace;

	_SendMsgTo(msg.GetBuf(), GavaNetClient->SessionKeyChannel, ID, DEF::MsgName(), FALSE, GavaNetClient->ConnectionTimeOutSec);
}

void PM::CLIENT::HWINFO_NTF::Send(const FString &cpu, const FString &gpu, DWORD mem, BYTE *adapterAddress)
{
	CHECK_NO_CONNECTION()

	TMSG msg;
	DEF &def = msg.Data();

	appStrncpy(def.cpu_id, *cpu, SIZE_HWID + 1);
	appStrncpy(def.gpu_id, *gpu, SIZE_HWID + 1);
	def.mem_size = mem;
	appMemcpy(def.adapterAddress, adapterAddress, SIZE_ADAPTER_ADDRESS);

	SEND_MSG_CHANNEL(msg);
}

void PM::CLIENT::SET_CONFIG_NTF::Send(const FString &ConfigStr1, const FString &ConfigStr2)
{
	CHECK_NO_CONNECTION()

	if (ConfigStr1.Len() > SIZE_CONFIGSTR1 || ConfigStr2.Len() > SIZE_CONFIGSTR2)
		return;

	TMSG msg;

	_tcscpy(msg.Data().configstr, *ConfigStr1);
	_tcscpy(msg.Data().configstr2, *ConfigStr2);

	SEND_MSG_CHANNEL(msg);
}

void PM::CLIENT::SET_RTTTEST_ADDR_NTF::Send(UDP_HOST_INFO &HostInfo)
{
#ifdef EnableRttTest
	CHECK_NO_CONNECTION()

	TMSG msg;

	msg.Data().addrInfo = HostInfo;

	SEND_MSG_AUTO_F(msg);
#endif
}

void PM::CLIENT::GUILD_CONNECT_REQ::Send(WORD versionProtocol, WORD versionClient, TID_ACCOUNT idAccount, TID_GUILD idGuild, const FString& key, BYTE reconnect, __int64 addr)
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	TMSG msg(TMSG::BODY_LENGTH + PM::_GetStringSize((WCHAR*)*key));
	DEF &def = msg.Data();

	def.versionProtocol = versionProtocol;
	def.versionClient = versionClient;
	def.idAccount = idAccount;
	def.idGuild = idGuild;
	def.reconnect = reconnect;
	def.addr = addr;

	PM::_MsgString KeyString(msg, msg.Data().key);
	KeyString.SetOffset(0);
	KeyString = *key;

	SEND_MSG_GUILD_P(msg);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcMsgSendErrorClient(WORD MsgID, const FString& Err)
{
	BYTE id = (MsgID & 0xff);

	using namespace PM::CLIENT;

	switch (id)
	{
		CASE_MSG_SEND_ERROR(Client, Connect, CONNECT_REQ, Err)
		CASE_MSG_SEND_ERROR(Client, CheckNick, CHECK_NICK_REQ, Err)
		CASE_MSG_SEND_ERROR(Client, HwInfo, HWINFO_NTF, Err)
		CASE_MSG_SEND_ERROR(Client, SetConfig, SET_CONFIG_NTF, Err)
		CASE_MSG_SEND_ERROR(Client, SetRttTestAddr, SET_RTTTEST_ADDR_NTF, Err)
		CASE_MSG_SEND_ERROR(Client, GuildConnect, GUILD_CONNECT_REQ, Err)
	default:
		_LOG(TEXT("Failed to send some CLIENT message. ID = %d"), id);
	}
}
