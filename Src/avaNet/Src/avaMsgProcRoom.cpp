/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaMsgProcRoom.cpp

	Description: Implementation of message processors

***/
#include "avaNet.h"

#define ASSERT check

#include "ComDef/Def.h"

using namespace Def;

#include "ComDef/MsgDef.h"
#include "ComDef/MsgDefRoom.h"

#include "avaNetClient.h"
#include "avaMsgProc.h"
#include "avaMsgSend.h"
#include "avaNetStateController.h"
#include "avaCommunicator.h"

// {{ 20070212 dEAthcURe|HM
#ifdef EnableHostMigration
#include "hostMigration.h"
#endif
// }} 20070212 dEAthcURe|HM



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Room

void PM::ROOM::CREATE_ANS::Proc(_LPMSGBUF pData)
{
	_LPMSGBUF pmsgbuf;
	CHECK_PENDING_MSG(CREATE_REQ, pmsgbuf);

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;

	PM::ROOM::CREATE_REQ::TMSG pmsg(pmsgbuf);
	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;

	if (def.result == RC_OK)
	{
		if (!Room.IsValid())// && Room.RoomInfo.state.numCurr == 0)
		{
			BYTE idSlot = (def.idSlot >= 0 && def.idSlot < MAX_ALL_PLAYER_PER_ROOM) ? def.idSlot : 0;

			appStrcpy(Room.RoomInfo.roomName, pmsg.Data().roomName);
			Room.RoomInfo.bPassword = (appStrlen(pmsg.Data().password) > 0);
			appStrcpy(Room.RoomInfo.hostName, _StateController->GetMyNickname());
			Room.RoomInfo.setting = pmsg.Data().setting;
			Room.RoomInfo.idRoom = def.idRoom;
			Room.RoomInfo.state.playing = Def::RIP_WAIT;
			Room.RoomInfo.state.bMatch = (_StateController->ChannelInfo.IsMatchChannel() && _StateController->PlayerInfo.GetCurrentChannelMaskLevel() == _CML_REFREE);

			// {{ 20080213 dEAthcURe|SUL
			#ifdef EnableSecureUdpLayer
			GsulRandomSeed = Room.RoomInfo.idRoom;
			#endif
			// }} 20080213 dEAthcURe|SUL

			Room.PlayerList.PlayerList[idSlot].Set(_StateController->PlayerInfo.PlayerInfo);
			FRoomPlayerInfo *Info = &Room.PlayerList.PlayerList[idSlot];//Room.AddPlayer(_StateController->PlayerInfo.PlayerInfo);
			check(Info);

			//Info->PlayerInfo = _StateController->PlayerInfo.PlayerInfo;
			//Info->RoomPlayerInfo = _StateController->PlayerInfo.PlayerInfo;

			Info->RoomPlayerInfo.idSlot = idSlot;
			//Info->RoomPlayerInfo.idTeam = FRoomInfo::SlotToTeam(0);
			Info->RoomPlayerInfo.pcBang = (/*!_StateController->ChannelInfo.IsMyClanChannel() &&*/ _StateController->PlayerInfo.IsPcBang()) ? 1 : 0;
			//Info->RttValue = 0.0;

			_StateController->PlayerInfo.PlayerInfo.lastTeam = RT_EU;

			Room.HostIdx = idSlot;
			_StateController->MyRoomSlotIdx = idSlot;

			_StateController->GoToState(_AN_ROOM);

			GavaNetClient->SelectP2PServer(def.idRoom);

			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Create, TEXT("ok"), Room.RoomInfo.roomName, 0, 0);

			Room.DumpRoomInfo();

#ifdef EnableRttTest
			if (GIsGame && !GIsEditor && !_StateController->AmIStealthMode())
			{
				//debugf(TEXT("[dEAthcURe] initializing RTT test..."));
				GavaNetClient->p2pNetTester.initRttTest();
				//GavaNetClient->p2pNetTester.activateRttTest(true, true); // 20070517 dEAthcURe|RTT // GavaNetClient->p2pNetTester.setActive();
			}
#endif
		}
		else
		{
			_LOG(TEXT("Room is already created."));

			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Create, TEXT("already created"), TEXT(""), 0, 0);
		}
	}
	else if (def.result == RC_INVALID_ROOMNAME)
	{
		_LOG(TEXT("Invalid room name."));
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Create, TEXT("invalid name"), TEXT(""), 0, 0);
	}
	else
	{
		_LOG(TEXT("Room creation failed."));

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Create, TEXT("failed"), TEXT(""), 0, 0);
	}

	pmsgbuf->Delete();
}

UBOOL _ProcJoinRoom(BYTE Result, TID_ROOM idRoom, ROOM_SETTING &Setting, ROOM_STATE &State, ROOM_PLAYER_INFO *PlayerList, INT PlayerCount, BYTE idHost)
{
	UBOOL bJoined = FALSE;
	FRoomInfo &Room = _StateController->RoomInfo;
	if (Result == RC_OK && PlayerList)
	{
		bJoined = TRUE;

		_LOG(TEXT("Joined the room. idRoom = %d"), idRoom);

		Room.RoomInfo.idRoom = idRoom;
		Room.RoomInfo.setting = Setting;
		Room.RoomInfo.state = State;
		Room.HostIdx = idHost;

		// {{ 20080213 dEAthcURe|SUL
		#ifdef EnableSecureUdpLayer
		GsulRandomSeed = idRoom;
		#endif
		// }} 20080213 dEAthcURe|SUL

		if (_StateController->AutoMoveDest.IsMoving())
		{
			// 자동 이동 중이었음
			appStrncpy(Room.RoomInfo.roomName, *_StateController->AutoMoveDest.RoomName, SIZE_ROOM_NAME + 1);
		}
		else
		{
			FRoomDispInfo *Info = _StateController->RoomList.Find(idRoom);
			if (Info)
				appStrncpy(Room.RoomInfo.roomName, Info->RoomInfo.roomName, SIZE_ROOM_NAME + 1);
		}

		Room.PlayerList.Clear();
		for (INT i = 0; i < PlayerCount; ++i)
		{
			if (PlayerList[i].idAccount == ID_INVALID_ACCOUNT)
				continue;

			FRoomPlayerInfo *Player = Room.PlayerList.Add(PlayerList[i]);
			if (Player)
			{
				Player->DumpPlayerInfo();
				if (Player->PlayerInfo.idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
				{
					// it's me
					_StateController->MyRoomSlotIdx = Player->RoomPlayerInfo.idSlot;
					//Player->Set(_StateController->PlayerInfo.PlayerInfo);

					_StateController->PlayerInfo.PlayerInfo.lastTeam = Player->GetTeamID();
				}

				_StateController->SyncPlayerLevel(PlayerList[i].idAccount, PlayerList[i].level);
			}
		}

		//check(_StateController->MyRoomSlotIdx != ID_INVALID_ROOM_SLOT);

		Room.RefreshPlayerCount();

		Room.DumpRoomInfo();

		_StateController->GoToState(_AN_ROOM);
		_StateController->SetRoomReadyDue();

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Join, TEXT("ok"), Room.RoomInfo.roomName, 0, 0);

		GavaNetClient->SelectP2PServer(idRoom);

		ROOM_PLAYER_INFO *pHost = NULL;
		if ( !Room.PlayerList.IsEmpty(Room.HostIdx) )
		{
			pHost = &Room.PlayerList.PlayerList[Room.HostIdx].RoomPlayerInfo;
			_StateController->LogChatConsole(*FString::Printf(
				*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_HostRating"), TEXT("AVANET")),
				pHost->nickname, pHost->hostRating),
				EChat_ReadyRoom);
		}

#ifdef EnableRttTest
		if (GIsGame && !GIsEditor && !_StateController->AmIStealthMode())
		{
			if ( pHost )
			{
				_LOG(TEXT("Starting RTT test with the host. idAccount = %d"), pHost->idAccount);
				//GavaNetClient->chsImpl.AddPlayer(PlayerInfo[Room.HostIdx].idAccount);
				FLOAT RttValue = GavaNetClient->chsImpl.CheckAndGetRttValue(pHost->idAccount);
				if (RttValue >= 0)
				{
					_LOG(TEXT("This player's RTT value is cached before and valid."));
					GetAvaNetRequest()->SetMyRttValue(RttValue);
				}
				else
				{
					if (pHost->bReady != _READY_LOADING)
					{
						GavaNetClient->p2pNetTester.addToTestList(pHost->idAccount);
						_LOG(TEXT("[dEAthcURe] activating RTT test..."));
						GavaNetClient->p2pNetTester.activateRttTest(true, _StateController->AmIHost()==TRUE?true:false); // 20070517 dEAthcURe|RTT // GavaNetClient->p2pNetTester.setActive();
					}
					else
					{
						// 방장이 게임 로딩 중이라면 테스트 불가능
						_LOG(TEXT("Host is loading, so pending the RTT test"));
					}
				}
			}
		}
#endif
	}
	else
	{
		bJoined = FALSE;

		Room.RoomInfo.idRoom = ID_INVALID_ROOM;
		_LOG(TEXT("Room join failed."));

		switch (Result)
		{
		case RC_ROOM_PLAYER_FULL:
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Join, TEXT("full"), TEXT(""), 0, 0);
			break;
		case RC_QUICKJOIN_ROOM_NOTFOUND:
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Join, TEXT("not found"), TEXT(""), 0, 0);
			break;
		case RC_ROOM_INVALID_PASSWORD:
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Join, TEXT("invalid password"), TEXT(""), 0, 0);
			break;
		case RC_ROOM_INVALID_LEVEL:
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Join, TEXT("invalid level"), TEXT(""), 0, 0);
			break;
		case RC_ROOM_INVALID_CLAN:
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Join, TEXT("invalid clan"), TEXT(""), 0, 0);
			break;
		case RC_ROOM_BAN_PLAYER:
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Join, TEXT("kicked"), TEXT(""), 0, 0);
			break;
		default:
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Join, TEXT("failed"), TEXT(""), 0, 0);
		}
	}

	return bJoined;
}


void PM::ROOM::JOIN_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(JOIN_REQ);

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();
	PM::_MsgBuffer<ROOM_PLAYER_INFO> PlayerList(msg, def.playerList);

	if ( _ProcJoinRoom(def.result, def.idRoom, def.setting, def.state, PlayerList.GetBuffer(), PlayerList.GetCount(), def.idHost) )
	{
		_StateController->ProcAutoMove();
	}
	else
	{
		_StateController->StopAutoMove();
	}
}

void PM::ROOM::JOIN_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	if (!FRoomInfo::IsSlotIDValid(def.playerInfo.idSlot, FALSE))
	{
		_LOG(TEXT("Player slot id is invalid."));
		return;
	}

	_LOG(TEXT("Player joined."));

	_StateController->RoomHostKickCount = -1;

	FRoomPlayerInfo *Player = Room.AddPlayer(def.playerInfo);
	if (Player)
	{
		_StateController->SyncPlayerLevel(def.playerInfo.idAccount, def.playerInfo.level);

		Player->DumpPlayerInfo();

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Join, TEXT("notify"), Player->RoomPlayerInfo.nickname, Player->RoomPlayerInfo.idSlot, 0);

#ifdef EnableRttTest
		if (GIsGame && !GIsEditor)
		{
			// start rtt test
			//if (def.playerInfo.idSlot < MAX_PLAYER_PER_ROOM)
			//{
			//	//GavaNetClient->chsImpl.AddPlayer(def.playerInfo.idSlot, def.playerInfo.idAccount);
			//	GavaNetClient->chsImpl.AddPlayer(def.playerInfo.idAccount);
			//}
		}
#endif
	}
}

void PM::ROOM::INFO_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(INFO_REQ);

	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;

	if (Room.IsValid())
	{
		if (Room.RoomInfo.idRoom == def.idRoom)
		{
			_LOG(TEXT("Getting room info. idRoom = %d"), def.idRoom);

			Room.RoomInfo.setting = def.setting;
			Room.RoomInfo.state = def.state;
			Room.HostIdx = def.idHost;

			Room.DumpRoomInfo();

			PM::_MsgBuffer<ROOM_PLAYER_INFO> PlayerList(msg, def.playerList);
			WORD Count = PlayerList.GetCount();

			Room.PlayerList.Clear();
			for (INT i = 0; i < Count; ++i)
			{
				if (PlayerList[i].idAccount == ID_INVALID_ACCOUNT)
					continue;

				FRoomPlayerInfo *Player = Room.PlayerList.Add(PlayerList[i]);
				if (Player)
				{
					Player->DumpPlayerInfo();
					if (Player->PlayerInfo.idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
					{
						// it's me
						Player->Set(_StateController->PlayerInfo.PlayerInfo);
						_StateController->MyRoomSlotIdx = Player->RoomPlayerInfo.idSlot;
					}
				}
			}

			Room.RefreshPlayerCount();

			//check(_StateController->MyRoomSlotIdx != ID_INVALID_ROOM_SLOT);

			//_StateController->GoToState(_AN_ROOM);

			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Info, TEXT("ok"), Room.RoomInfo.roomName, 0, 0);
		}
		else
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Info, TEXT("invalid"), TEXT(""), 0, 0);
		}
	}
	else
	{
		//Room.RoomInfo.idRoom = ID_INVALID_ROOM;
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Info, TEXT("failed"), TEXT(""), 0, 0);
	}
}

void PM::ROOM::LEAVE_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(LEAVE_REQ);

	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (_StateController->GetNetState() == _AN_ROOM)
	{
		if (def.result == RC_OK)
		{
			// return to lobby
			if (_StateController->IsCountingDown())
				_StateController->CancelCountDown();

			GetAvaNetHandler()->RoomStartingPlayerList.Empty();

			GCallbackEvent->Send(CALLBACK_PlayerListUpdated);

			#ifdef EnableP2pConn
			// {{ 20070120 dEAthcURe	
			delete GavaNetClient->pp2pConn;
			GavaNetClient->pp2pConn = 0x0;
			_ahead_deinitSocket();
			// }} 20070120 dEAthcURe
			#endif

			_StateController->GoToState(_AN_LOBBY);
			if ( !_StateController->ProcAutoMove() )
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Leave, TEXT("ok"), TEXT(""), 0, 0);
			}
		}
		else
		{
			_LOG(TEXT("Failed to leave the room."));
			_StateController->StopAutoMove();
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Leave, TEXT("failed"), TEXT(""), 0, 0);
		}
	}
	else // _AN_INGAME
	{
		if (def.result != RC_OK)
		{
			//GavaNetClient->GetEventHandler()->Error(GavaNetClient->CurrentConnection(), 0);
			//GavaNetClient->CloseConnection();
			//_StateController->GoToState(_AN_DISCONNECTED);
		}

		_LOG(TEXT("Returning to avaEntry..."));
		GetAvaNetHandler()->DisconnectFromGame(_AN_LOBBY);

		#ifdef EnableP2pConn
		// {{ 20070120 dEAthcURe	
		delete GavaNetClient->pp2pConn;
		GavaNetClient->pp2pConn = 0x0;
		_ahead_deinitSocket();
		// }} 20070120 dEAthcURe
		#endif
	}
}

void PM::ROOM::LEAVE_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	if (!FRoomInfo::IsSlotIDValid(def.idSlot, FALSE))
	{
		_LOG(TEXT("Player slot id is invalid."));
		return;
	}

	// remove from RoomStartingPlayerList
	for (INT i = 0; i < GetAvaNetHandler()->RoomStartingPlayerList.Num(); ++i)
	{
		if (GetAvaNetHandler()->RoomStartingPlayerList(i).AccountID == def.idAccount)
		{
			GetAvaNetHandler()->RoomStartingPlayerList.Remove(i);
			GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("cancel"), TEXT(""), i, 0);
			break;
		}
	}
	GCallbackEvent->Send(CALLBACK_PlayerListUpdated);

	FRoomPlayerInfo *Player = NULL;
	if (Room.PlayerList.IsEmpty(def.idSlot))
	{
		_LOG(TEXT("Warning!! Leaving player slot is empty!! Finding the player by idAccount. idSlot = %d"), def.idSlot);
		Player = Room.PlayerList.Find(def.idAccount);
	}
	else
	{
		Player = &Room.PlayerList.PlayerList[def.idSlot];
	}

	if (Player)
	{
		if (Player->RoomPlayerInfo.idSlot != def.idSlot || Player->RoomPlayerInfo.idAccount != def.idAccount)
		{
			_LOG(TEXT("Warning!! Received slot id or account id is wrong! idSlot = %d <-> %d, idAccount = %d <-> %d"),
						Player->RoomPlayerInfo.idSlot, def.idSlot, Player->RoomPlayerInfo.idAccount, def.idAccount);
		}

		if (Player->RoomPlayerInfo.idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
		{
			_LOG(TEXT("Warning!! You are leaving!! This message is not possible!!"));

			if (_StateController->GetNetState() == _AN_ROOM)
			{
				// return to lobby
				if (_StateController->IsCountingDown())
					_StateController->CancelCountDown();

				GetAvaNetHandler()->RoomStartingPlayerList.Empty();
				GCallbackEvent->Send(CALLBACK_PlayerListUpdated);

				_StateController->GoToState(_AN_LOBBY);
				GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Leave, TEXT("ok"), TEXT(""), 0, 0);

				#ifdef EnableP2pConn
				// {{ 20070120 dEAthcURe	
				delete GavaNetClient->pp2pConn;
				GavaNetClient->pp2pConn = 0x0;
				_ahead_deinitSocket();
				// }} 20070120 dEAthcURe
				#endif
			}
			else	// _AN_INGAME
			{
				#ifdef EnableP2pConn
				// {{ 20070120 dEAthcURe	
				delete GavaNetClient->pp2pConn;
				GavaNetClient->pp2pConn = 0x0;
				_ahead_deinitSocket();
				// }} 20070120 dEAthcURe
				#endif

				GetAvaNetHandler()->RoomStartingPlayerList.Empty();
				GCallbackEvent->Send(CALLBACK_PlayerListUpdated);

				_LOG(TEXT("Returning to avaEntry..."));
				GetAvaNetHandler()->DisconnectFromGame(_AN_LOBBY);
			}
			return;
		}

		_LOG(TEXT("[%d]%s left."), Player->RoomPlayerInfo.idSlot, Player->RoomPlayerInfo.nickname);

		if (Player->RoomPlayerInfo.idSlot == Room.HostIdx && _StateController->IsCountingDown())
		{
			_StateController->CancelCountDown();
			Room.RoomInfo.state.playing = RIP_WAIT;
			//FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
			//if (MyInfo)
			//	MyInfo->RoomPlayerInfo.bReady = _READY_NONE;
		}

		_StateController->RoomHostKickCount = -1;

		Room.RemoveSlot(Player->RoomPlayerInfo.idSlot);

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Leave, TEXT("notify"), Player->RoomPlayerInfo.nickname, Player->RoomPlayerInfo.idSlot, 0);
	}
	else
	{
		_LOG(TEXT("Invalid room leave notification received. idSlot = %d, idAccount = %d"), def.idSlot, def.idAccount);
	}

	if (_StateController->ChannelInfo.IsFriendlyGuildChannel())
	{
		// 친선 클랜전
		Room.RefreshClanInfo();
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ClanInfo, TEXT(""), TEXT(""), 0, 0);
	}
}

void PM::ROOM::CHAT_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	FRoomPlayerInfo *Player = Room.PlayerList.Find(def.idAccount);
	if (Player)
	{
		PM::_MsgString ChatMsg(msg, def.chatmsg);
		FString ParsedMsg = (LPCWSTR)ChatMsg;
		UBOOL bCoded = _Communicator().IsMsgCoded(*ParsedMsg);
		INT MsgType = EChat_Normal;
		if (bCoded)
		{
			MsgType = _Communicator().ParseCodedMsgType(*ParsedMsg, MsgType);
			ParsedMsg = _Communicator().ParseCodedMsg(*ParsedMsg);
		}

		_WordCensor().ReplaceChatMsg((TCHAR*)*ParsedMsg);

		FString ChatStr = FString::Printf(TEXT("%s : %s"), Player->RoomPlayerInfo.nickname, *ParsedMsg);

		_StateController->LogChatConsole(*ChatStr, MsgType);
	}
	else
	{
		_LOG(TEXT("Invalid room chat notification received."));
	}
}


void PM::ROOM::PLAYER_INFO_ANS::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (def.result == RC_OK)
	{
		FPlayerDispInfo *Player = _StateController->FindPlayerFromList(def.playerInfo.idAccount);
		if (Player)
		{
			Player->Set(def.playerInfo);
			Player->DumpPlayerInfo();

			_Communicator().SyncPlayerInfo(*Player);
			if (_StateController->GuildInfo.IsValid())
				_StateController->GuildInfo.SyncPlayerInfo(*Player);
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_PlayerInfo, TEXT("ok"), TEXT(""), 0, 0);
		return;
	}

	GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_PlayerInfo, TEXT("failed"), TEXT(""), 0, 0);
}

#ifdef EnableHostMigration
extern bool _hm_isGameFinished(void);
#endif

// 방장 변경
void PM::ROOM::CHANGE_HOST_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	if (def.idHost < MAX_ALL_PLAYER_PER_ROOM && Room.HostIdx != def.idHost && !Room.PlayerList.IsEmpty(def.idHost))
	{
		// 이전 방장의 상태 리셋
		Room.PlayerList.PlayerList[Room.HostIdx].RoomPlayerInfo.bReady = _READY_NONE;

		// 새 방장 설정
		Room.HostIdx = def.idHost;
		_LOG(TEXT("[%d]%s is new host."), def.idHost, Room.PlayerList.PlayerList[def.idHost].RoomPlayerInfo.nickname);

#ifdef EnableHostMigration
		if(!_hm_isGameFinished()) {
#endif
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeHost, Room.PlayerList.PlayerList[def.idHost].RoomPlayerInfo.nickname, TEXT(""), def.idHost, 0);
#ifdef EnableHostMigration
		}
#endif

		if (_StateController->IsCountingDown())
		{
			_StateController->CancelCountDown();
			if (!Room.PlayerList.PlayerList[Room.HostIdx].IsPlaying())
				Room.RoomInfo.state.playing = RIP_WAIT;
			//FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
			//if (MyInfo)
			//	MyInfo->RoomPlayerInfo.bReady = _READY_NONE;
		}

		//Room.PlayerList.PlayerList[def.idHost].RoomPlayerInfo.bReady = _READY_NONE;

		_StateController->SetRoomReadyDue();

		if ( _StateController->AmIHost() )
		{
			_StateController->LogChatConsole(*FString::Printf(
				*Localize(TEXT("UIRoomScene"), TEXT("Msg_YouAreHost"), TEXT("AVANET")), 
				Room.PlayerList.PlayerList[def.idHost].RoomPlayerInfo.hostRating),
				EChat_ReadyRoom);
		}
		else
		{
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("UIRoomScene"), TEXT("Msg_HostChanged"), TEXT("AVANET")),
												Room.PlayerList.PlayerList[def.idHost].RoomPlayerInfo.nickname),
											EChat_ReadyRoom);
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_HostRating"), TEXT("AVANET")),
												Room.PlayerList.PlayerList[def.idHost].RoomPlayerInfo.nickname,
												Room.PlayerList.PlayerList[def.idHost].RoomPlayerInfo.hostRating),
											EChat_ReadyRoom);
		}

#ifdef EnableRttTest
		if ( !Room.PlayerList.IsEmpty(Room.HostIdx) )
		{
			// RTT 값 클리어
			FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
			if (MyInfo)
			{
				MyInfo->RttValue = -1;
				MyInfo->RoomPlayerInfo.rttScore = -1;
			}

			// 게임을 플레이 중인 사람은 값을 클리어하지 않는다.
			for (INT i = 0; i < MAX_PLAYER_PER_ROOM; ++i)
			{
				if (!Room.PlayerList.PlayerList[i].IsPlaying())
				{
					Room.PlayerList.PlayerList[i].RttValue = -1;
					Room.PlayerList.PlayerList[i].RoomPlayerInfo.rttScore = -1;
				}
			}

			FRoomPlayerInfo &Host = Room.PlayerList.PlayerList[Room.HostIdx];
			//Host.RttValue = -1;
			//Host.RoomPlayerInfo.rttScore = -1;

			if(!_StateController->IsStatePlaying() && GIsGame && !GIsEditor && !_StateController->AmIStealthMode())
			{
				// RTT 테스트 새로 해야 함
				if ( _StateController->AmIHost() )
				{
					GavaNetClient->p2pNetTester.initRttTest();
				}
				else
				{
					_LOG(TEXT("Starting RTT test with the host. idAccount = %d"), Host.RoomPlayerInfo.idAccount);
					//GavaNetClient->chsImpl.AddPlayer(Host.RoomPlayerInfo.idAccount);

					FLOAT RttValue = GavaNetClient->chsImpl.CheckAndGetRttValue(Host.RoomPlayerInfo.idAccount);
					if (RttValue >= 0)
					{
						_LOG(TEXT("This player's RTT value is cached before and valid."));
						GetAvaNetRequest()->SetMyRttValue(RttValue);
					}
					else if (!Host.IsPlaying())
					{
						GavaNetClient->p2pNetTester.addToTestList(Host.RoomPlayerInfo.idAccount);
						_LOG(TEXT("[dEAthcURe] activating RTT test..."));
						GavaNetClient->p2pNetTester.activateRttTest(true, _StateController->AmIHost()==TRUE?true:false); // 20070517 dEAthcURe|RTT // GavaNetClient->p2pNetTester.setActive();
					}
				}
			}
		}
#endif

		_StateController->CheckRoomHostKickCondition();

		// {{ 20070124 dEAthcURe
#ifdef EnableP2pConn
		if (_StateController->IsStatePlaying())
		{
#ifdef EnableHostMigration			
			if(!_hm_isGameFinished()) {
#else
			{
#endif
				if(Room.PlayerList.PlayerList[def.idHost].RoomPlayerInfo.idAccount == GavaNetClient->StateController->PlayerInfo.PlayerInfo.idAccount) {
					// 내가 새호스트다
					_ahead_deinitSocket();
					if(_ahead_initSocket() && GavaNetClient) {
						if(GavaNetClient->pp2pConn) delete GavaNetClient->pp2pConn;
						GavaNetClient->pp2pConn = new p2pConn_t(GavaNetClient); // 20080213 dEAthcURe +GavaNetClient
						if(GavaNetClient->pp2pConn) {
							SOCKADDR myAddrLocal, myAddrPublic;
							memset(&myAddrLocal, 0, sizeof(SOCKADDR));
							memset(&myAddrPublic, 0, sizeof(SOCKADDR));

							GavaNetClient->pp2pConn->setId(GavaNetClient->StateController->PlayerInfo.PlayerInfo.idAccount);	

							if(GavaNetClient->pp2pConn->initUdp(_ahead_socket->Socket, _ahead_port)) {
								if(GavaNetClient->pp2pConn->waitForConnect()) {
									if(GavaNetClient->pp2pConn->getMyAddress(30000, &myAddrLocal, &myAddrPublic)) { // 동NAT 테스트를 위해 필요하다
										TCHAR str[256];
										SOCKADDR_IN* pAddr = (SOCKADDR_IN*)&GavaNetClient->pp2pConn->mySockAddr;
										mbstowcs(str, inet_ntoa(pAddr->sin_addr), 255);
										debugf(TEXT("          <---------- [PM::GAME::START_NTF::Send] my addr %s %d"), str, ntohs(pAddr->sin_port));										
									}									
									else {
										debugf(TEXT("          <----------------Resolving my UDP addr failed."));
									}
								}
								else {
									debugf(TEXT("          <---------------- waitForConnect failed."));
								}								
							}
							
							IPADDR_INFO ipIntAddr;
							ipIntAddr.ipAddress = ((SOCKADDR_IN*)&myAddrLocal)->sin_addr.S_un.S_addr;
							ipIntAddr.port = ((SOCKADDR_IN*)&myAddrLocal)->sin_port;

							IPADDR_INFO ipExtAddr;
							ipExtAddr.ipAddress = ((SOCKADDR_IN*)&myAddrPublic)->sin_addr.S_un.S_addr;
							ipExtAddr.port = ((SOCKADDR_IN*)&myAddrPublic)->sin_port;

							PM::ROOM::SET_HOSTADDR_NTF::Send(ipIntAddr, ipExtAddr);							
						}
					}	
					// {{ 20070212 dEAthcURe|HM perform HM/host traverse
#ifdef EnableHostMigration
					#ifdef EnableHmFastLoading
					GIsHostMigrating = true;
					#endif
					debugf(TEXT("[dEAthcURe] HM/Host traverse NetState%d"), _StateController->GetNetState());
					if (_StateController->IsStatePlaying()) {
						_StateController->GetOpenURL(g_hostMigration.furl);
						debugf(TEXT("[dEAthcURe] HM/Host traverse %s"), *g_hostMigration.furl.String());
						g_hostMigration.gotoState(hmsNewHost);
					}
#endif
					// }} 20070212 dEAthcURe|HM perform HM/host traverse
				}
				else {					
					#ifdef EnableHmFastLoading
					GIsHostMigrating = true;
					#endif
					g_hostMigration.gotoState(hmsNewClientPrepare); // [+] 20070502 dEAthcURe|HM
					return; // [!] 20070226 ready_ntf받기전까지 아무짓도 안한다
					// {{ 20070212 dEAthcURe|HM perform HM/client traverse
#ifdef EnableHostMigration
					debugf(TEXT("[dEAthcURe] HM/Client traverse NetState%d"), _StateController->GetNetState());
					if (_StateController->IsStatePlaying()) {
						// hole punching to new host

						// test
						TCHAR ipAddress[256] = TEXT("10.20.16.48");
						GavaNetClient->Settings.Set(CFG_HOSTADDRESS, ipAddress);
						// test
						_StateController->GetOpenURL(g_hostMigration.furl);				
						debugf(TEXT("[dEAthcURe] HM/Client traverse %s"), *g_hostMigration.furl.String());
						g_hostMigration.gotoState(hmsNewClient);
					}
					else {
						debugf(TEXT("[dEAthcURe] HM/Unexpected client state %s"), _StateController->GetNetState());
					}
#endif
					// }} 20070212 dEAthcURe|HM perform HM/client traverse
				}
			}
#ifdef EnableHostMigration
			else {
				//_StateController->GoToState(_AN_ROOM);
				//if (_StateController->RoomInfo.IsValid())
				//	_StateController->RoomInfo.RoomInfo.state.playing = RIP_PLAYING;
				////GEngine->SetClientTravel(TEXT("avaEntry.ut3?closed"), TRAVEL_Absolute);
				//GEngine->Exec(TEXT("DISCONNECT"));
				GetAvaNetHandler()->eventProcHMEndGame(_StateController->AmIHost());
			}
#endif
		}
#endif
		// }} 20070124 dEAthcURe
	}
}

void PM::ROOM::KICK_PLAYER_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	if (!FRoomInfo::IsSlotIDValid(def.idSlot, FALSE))
	{
		_LOG(TEXT("Player slot id is invalid."));
		return;
	}

	// remove from RoomStartingPlayerList
	for (INT i = 0; i < GetAvaNetHandler()->RoomStartingPlayerList.Num(); ++i)
	{
		if (GetAvaNetHandler()->RoomStartingPlayerList(i).AccountID == def.idAccount)
		{
			GetAvaNetHandler()->RoomStartingPlayerList.Remove(i);
			GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("cancel"), TEXT(""), i, 0);
			break;
		}
	}
	GCallbackEvent->Send(CALLBACK_PlayerListUpdated);

	FRoomPlayerInfo *Player = NULL;
	if (Room.PlayerList.IsEmpty(def.idSlot))
	{
		_LOG(TEXT("Warning!! Kicked player slot is empty!! Find the player by idAccount."));
		Player = Room.PlayerList.Find(def.idAccount);
	}
	else
	{
		Player = &Room.PlayerList.PlayerList[def.idSlot];
	}

	if (Player)
	{
		if (Player->RoomPlayerInfo.idSlot != def.idSlot || Player->RoomPlayerInfo.idAccount != def.idAccount)
		{
			_LOG(TEXT("Warning!! Received slot id or account id is wrong!"));
		}

		if (Player->RoomPlayerInfo.idSlot == Room.HostIdx && def.reason != KR_LAG &&
			def.reason != KR_INVALID_LEVEL && def.reason != KR_INVALID_CLAN && def.reason != KR_INVALID_SD && def.reason != KR_PCBANG_ONLY)
		{
			_LOG(TEXT("Host cannot be kicked."));
			return;
		}

		FString Reason;
		switch (def.reason)
		{
		case KR_BAN:
			// 방장의 강퇴/게임 내 강퇴 투표
			Reason = TEXT("banned");
			break;
		case KR_LOADING_TIMEOUT:
			// 클라이언트의 로딩이 느림
			Reason = TEXT("loading time out");
			break;
		case KR_SETTING_CHANGED:
			// 관전 슬롯이 막히면서 관전자들 강퇴
			Reason = TEXT("setting changed");
			break;
		case KR_ROOM_DESTROYED:
			// 방에 플레이어들이 모두 나가면서 관전자와 스텔스 상태의 GM들 강퇴
			Reason = TEXT("room destroyed");
			break;
		case KR_LAG:
			// 방장이 랙에 의해 자동 강퇴 당함
			Reason = TEXT("lag");
			break;
		case KR_INVALID_LEVEL:
			// 초보 채널에서 플레이어가 하사로 진급 했을 때
			Reason = TEXT("invalid level");
			break;
		case KR_INVALID_CLAN:
			// 클랜전 채널에 있는데 클랜에서 추방당했을 때
			Reason = TEXT("invalid clan");
			break;
		case KR_INVALID_SD:
			// S/D 채널에 있는데 S/D가 기준치를 넘었을 때
			Reason = TEXT("invalid sd");
			break;
		case KR_PCBANG_ONLY:
			// 유료 피시방 시간이 만료 되었을 때
			Reason = TEXT("pcbang only");
			break;
		}

		if (Player->RoomPlayerInfo.idSlot == _StateController->MyRoomSlotIdx/* && Player->RoomPlayerInfo.idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount*/)
		{
			if (Reason.Len() == 0)
				Reason = TEXT("banned");

			_LOG(TEXT("You are kicked. reason = %s"), *Reason);

			_StateController->StopAutoMove();

			if (def.reason == KR_INVALID_LEVEL || def.reason == KR_INVALID_SD)
			{
				// 채널을 나가야 함
				_StateController->PlayerInfo.RoomKickedReason = def.reason;
				if (_StateController->IsStatePlaying())
				{
					GetAvaNetHandler()->DisconnectFromGame(_AN_ROOM);
					GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeState, TEXT(""), TEXT(""), 0, 0);
				}
			}
			else if (def.reason == KR_INVALID_CLAN || def.reason == KR_PCBANG_ONLY)
			{
				// 채널을 나가야 함
				if (_StateController->IsStatePlaying())
				{
					_StateController->PlayerInfo.RoomKickedReason = def.reason;
					GetAvaNetHandler()->DisconnectFromGame(_AN_ROOM);
					GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeState, TEXT(""), TEXT(""), 0, 0);
				}
				else
				{
					_StateController->GoToState(_AN_CHANNELLIST);
				//	GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Kick, TEXT("you"), *Reason, 0, 0);
				}
			}
			else
			{
				if (_StateController->IsStatePlaying())
				{
					// cancel pending level if it exists
					//((UGameEngine*)GEngine)->CancelPending();

					GetAvaNetHandler()->DisconnectFromGame(_AN_LOBBY);
				}

				#ifdef EnableP2pConn
				// {{ 20070120 dEAthcURe	
				delete GavaNetClient->pp2pConn;
				GavaNetClient->pp2pConn = 0x0;
				_ahead_deinitSocket();
				// }} 20070120 dEAthcURe
				#endif

				// return to lobby
				_StateController->GoToState(_AN_LOBBY);
				GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Kick, TEXT("you"), *Reason, 0, 0);
			}
		}
		else
		{
			_LOG(TEXT("[%d]%s kicked."), Player->RoomPlayerInfo.idSlot, Player->RoomPlayerInfo.nickname);
			Room.RemoveSlot(Player->RoomPlayerInfo.idSlot);

#ifdef _APPLY_VOTE_FEE
			//if (def.reason == KR_BAN && def.idKicker == _StateController->PlayerInfo.PlayerInfo.idAccount)
			//{
			//	// 내가 방장 권한이나 투표 발의를 통해 플레이어를 강퇴시킨거라면 50유로 삭감
			//	if (_StateController->GetNetState() == _AN_INGAME && !_StateController->AmIAdmin())
			//	{
			//		// 게임 중 강퇴일 때에만 비용 지불(관리자 제외)
			//		_StateController->PlayerInfo.PlayerInfo.money -= VOTE_FEE;
			//		if (_StateController->PlayerInfo.PlayerInfo.money < 0)
			//			_StateController->PlayerInfo.PlayerInfo.money = 0;
			//	}
			//}
#endif

			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Kick, TEXT("notify"), *Reason, Player->RoomPlayerInfo.idSlot, 0);

			if (_StateController->ChannelInfo.IsFriendlyGuildChannel())
			{
				// 친선 클랜전
				Room.RefreshClanInfo();
				GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ClanInfo, TEXT(""), TEXT(""), 0, 0);
			}
		}

		_StateController->RoomHostKickCount = -1;
	}
	else
	{
		_LOG(TEXT("Invalid room kick notification received."));
	}
}

void PM::ROOM::CHANGE_SETTING_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(CHANGE_SETTING_REQ);

	if (_StateController->GetNetState() != _AN_ROOM)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (def.result == RC_OK)
	{
		_LOG(TEXT("Room setting changed."));

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSetting, TEXT("ok"), TEXT(""), 0, 0);
	}
	else
	{
		_LOG(TEXT("Failed to change room setting."));

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSetting, TEXT("failed"), TEXT(""), 0, 0);
	}
}

void PM::ROOM::CHANGE_SETTING_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	Room.RoomInfo.setting = def.setting;

	_StateController->CheckRoomHostKickCondition();

	GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSetting, TEXT("notify"), TEXT(""), 0, 0);
}

// 방의 상태 바뀜
void PM::ROOM::CHANGE_STATE_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	ROOM_STATE OldState = Room.RoomInfo.state;
	ROOM_STATE NewState = def.state;

	Room.RoomInfo.state = NewState;
	_LOG(TEXT("playing = %d, numCurr = %d, currRound = %d, bMatch = %d"),
			NewState.playing, NewState.numCurr, NewState.currRound, NewState.bMatch);

	if (OldState.playing == RIP_PLAYING && NewState.playing == RIP_WAIT)
	{
		// 게임이 끝나고 대기 상태로

		GetAvaNetHandler()->RoomStartingPlayerList.Empty();

		GCallbackEvent->Send(CALLBACK_PlayerListUpdated);

		if (_StateController->GetNetState() == _AN_ROOM)
		{
			// 나는 대기실에 있는 상태
			if (_StateController->IsCountingDown())
			{
				_StateController->CancelCountDown();

				//FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
				//if (MyInfo)
				//	MyInfo->RoomPlayerInfo.bReady = _READY_NONE;

			}
		}
		else //if (_StateController->GetNetState() == _AN_INGAME)
		{
			// 게임 종료

			//PM::ROOM::INFO_REQ::Send();

			GetAvaNetHandler()->DisconnectFromGame(_AN_ROOM);
			_StateController->SetRoomReadyDue();

#ifdef EnableRttTest
			if ( !_StateController->AmIHost() && !Room.PlayerList.IsEmpty(Room.HostIdx) )
			{
				// 게임이 끝났을 때, 방장과의 RTT 테스트가 이루어지지 않았다면 다시 시작
				FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
				if (MyInfo && MyInfo->RttValue < 0)
				{
					FRoomPlayerInfo &Host = Room.PlayerList.PlayerList[Room.HostIdx];
					_LOG(TEXT("Starting RTT test with the host. idAccount = %d"), Host.RoomPlayerInfo.idAccount);

					//GavaNetClient->chsImpl.AddPlayer(Room.PlayerList.PlayerList[Room.HostIdx].RoomPlayerInfo.idAccount);
					FLOAT RttValue = GavaNetClient->chsImpl.CheckAndGetRttValue(Host.RoomPlayerInfo.idAccount);
					if (RttValue >= 0)
					{
						_LOG(TEXT("This player's RTT value is cached before and valid."));
						GetAvaNetRequest()->SetMyRttValue(RttValue);
					}
					else
					{
						GavaNetClient->p2pNetTester.addToTestList(Host.RoomPlayerInfo.idAccount);
						_LOG(TEXT("[dEAthcURe] activating RTT test..."));
						GavaNetClient->p2pNetTester.activateRttTest(true, _StateController->AmIHost()==TRUE?true:false, 5000); // 20070517 dEAthcURe|RTT // GavaNetClient->p2pNetTester.setActive();
					}
				}
			}
#endif
		}

		_StateController->RefreshCurrentRoom();
	}

	_StateController->CheckRoomHostKickCondition();

	GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeState, TEXT(""), TEXT(""), 0, 0);
}

void PM::ROOM::READY_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	if (FRoomInfo::IsSlotIDValid(def.idSlot) && !Room.PlayerList.IsEmpty(def.idSlot) && def.bReady <= _READY_PLAYING)
	{
		Def::ROOM_PLAYER_INFO &Info = Room.PlayerList.PlayerList[def.idSlot].RoomPlayerInfo;

		Info.bReady = def.bReady;
		_LOG(TEXT("[%d]%s is %s"), Info.idSlot, Info.nickname,
			Info.bReady == _READY_NONE ? TEXT("not ready.") : Info.bReady == _READY_WAIT ? TEXT("ready.") : Info.bReady == _READY_LOADING ? TEXT("loading.") : TEXT("playing."));

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Ready, TEXT(""), TEXT(""), def.idSlot, def.bReady);

		_StateController->CheckRoomHostKickCondition();
	}
	else
	{
		_LOG(TEXT("Invalid ready notification received. idSlot = %d"), def.idSlot);
	}
}

void PM::ROOM::CHANGE_SLOT_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	BREAK_SECTION_BEGIN()
	{
		if (def.idSlot == def.newSlot || def.idSlot >= Def::MAX_ALL_PLAYER_PER_ROOM || def.newSlot >= Def::MAX_ALL_PLAYER_PER_ROOM || Room.PlayerList.IsEmpty(def.idSlot))
			break;
		//if (def.idTeam >= RT_MAX)
		//	break;

		// bSwap이 참이면, 두 플레이어가 슬롯을 맞바꿈
		UBOOL bSwap = (!Room.PlayerList.IsEmpty(def.newSlot));

		FRoomPlayerInfo &OldSlot = Room.PlayerList.PlayerList[def.idSlot];
		FRoomPlayerInfo &NewSlot = Room.PlayerList.PlayerList[def.newSlot];
		//if (NewSlot.RoomPlayerInfo.idTeam != def.idTeam)
		//	break;

		if (bSwap)
		{
			// 슬롯 정보 맞바꿈
			FRoomPlayerInfo TempSlot;
			TempSlot = NewSlot;
			NewSlot = OldSlot;
			OldSlot = TempSlot;

			NewSlot.RoomPlayerInfo.idSlot = def.newSlot;
			OldSlot.RoomPlayerInfo.idSlot = def.idSlot;

			//NewSlot.RoomPlayerInfo.idTeam = FRoomInfo::SlotToTeam(def.newSlot);
			//OldSlot.RoomPlayerInfo.idTeam = FRoomInfo::SlotToTeam(def.idSlot);

			if (Room.HostIdx == def.idSlot)
				Room.HostIdx = def.newSlot;
			else if (Room.HostIdx == def.newSlot)
				Room.HostIdx = def.idSlot;

			if (_StateController->MyRoomSlotIdx == def.idSlot)
				_StateController->MyRoomSlotIdx = def.newSlot;
			else if (_StateController->MyRoomSlotIdx == def.newSlot)
				_StateController->MyRoomSlotIdx = def.idSlot;

			if (_StateController->RoomInfo.PlayerList.ListIndex == def.idSlot)
				_StateController->RoomInfo.PlayerList.ListIndex = def.newSlot;
			else if (_StateController->RoomInfo.PlayerList.ListIndex == def.newSlot)
				_StateController->RoomInfo.PlayerList.ListIndex = def.idSlot;
		}
		else
		{
			// 새 슬롯으로 옮기고 이전 슬롯 비움
			NewSlot = OldSlot;
			NewSlot.RoomPlayerInfo.idSlot = def.newSlot;
			//NewSlot.RoomPlayerInfo.idTeam = FRoomInfo::SlotToTeam(def.newSlot);

			Room.PlayerList.Empty(def.idSlot);

			if (Room.HostIdx == def.idSlot)
				Room.HostIdx = def.newSlot;
			if (_StateController->MyRoomSlotIdx == def.idSlot)
				_StateController->MyRoomSlotIdx = def.newSlot;
			if (_StateController->RoomInfo.PlayerList.ListIndex == def.idSlot)
				_StateController->RoomInfo.PlayerList.ListIndex = def.newSlot;
		}

		_LOG(TEXT("[%d]%s changed slot to %d(%s)"), def.idSlot, NewSlot.RoomPlayerInfo.nickname, def.newSlot,
							NewSlot.GetTeamID() == RT_EU ? TEXT("EU") : NewSlot.GetTeamID() == RT_NRF ? TEXT("NRF") : TEXT("Spectator"));
		if (bSwap)
		{
			_LOG(TEXT("[%d]%s was swapped to %d(%s)"), def.newSlot, OldSlot.RoomPlayerInfo.nickname, def.idSlot,
							OldSlot.GetTeamID() == RT_EU ? TEXT("EU") : OldSlot.GetTeamID() == RT_NRF ? TEXT("NRF") : TEXT("Spectator"));
		}

		Room.RefreshPlayerCount();

		if (def.newSlot == _StateController->MyRoomSlotIdx || (bSwap && def.idSlot == _StateController->MyRoomSlotIdx))
		{
			// it's me
			_StateController->PlayerInfo.PlayerInfo.lastTeam = FRoomInfo::SlotToTeam(_StateController->MyRoomSlotIdx);
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSlot, TEXT("ok"), TEXT(""), def.idSlot, def.newSlot);

		if (_StateController->ChannelInfo.IsFriendlyGuildChannel())
		{
			// 친선 클랜전
			Room.RefreshClanInfo();
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ClanInfo, TEXT(""), TEXT(""), 0, 0);
		}

		return;
	}
	BREAK_SECTION_END()

	_LOG(TEXT("Invalid team notification received. idSlot = %d, newSlot = %d, newTeam = %d"), def.idSlot, def.newSlot, def.idTeam);
}

void PM::ROOM::CHANGE_CLASS_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	if (FRoomInfo::IsSlotIDValid(def.idSlot) && !Room.PlayerList.IsEmpty(def.idSlot) && def.idClass < _CLASS_MAX)
	{
		Def::ROOM_PLAYER_INFO &Player = Room.PlayerList.PlayerList[def.idSlot].RoomPlayerInfo;

		Player.currentClass = def.idClass;
		//if (Room.PlayerList.PlayerList[def.idSlot].IsFullInfo())
		//{
		//	Room.PlayerList.PlayerList[def.idSlot].PlayerInfo.currentClass = def.idClass;
		//}

		_LOG(TEXT("Player %d changed class to %s"),
									def.idSlot,
									def.idClass == Def::_CLASS_POINTMAN ? TEXT("Pointman") : def.idClass == Def::_CLASS_RIFLEMAN ? TEXT("Rifleman") : TEXT("Sniper"));

		if (def.idSlot == _StateController->MyRoomSlotIdx)
		{
			// it's me
			check(Player.idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount);
			_StateController->PlayerInfo.PlayerInfo.currentClass = def.idClass;
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeClass, TEXT(""), TEXT(""), def.idSlot, 0);
	}
	else
	{
		_LOG(TEXT("Invalid class notification received. idSlot = %d"), def.idSlot);
	}
}

void PM::ROOM::CHANGE_WEAPON_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	if (FRoomInfo::IsSlotIDValid(def.idSlot) && def.equipSlot >= 0 && def.equipSlot < MAX_WEAPONSET_SIZE && !Room.PlayerList.IsEmpty(def.idSlot))
	{
		Def::ROOM_PLAYER_INFO &Player = Room.PlayerList.PlayerList[def.idSlot].RoomPlayerInfo;

		SLOT_DESC *pSlot = _ItemDesc().GetWeaponSlot(def.equipSlot);
		if (!pSlot)
		{
			_LOG(TEXT("Error changing weapon; invalid slot"));
			return;
		}

		//if ((Player.currentClass == _CLASS_POINTMAN && pSlot->slotType & _EP_P1) ||
		//	(Player.currentClass == _CLASS_RIFLEMAN && pSlot->slotType & _EP_R1) ||
		//	(Player.currentClass == _CLASS_SNIPER && pSlot->slotType & _EP_S1))
		//{
		//	Player.equipWeapon = def.equipWeapon;
		//}

		Player.weaponItem[def.equipSlot].id = def.equipWeapon;
		Player.weaponItem[def.equipSlot].limitPerc = def.limitPerc;

		//if (Room.PlayerList.PlayerList[def.idSlot].bFullInfo)
		//{
		//	Room.PlayerList.PlayerList[def.idSlot].PlayerInfo.weaponItem[def.equipSlot] = def.equipWeapon;
		//}

		if (pSlot->slotType & _EP_WEAP_PRIMARY)
		{
			INT ClassID = ((pSlot->slotType & _EP_P1) ? _CLASS_POINTMAN : (pSlot->slotType & _EP_R1) ? _CLASS_RIFLEMAN : (pSlot->slotType & _EP_S1) ? _CLASS_SNIPER : _CLASS_NONE);
			if (ClassID == _CLASS_NONE)
			{
				_LOG(TEXT("Error changing weapon; invalid class"));
				return;
			}

			for (INT csi = 0; csi < _CSI_MAX; ++csi)
				Player.customItem[ClassID][csi] = def.customItem[csi];
		}

		_LOG(TEXT("Player [%d]%s changed weapon (slot %d) to %s(%s)"),
									def.idSlot,
									Room.PlayerList.PlayerList[def.idSlot].RoomPlayerInfo.nickname,
									def.equipSlot,
									_ItemDesc().GetItem(def.equipWeapon) ? _ItemDesc().GetItem(def.equipWeapon)->GetName() : TEXT("???"),
									*GetAvaNetHandler()->GetWeaponIconCode(def.equipWeapon));

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeWeapon, TEXT(""), TEXT(""), def.idSlot, 0);
	}
	else
	{
		_LOG(TEXT("Invalid weapon notification received. idSlot = %d"), def.idSlot);
	}
}

void PM::ROOM::RTT_UPDATE_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	if (!Room.IsValid())
		return;

	if (!FRoomInfo::IsSlotIDValid(def.idSlot, FALSE))
	{
		_LOG(TEXT("Player slot id is invalid."));
		return;
	}

	if (!Room.PlayerList.IsEmpty(def.idSlot))
	{
		FRoomPlayerInfo &Info = Room.PlayerList.PlayerList[def.idSlot];

		if (Info.IsPlaying())
		{
			Info.RttValue = Info.RoomPlayerInfo.rttScore = (def.rttScore >= 0 ? def.rttScore : 10.0f);
		}
		else
		{
			Info.RttValue = Info.RoomPlayerInfo.rttScore = def.rttScore;
		}

		_LOG(TEXT("RTT score = [%d]%s --> %.2f"),
					def.idSlot, Info.RoomPlayerInfo.nickname, Info.RttValue);

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_RttRating, TEXT("end"), TEXT(""), 0, 0);
	}
}

//void PM::ROOM::SET_RTTTEST_ADDR_NTF::Proc(_LPMSGBUF pData)
//{
//	if (!_StateController->IsStateInRoom())
//		return;
//
//	TMSG msg(pData);
//	DEF &def = msg.Data();
//
//	FRoomInfo &Room = _StateController->RoomInfo;
//	if (!Room.IsValid())
//		return;
//
//	if (def.idSlot < MAX_ALL_PLAYER_PER_ROOM)
//	{
//		Room.PlayerList.PlayerList[def.idSlot].RoomPlayerInfo.rttTestAddr = def.addrInfo;
//	}
//}

void PM::ROOM::SET_HOSTADDR_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	//def.hostInfo
	TCHAR IPAddress[20];
	def.hostInfo.extAddr.GetAddress(IPAddress);
	GavaNetClient->Settings.Set(CFG_HOSTADDRESS, IPAddress); // [+] 20070226 dEAthcURe

	((SOCKADDR_IN*)&GavaNetClient->HostAddrLocal)->sin_family = AF_INET;
	((SOCKADDR_IN*)&GavaNetClient->HostAddrLocal)->sin_addr.S_un.S_addr = def.hostInfo.intAddr.ipAddress;
	((SOCKADDR_IN*)&GavaNetClient->HostAddrLocal)->sin_port = def.hostInfo.intAddr.port;

	((SOCKADDR_IN*)&GavaNetClient->HostAddrPublic)->sin_family = AF_INET;
	((SOCKADDR_IN*)&GavaNetClient->HostAddrPublic)->sin_addr.S_un.S_addr = def.hostInfo.extAddr.ipAddress;
	((SOCKADDR_IN*)&GavaNetClient->HostAddrPublic)->sin_port = def.hostInfo.extAddr.port;	

	// 호스트가 변경 되었을 때, 새 호스트는 이 메시지를 서버에게 보내고, 서버는 이 메시지를 전체에게 브로드캐스트한다.
	// 새 호스트에게 다시 접속하는 시퀀스
}

void PM::ROOM::RTTT_START_ANS::Proc(_LPMSGBUF pData)
{
#ifdef EnableRttTest
	_LPMSGBUF pmsgbuf;
	CHECK_PENDING_MSG(RTTT_START_REQ, pmsgbuf);

	if (!_StateController->PlayerInfo.IsValid() || !_StateController->RoomInfo.IsValid())
	{
		pmsgbuf->Delete();
		return;
	}

	if (GIsGame && !GIsEditor)
	{
		PM::ROOM::RTTT_START_REQ::TMSG pmsg(pmsgbuf);
		TMSG msg(pData);
		DEF &def = msg.Data();

		if (msg.Data().result == RC_OK)
		{
			SOCKADDR_IN addrLocal, addrPublic;
			addrLocal.sin_family = AF_INET;
			addrLocal.sin_addr.S_un.S_addr = def.hostInfo.intAddr.ipAddress;
			addrLocal.sin_port = def.hostInfo.intAddr.port;
			addrPublic.sin_family = AF_INET;
			addrPublic.sin_addr.S_un.S_addr = def.hostInfo.extAddr.ipAddress;
			addrPublic.sin_port = def.hostInfo.extAddr.port;
			GavaNetClient->p2pNetTester.onAckP2pConnect(pmsg.Data().idAccount, &addrLocal, &addrPublic);
		}
	}

	pmsgbuf->Delete();
#endif
}

void PM::ROOM::RTTT_START_NTF::Proc(_LPMSGBUF pData)
{
#ifdef EnableRttTest
	if (!_StateController->RoomInfo.IsValid())
		return;

	if (GIsGame && !GIsEditor)
	{
		TMSG msg(pData);
		DEF &def = msg.Data();

		SOCKADDR_IN addrLocal, addrPublic;
		addrLocal.sin_family = AF_INET;
		addrLocal.sin_addr.S_un.S_addr = def.hostInfo.intAddr.ipAddress;
		addrLocal.sin_port = def.hostInfo.intAddr.port;
		addrPublic.sin_family = AF_INET;
		addrPublic.sin_addr.S_un.S_addr = def.hostInfo.extAddr.ipAddress;
		addrPublic.sin_port = def.hostInfo.extAddr.port;
		GavaNetClient->p2pNetTester.onNtfP2pConnect(def.idAccount, &addrLocal, &addrPublic);
	}

#endif
}


void PM::ROOM::QUICK_JOIN_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(QUICK_JOIN_REQ);

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();
	PM::_MsgBuffer<ROOM_PLAYER_INFO> PlayerList(msg, def.playerList);

	_ProcJoinRoom(def.result, def.idRoom, def.setting, def.state, PlayerList.GetBuffer(), PlayerList.GetCount(), def.idHost);
}

// 모든 플레이어의 슬롯 위치 재설정
void PM::ROOM::REPOSITION_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;
	if (!_StateController->RoomInfo.IsValid())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;

	// 기존 리스트 우선 백업; 관전은 제외
	FRoomPlayerInfo OldPlayerList[Def::MAX_PLAYER_PER_ROOM];
	appMemcpy(OldPlayerList, Room.PlayerList.PlayerList, sizeof(OldPlayerList));

	for (INT i = 0; i < MAX_PLAYER_PER_ROOM; ++i)
	{
		Room.PlayerList.PlayerList[i].Clear();
	}

	INT NewHostIdx = -1;
	INT NewMyIdx = -1;
	INT NewListIdx = -1;

	for (INT i = 0; i < MAX_PLAYER_PER_ROOM; ++i)
	{
		if (def.reposition[i] == ID_INVALID_ACCOUNT)
			continue;

		for (INT x = 0; x < MAX_PLAYER_PER_ROOM; ++x)
		{
			if (OldPlayerList[x].PlayerInfo.idAccount == def.reposition[i])
			{
				appMemcpy(&Room.PlayerList.PlayerList[i], &OldPlayerList[x], sizeof(FRoomPlayerInfo));
				Room.PlayerList.PlayerList[i].RoomPlayerInfo.idSlot = i;
				//Room.PlayerList.PlayerList[i].RoomPlayerInfo.idTeam = FRoomInfo::SlotToTeam(i);

				if (Room.HostIdx == x)
					NewHostIdx = i;
				if (_StateController->MyRoomSlotIdx == x)
					NewMyIdx = i;
				if (Room.PlayerList.ListIndex == x)
					NewListIdx = i;

				if (!def.bKeepReady && Room.PlayerList.PlayerList[i].IsReady())
					Room.PlayerList.PlayerList[i].RoomPlayerInfo.bReady = _READY_NONE;

				break;
			}
		}
	}

	if (NewHostIdx != -1)
		Room.HostIdx = NewHostIdx;
	if (NewMyIdx != -1)
		_StateController->MyRoomSlotIdx = NewMyIdx;
	if (NewListIdx != -1)
		Room.PlayerList.ListIndex = NewListIdx;

	Room.RefreshPlayerCount();
	_StateController->PlayerInfo.PlayerInfo.lastTeam = FRoomInfo::SlotToTeam(_StateController->MyRoomSlotIdx);

	GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSlot, TEXT("ok"), TEXT(""), 0, 0);

	if (_StateController->ChannelInfo.IsFriendlyGuildChannel())
	{
		// 친선 클랜전의 경우 양 진영의 클랜 정보 재설정 필요

		//Room.RefreshClanInfo();
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ClanInfo, TEXT(""), TEXT(""), 0, 0);

		Room.DumpClanInfo();
	}

	if (def.reason == RR_GAMEREQ)
	{
		// 게임 스크립트에서 팀 바꾸기를 한 상태이므로, 스크립트로 다시 리포트해야 함.
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_SwapTeam, TEXT("ok"), TEXT(""), 0, 0);
	}
}

// 방장 지수
void PM::ROOM::HOST_RATING_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	if (!Room.IsValid())
		return;

	if (!FRoomInfo::IsSlotIDValid(def.idSlot, FALSE))
	{
		_LOG(TEXT("Player slot id is invalid."));
		return;
	}

	if (!Room.PlayerList.IsEmpty(def.idSlot))
	{
		FRoomPlayerInfo &Info = Room.PlayerList.PlayerList[def.idSlot];
		//if (def.idSlot == Room.HostIdx)
		//{
			Info.RoomPlayerInfo.hostRating = def.rating;
			_LOG(TEXT("Host rating = [%d]%s --> %d"),
						def.idSlot, Info.RoomPlayerInfo.nickname, Info.RoomPlayerInfo.hostRating);
		//}

		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_RttRating, TEXT("end"), TEXT(""), 0, 0);
	}
}

void PM::ROOM::ITEM_REPAIR_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;
	if (!_StateController->RoomInfo.IsValid())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;

	if (!Room.PlayerList.IsEmpty(def.idSlot))
	{
		FRoomPlayerInfo &Info = Room.PlayerList.PlayerList[def.idSlot];
		if (def.equipSlot >= 0 && def.equipSlot < MAX_WEAPONSET_SIZE && Info.RoomPlayerInfo.weaponItem[def.equipSlot].id == def.idItem)
		{
			_LOG(TEXT("[%d]%s repaired his weapon [%d]%d"),
							def.idSlot, Info.RoomPlayerInfo.nickname, def.equipSlot, def.idItem);

			Info.RoomPlayerInfo.weaponItem[def.equipSlot].limitPerc = 100;
		}
		else
		{
			_LOG(TEXT("invalid parameter; idSlot = %d, equipSlot = %d, idItem = %d, actual id = %d"),
							def.idSlot, def.equipSlot, def.idItem, Info.RoomPlayerInfo.weaponItem[def.equipSlot].id);
		}
	}
	else
	{
		_LOG(TEXT("slot [%d] is empty!"), def.idSlot);
	}
}

void PM::ROOM::CLAN_INFO_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;
	if (!_StateController->ChannelInfo.IsFriendlyGuildChannel())
		return;
	if (!_StateController->RoomInfo.IsValid())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	// --> deprecated
	//appMemcpy(_StateController->RoomInfo.RoomClanInfo, def.clanInfo, 2 * sizeof(ROOM_CLAN_INFO));
	//_StateController->RoomInfo.DumpClanInfo();
}

void PM::ROOM::UPDATE_PCBANG_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;
	if (!_StateController->RoomInfo.IsValid())
		return;

	TMSG msg(pData);

	FRoomPlayerInfo *Info = _StateController->RoomInfo.PlayerList.Find(msg.Data().idAccount);
	if (Info && Info->RoomPlayerInfo.idAccount != _StateController->PlayerInfo.PlayerInfo.idAccount)
	{
		Info->RoomPlayerInfo.pcBang = 0;
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_PlayerList, TEXT("ok"), TEXT(""), 0, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void PM::ROOM::Proc(_LPMSGBUF pData)
{
	MSG_HEADER *pHeader = (MSG_HEADER*)pData->GetData();

	switch (pHeader->msg_id)
	{
		CASE_MSG_PROC(CREATE_ANS)
		CASE_MSG_PROC(JOIN_ANS)
		CASE_MSG_PROC(JOIN_NTF)
		CASE_MSG_PROC(INFO_ANS)
		CASE_MSG_PROC(LEAVE_ANS)
		CASE_MSG_PROC(LEAVE_NTF)
		CASE_MSG_PROC(CHAT_NTF)
		CASE_MSG_PROC(PLAYER_INFO_ANS)
		CASE_MSG_PROC(CHANGE_HOST_NTF)
		CASE_MSG_PROC(KICK_PLAYER_NTF)
		CASE_MSG_PROC(CHANGE_SETTING_ANS)
		CASE_MSG_PROC(CHANGE_SETTING_NTF)
		CASE_MSG_PROC(CHANGE_STATE_NTF)
		CASE_MSG_PROC(READY_NTF)
		CASE_MSG_PROC(CHANGE_SLOT_NTF)
		CASE_MSG_PROC(CHANGE_CLASS_NTF)
		CASE_MSG_PROC(CHANGE_WEAPON_NTF)
		CASE_MSG_PROC(RTT_UPDATE_NTF)
		//CASE_MSG_PROC(SET_RTTTEST_ADDR_NTF)
		CASE_MSG_PROC(SET_HOSTADDR_NTF)
		CASE_MSG_PROC(RTTT_START_ANS)
		CASE_MSG_PROC(RTTT_START_NTF)
		CASE_MSG_PROC(QUICK_JOIN_ANS)
		CASE_MSG_PROC(REPOSITION_NTF)
		CASE_MSG_PROC(HOST_RATING_NTF)
		CASE_MSG_PROC(ITEM_REPAIR_NTF)
		CASE_MSG_PROC(CLAN_INFO_NTF)
		CASE_MSG_PROC(UPDATE_PCBANG_NTF)

	default:
		_LOG(TEXT("Invalid ROOM message received. ID = %d"), pHeader->msg_id);
	}
}

void PM::ROOM::ProcTimeOut(const BYTE *Buffer, INT BufferLen)
{
	MSG_HEADER *pHeader = (MSG_HEADER*)Buffer;

	switch (pHeader->msg_id)
	{
		CASE_MSG_TIMEOUT_PROC(Room, Create, CREATE_REQ)
		CASE_MSG_TIMEOUT_PROC(Room, Join, JOIN_REQ)
		CASE_MSG_TIMEOUT_PROC(Room, Leave, LEAVE_REQ)
		CASE_MSG_TIMEOUT_PROC(Room, ChangeSetting, CHANGE_SETTING_REQ)
		CASE_MSG_TIMEOUT_PROC(Room, RtttStart, RTTT_START_REQ)
		CASE_MSG_TIMEOUT_PROC(Room, QuickJoin, QUICK_JOIN_REQ)
	default:
		_LOG(TEXT("Some ROOM message timed out. ID = %d"), pHeader->msg_id);
	}
}
