/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaMsgProcClient.cpp

	Description: Implementation of message processors

***/
#include "avaNet.h"

#define ASSERT check

#include "ComDef/Def.h"

using namespace Def;

#include "ComDef/MsgDef.h"
#include "ComDef/MsgDefClient.h"

#include "avaNetClient.h"
#include "avaMsgProc.h"
#include "avaMsgSend.h"
#include "avaConnection.h"
#include "avaNetStateController.h"
#include "avaCommunicator.h"
#include "avaWebInClient.h"
#include "hwPerformanceCounter.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Client


void PM::CLIENT::CONNECT_ANS::Proc(_LPMSGBUF pData)
{
	//CHECK_AND_DELETE_PENDING_MSG(CONNECT_REQ);
	if (GavaNetClient->ConnectionTimeOutDue > 0.0)
	{
		GavaNetClient->EndConnectionTimeOutCheck();
	}

	TMSG msg(pData);
	DEF &def = msg.Data();

	//CavaConnection *Conn = GavaNetClient->CurrentConnection();
	//check(Conn);

	if (GavaNetClient->CLState != CLS_CreatingSession && GavaNetClient->CLState != CLS_ChangingSession)
	{
		_LOG(TEXT("Error: Invalid connection state = %d"), (INT)GavaNetClient->CLState);
		return;
	}
	if (_StateController->GetNetState() > _AN_CHANNELLIST)
	{
		_LOG(TEXT("Error: Invalid net state = %d"), (INT)_StateController->GetNetState());
		return;
	}

	GavaNetClient->CurrentChannelAddress.address64 = def.addr;

	switch (def.result)
	{
	case RC_OK:
		{
			_LOG(TEXT("CONNECT_REQ succeeded."));

			GavaNetClient->CLState = CLS_Connected;

			if (GavaNetClient->EmergencyCheckTime == 0.0)
				GavaNetClient->EmergencyCheckTime = appSeconds();

			//if (!_StateController->PlayerInfo.IsValid())
			//{
			//	_StateController->PlayerInfo.Set(def.playerInfo);
			//	//_StateController->PlayerInfo.PlayerInfo.idAccount = def.idAccount;
			//	_StateController->PlayerInfo.DumpPlayerInfo();
			//	//PlayerInfo->DumpGuildInfo();
			//	_StateController->PlayerInfo.DumpItemInfo();
			//	_StateController->PlayerInfo.PrivLevel = def.gmLevel;
			//}

			_StateController->PlayerInfo.PrivLevel = def.gmLevel;

			if (_StateController->GetNetState() == _AN_CHANNELLIST)
			{
				// 이미 한 번 접속하고 채널 목록을 보고 있던 상태

				_StateController->GoToState(_AN_LOBBY);

				if ( !_StateController->ProcAutoMove() )
				{
					GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("ok"), _StateController->ChannelInfo.ChannelName, 0, 0);
				}

				if (_StateController->ChannelInfo.IsValid())
				{
					FChannelInfo *Info = _StateController->ChannelList.Find(_StateController->ChannelInfo.idChannel);
					if (Info)
					{
						//_StateController->ChannelInfo.ChannelName = Info->ChannelName;
						//_StateController->ChannelInfo.Count = Info->Count;
						_StateController->ChannelInfo = *Info;
					}
				}

				_WebInClient().ChannelSet(_StateController->ChannelInfo.ChannelNameShort);
			}
			else
			{
				// 처음으로 접속하는 상태

				_StateController->GoToState(_AN_CHANNELLIST);

				//if (_StateController->PlayerInfo.IsAdmin())
				//{
				//	// I am administrator
				//	// check for stealth option
				//	if ( ParseParam(appCmdLine(),TEXT("STEALTH")) )
				//		PM::ADMIN::SET_VISIBILITY_REQ::Send(FALSE);
				//	else
				//		PM::ADMIN::SET_VISIBILITY_REQ::Send(TRUE);
				//}

				//_StateController->SetLastConnectResult(TEXT("loading"));
				_StateController->SetLastConnectResult(TEXT("ok"));

				_WebInClient().ChannelSet(Localize(TEXT("Channel"), TEXT("Text_Loc_ChannelList"), TEXT("AVANET")));

				// Send hardware information to the server, for once
				PM::CLIENT::HWINFO_NTF::Send(GavaNetClient->PerformanceCounter->myHwInfo.cpuId,
											GavaNetClient->PerformanceCounter->myHwInfo.gpuKey,
											(DWORD)GavaNetClient->PerformanceCounter->myHwInfo.memorySize,
											GavaNetClient->PerformanceCounter->myHwInfo.adapterAddress);

				GavaNetClient->RtttInit();
			}
		}
		break;
	case RC_LOGIN_NO_NICK:
		{
			_LOG(TEXT("Connected first time; need to create a character and a nickname"));
			GavaNetClient->CLState = CLS_Connected;
			_StateController->GoToState(_AN_NEWCHARACTER);

			if (!_StateController->PlayerInfo.IsValid())
				_StateController->PlayerInfo.PlayerInfo.idAccount = def.idAccount;

			_StateController->SetLastConnectResult(TEXT("no nick"));
		}
		break;
	case RC_AUTH_ALREADY_EXIST:
		{
			_StateController->StopAutoMove();

			_LOG(TEXT("Already connected"));
			_StateController->SetLastConnectResult(TEXT("already connected"));
		}
		break;
	case RC_AUTH_ADMIN_KICKED:
		{
			_StateController->StopAutoMove();

			_LOG(TEXT("You are kicked from admin"));
			_StateController->SetLastConnectResult(TEXT("admin kicked"));
		}
		break;
	case RC_INVALID_VERSION:
		{
			_StateController->StopAutoMove();

			_LOG(TEXT("Invalid version"));
			_StateController->SetLastConnectResult(TEXT("invalid version"));
		}
		break;
	case RC_CHANNEL_PLAYER_FULL:
		{
			_StateController->StopAutoMove();

			_LOG(TEXT("Channel is full"));
			_StateController->SetLastConnectResult(TEXT("channel full"));
			_StateController->GoToState(_AN_CHANNELLIST);
			GavaNetClient->CLState = CLS_Connected;
		}
		break;
	default:
		{
			_StateController->StopAutoMove();

			_LOG(TEXT("ERROR on CONNECT_REQ (code = %d)"), def.result);
			//GavaNetClient->GetEventHandler()->Error(Conn, 0);
			_StateController->SetLastConnectResult(TEXT("failed"));
		}
	}
}

void PM::CLIENT::CHECK_NICK_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(CHECK_NICK_REQ);

	if (_StateController->GetNetState() != _AN_NEWCHARACTER)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	switch (def.result)
	{
	case RC_OK:
		//_StateController->PlayerInfo.Set(def.playerInfo);
		//_StateController->PlayerInfo.DumpPlayerInfo();
		//PlayerInfo->DumpGuildInfo();
		//_StateController->PlayerInfo.DumpItemInfo();

		_StateController->GoToState(_AN_CHANNELLIST);
		GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_CheckNick, TEXT("ok"), TEXT(""), 0, 0);
		break;

	case RC_NICK_ALREADY_EXIST:
		_LOG(TEXT("Same nickname already exists."));
		GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_CheckNick, TEXT("already exists"), TEXT(""), 0, 0);
		break;

	case RC_INVALID_NICKNAME:
		_LOG(TEXT("Invalid nickname."));
		GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_CheckNick, TEXT("invalid name"), TEXT(""), 0, 0);
		break;

	default:
		_LOG(TEXT("Creating a new character failed."));
		GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_CheckNick, TEXT("failed"), TEXT(""), 0, 0);
		break;
	}
}

void PM::CLIENT::PLAYER_INFO_NTF::Proc(_LPMSGBUF pData)
{
	//if (_StateController->GetNetState() < _AN_CHANNELLIST ||
	//	_StateController->GetNetState() > _AN_LOBBY)
	//	return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	//_StateController->PlayerInfo.Set(def.playerInfo);
	_StateController->PlayerInfo.PlayerInfo.idAccount = def.idAccount;
	appStrcpy(_StateController->PlayerInfo.PlayerInfo.nickname, def.nickname);
	_StateController->PlayerInfo.PlayerInfo.faceType = def.faceType;
	_StateController->PlayerInfo.PlayerInfo.level = def.level;
	_StateController->PlayerInfo.PlayerInfo.currentClass = def.currentClass;
	_StateController->PlayerInfo.PlayerInfo.lastTeam = def.lastTeam;
	_StateController->PlayerInfo.PlayerInfo.straightWin = def.straightWin;
	_StateController->PlayerInfo.PlayerInfo.xpProgress = def.xpProgress;
	_StateController->PlayerInfo.PlayerInfo.xp = def.xp;
	_StateController->PlayerInfo.PlayerInfo.supplyPoint = def.supplyPoint;
	_StateController->PlayerInfo.PlayerInfo.money = def.money;
	appStrcpy(_StateController->PlayerInfo.PlayerInfo.configstr, def.configstr);
	appStrcpy(_StateController->PlayerInfo.PlayerInfo.configstr2, def.configstr2);
	_StateController->PlayerInfo.PlayerInfo.biaXP = def.biaXP;

	appMemcpy(_StateController->PlayerInfo.PlayerInfo.channelMaskList, def.channelMaskList, MAX_CHANNEL_MASK * sizeof(BYTE));

	_StateController->PlayerInfo.SetSkillInfo(def.skillInfo);
	_StateController->PlayerInfo.SetScoreInfo(def.scoreInfo);

	_StateController->PlayerInfo.DumpPlayerInfo();
	//PlayerInfo->DumpGuildInfo();
	//_StateController->PlayerInfo.DumpItemInfo();

	_StateController->PlayerInfo.OldCurrentClass = def.currentClass;

	appMemzero(&GavaNetClient->CurrentClientAddress, sizeof(RxGate::RXNERVE_ADDRESS));
	appMemcpy(GavaNetClient->CurrentClientAddress.address, "AVAC", 4);
	appMemcpy(GavaNetClient->CurrentClientAddress.address + 4, &def.idAccount, 4);

	GavaNetClient->RtttSetActive(def.idAccount);

	// Get buddy list
	_Communicator().GetBuddyList();

	if (_StateController->PlayerInfo.IsLoaded())
		_StateController->SetLastConnectResult(TEXT("ok"));
}

void PM::CLIENT::PLAYER_ITEM_INFO_NTF::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);
	DEF &def = msg.Data();

	PM::_MsgBuffer<ITEM_INFO> WeaponInven(msg, def.weaponInven);
	PM::_MsgBuffer<ITEM_INFO> EquipInven(msg, def.equipInven);
	PM::_MsgBuffer<CUSTOM_ITEM_INFO> CustomInven(msg, def.customWeapon);
	PM::_MsgBuffer<EFFECT_ITEM_INFO> EffectInven(msg, def.effectInven);

	_StateController->PlayerInfo.SetItemInfo((ITEM_INFO*)WeaponInven, WeaponInven.GetCount(),
											(ITEM_INFO*)EquipInven, EquipInven.GetCount(),
											(CUSTOM_ITEM_INFO*)CustomInven, CustomInven.GetCount(),
											(EFFECT_ITEM_INFO*)EffectInven, EffectInven.GetCount(),
											def.weaponSet, def.equipSet, def.effectSet);
	_StateController->PlayerInfo.DumpItemInfo();

	GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_PlayerInfo, TEXT(""), TEXT(""), 0, 0);

	if (_StateController->PlayerInfo.IsLoaded())
		_StateController->SetLastConnectResult(TEXT("ok"));
}

void PM::CLIENT::PLAYER_AWARD_INFO_NTF::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);
	DEF &def = msg.Data();

	_StateController->PlayerInfo.SetAwardInfo(def.awardInfo);

	if (_StateController->PlayerInfo.IsLoaded())
		_StateController->SetLastConnectResult(TEXT("ok"));
}

void PM::CLIENT::PLAYER_GUILD_INFO_NTF::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);
	DEF &def = msg.Data();

	_StateController->PlayerInfo.SetGuildInfo(def.guildInfo);

	if (_StateController->PlayerInfo.IsLoaded())
		_StateController->SetLastConnectResult(TEXT("ok"));

	if (def.guildInfo.idGuild != ID_INVALID_GUILD)
	{
		_StateController->PlayerInfo.DumpGuildInfo();

		_StateController->GuildInfo.Set(def.guildInfo);

		if (def.addr > 0)
		{
			// connect to guild channel
			_LOG(TEXT("Connecting to the guild channel; id = %d"), def.guildInfo.idGuild);

			_StateController->PlayerInfo.SetRxGuildAddress(def.addr);

			GavaNetClient->CurrentGuildAddress.addrType = 0;
			GavaNetClient->CurrentGuildAddress.address64 = def.addr;

			GavaNetClient->CreateRxGateSession(GavaNetClient->CurrentGuildAddress);
		}
		else
		{
			_LOG(TEXT("No guild channel address; This will be received later."));
		}
	}
	else
	{
		_LOG(TEXT("No guild info"));
	}
}

void PM::CLIENT::PLAYER_GUILD_ADDR_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->GuildInfo.IsValid())
	{
		_LOG(TEXT("Error! Invalid guild info."));
		return;
	}

	TMSG msg(pData);
	DEF &def = msg.Data();

	// connect to guild channel
	_LOG(TEXT("Connecting to the guild channel; id = %d"), _StateController->GuildInfo.GuildInfo.idGuild);

	_StateController->PlayerInfo.SetRxGuildAddress(def.addr);

	GavaNetClient->CurrentGuildAddress.addrType = 0;
	GavaNetClient->CurrentGuildAddress.address64 = def.addr;

	GavaNetClient->CreateRxGateSession(GavaNetClient->CurrentGuildAddress);
}

void PM::CLIENT::CREATE_ACCOUNT_ANS::Proc(_LPMSGBUF pData)
{
	// 테스트용 코드

	//CHECK_AND_DELETE_PENDING_MSG(CREATE_ACCOUNT_REQ);

	//if (_StateController->GetNetState() != _AN_CONNECTING)
	//	return;

	//TMSG msg(pData);
	//DEF &def = msg.Data();

	//if (def.result == RC_OK)
	//{
	//	_LOG(TEXT("Connected first time; need to create a character and a nickname"));
	//	GavaNetClient->CurrentConnection()->ConnState = CS_Connected;
	//	//_StateController->GoToState(_AN_NEWCHARACTER);

	//	//if (!_StateController->PlayerInfo.IsValid())
	//	//	_StateController->PlayerInfo.PlayerInfo.idAccount = def.idAccount;

	//	GetAvaNetHandler()->ProcMessage(EMsg_Client, TEXT("CREATE_ACCOUNT"), TEXT("ok"), TEXT(""), 0, 0);
	//}
	//else
	//{
	//	_LOG(TEXT("ERROR creating a new account."));
	//	//GavaNetClient->GetEventHandler()->Error(GavaNetClient->CurrentConnection(), 0);
	//	GetAvaNetHandler()->ProcMessage(EMsg_Client, TEXT("CREATE_ACCOUNT"), TEXT("failed"), TEXT(""), 0, 0);
	//}
}

void PM::CLIENT::MAIN_NOTICE_NTF::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);
	DEF &def = msg.Data();

	_StateController->MainNotice = msg.Data().msg;

	GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_MainNotice, _StateController->MainNotice, TEXT(""), 0, 0);
}

void PM::CLIENT::KICK_NTF::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);

	GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_Kick, TEXT("forced connection"), TEXT(""), 0, 0);
}

void PM::CLIENT::GUILD_CONNECT_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(GUILD_CONNECT_REQ);

	TMSG msg(pData);

	switch (msg.Data().result)
	{
	case RC_OK:
		_LOG(TEXT("Connected to the guild channel."));
		break;
	case RC_INVALID_CLAN:
		_LOG(TEXT("Invalid guild."));
		GavaNetClient->SessionKeyGuild = RXGATE_INVALID_SESSION_KEY;
		break;
	case RC_NOT_CLANMEMBER:
		_LOG(TEXT("Not a guild member."));
		GavaNetClient->SessionKeyGuild = RXGATE_INVALID_SESSION_KEY;
		break;
	default:
		_LOG(TEXT("Failed to connect to the guild channel; result = %d."), msg.Data().result);
		GavaNetClient->SessionKeyGuild = RXGATE_INVALID_SESSION_KEY;
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void PM::CLIENT::Proc(_LPMSGBUF pData)
{
	MSG_HEADER *pHeader = (MSG_HEADER*)pData->GetData();

	switch (pHeader->msg_id)
	{
		CASE_MSG_PROC(CONNECT_ANS)
		CASE_MSG_PROC(CHECK_NICK_ANS)
		CASE_MSG_PROC(PLAYER_INFO_NTF)
		CASE_MSG_PROC(PLAYER_ITEM_INFO_NTF)
		CASE_MSG_PROC(PLAYER_AWARD_INFO_NTF)
		CASE_MSG_PROC(PLAYER_GUILD_INFO_NTF)
		CASE_MSG_PROC(PLAYER_GUILD_ADDR_NTF)
		CASE_MSG_PROC(MAIN_NOTICE_NTF)
		CASE_MSG_PROC(KICK_NTF)
		CASE_MSG_PROC(GUILD_CONNECT_ANS)

	default:
		_LOG(TEXT("Invalid CLIENT message received. ID = %d"), pHeader->msg_id);
	}
}

void PM::CLIENT::ProcTimeOut(const BYTE *Buffer, INT BufferLen)
{
	MSG_HEADER *pHeader = (MSG_HEADER*)Buffer;

	switch (pHeader->msg_id)
	{
		CASE_MSG_TIMEOUT_PROC(Client, Connect, CONNECT_REQ)
		CASE_MSG_TIMEOUT_PROC(Client, CheckNick, CHECK_NICK_REQ)
		CASE_MSG_TIMEOUT_PROC(Client, GuildConnect, GUILD_CONNECT_REQ)
	default:
		_LOG(TEXT("Some CLIENT message timed out. ID = %d"), pHeader->msg_id);
	}
}
