/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaMsgProcChannel.cpp

	Description: Implementation of message processors

***/
#include "avaNet.h"

#define ASSERT check

#include "ComDef/Def.h"

using namespace Def;

#include "ComDef/MsgDef.h"
#include "ComDef/MsgDefChannel.h"

#include "ComDef/compress.h"

#include "avaNetClient.h"
#include "avaMsgProc.h"
#include "avaMsgSend.h"
#include "avaConnection.h"
#include "avaNetEventHandler.h"
#include "avaNetStateController.h"
#include "avaCommunicator.h"
#include "avaWebInClient.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Channel

void PM::CHANNEL::CHANNEL_LIST_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(CHANNEL_LIST_REQ);

	if (_StateController->GetNetState() <= _AN_CONNECTING)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (def.result == RC_OK)
	{
		PM::_MsgBuffer<CHANNEL_INFO> ChannelList(msg, def.channelInfo);
		INT Count = ChannelList.GetCount();

		if (Count > 0)
		{
			_LOG(TEXT("Channel list received. count = %d"), Count);
		}
		else
		{
			_LOG(TEXT("Error! Invalid channel list."));
			return;
		}

		// 우선 모든 채널을 invalidate 시킴
		_StateController->ChannelList.InvalidateAll();

		for (INT i = 0; i < Count; ++i)
		{
			CHANNEL_INFO &Info = ChannelList[i];
			FChannelInfo *Channel = _StateController->ChannelList.Find(Info.idChannel);
			if (Channel)
			{
				// count < 0 이면 invalid한 상태.
				Channel->Count = Info.cnt_player;
			}
			else
			{
				// CHANNEL_DESC가 없는 채널의 정보를 받았음
				_LOG(TEXT("Invalid channel info received. id = %d"), Info.idChannel);
			}
		}

		_StateController->ChannelList.DumpChannelList();

		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_List, TEXT("ok"), TEXT(""), Count, 0);
	}
	else
	{
		// failed; disconnect
		_LOG(TEXT("Failed to receive channel list; disconnecting..."));
		//GavaNetClient->GetEventHandler()->Error(GavaNetClient->CurrentConnection(), 0);
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_List, TEXT("failed"), TEXT(""), 0, 0);
		//GavaNetClient->CloseConnection();
	}
}

void PM::CHANNEL::CHANNEL_JOIN_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(CHANNEL_JOIN_REQ);

	if (_StateController->GetNetState() != _AN_CHANNELLIST)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (_StateController->ChannelInfo.IsValid())
	{
		if (def.result == RC_OK)
		{
			_LOG(TEXT("Channel joined."));

			_StateController->QuickJoinChannelList.Empty();

			// 나 자신을 우선 로비 플레이어 목록에 추가
			_StateController->LobbyPlayerList.Add(_StateController->PlayerInfo.PlayerInfo);

			GavaNetClient->Settings.Set(CFG_LASTCHANNEL, *appItoa(_StateController->ChannelInfo.idChannel));
			_StateController->GoToState(_AN_LOBBY);

			if (_StateController->ChannelInfo.IsValid())
			{
				FChannelInfo *Info = _StateController->ChannelList.Find(_StateController->ChannelInfo.idChannel);
				if (Info)
				{
					//_StateController->ChannelInfo.Name = Info->Name;
					_StateController->ChannelInfo.Count = Info->Count;
				}
			}

			// 웹에 현재 위치 설정
			_WebInClient().ChannelSet(_StateController->ChannelInfo.ChannelNameShort);

			// 자동 이동 중이었으면 처리함
			if ( !_StateController->ProcAutoMove() )
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("ok"), _StateController->ChannelInfo.ChannelName, 0, 0);
			}

			return;
		}
		else if (def.result == RC_SESSION_MOVE || def.result == RC_SERVER_MOVE)
		{
			// 채널 이동이 필요하므로, 새 채널로 세션을 이동을 요청한다.
			// connect 과정을 다시 거쳐야 함

			_LOG(TEXT("Moving session to the joining channel."));

			_StateController->QuickJoinChannelList.Empty();

			CavaConnection *Connection = GavaNetClient->CurrentConnection();
			check(Connection);

			// Request to change session
			ScopedMsgBufPtr msgbuf = CreateMsgBufN(1024);
			check(msgbuf);
			CREATE_RXADDRESS(RxAddr, _StateController->ChannelInfo);
			if ( RxGateTranslator::MsgChangeSessionReq(msgbuf, GavaNetClient->ClientKey, GavaNetClient->SessionKeyChannel, &RxAddr) )
			{
				Connection->Send(msgbuf);
				GavaNetClient->CLState = CLS_ChangingSession;
			}
			else
			{
				_LOG(TEXT("Error creating ChangeSessionReq message."));
				GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Session_FailedToCreate);
				Connection->Disconnect();
			}

			GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("moving"), TEXT(""), 0, 0);
		}
		else
		{
			_LOG(TEXT("Error joining the channel."));
			_StateController->ChannelInfo.idChannel = ID_INVALID_CHANNEL;

			if (_StateController->QuickJoinChannelList.Num() > 0)
			{
				_StateController->ProcQuickJoinChannel();
			}
			else
			{
				_StateController->StopAutoMove();

				switch (def.result)
				{
				case RC_CHANNEL_PLAYER_FULL:
					GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("full"), TEXT(""), 0, 0);
					break;
				case RC_CHANNEL_INVALID_LEVEL:
					GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("invalid level"), TEXT(""), 0, 0);
					break;
				case RC_CHANNEL_NO_PRIV:
					GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("no priv"), TEXT(""), 0, 0);
					break;
				case RC_CHANNEL_INVALID_SD:
					GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("newbie only"), TEXT(""), 0, 0);
					break;
				case RC_CHANNEL_INVALID_CLAN:
					if (_StateController->GuildInfo.IsValid() && !_StateController->GuildInfo.IsRegularGuild())
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("regular guild only"), TEXT(""), 0, 0);
					}
					else
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("guild only"), TEXT(""), 0, 0);
					}
					break;
				case RC_CHANNEL_PCBANG_ONLY:
					GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("pcbang only"), TEXT(""), 0, 0);
					break;
				default:
					GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("failed"), TEXT(""), 0, 0);
				}
			}
		}
	}

}

void PM::CHANNEL::CHANNEL_LEAVE_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(CHANNEL_LEAVE_REQ);

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (def.result == RC_OK)
	{
		_StateController->GoToState(_AN_CHANNELLIST);

		if ( !_StateController->ProcAutoMove() )
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Leave, TEXT("ok"), TEXT(""), 0, 0);
		}

		//_WebInClient().ChannelSet(Localize(TEXT("Channel"), TEXT("Text_Loc_ChannelList"), TEXT("AVANET")));
	}
	else
	{
		_StateController->StopAutoMove();

		//GavaNetClient->GetEventHandler()->Error(GavaNetClient->CurrentConnection(), 0);
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Leave, TEXT("failed"), TEXT(""), 0, 0);
	}
}

void PM::CHANNEL::ROOM_LIST_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(ROOM_LIST_REQ);

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (def.result == RC_OK)
	{
		_StateController->RoomList.Clear();
#ifdef _SERVER_PUSH
		_StateController->RoomList.DiffIndex = def.diffIndex;
#endif

		_LOG(TEXT("Receiving room lists... %d room(s)."), def.count);

		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomList, TEXT("ok"), TEXT(""), def.count, 0);
	}
	else
	{
		_LOG(TEXT("Room list request failed."));
		// leave channel?
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomList, TEXT("failed"), TEXT(""), 0, 0);
	}
}

void PM::CHANNEL::ROOM_LIST_NTF::Proc(_LPMSGBUF pData)
{
	if (_StateController->GetNetState() != _AN_LOBBY)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();
	PM::_MsgBuffer<ROOM_INFO> ListBuf(msg, def.roomInfo);
	INT Count = ListBuf.GetCount();

	if (Count < 0)
	{
		_LOG(TEXT("Error! Invalid room list."));
		return;
	}

	for (INT i = 0; i < Count; ++i)
	{
		FRoomDispInfo *Info = _StateController->RoomList.Add(ListBuf[i]);
		Info->DumpRoomInfo();
	}

	if (def.result == RC_OK)
	{
		_LOG(TEXT("Done."));

		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomList, TEXT("done"), TEXT(""), 0, 0);
	}
}

void PM::CHANNEL::ROOM_INFO_ANS::Proc(_LPMSGBUF pData)
{
	//CHECK_AND_DELETE_PENDING_MSG(ROOM_INFO_REQ);

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (def.result == RC_OK)
	{
		_LOG(TEXT("Received room information. (idRoom = %d)"), def.room_id);

		FRoomDispInfo *Room = _StateController->RoomList.Find(def.room_id);
		if (Room)
		{
			//Room->RoundCount = def.roundCount;
			//Room->PlayerList.Clear();

			//for (INT i = 0; i < Def::MAX_PLAYER_PER_ROOM; ++i)
			//{
			//	appStrcpy(Room->PlayerList.PlayerList[i].RoomPlayerInfo.nickname, def.playerList[i]);
			//}

			GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomInfo, TEXT("ok"), TEXT(""), 0, 0);
		}
	}
	else
	{
		_LOG(TEXT("Room information request failed."));
		// do not pop up error message; just remove the room from the list
		_StateController->RoomList.Remove(def.room_id);
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomInfo, TEXT("ok"), TEXT(""), 0, 0);
	}
}

void PM::CHANNEL::PLAYER_LIST_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(PLAYER_LIST_REQ);

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (def.result == RC_OK)
	{
		_StateController->LobbyPlayerList.Clear();
		_LOG(TEXT("Receiving player lists... %d player(s)."), def.count);
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_PlayerList, TEXT("ok"), TEXT(""), def.count, 0);
	}
	else
	{
		_LOG(TEXT("Player list request failed."));
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_PlayerList, TEXT("failed"), TEXT(""), 0, 0);
	}
}

void PM::CHANNEL::PLAYER_LIST_NTF::Proc(_LPMSGBUF pData)
{
	if (_StateController->GetNetState() != _AN_LOBBY)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();
	PM::_MsgBuffer<LOBBY_PLAYER_INFO> ListBuf(msg, def.playerInfo);
	INT Count = ListBuf.GetCount();

	if (Count <= 0)
	{
		_LOG(TEXT("Error! Invalid lobby player list."));
		return;
	}

	for (INT i = 0; i < Count; ++i)
	{
		FPlayerDispInfo *Info = _StateController->LobbyPlayerList.Add(ListBuf[i]);
		if (Info)
		{
			if (Info->PlayerInfo.idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
				Info->Set(_StateController->PlayerInfo.PlayerInfo);

			_StateController->SyncPlayerLevel(Info->PlayerInfo.idAccount, Info->PlayerInfo.level);
			Info->DumpPlayerInfo();
		}
	}

	if (def.result == RC_OK)
	{
		_LOG(TEXT("Done."));
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_PlayerList, TEXT("done"), TEXT(""), 0, 0);
	}
}

void PM::CHANNEL::PLAYER_INFO_ANS::Proc(_LPMSGBUF pData)
{
	//if (_StateController->GetNetState() != _AN_LOBBY)
	//	return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (def.result != RC_OK)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_PlayerInfo, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	_LOG(TEXT("idGuildMark = %d"), def.playerInfo.idGuildMark);

	FPlayerDispInfo *Player = _StateController->FindPlayerFromList(def.playerInfo.idAccount);
	if (Player)
	{
		Player->Set(def.playerInfo);
		Player->DumpPlayerInfo();
	}

	_Communicator().SyncPlayerInfo(def.playerInfo);
	if (_StateController->GuildInfo.IsValid())
		_StateController->GuildInfo.SyncPlayerInfo(def.playerInfo);

	GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_PlayerInfo, TEXT("ok"), TEXT(""), 0, 0);
}

void PM::CHANNEL::PLAYER_LOCATION_ANS::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);
	DEF &def = msg.Data();

	if (def.result != RC_OK)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_PlayerInfo, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	_LOG(TEXT("idAccount = %d, idChannel = %d, idRoom = %d"));

	_Communicator().SyncPlayerInfo(def.idAccount, def.idChannel, def.idRoom);
	if (_StateController->GuildInfo.IsValid())
		_StateController->GuildInfo.SyncPlayerInfo(def.idAccount, def.idChannel, def.idRoom);
	GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_PlayerInfo, TEXT("ok"), TEXT(""), 0, 0);
}

void PM::CHANNEL::FOLLOW_PLAYER_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(FOLLOW_PLAYER_REQ);

	TMSG msg(pData);
	DEF &def = msg.Data();

	switch (def.result)
	{
	case RC_OK:
		_Communicator().SyncPlayerInfo(def.idAccount, def.idChannel, def.idRoom);
		if (_StateController->GuildInfo.IsValid())
			_StateController->GuildInfo.SyncPlayerInfo(def.idAccount, def.idChannel, def.idRoom);
		if (def.bPassword > 0 && !_StateController->AutoMoveDest.IsPasswordSet())
		{
			_Communicator().SyncPlayerInfo(def.idAccount, def.idChannel, def.idRoom);
			if (_StateController->GuildInfo.IsValid())
				_StateController->GuildInfo.SyncPlayerInfo(def.idAccount, def.idChannel, def.idRoom);
			// 암호를 더 입력 받아야 하므로, 실제 이동은 하지 않고 대기
			_StateController->AutoMoveDest.Pause();
			GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_FollowPlayer, TEXT("need password"), TEXT(""), 0, 0);
		}
		else
		{
			// 따라가기
			if (def.idAccount == ID_INVALID_CHANNEL)
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_FollowPlayer, TEXT("failed"), TEXT(""), 0, 0);
				break;
			}

			GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_FollowPlayer, TEXT("ok"), TEXT(""), 0, 0);

			FPlayerDispInfo *Player = _StateController->FindPlayerFromList(def.idAccount);
			if (Player)
			{
				// 이미 같은 위치에 있음
				_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_SameLocationWithPlayer"), TEXT("AVANET")), Player->PlayerInfo.nickname),
												EChat_PlayerSystem);
				break;
			}

			_StateController->AutoMoveDest.SetMoveDestTo(def.idChannel, def.idRoom, def.roomName, def.idAccount);
			_StateController->ProcAutoMove();
		}
		return;
	case RC_CHANNEL_PLAYER_FULL:
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowToFullChannel"), TEXT("AVANET")),
										EChat_PlayerSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_FollowPlayer, TEXT("failed"), TEXT(""), 0, 0);
		break;
	case RC_CHANNEL_INVALID_LEVEL:
		//GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_FollowPlayer, TEXT("invalid level"), TEXT(""), 0, 0);
		//break;
	case RC_CHANNEL_NO_PRIV:
		//GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_FollowPlayer, TEXT("no priv"), TEXT(""), 0, 0);
		//break;
	case RC_CHANNEL_INVALID_SD:
		//GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_FollowPlayer, TEXT("newbie only"), TEXT(""), 0, 0);
		//break;
	case RC_CHANNEL_INVALID_CLAN:
		//if (_StateController->GuildInfo.IsValid() && !_StateController->GuildInfo.IsRegularGuild())
		//{
		//	GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_FollowPlayer, TEXT("regular guild only"), TEXT(""), 0, 0);
		//}
		//else
		//{
		//	GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_FollowPlayer, TEXT("guild only"), TEXT(""), 0, 0);
		//}
		//break;
	case RC_CHANNEL_PCBANG_ONLY:
		//GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_FollowPlayer, TEXT("pcbang only"), TEXT(""), 0, 0);
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowToChannel"), TEXT("AVANET")),
														*_StateController->AutoMoveDest.Nickname),
										EChat_PlayerSystem);
		break;
	case RC_FAIL:
		if (def.idRoom == USHRT_MAX)
		{
			// 채널 선택 중
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowToChannelList"), TEXT("AVANET")),
															*_StateController->AutoMoveDest.Nickname),
											EChat_PlayerSystem);
		}
		else
		{
			_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_FollowFailed"), TEXT("AVANET")),
											EChat_PlayerSystem);
			GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_FollowPlayer, TEXT("failed"), TEXT(""), 0, 0);
		}
		break;
	default:
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_FollowFailed"), TEXT("AVANET")),
										EChat_PlayerSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_FollowPlayer, TEXT("failed"), TEXT(""), 0, 0);
		break;
	}

	_StateController->StopAutoMove();
}

void PM::CHANNEL::LOBBY_JOIN_NTF::Proc(_LPMSGBUF pData)
{
	// 내 클랜 홈에서 사용

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;
	if (!_StateController->ChannelInfo.IsMyClanChannel())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	_LOG(TEXT("Player joined..."));

	FLobbyPlayerInfo *Info = _StateController->LobbyPlayerList.Add(def.playerInfo);
	if (Info)
	{
		Info->DumpPlayerInfo();

		_StateController->SyncPlayerLevel(def.playerInfo.idAccount, def.playerInfo.level);

		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_LobbyJoin, TEXT(""), TEXT(""), def.playerInfo.idAccount, 0);
	}
}

void PM::CHANNEL::LOBBY_LEAVE_NTF::Proc(_LPMSGBUF pData)
{
	// 내 클랜 홈에서 사용

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;
	if (!_StateController->ChannelInfo.IsMyClanChannel())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

#if !FINAL_RELEASE
	FLobbyPlayerInfo *Info = _StateController->LobbyPlayerList.Find(def.idAccount);
	if (Info)
		_LOG(TEXT("[%d]%s left..."), Info->PlayerInfo.idAccount, Info->PlayerInfo.nickname);
#endif

	_StateController->LobbyPlayerList.Remove(def.idAccount);

	GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_LobbyLeave, TEXT(""), TEXT(""), def.idAccount, 0);
}

void PM::CHANNEL::LOBBY_CHAT_NTF::Proc(_LPMSGBUF pData)
{
	if (_StateController->GetNetState() != _AN_LOBBY)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FLobbyPlayerInfo *Player = _StateController->LobbyPlayerList.Find(def.idAccount);
	if (!Player)
		return;

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

	FString ChatStr = FString::Printf(TEXT("%s : %s"), Player->PlayerInfo.nickname, *ParsedMsg);

	_StateController->LogChatConsole(*ChatStr, MsgType);
}

void PM::CHANNEL::WHISPER_NTF::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);
	DEF &def = msg.Data();

	if (_Communicator().BlockList.Find(msg.Data().idAccount))
		return;

	_Communicator().LastWhisperedPlayer = def.nickname;
	if (_Communicator().LastWhisperedPlayer.Len() == 0)
		return;

	PM::_MsgString ChatMsgBuf(msg, def.chatmsg);
	_Communicator().ProcBuddyChat(def.idAccount, def.nickname, (LPWSTR)ChatMsgBuf);
}

void PM::CHANNEL::ROOM_UPDATE_STATE_NTF::Proc(_LPMSGBUF pData)
{
	// 내 클랜 홈에서 사용

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;
	if (!_StateController->ChannelInfo.IsMyClanChannel())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomDispInfo *Info = _StateController->RoomList.Find(def.idRoom);
	if (Info)
	{
		Info->RoomInfo.state = def.state;
		_LOG(TEXT("Room %d state updated; playing = %d, numCurr = %d"), def.idRoom, def.state.playing, def.state.numCurr);

		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomState, TEXT(""), TEXT(""), def.idRoom, 0);
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomList, TEXT("done"), TEXT(""), 0, 0);
	}
}

void PM::CHANNEL::ROOM_UPDATE_SETTING_NTF::Proc(_LPMSGBUF pData)
{
	// 내 클랜 홈에서 사용

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;
	if (!_StateController->ChannelInfo.IsMyClanChannel())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomDispInfo *Info = _StateController->RoomList.Find(def.idRoom);
	if (Info)
	{
		Info->RoomInfo.setting = def.setting;
		_LOG(TEXT("Room %d setting updated; idMap = %d, tkLevel = %d, autoBalance = %d, allowInterrupt = %d, roundToWin = %d, numMax = %d"),
				def.idRoom, def.setting.tkLevel, def.setting.autoBalance, def.setting.allowInterrupt, def.setting.roundToWin, def.setting.numMax);

		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomSetting, TEXT(""), TEXT(""), def.idRoom, 0);
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomList, TEXT("done"), TEXT(""), 0, 0);
	}
}

//#ifdef _SERVER_PUSH

void PM::CHANNEL::ROOM_LIST_DIFF_NTF::Proc(_LPMSGBUF pData)
{
	// 내 클랜 홈을 제외한 채널에서 사용

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;
	if (!_StateController->ChannelInfo.IsNormalChannel())
		return;

	//if (_StateController->RoomList.DiffIndex == 0)
	//{
	//	_LOG(TEXT("Error! Room list is not updated yet; Message ignored."));
	//	return;
	//}

	TMSG msg(pData);
	DEF &def = msg.Data();
	PM::_MsgBuffer<BYTE> DiffBuf(msg, def.diff);

	if (def.diffIndex != _StateController->RoomList.DiffIndex + 1)
	{
		_LOG(TEXT("Error! Invalid diff of the room list received(old = %d, new = %d); Requesting new room list."), _StateController->RoomList.DiffIndex, def.diffIndex);

		ROOM_LIST_REQ::Send();
		return;
	}

	if (DiffBuf.GetLength() <= 0)
	{
		_LOG(TEXT("Error! Invalid diff buffer received; Requesting new room list."));

		ROOM_LIST_REQ::Send();
		return;
	}

	_StateController->RoomList.DiffIndex = def.diffIndex;

	BYTE Buf[FRoomList::BUF_MAX_SIZE];

	INT BufSize = expand_rle(DiffBuf.GetBuffer(), Buf, DiffBuf.GetLength());

	if (BufSize != FRoomList::BUF_MAX_SIZE )
	{
		_LOG(TEXT("Error! Failed to expand the compressed diff buffer; Requesting new room list."));

		ROOM_LIST_REQ::Send();
		return;
	}

	_StateController->RoomList.MergeFromDiff(Buf, BufSize);

	GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomList, TEXT("done"), TEXT(""), 0, 0);

#if !FINAL_RELEASE
	_StateController->RoomList.DumpRoomList();
#endif

	// 채널이나 로비에 50명 이상이 있는데 방이 하나뿐이라면, 패킷이 뭔가 잘못됐다고 유추할 수 있다.
	if (_StateController->RoomList.RoomList.Num() <= 1 &&
		(_StateController->ChannelInfo.Count >= 50 || _StateController->LobbyPlayerList.PlayerList.Num() >= 50))
	{
		ROOM_LIST_REQ::Send();
	}
}

void PM::CHANNEL::PLAYER_LIST_DIFF_NTF::Proc(_LPMSGBUF pData)
{
	// 내 클랜 홈을 제외한 채널에서 사용

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;
	if (!_StateController->ChannelInfo.IsNormalChannel())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	PM::_MsgBuffer<LOBBY_PLAYER_INFO> DiffBufIn(msg, def.diffIn);
	for (INT i = 0; i < DiffBufIn.GetCount(); ++i)
	{
		FLobbyPlayerInfo *Info = _StateController->LobbyPlayerList.Add(DiffBufIn[i]);

		if (Info)
			_StateController->SyncPlayerLevel(Info->PlayerInfo.idAccount, Info->PlayerInfo.level);
	}

	PM::_MsgBuffer<TID_ACCOUNT> DiffBufOut(msg, def.diffOut);
	for (INT i = 0; i < DiffBufOut.GetCount(); ++i)
	{
		_StateController->LobbyPlayerList.Remove(DiffBufOut[i]);
	}

	GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_PlayerList, TEXT("done"), TEXT(""), 0, 0);
}

//#else

void PM::CHANNEL::ROOM_CREATE_NTF::Proc(_LPMSGBUF pData)
{
	// 내 클랜 홈에서 사용

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;
	if (!_StateController->ChannelInfo.IsMyClanChannel())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	_LOG(TEXT("Room created..."));

	FRoomDispInfo *Info = _StateController->RoomList.Add(def.roomInfo);
	if (Info)
	{
		FString PlayerName = def.roomInfo.hostName;
		FLobbyPlayerInfo *Host = _StateController->LobbyPlayerList.Find(PlayerName);
		if (Host)
			_StateController->LobbyPlayerList.Remove(Host->PlayerInfo.idAccount);

		//PM::CHANNEL::ROOM_LIST_VIEW_ADD_NTF::Send(Info->RoomInfo.idRoom);

		Info->DumpRoomInfo();

		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomList, TEXT("ok"), TEXT(""), def.roomInfo.idRoom, 0);
	}
}

void PM::CHANNEL::ROOM_DELETE_NTF::Proc(_LPMSGBUF pData)
{
	// 내 클랜 홈에서 사용

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;
	if (!_StateController->ChannelInfo.IsMyClanChannel())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

#if !FINAL_RELEASE
	FRoomDispInfo *Info = _StateController->RoomList.Find(def.idRoom);
	if (Info)
		_LOG(TEXT("Room <%d> %s deleted..."), Info->RoomInfo.idRoom, Info->RoomInfo.roomName);
	else
		_LOG(TEXT("Room <%d> is not found"), def.idRoom);
#endif

	_StateController->RoomList.Remove(def.idRoom);

	GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomList, TEXT("ok"), TEXT(""), def.idRoom, 0);
}

//#endif

void PM::CHANNEL::CHANNEL_DESC_NTF::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);

	PM::_MsgBuffer<CHANNEL_DESC> DescBuf(msg, msg.Data().channelDesc);
	PM::_MsgString NameBuf(msg, msg.Data().nameList);
	FString NameStr = NameBuf.GetString() ? NameBuf.GetString() : TEXT("");
	TArray<FString> NameList;

	NameStr.ParseIntoArray(&NameList, TEXT("\n"), FALSE);

	for (INT i = 0; i < DescBuf.GetCount(); ++i)
	{
		CHANNEL_DESC &Desc = DescBuf[i];
		_StateController->ChannelList.Update(Desc, (i < NameList.Num()) ? NameList(i) : TEXT(""));
	}
}




/////////////////////////////////////////////////////////////////////////////////////////////////////

void PM::CHANNEL::Proc(_LPMSGBUF pData)
{
	MSG_HEADER *pHeader = (MSG_HEADER*)pData->GetData();

	switch (pHeader->msg_id)
	{
		CASE_MSG_PROC(CHANNEL_LIST_ANS)
		CASE_MSG_PROC(CHANNEL_JOIN_ANS)
		CASE_MSG_PROC(CHANNEL_LEAVE_ANS)

		CASE_MSG_PROC(ROOM_LIST_ANS)
		CASE_MSG_PROC(ROOM_LIST_NTF)
		CASE_MSG_PROC(ROOM_INFO_ANS)

		CASE_MSG_PROC(PLAYER_LIST_ANS)
		CASE_MSG_PROC(PLAYER_LIST_NTF)
		CASE_MSG_PROC(PLAYER_INFO_ANS)
		CASE_MSG_PROC(PLAYER_LOCATION_ANS)
		CASE_MSG_PROC(FOLLOW_PLAYER_ANS)

		CASE_MSG_PROC(LOBBY_JOIN_NTF)
		CASE_MSG_PROC(LOBBY_LEAVE_NTF)

		CASE_MSG_PROC(LOBBY_CHAT_NTF)
		CASE_MSG_PROC(WHISPER_NTF)

		CASE_MSG_PROC(ROOM_UPDATE_STATE_NTF)
		CASE_MSG_PROC(ROOM_UPDATE_SETTING_NTF)
//#ifdef _SERVER_PUSH
		CASE_MSG_PROC(ROOM_LIST_DIFF_NTF)
		CASE_MSG_PROC(PLAYER_LIST_DIFF_NTF)
//#else
		CASE_MSG_PROC(ROOM_CREATE_NTF)
		CASE_MSG_PROC(ROOM_DELETE_NTF)
//#endif
		CASE_MSG_PROC(CHANNEL_DESC_NTF)

	default:
		_LOG(TEXT("Invalid CHANNEL message received. ID = %d"), pHeader->msg_id);
	}
}

void PM::CHANNEL::ProcTimeOut(const BYTE *Buffer, INT BufferLen)
{
	MSG_HEADER *pHeader = (MSG_HEADER*)Buffer;

	switch (pHeader->msg_id)
	{
		CASE_MSG_TIMEOUT_PROC(Channel, List, CHANNEL_LIST_REQ)
		CASE_MSG_TIMEOUT_PROC(Channel, Join, CHANNEL_JOIN_REQ)
		CASE_MSG_TIMEOUT_PROC(Channel, Leave, CHANNEL_LEAVE_REQ)
		CASE_MSG_TIMEOUT_PROC(Channel, PlayerList, PLAYER_LIST_REQ)
		CASE_MSG_TIMEOUT_PROC(Channel, RoomList, ROOM_LIST_REQ)
		CASE_MSG_TIMEOUT_PROC(Channel, FollowPlayer, FOLLOW_PLAYER_REQ)
	default:
		_LOG(TEXT("Some CHANNEL message timed out. ID = %d"), pHeader->msg_id);
	}
}
