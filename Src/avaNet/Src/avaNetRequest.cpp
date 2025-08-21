/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaNetRequest.cpp

	Description: Implementation of avaNetRequest

***/
#include "avaNet.h"
#include "avaNetStateController.h"
#include "avaMsgSend.h"
#include "avaConnection.h"
#include "avaStaticData.h"
#include "avaCommunicator.h"
#include "avaWebInClient.h"

#include "ComDef/Inventory.h"
#include "RxGateTranslator/RxGateTranslator.h"



#define CHECK_GUILD_PRIV(_p)																										\
	if (!_StateController->DoIHaveGuildPriv(_p))																					\
	{																																\
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoPriv"), TEXT("AVANET")),			\
										EChat_GuildSystem);																			\
		return;																														\
	}



IMPLEMENT_CLASS(UavaNetRequest);

INT GetInvenIndexFromSlot( INT InvenSlot )
{
	INT InvenIndex = INDEX_NONE;

	// EquipInven
	if( InvenSlot >= 10000 )
		InvenIndex = InvenSlot % 10000;

	// WeaponInven
	else if ( (0 <= InvenSlot && InvenSlot < 100 * Def::MAX_WEAPONSET_SIZE)
		&& (InvenSlot %= 100) < Def::MAX_INVENTORY_SIZE )
	{
		InvenIndex = InvenSlot;
	}
	
	return InvenIndex;
}


void UavaNetRequest::NPGameSendAuth(DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3)
{
	ScopedMsgBufPtr MsgBuf = CreateMsgBufN(1024);
	check(MsgBuf);
	if ( RxGateTranslator::MsgGameGuardAuthAns(MsgBuf, GavaNetClient->ClientKey, dwIndex, dwValue1, dwValue2, dwValue3) )
	{
		GavaNetClient->CurrentConnection()->Send(MsgBuf);
		_LOG(TEXT("[MsgGameGuardAuthAns] Sent; ClientKey = %d, dwIndex = %d, dwValue1 = %d, dwValue2 = %d, dwValue3 = %d"),
					GavaNetClient->ClientKey, dwIndex, dwValue1, dwValue2, dwValue3);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Connection

UBOOL UavaNetRequest::AutoConnect(UBOOL bForce, const FString& USN, const FString& UserID, const FString& Key)
{
	if (GavaNetClient)
	{
		//TCHAR *Str = (TCHAR*)*UserID;
		//if ( wcspbrk(Str, TEXT(" '\"?#,`")) )
		//{
		//	GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_Connect, TEXT("invalid id"), TEXT(""), 0, 0);
		//	return FALSE;
		//}

		//GavaNetClient->Settings.Set(CFG_USERID, *UserID);
		//GavaNetClient->Settings.Set(CFG_USERPASSWORD, *UserPassword);
		_StateController->LastConnectResult = TEXT("");

		GavaNetClient->bForceConnect = bForce;

		if (USN.Len() > 0)
			GavaNetClient->Settings.Set(CFG_USERSN, *USN);
		if (UserID.Len() > 0)
			GavaNetClient->Settings.Set(CFG_USERID, *UserID);
		if (Key.Len() > 0)
			GavaNetClient->Settings.Set(CFG_KEYSTRING, *Key);
		else if (USN.Len() > 0 && UserID.Len() > 0)
			GavaNetClient->Settings.Set(CFG_KEYSTRING, *FString::Printf(TEXT("%s|%s|1171442665|20e6573de6b850b4900c62fb1ddcd9d0 01401900"), *USN, *UserID));
		return GavaNetClient->AutoConnect();
	}
	else
		return FALSE;
}

void UavaNetRequest::CloseConnection(INT ExitCode)
{
	if (GavaNetClient)
		GavaNetClient->CloseConnection((DWORD)ExitCode);
}


void UavaNetRequest::CreateCharacter(const FString& Nickname, INT FaceIndex)
{
	if (Nickname.Len() < SIZE_NICKNAME_MIN)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_CheckNick, TEXT("short name"), TEXT(""), 0, 0);
		return;
	}

	if (FaceIndex < 1 || FaceIndex > FACETYPE_MAX)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_CheckNick, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	// 닉네임은 서버가 체크
	//TCHAR *Str = (TCHAR*)*Nickname;
	//if ( !_WordCensor().CheckCharacterName(Str) )
	//{
	//	GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_CheckNick, TEXT("invalid name"), TEXT(""), 0, 0);
	//	return;
	//}

	PM::CLIENT::CHECK_NICK_REQ::Send((FString&)Nickname, FaceIndex);
}

// 서버에게 종료 통보 후 게임 종료
void UavaNetRequest::Quit(UBOOL bGraceful)
{
	if (GavaNetClient)
		GavaNetClient->Quit(bGraceful);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Channel list

void UavaNetRequest::ListChannel()
{
	PM::CHANNEL::CHANNEL_LIST_REQ::Send();
}

void UavaNetRequest::JoinChannel(INT ListIndex)
{
	// ListIndex = UIList에서 선택된 리스트 인덱스

	int Idx = -1;

	_LOG(TEXT("JoinChannel; ListIndex = %d"), ListIndex);
	if (ListIndex >= 0 && ListIndex < _StateController->ChannelList.ChannelList.Num())
	{
		if (_StateController->ChannelList.ChannelList(ListIndex).IsValid())
			Idx = _StateController->ChannelList.ChannelList(ListIndex).idChannel;
	}
	else
	{
		_StateController->ProcQuickJoinChannel();
	}

	JoinChannelByID(Idx);
}

void UavaNetRequest::JoinChannelByID(INT ID, UBOOL bFollowing)
{
	// ID = 채널 아이디

	if (!_StateController->PlayerInfo.IsValid())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	_LOG(TEXT("JoinChannel; ID = %d, following = %d"), ID, bFollowing);
	FChannelInfo *Channel = _StateController->ChannelList.Find(ID);
	if (!Channel || !Channel->IsValid())
	{
		if (_StateController->QuickJoinChannelList.Num() > 0)
		{
			_StateController->ProcQuickJoinChannel();
		}
		else
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("not available"), TEXT(""), 0, 0);
		}
		return;
	}

	FString JoinResult = Channel->IsJoinable();
	if (JoinResult != TEXT("ok"))
	{
		if (_StateController->QuickJoinChannelList.Num() > 0)
		{
			_StateController->ProcQuickJoinChannel();
		}
		else
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, *JoinResult, TEXT(""), 0, 0);
		}
		return;
	}

	_StateController->ChannelInfo = *Channel;

	PM::CHANNEL::CHANNEL_JOIN_REQ::Send(ID, bFollowing);
}

void UavaNetRequest::QuickJoinChannel(const FString &Flags)
{
	if (!_StateController->PlayerInfo.IsValid())
	{
		_LOG(TEXT("QuickJoin failed"));
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("quickjoin failed"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->ChannelList.ChannelList.Num() == 0)
	{
		_LOG(TEXT("QuickJoin failed"));
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("quickjoin failed"), TEXT(""), 0, 0);
		return;
	}

	// 플래그가 초보 채널이고 플레이어 레벨이 하사 이상일 때, 일반 채널로 변경
	//if (Flag == EChannelFlag_Trainee && _StateController->PlayerInfo.PlayerInfo.level >= _LEV_SERGEANT)
	//	Flag = EChannelFlag_Normal;

	//if (Flag < 0)
	//{
	//	// Flag == -1 -> 플레이어 레벨에 따라서 초보자 또는 일반 채널로 퀵조인
	//	Flag = (_StateController->PlayerInfo.PlayerInfo.level >= _LEV_SERGEANT ? EChannelFlag_Normal : EChannelFlag_Trainee);
	//}

	TArray<FString> FlagList;
	if (Flags.TrimQuotes().ParseIntoArray(&FlagList, TEXT(","), TRUE) == 0)
	{
		_LOG(TEXT("QuickJoin failed"));
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("quickjoin failed"), TEXT(""), 0, 0);
		return;
	}

	_StateController->QuickJoinChannelList.Empty();

	for (INT k = 0; k < FlagList.Num(); ++k)
	{
		INT Flag = appAtoi(*FlagList(k));

		if (Flag < 0)
		{
			// Flag == -1 -> 플레이어 레벨에 따라서 초보자 또는 일반 채널로 퀵조인
			Flag = (_StateController->PlayerInfo.PlayerInfo.level >= _LEV_SERGEANT ? EChannelFlag_Normal : EChannelFlag_Trainee);
		}

		// check the flag validity
		FString Result = FChannelInfo::IsJoinable(Flag);
		if ( Result != TEXT("ok") )
		{
			//GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, *Result, TEXT(""), 0, 0);
			//return;
			continue;
		}

		// 최대 인원이 95% 미만인 채널을 인원수로 정렬
		for (INT i = 0; i < _StateController->ChannelList.ChannelList.Num(); ++i)
		{
			FChannelInfo *Channel = &_StateController->ChannelList.ChannelList(i);
			if (!Channel || !Channel->IsValid() || Channel->Flag != Flag || Channel->Count >= Channel->MaxPlayers * 0.95 || Channel->IsJoinable() != TEXT("ok"))
				continue;

			INT Idx = 0;
			for ( ; Idx < _StateController->QuickJoinChannelList.Num(); ++Idx)
			{
				if ( Channel->Count > _StateController->ChannelList.ChannelList( _StateController->QuickJoinChannelList(Idx) ).Count )
					break;
			}

			if (_StateController->QuickJoinChannelList.FindItemIndex(i) == INDEX_NONE)
			{
				_LOG(TEXT("Adding %d"), i);
				_StateController->QuickJoinChannelList.InsertItem(i, Idx);
			}
		}
	}

	for (INT k = 0; k < FlagList.Num(); ++k)
	{
		INT Flag = appAtoi(*FlagList(k));

		if (Flag < 0)
		{
			// Flag == -1 -> 플레이어 레벨에 따라서 초보자 또는 일반 채널로 퀵조인
			Flag = (_StateController->PlayerInfo.PlayerInfo.level >= _LEV_SERGEANT ? EChannelFlag_Normal : EChannelFlag_Trainee);
		}

		_LOG(TEXT("Flag = %d"), Flag);

		// check the flag validity
		FString Result = FChannelInfo::IsJoinable(Flag);
		if ( Result != TEXT("ok") )
		{
			//GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, *Result, TEXT(""), 0, 0);
			//return;
			continue;
		}

		// 최대 인원이 95% 이상인 채널들을 목록 마지막으로
		for (INT i = 0; i < _StateController->ChannelList.ChannelList.Num(); ++i)
		{
			FChannelInfo *Channel = &_StateController->ChannelList.ChannelList(i);
			if (!Channel || !Channel->IsValid() || Channel->Flag != Flag || Channel->Count < Channel->MaxPlayers * 0.95 || Channel->IsJoinable() != TEXT("ok"))
				continue;

			if (_StateController->QuickJoinChannelList.FindItemIndex(i) == INDEX_NONE)
			{
				_LOG(TEXT("Adding %d"), i);
				_StateController->QuickJoinChannelList.Push(i);
			}
		}
	}

	if (_StateController->QuickJoinChannelList.Num() > 0)
	{
		_StateController->ProcQuickJoinChannel();
	}
	else
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Join, TEXT("quickjoin failed"), TEXT(""), 0, 0);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lobby

void UavaNetRequest::LeaveChannel()
{
	if (_StateController->ChannelInfo.IsNormalChannel())
	{
		PM::CHANNEL::CHANNEL_LEAVE_REQ::Send();
	}
	else if (_StateController->ChannelInfo.IsMyClanChannel())
	{
		PM::GUILD::LOBBY_LEAVE_REQ::Send();
	}
	else
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Leave, TEXT("failed"), TEXT(""), 0, 0);
	}
}

void UavaNetRequest::ListRoom()
{
	PM::CHANNEL::ROOM_LIST_REQ::Send();
}

void UavaNetRequest::ListLobbyPlayer()
{
	PM::CHANNEL::PLAYER_LIST_REQ::Send();
}

void UavaNetRequest::LobbySelectRoom(INT ListIndex)
{
	// 방 목록에 마우스 오버 시 호출.
	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		//GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomInfo, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->RoomList.RoomList.IsValidIndex(ListIndex))
	{
		_StateController->RoomList.ListIndex = ListIndex;
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomList, TEXT("selected"), TEXT(""), 0, 0);
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_RoomInfo, TEXT("ok"), TEXT(""), 0, 0);
	}
}

void UavaNetRequest::CreateRoom(const FString& RoomName, const FString& RoomPassword, FavaRoomSetting RoomSetting)
{
	if (_StateController->GetNetState() != _AN_LOBBY || _StateController->RoomInfo.IsValid())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Create, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->AmIStealthMode())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Create, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->ChannelInfo.IsMaskTurnedOn() && _StateController->PlayerInfo.GetCurrentChannelMaskLevel() == _CML_BROADCAST)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Create, TEXT("no priv"), TEXT(""), 0, 0);
		return;
	}

	// 방 제목은 서버가 체크
	//TCHAR *Str = (TCHAR*)*RoomName;
	//if ( !_WordCensor().CheckRoomName(Str) )
	//{
	//	GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Create, TEXT("invalid name"), TEXT(""), 0, 0);
	//	return;
	//}

	FMapInfo *MapInfo = _StateController->MapList.Find(RoomSetting.idMap);
	if (MapInfo == NULL)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Create, TEXT("invalid map"), TEXT(""), 0, 0);
		return;
	}
	if (MapInfo->ExclChannelGroups.FindItemIndex(_StateController->ChannelInfo.Flag) != INDEX_NONE)
	{
		// 허용되지 않는 맵
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Create, TEXT("invalid map"), TEXT(""), 0, 0);
		return;
	}

	//if (_StateController->PlayerInfo.PlayerInfo.level < Setting.LimitLevelFrom ||
	//	_StateController->PlayerInfo.PlayerInfo.level > Setting.LimitLevelTo)
	//{
	//	GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Create, TEXT("invalid level"), TEXT(""), 0, 0);
	//	return;
	//}

	FavaNetChannelSettingInfo &Info = _StateController->GetChannelSettingInfo();

	ROOM_SETTING setting;
	setting.idMap = RoomSetting.idMap;
	setting.tkLevel = (Info.EnableTKLevel ? RoomSetting.tkLevel : Info.DefaultTKLevel);
	setting.autoBalance = (Info.EnableAutoBalance ? RoomSetting.autoBalance : Info.DefaultAutoBalance);
	setting.allowSpectator = (Info.EnableSpectator ? RoomSetting.allowSpectator : Info.DefaultSpectator);
	setting.allowInterrupt = (Info.EnableInterrupt ? RoomSetting.allowInterrupt : Info.DefaultInterrupt);
	setting.allowBackView = (Info.EnableBackView ? RoomSetting.allowBackView : Info.DefaultBackView);
	setting.allowGhostChat = (Info.EnableGhostChat ? RoomSetting.allowGameGhostChat : Info.DefaultGhostChat);
	setting.roundToWin = (RoomSetting.roundToWin >= 0 ? RoomSetting.roundToWin : 6);	//! 호위임무에서 roundToWin은 0값을 사용한다.(2007/11/26 고광록)
	setting.numMax = (Info.DefaultMaxPlayers > 0 ? Info.DefaultMaxPlayers : RoomSetting.MaxPlayer > 0 ? RoomSetting.MaxPlayer : Def::MAX_PLAYER_PER_ROOM);
	setting.autoSwapTeam = (Info.EnableAutoSwapTeam ? RoomSetting.autoSwapTeam : Info.DefaultAutoSwapTeam);
	setting.mapOption = (MapInfo->AllowMapOption ? RoomSetting.mapOption : 0);

	//_StateController->RoomInfo.DumpRoomInfo();

	// 방에 들어가기 전에, 로비에서 병과를 변경했다면, 우선 서버에게 통보함
	_StateController->PlayerInfo.UpdateCurrentClass();

	PM::ROOM::CREATE_REQ::Send(RoomName, RoomPassword, setting);
}

UBOOL UavaNetRequest::JoinRoom(INT ListIndex,const FString& RoomPassword)
{
	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Join, TEXT("failed"), TEXT(""), 0, 0);
		return FALSE;
	}
	if (!_StateController->PlayerInfo.IsValid() || !_StateController->RoomList.RoomList.IsValidIndex(ListIndex) || _StateController->RoomInfo.IsValid())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Join, TEXT("failed"), TEXT(""), 0, 0);
		return FALSE;
	}

	FRoomDispInfo &Room = _StateController->RoomList.RoomList(ListIndex);

	//if (_StateController->PlayerInfo.PlayerInfo.level < Room.RoomInfo.setting.limitLevelFrom ||
	//	_StateController->PlayerInfo.PlayerInfo.level > Room.RoomInfo.setting.limitLevelTo)
	//{
	//	GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Join, TEXT("invalid level"), TEXT(""), 0, 0);
	//	return;
	//}

	//_StateController->RoomInfo.RoomInfo.idRoom = Room.RoomInfo.idRoom;
	appStrncpy(_StateController->RoomInfo.RoomInfo.roomName, Room.RoomInfo.roomName, SIZE_ROOM_NAME + 1);
	//_StateController->RoomInfo.RoomInfo.state.numCurr = 0;

	// 방에 들어가기 전에, 로비에서 병과를 변경했다면, 우선 서버에게 통보함
	_StateController->PlayerInfo.UpdateCurrentClass();

	PM::ROOM::JOIN_REQ::Send(Room.RoomInfo.idRoom, RoomPassword);

	return TRUE;
}

UBOOL UavaNetRequest::QuickJoinRoom(INT idMap)
{
	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Join, TEXT("failed"), TEXT(""), 0, 0);
		return FALSE;
	}

	// 퀵 조인은 서버가 처리함

	//TArray<INT> RoomList;
	//for (INT i = 0; i < _StateController->RoomList.RoomList.Num(); ++i)
	//{
	//	Def::ROOM_INFO &Info = _StateController->RoomList.RoomList(i).RoomInfo;
	//	if (Info.bPassword == 0 && Info.state.numCurr < Info.setting.numMax && (Info.setting.allowInterrupt > 0 || Info.state.playing == RIP_WAIT))
	//	{
	//		INT Idx = 0;
	//		for ( ; Idx < RoomList.Num(); ++Idx)
	//		{
	//			if (Info.state.numCurr > _StateController->RoomList.RoomList( RoomList(Idx) ).RoomInfo.state.numCurr )
	//				break;
	//		}

	//		RoomList.InsertItem(i, Idx);
	//	}
	//}

	////if (_StateController->RoomList.RoomList.Num() > 0)
	//if (RoomList.Num() > 0)
	//{
	//	for (INT i = 0; i < RoomList.Num(); ++i)
	//	{
	//		if ( !_StateController->IsHostUnreachable(RoomList(i)) )
	//		{
	//			return JoinRoom(RoomList(i), TEXT(""));
	//		}
	//	}

	//	//INT Idx = appRand() % RoomList.Num();//_StateController->RoomList.RoomList.Num();
	//	//return JoinRoom(RoomList(0), TEXT(""));
	//	//return JoinRoom(Idx, TEXT(""));
	//	//PM::ROOM::JOIN_REQ::Send(_StateController->RoomList.RoomList(Idx).RoomInfo.idRoom, TEXT(""));
	//}

	// 방에 들어가기 전에, 로비에서 병과를 변경했다면, 우선 서버에게 통보함
	_StateController->PlayerInfo.UpdateCurrentClass();

	PM::ROOM::QUICK_JOIN_REQ::Send(idMap);

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Room

void UavaNetRequest::LeaveRoom(BYTE LeavingReason)
{
	if (!_StateController->RoomInfo.IsValid() || !_StateController->PlayerInfo.IsValid())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Leave, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->IsCountingDown() && _StateController->RoomInfo.PlayerList.PlayerList[_StateController->MyRoomSlotIdx].RoomPlayerInfo.bReady > 0)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Leave, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (AmIHost() && _StateController->IsStatePlaying())
	{
		// 방장이 나갈려고 하면, 나가기 전에 자신의 전적을 서버에 보고
		GetAvaNetHandler()->eventSendPlayerResult(_StateController->PlayerInfo.PlayerInfo.idAccount);
	}

	switch (LeavingReason)
	{
	case ELR_Idle:
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Kick, TEXT("you"), TEXT("idle"), 0, 0);
		break;
	case ELR_DidntReady:
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Kick, TEXT("you"), TEXT("didn't ready"), 0, 0);
		break;
	case ELR_HostDidntStart:
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Kick, TEXT("you"), TEXT("didn't start"), 0, 0);
		break;
	case ELR_PackageMismatch:
		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("client package mismatch"), TEXT(""), 0, 0);
		break;
	case ELR_PackageNotFound:
		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("client package not found"), TEXT(""), 0, 0);
		break;
	case ELR_FailedToConnectHost:
		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("client failed to connect"), TEXT(""), 0, 0);
		break;
	case ELR_RejectedByHost:
		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("rejected by host"), TEXT(""), 0, 0);
		break;
	case ELR_MD5Failed:
		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("md5 failed"), TEXT(""), 0, 0);
		break;
	case ELR_P2PConnectionFailed:
		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("p2p connection failed"), TEXT(""), 0, 0);
		break;
	}

	PM::ROOM::LEAVE_REQ::Send(LeavingReason);
}

void UavaNetRequest::RoomKickPlayer(const FString& Nickname, INT reason)
{
	if (!_StateController->RoomInfo.IsValid() || (!_StateController->AmIHost() && !_StateController->AmIAdmin()))
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Kick, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->IsCountingDown() ||
		Nickname == _StateController->PlayerInfo.PlayerInfo.nickname)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Kick, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	FRoomPlayerInfo *Info = _StateController->RoomInfo.PlayerList.Find((FString&)Nickname);
	if (!Info)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Kick, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

#ifdef _APPLY_VOTE_FEE
	if (_StateController->GetNetState() == _AN_INGAME && !_StateController->AmIAdmin())
	{
		// 게임 중일 때에는 강퇴 비용 지불 (관리자 제외)
		if (_StateController->PlayerInfo.PlayerInfo.money < VOTE_FEE)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Kick, TEXT("not enough fee"), TEXT(""), 0, 0);
			return;
		}
	}
#endif

	PM::ROOM::KICK_PLAYER_NTF::Send(Info->RoomPlayerInfo.idSlot, Info->RoomPlayerInfo.idAccount, reason);
}

void UavaNetRequest::RoomSetting(FavaRoomSetting RoomSetting)
{
	if (!_StateController->RoomInfo.IsValid() || !_StateController->AmIHost() ||
		_StateController->IsCountingDown())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSetting, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	FMapInfo *MapInfo = _StateController->MapList.Find(RoomSetting.idMap);
	if (MapInfo == NULL)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSetting, TEXT("invalid map"), TEXT(""), 0, 0);
		return;
	}

	if (RoomSetting.idMap != _StateController->RoomInfo.RoomInfo.setting.idMap)
	{
		// 특정 맵만 선택 가능
		if (MapInfo->ExclChannelGroups.FindItemIndex(_StateController->ChannelInfo.Flag) != INDEX_NONE)
		{
			// 허용되지 않는 맵
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSetting, TEXT("invalid map"), TEXT(""), 0, 0);
			return;
		}
	}

	FavaNetChannelSettingInfo &Info = _StateController->GetChannelSettingInfo();

	ROOM_SETTING setting;
	setting.idMap = RoomSetting.idMap;
	setting.tkLevel = (Info.EnableTKLevel ? RoomSetting.tkLevel : Info.DefaultTKLevel);
	setting.autoBalance = (Info.EnableAutoBalance ? RoomSetting.autoBalance : Info.DefaultAutoBalance);
	setting.allowSpectator = (Info.EnableSpectator ? RoomSetting.allowSpectator : Info.DefaultSpectator);
	setting.allowInterrupt = (Info.EnableInterrupt ? RoomSetting.allowInterrupt : Info.DefaultInterrupt);
	setting.allowBackView = (Info.EnableBackView ? RoomSetting.allowBackView : Info.DefaultBackView);
	setting.allowGhostChat = (Info.EnableGhostChat ? RoomSetting.allowGameGhostChat : Info.DefaultGhostChat);
	setting.roundToWin = (RoomSetting.roundToWin >= 0 ? RoomSetting.roundToWin : 6);	//! 호위임무에서 roundToWin은 0값을 사용한다.(2007/11/26 고광록)
	setting.numMax = (Info.DefaultMaxPlayers > 0 ? Info.DefaultMaxPlayers : RoomSetting.MaxPlayer > 0 ? RoomSetting.MaxPlayer : Def::MAX_PLAYER_PER_ROOM);
	setting.autoSwapTeam = (Info.EnableAutoSwapTeam ? RoomSetting.autoSwapTeam : Info.DefaultAutoSwapTeam);
	setting.mapOption = (MapInfo->AllowMapOption ? RoomSetting.mapOption : 0);

	// 바꿀 것이 없으면 리턴
	if (appMemcmp(&setting, &_StateController->RoomInfo.RoomInfo.setting, sizeof(setting)) == 0)
		return;

	//_LOG(TEXT("Room Setting: %d %d %d %d %d %d"), setting.idMap, setting.tkLevel, setting.autoBalance, setting.allowInterrupt, setting.roundToWin, setting.numMax);

	PM::ROOM::CHANGE_SETTING_REQ::Send(setting);
}

void UavaNetRequest::RoomReady(UBOOL Ready)
{
	if (!_StateController->RoomInfo.IsValid() ||
		_StateController->AmISpectator())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Ready, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->AmIStealthMode())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Ready, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->RoomInfo.IsPlaying() || _StateController->IsCountingDown())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Ready, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	PM::ROOM::READY_NTF::Send(Ready ? 1 : 0);

	_StateController->SetRoomReadyDue();
}

void UavaNetRequest::RoomReadyToggle()
{
	if (!_StateController->RoomInfo.IsValid() || _StateController->MyRoomSlotIdx == ID_INVALID_ROOM_SLOT)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Ready, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->AmISpectator())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Ready, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->AmIStealthMode())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Ready, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->IsCountingDown() || _StateController->RoomInfo.IsPlaying())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Ready, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	FRoomPlayerInfo &Info = _StateController->RoomInfo.PlayerList.PlayerList[_StateController->MyRoomSlotIdx];
	if (!Info.IsValid() || Info.RoomPlayerInfo.bReady > 1)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Ready, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	PM::ROOM::READY_NTF::Send(Info.RoomPlayerInfo.bReady ? 0 : 1);

	_StateController->SetRoomReadyDue();
}

void UavaNetRequest::RoomChangeTeam(INT idTeam)
{
	FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
	if (!MyInfo ||
		!_StateController->RoomInfo.IsValid() ||
		_StateController->AmIStealthMode() ||
		_StateController->IsCountingDown() ||
		idTeam == MyInfo->GetTeamID() ||
		_StateController->GetChannelSetting(EChannelSetting_RoomChangeTeam) == 0
		)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSlot, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (idTeam < RT_EU || idTeam > RT_SPECTATOR)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSlot, TEXT("invalid team"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->ChannelInfo.IsMatchChannel())
	{
		// 대회 채널
		INT MaskLevel = _StateController->PlayerInfo.GetCurrentChannelMaskLevel();
		UBOOL CanMove = TRUE;
		if ((MaskLevel == _CML_NONE) ||
			(MaskLevel == _CML_PLAYER && idTeam == RT_SPECTATOR) ||
			((MaskLevel == _CML_REFREE || MaskLevel == _CML_BROADCAST) && (idTeam == RT_EU || idTeam == RT_NRF))
			)
		{
			// 선수는 관전 슬롯으로 이동할 수 없다.
			// 심판과 스탭은 선수 슬롯으로 이동할 수 없다.
			CanMove = FALSE;
		}
		if (!CanMove)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSlot, TEXT("no priv"), TEXT(""), 0, 0);
			return;
		}
	}

	//if (idTeam == RT_SPECTATOR && MyInfo->GetTeamID() == RT_SPECTATOR)
	//{
	//	GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSlot, TEXT("invalid spectator"), TEXT(""), 0, 0);
	//	return;
	//}
	if (idTeam == RT_SPECTATOR && _StateController->RoomInfo.RoomInfo.setting.allowSpectator == 0)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSlot, TEXT("spectator not allowed"), TEXT(""), 0, 0);
		return;
	}

	INT idSlot = _StateController->RoomInfo.FindEmptySlot(idTeam);

	if (idSlot != -1)
	{
		PM::ROOM::CHANGE_SLOT_NTF::Send(idSlot);
	}
	else
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSlot, TEXT("no empty slot"), TEXT(""), 0, 0);
	}
}

void UavaNetRequest::RoomChangeSlot(INT Slot)
{
	FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
	INT idTeam = FRoomInfo::SlotToTeam(Slot);
	if (!MyInfo ||
		!_StateController->RoomInfo.IsValid() ||
		_StateController->MyRoomSlotIdx == ID_INVALID_ROOM_SLOT ||
		_StateController->AmIStealthMode() ||
		_StateController->IsCountingDown() ||
		(_StateController->GetChannelSetting(EChannelSetting_RoomChangeTeam) == 0 && FRoomInfo::SlotToTeam(_StateController->MyRoomSlotIdx) != idTeam)
		)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSlot, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->ChannelInfo.IsMatchChannel())
	{
		// 대회 채널
		INT MaskLevel = _StateController->PlayerInfo.GetCurrentChannelMaskLevel();
		UBOOL CanMove = TRUE;
		if ((MaskLevel == _CML_NONE) ||
			(MaskLevel == _CML_PLAYER && idTeam == RT_SPECTATOR) ||
			((MaskLevel == _CML_REFREE || MaskLevel == _CML_BROADCAST) && (idTeam == RT_EU || idTeam == RT_NRF))
			)
		{
			// 선수는 관전 슬롯으로 이동할 수 없다.
			// 심판과 스탭은 선수 슬롯으로 이동할 수 없다.
			CanMove = FALSE;
		}
		if (!CanMove)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSlot, TEXT("no priv"), TEXT(""), 0, 0);
			return;
		}
	}

	if (!_StateController->RoomInfo.IsOpenSlot(Slot))
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSlot, TEXT("invalid slot"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->RoomInfo.PlayerList.IsEmpty(Slot))
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSlot, TEXT("slot is not empty"), TEXT(""), 0, 0);
		return;
	}

	//if (_StateController->AmIHost() && idTeam == RT_SPECTATOR)
	//	return;

	PM::ROOM::CHANGE_SLOT_NTF::Send(Slot);
}

void UavaNetRequest::RoomChangeTeamToggle()
{
	FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
	if (!MyInfo ||
		!_StateController->RoomInfo.IsValid() ||
		_StateController->AmIStealthMode() ||
		_StateController->IsCountingDown() ||
		_StateController->GetChannelSetting(EChannelSetting_RoomChangeTeam) == 0
		)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeSlot, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	RoomChangeTeam(MyInfo->GetTeamID() == RT_EU ? RT_NRF : RT_EU);
}

void UavaNetRequest::RoomChangeClass(INT idClass)
{
	if (idClass < _CLASS_POINTMAN || idClass > _CLASS_SNIPER)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeClass, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->AmIStealthMode())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeClass, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->GetNetState() == _AN_ROOM)
	{
		FRoomPlayerInfo *Info = _StateController->GetMyRoomPlayerInfo();
		if (!_StateController->RoomInfo.IsValid() ||
			!Info ||
			_StateController->AmISpectator() ||
			(_StateController->IsCountingDown() && Info->RoomPlayerInfo.bReady))
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ChangeClass, TEXT("failed"), TEXT(""), 0, 0);
			return;
		}

		PM::ROOM::CHANGE_CLASS_NTF::Send(idClass);
	}
	else// if (_StateController->GetNetState() == _AN_INVENTORY)
	{
		//PM::INVENTORY::CHANGE_CLASS_NTF::Send(idClass);

		_StateController->PlayerInfo.PlayerInfo.currentClass = idClass;
		//_StateController->CurrentClass = idClass;
	}
}

void UavaNetRequest::RoomStart()
{
	// 게임 시작 버튼을 눌렀을때 시작 혹은 난입 가능한지 체크
	FString ErrorTypeStr;
	if (!GetAvaNetHandler()->IsGameStartableEx( ErrorTypeStr ) )
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT("failed"), *ErrorTypeStr, 0, 0);
		return;
	}

	//if (_StateController->AmIStealthMode())
	//	return;

	if (_StateController->AmIHost())
	{
		PM::GAME::START_COUNT_NTF::Send();
	}
	else
	{
		if (_StateController->IsCountingDown())
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT("failed"), TEXT(""), 0, 0);
			return;
		}
		PM::GAME::START_COUNT_NTF::Send();
	}

	//if (_StateController->RoomInfo.IsValid())
	//{
	//	if (_StateController->AmIHost() && _StateController->RoomInfo.RoomInfo.state.playing == RIP_WAIT && _StateController->CountDown == 0)
	//	{
	//		if (_StateController->RoomInfo.RoomInfo.setting.allowInterrupt == 0)
	//		{
	//			for (INT i = 0; i < MAX_PLAYER_PER_ROOM; ++i)
	//			{
	//				if (_StateController->RoomInfo.PlayerList.IsEmpty(i) || _StateController->RoomInfo.HostIdx == i)
	//					continue;
	//				if (_StateController->RoomInfo.PlayerList.PlayerList[i].RoomPlayerInfo.bReady != _READY_WAIT)
	//				{
	//					GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT("all players must ready"), TEXT(""), 0, 0);
	//					return;
	//				}
	//			}
	//		}

	//		//PM::GAME::START_NTF::Send();
	//		PM::GAME::START_COUNT_NTF::Send();
	//	}
	//	else if (!_StateController->AmIHost() && _StateController->RoomInfo.RoomInfo.state.playing == RIP_PLAYING &&
	//		_StateController->CountDown == 0 && _StateController->RoomInfo.RoomInfo.setting.allowInterrupt > 0)
	//	{
	//		//PM::GAME::START_NTF::Send();
	//		//_StateController->CountDown = 6;
	//		//GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_StartCount, TEXT(""), TEXT(""), 0, 0);
	//		FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
	//		if (!MyInfo || MyInfo->RoomPlayerInfo.bReady != _READY_NONE)
	//			return;
	//		MyInfo->RoomPlayerInfo.bReady = _READY_WAIT;
	//		PM::GAME::START_COUNT_NTF::Send();
	//	}
	//}
}

void UavaNetRequest::RoomCancelStart()
{
	if (_StateController->RoomInfo.IsValid())
	{
		// Host는 RIP_WAIT상태일때 Cancel이 오고,
		// Client(Spectator)는 RIP_PLAYING상태일 때 Cancel이 온다.(2007/02/23 고광록)
		if ( (_StateController->RoomInfo.RoomInfo.state.playing == RIP_WAIT || _StateController->RoomInfo.RoomInfo.state.playing == RIP_PLAYING) && _StateController->IsCountingDown())
		{
			_StateController->CancelCountDown();

			if (_StateController->AmIHost())
			{
				PM::GAME::CANCEL_COUNT_NTF::Send();
			}
			else
			{
				FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
				if (MyInfo)
				{
					MyInfo->RoomPlayerInfo.bReady = _READY_NONE;
					_StateController->SetRoomReadyDue();
				}
			}

			return;
		}
	}

	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_CancelCount, TEXT("failed"), TEXT(""), 0, 0);
}

void UavaNetRequest::RoomSwapTeam(INT Reason)
{
	if (_StateController->AmIStealthMode())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_SwapTeam, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (!_StateController->IsStateInRoom() || !_StateController->RoomInfo.IsValid())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_SwapTeam, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->AmIHost() || _StateController->AmIAdmin())
	{
		PM::ROOM::SWAP_TEAM_NTF::Send(Reason);
	}
}

void UavaNetRequest::RoomClaimHost()
{
	if (_StateController->AmIStealthMode())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ClaimHost, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (!_StateController->IsStateInRoom() || !_StateController->RoomInfo.IsValid())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ClaimHost, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->IsStatePlaying())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ClaimHost, TEXT("playing"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->PlayerInfo.GetCurrentChannelMaskLevel() != _CML_REFREE)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_ClaimHost, TEXT("no priv"), TEXT(""), 0, 0);
		return;
	}

	PM::ROOM::CLAIM_HOST_NTF::Send();
}

void UavaNetRequest::SetMyRttValue(FLOAT RttValue)
{
	if ( !(_StateController->RoomInfo.IsValid() && !_StateController->AmIHost() && !_StateController->AmIStealthMode()) )
		return;

	FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
	if (MyInfo)
	{
		MyInfo->RoomPlayerInfo.rttScore = RttValue;
		MyInfo->RttValue = RttValue;
	}

	PM::ROOM::RTT_UPDATE_NTF::Send(RttValue);
	GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_RttRating, TEXT("end"), TEXT(""), 0, 0);

	if (RttValue == -2 && !_StateController->RoomInfo.PlayerList.IsEmpty(_StateController->RoomInfo.HostIdx))
	{
		_StateController->LogChatConsole(*FString::Printf(
			*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_RttInvalidPing"), TEXT("AVANET")),
			_StateController->RoomInfo.PlayerList.PlayerList[_StateController->RoomInfo.HostIdx].RoomPlayerInfo.nickname),
			EChat_ReadyRoom);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lobby & Room

void UavaNetRequest::Chat(const FString &ChatMsg)
{
	if (_StateController->AmIStealthMode())
		return;

	if (ChatMsg.Len() == 0)
		return;

	// parse chat commands
	if (ParseChatCommand(*ChatMsg))
	{
		return;
	}

	// send chat
	TCHAR *Str = const_cast<TCHAR*>(*ChatMsg);

	_WordCensor().ReplaceChatMsg(Str);
	if (appStrlen(Str) == 0)
		return;

	if (_StateController->CheckChatBlocked(Str))
		return;

	FString ParsedStr(Str);
	if (_StateController->AmIAdmin() ||
		(_StateController->ChannelInfo.IsMaskTurnedOn() && _StateController->PlayerInfo.GetCurrentChannelMaskLevel() >= _CML_REFREE))
	{
		ParsedStr = FString::Printf(TEXT("\n%d\n%s"), (INT)EChat_GMWhisper, Str);
	}

	if (_StateController->IsStateInRoom())
	{
		PM::ROOM::CHAT_NTF::Send(*ParsedStr);
	}
	else if (_StateController->GetNetState() == _AN_LOBBY)
	{
		PM::CHANNEL::LOBBY_CHAT_NTF::Send(*ParsedStr);
	}

	_StateController->SentChatMsgList.Add(Str);
	while (_StateController->SentChatMsgList.Num() > 5)
		_StateController->SentChatMsgList.ChatList.RemoveNode(_StateController->SentChatMsgList.ChatList.GetHead());

	_StateController->CheckChatProhibition();
}

UBOOL UavaNetRequest::ParseChatCommand(const FString &Cmd)
{
	return _StateController->ParseChatCommand(*Cmd);
}

void UavaNetRequest::FilterChatMsg(FString &ChatMsg)
{
	if (ChatMsg.Len() == 0)
		return;

	TCHAR *Str = (TCHAR*)*ChatMsg;
	_WordCensor().ReplaceChatMsg(Str);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// In-game

void UavaNetRequest::LeaveGame()
{
	if (!GavaNetClient->IsValid())
	{
		GEngine->Exec(TEXT("QUIT"));
		return;
	}

	if (!_StateController->RoomInfo.IsValid())
		return;

	GetAvaNetHandler()->RoomStartingPlayerList.Empty();

	GCallbackEvent->Send(CALLBACK_PlayerListUpdated);

	_StateController->LastResultInfo.Clear();

#ifndef EnableHostMigration
	if (AmIHost())
	{
		TArray<FavaPlayerScoreInfo> arr;
		GetAvaNetHandler()->EndGame(arr);
	}
	else
#endif
	{
		//if (AmIHost())
		//{
		//	LeaveRoom();
		//}
		//else
		{
			PM::GAME::LEAVE_NTF::Send();

			//++_StateController->PlayerInfo.PlayerInfo.scoreInfo.disconnectCount;

			_LOG(TEXT("Returning to avaEntry..."));
			GetAvaNetHandler()->DisconnectFromGame(_AN_ROOM);
			_StateController->SetRoomReadyDue();
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Communication

void UavaNetRequest::BuddyListBegin()
{
	_Communicator().BeginBuddyList();
}

void UavaNetRequest::BuddyListEnd()
{
	_Communicator().EndBuddyList();
}

void UavaNetRequest::BuddyAdd(UBOOL bForce, const FString &Nickname)
{
	_Communicator().AddBuddy(bForce, ID_INVALID_ACCOUNT, Nickname);
}

void UavaNetRequest::BuddyAddAns(UBOOL bAccept, const FString &Nickname)
{
	_Communicator().AddBuddyAns(bAccept, Nickname);
}

void UavaNetRequest::BuddyDelete(const FString &Nickname)
{
	_Communicator().DeleteBuddy(Nickname);
}

void UavaNetRequest::BlockAdd(UBOOL bForce, const FString &Nickname)
{
	_Communicator().AddBlock(bForce, ID_INVALID_ACCOUNT, Nickname);
}

void UavaNetRequest::BlockDelete(const FString &Nickname)
{
	_Communicator().DeleteBlock(Nickname);
}

void UavaNetRequest::InviteGame(const FString &InNickname)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	if (_StateController->GetNetState() < _AN_CHANNELLIST)
		return;

	return;

	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;

	if (!_StateController->IsStateInRoom())
	{
		// 방에 있지 않다면 초대할 수 없음
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_GameInvitationNotAllowed"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	if (Nickname == _StateController->PlayerInfo.PlayerInfo.nickname)
	{
		// 자기 자신은 초대할 수 없음
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotInviteYourself"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	FPlayerDispInfo *Player = _StateController->FindPlayerFromList(Nickname);
	if (Player)
	{
		// 이미 같은 위치에 있음
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_SameLocationWithPlayer"), TEXT("AVANET")), Player->PlayerInfo.nickname),
										EChat_PlayerSystem);
		return;
	}

	if (_StateController->GuildInfo.IsValid())
	{
		// 클랜원 목록을 찾아봄
		FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(Nickname);
		if (Info)
		{
			// 클랜원 목록에서 발견
			if (Info->IsOnline())
			{
				// CLS를 통해서 게임 초대
				//PM::GUILD::INVITE_GAME_REQ::Send(Info->GuildPlayerInfo.idAccount);
			}
			else
			{
				// 오프라인
				_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerOffline"), TEXT("AVANET")), *Nickname),
												EChat_PlayerSystem);
			}
			return;
		}
	}

	// 클랜원 목록에 없음

	FBuddyInfo *Buddy = _Communicator().BuddyList.Find(Nickname);
	if (Buddy)
	{
		// 상대방이 친구 목록에 있음
		if (Buddy->IsBuddyBoth())
		{
			// CMS를 통해서 게임 초대
			_Communicator().InviteGame(Buddy->idAccount);
		}
		else
		{
			// 양방향 친구가 아님
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_NotBothType"), TEXT("AVANET")), *Nickname),
											EChat_PlayerSystem);
		}
		return;
	}

	// 친구/클랜원 목록에 없으므로 초대 불가
	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Nickname),
									EChat_PlayerSystem);
}

void UavaNetRequest::InviteGameAns(UBOOL bAccept, const FString &Nickname)
{
	_Communicator().InviteGameAns(bAccept, Nickname);
}

void UavaNetRequest::FollowPlayer(const FString &InNickname)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	if (_StateController->GetNetState() < _AN_CHANNELLIST)
		return;

	FString Nickname = ((FString)InNickname).Trim().TrimTrailing();
	if (Nickname.Len() == 0)
		return;

	if (_StateController->IsStatePlaying())
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowInGame"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	if (_StateController->GetNetState() == _AN_ROOM)
	{
		FRoomPlayerInfo *Info = _StateController->GetMyRoomPlayerInfo();
		if (Info && Info->IsReady())
		{
			// 레디 상태이면 따라가기 불가
			_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowInReadyState"), TEXT("AVANET")),
											EChat_PlayerSystem);
			return;
		}
	}

	if (Nickname == _StateController->PlayerInfo.PlayerInfo.nickname)
	{
		// 자기 자신은 따라갈 수 없음
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_CannotFollowYourself"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	FPlayerDispInfo *Player = _StateController->FindPlayerFromList(Nickname);
	if (Player)
	{
		// 이미 같은 위치에 있음
		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_SameLocationWithPlayer"), TEXT("AVANET")), Player->PlayerInfo.nickname),
										EChat_PlayerSystem);
		return;
	}

	FBuddyInfo *Buddy = _Communicator().BuddyList.Find(Nickname);

	if (_Communicator().bListingBuddy)
	{
		// 친구 탭을 열어놓았으므로 이 쪽으로 먼저 시도
		if (Buddy && Buddy->IsBuddyBoth())
		{
			if (Buddy->IsOnline())
			{
				TID_CHANNEL idChannel = (Buddy->ServerType == FChannelInfo::CT_GUILD ? ID_MY_CLAN_HOME : Buddy->idChannel);
				_Communicator().FollowPlayer(Buddy->idAccount, Buddy->Nickname, Buddy->idGuild, idChannel);
			}
			else
			{
				// 오프라인
				_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerOffline"), TEXT("AVANET")), *Nickname),
												EChat_PlayerSystem);
			}
			return;
		}
	}

	// 친구 탭을 열어놓지 않았거나, 상대방이 친구 목록에 없음

	if (_StateController->GuildInfo.IsValid())
	{
		// 클랜원 목록을 찾아봄
		FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(Nickname);
		if (Info)
		{
			// 클랜원 목록에서 발견
			if (Info->IsOnline())
			{
				// CLS에게 위치 확인
				_StateController->AutoMoveDest.SetFollowTarget(Info->GuildPlayerInfo.idAccount, Nickname);
				PM::GUILD::PLAYER_LOCATION_REQ::Send(Info->GuildPlayerInfo.idAccount);
			}
			else
			{
				// 오프라인
				_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerOffline"), TEXT("AVANET")), *Nickname),
												EChat_PlayerSystem);
			}
			return;
		}
	}

	// 클랜원 목록에도 없음

	if (Buddy)
	{
		// 친구 탭을 열어놓지는 않았지만, 상대방이 친구 목록에 있음
		if (Buddy->IsBuddyBoth())
		{
			// CMS에게 위치 확인
			_StateController->AutoMoveDest.SetFollowTarget(Buddy->idAccount, Nickname);
			_Communicator().GetLocationInfo(Buddy->idAccount);
		}
		else
		{
			// 양방향 친구가 아님
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_NotBothType"), TEXT("AVANET")), *Nickname),
											EChat_PlayerSystem);
		}
		return;
	}

	// 친구/클랜원 목록에 없으므로 따라가기 불가
	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerNotFound"), TEXT("AVANET")), *Nickname),
									EChat_PlayerSystem);
}

void UavaNetRequest::SetFollowPlayerPwd(const FString &Password)
{
	if (Password.Len() == 0)
	{
		_LOG(TEXT("Canceling follow action"));
		_StateController->AutoMoveDest.Clear();
		return;
	}
	if (!_StateController->AutoMoveDest.IsFollowing())
	{
		_LOG(TEXT("No target to follow"));
		return;
	}

	_StateController->AutoMoveDest.Continue(Password);
	_StateController->ProcAutoMove();
}

void UavaNetRequest::InviteGuild(const FString &Nickname)
{
	_Communicator().InviteGuild(Nickname);
}

void UavaNetRequest::InviteGuildAns(UBOOL bAccept, const FString &Nickname)
{
	_Communicator().InviteGuildAns(bAccept, Nickname);
}

void UavaNetRequest::Whisper(const FString &Nickname, const FString &ChatMsg)
{
	_Communicator().Whisper(Nickname, ChatMsg);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Inventory

void UavaNetRequest::EnterInven()
{
	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Enter, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->AmIStealthMode())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Enter, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	PM::INVENTORY::ENTER_NTF::Send();
	_StateController->GoToState(_AN_INVENTORY);
}

void UavaNetRequest::LeaveInven()
{
	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Leave, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->AmIStealthMode())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Leave, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	PM::INVENTORY::LEAVE_NTF::Send();
	_StateController->GoToState(_AN_LOBBY);
}


void UavaNetRequest::InvenSetWeapon(INT EquipSlot, INT InvenSlot)
{
	if (_StateController->GetNetState() != _AN_INVENTORY && _StateController->GetNetState() != _AN_ROOM)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid() || (InvenSlot = GetInvenIndexFromSlot( InvenSlot )) < 0 )
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	ITEM_DESC *ItemDesc = _ItemDesc().GetItem(_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot].id);
	if (!ItemDesc)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("invalid item"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->PlayerInfo.PlayerInfo.level < ItemDesc->useLimitLevel)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("low level"), TEXT(""), 0, 0);
		return;
	}

	SLOT_DESC *SlotDesc = NULL;
	if (EquipSlot >= 0 && EquipSlot < MAX_WEAPONSET_SIZE)
	{
		SlotDesc = _ItemDesc().GetWeaponSlot(EquipSlot);
	}
	else
	{
		SlotDesc = _ItemDesc().GetWeaponSlotByType(ItemDesc->slotType);
		if (SlotDesc)
			EquipSlot = SlotDesc->index;
	}
	if (!SlotDesc)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("invalid item"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[EquipSlot] == _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot].sn)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("same item"), TEXT(""), 0, 0);
		return;
	}

	PM::INVENTORY::WEAPONSET_REQ::Send(EquipSlot, _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot].sn);
}


void UavaNetRequest::InvenSetEquip(INT EquipSlot, INT InvenSlot)
{
	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	InvenSlot %= 10000;

	if (!_StateController->PlayerInfo.IsValid() || InvenSlot < 0 || InvenSlot >= Def::MAX_INVENTORY_SIZE)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	ITEM_DESC *ItemDesc = _ItemDesc().GetItem(_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[InvenSlot].id);
	if (!ItemDesc)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("invalid item"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->PlayerInfo.PlayerInfo.level < ItemDesc->useLimitLevel)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("low level"), TEXT(""), 0, 0);
		return;
	}

	SLOT_DESC *SlotDesc = NULL;
	if (EquipSlot >= 0 && EquipSlot < MAX_EQUIPSET_SIZE)
	{
		SlotDesc = _ItemDesc().GetEquipSlot(EquipSlot);
	}
	else
	{
		SlotDesc = _ItemDesc().GetEquipSlotByType(ItemDesc->slotType);
		if (SlotDesc)
			EquipSlot = SlotDesc->index;
	}
	if (!SlotDesc)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("invalid item"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->PlayerInfo.PlayerInfo.itemInfo.equipSet[EquipSlot] == _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[InvenSlot].sn)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("same item"), TEXT(""), 0, 0);
		return;
	}

	PM::INVENTORY::EQUIPSET_REQ::Send(EquipSlot, _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[InvenSlot].sn);
}

void UavaNetRequest::InvenSetCustom(INT InvenSlot, INT CustomIndex, INT OptionIndex)
{
	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid() || (InvenSlot = GetInvenIndexFromSlot(InvenSlot)) < 0)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	ITEM_INFO &Item = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot];
	if (Item.IsEmpty())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("empty inventory slot"), TEXT(""), 0, 0);
		return;
	}

	FavaShopItem *pShopItem = _ShopDesc().GetCustomItemByIndex(CustomIndex);
	if (!pShopItem || OptionIndex >= pShopItem->Options.Num())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("invalid item"), TEXT(""), 0, 0);
		return;
	}

	CUSTOM_ITEM_DESC *pDesc = static_cast<CUSTOM_ITEM_DESC*>(pShopItem->Options(OptionIndex).pItem);
	if (!pDesc || pDesc->item_id != Item.id)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("invalid item"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->PlayerInfo.PlayerInfo.level < pDesc->useLimitLevel)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("low level"), TEXT(""), 0, 0);
		return;
	}

	CInventory &Inven = _StateController->PlayerInfo.Inven;
	if (Inven.GetCustomInvenSize() == MAX_CUSTOM_INVENTORY_SIZE)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("full"), TEXT(""), 0, 0);
		return;
	}

	TMONEY RefundMoney = 0;
	CUSTOM_ITEM_INFO *pOldItem = Inven.GetCustomInvenToSlot(Item.sn, pDesc->customType);
	if (pOldItem)
	{
		CUSTOM_ITEM_DESC *pOldDesc = _ItemDesc().GetCustomItem(pOldItem->id);
		if (pOldDesc->priceType == _IPT_MONEY)
			RefundMoney = Inven.GetCustomItemRefundMoney(Item.sn, pDesc->customType);
	}

	if (pDesc->priceType == _IPT_MONEY)
	{
		if (_StateController->PlayerInfo.PlayerInfo.money + RefundMoney < pDesc->price)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("no money"), TEXT(""), 0, 0);
			return;
		}
	}

	PM::INVENTORY::CUSTOMSET_REQ::Send(Item.sn, pDesc->id, pDesc->customType);
}

void UavaNetRequest::InvenSetEffect(INT EquipSlot, INT InvenSlot)
{
	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid() || InvenSlot < 0 || InvenSlot >= MAX_EFFECT_INVENTORY_SIZE)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	EFFECT_ITEM_INFO &Item = _StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[InvenSlot];
	EFFECT_ITEM_DESC *ItemDesc = _ItemDesc().GetEffectItem(Item.id);
	if (!ItemDesc)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffSet, TEXT("invalid item"), TEXT(""), 0, 0);
		return;
	}

	SLOT_DESC *SlotDesc = NULL;
	if (EquipSlot >= 0 && EquipSlot < MAX_EFFECTSET_SIZE)
	{
		SlotDesc = _ItemDesc().GetEffectSlot(EquipSlot);
	}
	else
	{
		SlotDesc = _ItemDesc().GetEffectSlotByType(ItemDesc->slotType);
		if (SlotDesc)
			EquipSlot = SlotDesc->index;
	}
	if (!SlotDesc)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffSet, TEXT("invalid item"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->PlayerInfo.PlayerInfo.itemInfo.effectSet[EquipSlot] == Item.item_sn)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffSet, TEXT("same item"), TEXT(""), 0, 0);
		return;
	}

	PM::INVENTORY::EFFSET_REQ::Send(EquipSlot, Item.item_sn);
}

void UavaNetRequest::InvenUnsetWeapon(INT EquipSlot, INT InvenSlot)
{
	if (_StateController->GetNetState() != _AN_INVENTORY && _StateController->GetNetState() != _AN_ROOM)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid() || (InvenSlot = GetInvenIndexFromSlot(InvenSlot)) < 0)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	CInventory &Inven = _StateController->PlayerInfo.Inven;

	if ((EquipSlot < 0 || EquipSlot >= MAX_WEAPONSET_SIZE) && InvenSlot >= 0 && InvenSlot < MAX_INVENTORY_SIZE)
	{
		EquipSlot = Inven.GetWeaponSlot(_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot].sn);
		//for (INT i = 0; i < MAX_WEAPONSET_SIZE; ++i)
		//{
		//	if (_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[i] == _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot].sn)
		//	{
		//		EquipSlot = i;
		//		break;
		//	}
		//}
	}

	if (EquipSlot >= 0 && EquipSlot < MAX_WEAPONSET_SIZE)
	{
		SLOT_DESC *SlotDesc = _ItemDesc().GetWeaponSlot(EquipSlot);
		ITEM_INFO *Item = Inven.GetWeaponSet(EquipSlot);

		if (SlotDesc && Item)
		{
			if (SlotDesc->defaultItem != ID_INVALID_ITEM && SlotDesc->defaultItem == Item->id)
			{
				// already equipping default item of the slot
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("cannot unset default"), TEXT(""), 0, 0);
				return;
			}

			PM::INVENTORY::WEAPONSET_REQ::Send(EquipSlot, SN_INVALID_ITEM);
			return;
			//TSN_ITEM ItemSN = SN_INVALID_ITEM;

			//if (SlotDesc->defaultItem != ID_INVALID_ITEM)
			//{
			//	for (INT i = 0; i < MAX_INVENTORY_SIZE; ++i)
			//	{
			//		if (_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[i].id == SlotDesc->defaultItem)
			//		{
			//			ItemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[i].sn;
			//			break;
			//		}
			//	}

			//	check(ItemSN != SN_INVALID_ITEM);
			//}

			//if (_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[EquipSlot] != ItemSN)
			//{
			//	PM::INVENTORY::WEAPONSET_REQ::Send(EquipSlot, ItemSN);
			//	_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[EquipSlot] = ItemSN;
			//}
		}
	}

	GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("invalid item"), TEXT(""), 0, 0);
}


void UavaNetRequest::InvenUnsetEquip(INT EquipSlot, INT InvenSlot)
{
	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	InvenSlot = GetInvenIndexFromSlot(InvenSlot);

	CInventory &Inven = _StateController->PlayerInfo.Inven;

	if ((EquipSlot < 0 || EquipSlot >= MAX_EQUIPSET_SIZE) && InvenSlot >= 0 && InvenSlot < MAX_INVENTORY_SIZE)
	{
		EquipSlot = Inven.GetEquipSlot(_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[InvenSlot].sn);
		//for (INT i = 0; i < MAX_EQUIPSET_SIZE; ++i)
		//{
		//	if (_StateController->PlayerInfo.PlayerInfo.itemInfo.equipSet[i] == _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[InvenSlot].sn)
		//	{
		//		EquipSlot = i;
		//		break;
		//	}
		//}
	}

	if (EquipSlot >= 0 && EquipSlot < MAX_EQUIPSET_SIZE)
	{
		SLOT_DESC *SlotDesc = _ItemDesc().GetEquipSlot(EquipSlot);
		ITEM_INFO *Item = Inven.GetEquipSet(EquipSlot);

		if (SlotDesc && Item)
		{
			if (SlotDesc->defaultItem != ID_INVALID_ITEM && SlotDesc->defaultItem == Item->id)
			{
				// already equipping default item of the slot
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("cannot unset default"), TEXT(""), 0, 0);
				return;
			}

			PM::INVENTORY::EQUIPSET_REQ::Send(EquipSlot, SN_INVALID_ITEM);
			//TSN_ITEM ItemSN = SN_INVALID_ITEM;

			//if (SlotDesc->defaultItem != ID_INVALID_ITEM)
			//{
			//	for (INT i = 0; i < MAX_INVENTORY_SIZE; ++i)
			//	{
			//		if (_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[i].id == SlotDesc->defaultItem)
			//		{
			//			ItemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[i].sn;
			//			break;
			//		}
			//	}

			//	check(ItemSN != SN_INVALID_ITEM);
			//}

			//if (_StateController->PlayerInfo.PlayerInfo.itemInfo.equipSet[EquipSlot] != ItemSN)
			//{
			//	PM::INVENTORY::EQUIPSET_REQ::Send(EquipSlot, ItemSN);
			//	//_StateController->PlayerInfo.PlayerInfo.itemInfo.equipSet[EquipSlot] = ItemSN;
			//}
		}
	}

	GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("invalid item"), TEXT(""), 0, 0);
}

void UavaNetRequest::InvenUnsetCustom(INT InvenSlot, INT CustomSlot)
{
	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid() || (InvenSlot = GetInvenIndexFromSlot(InvenSlot)) < 0 || CustomSlot < 0 || CustomSlot >= _CSI_MAX)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	ITEM_INFO &Item = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot];
	if (Item.IsEmpty())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("already empty slot"), TEXT(""), 0, 0);
		return;
	}

	CInventory &Inven = _StateController->PlayerInfo.Inven;

	CUSTOM_ITEM_INFO *pCustomItem = Inven.GetCustomInvenToSlot(Item.sn, (Def::CUSTOM_SLOT_IDX)CustomSlot);

	if (!pCustomItem || pCustomItem->item_sn != Item.sn || pCustomItem->slot != CustomSlot)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("invalid item"), TEXT(""), 0, 0);
		return;
	}

	PM::INVENTORY::CUSTOMSET_REQ::Send(Item.sn, ID_INVALID_ITEM, CustomSlot);
}

void UavaNetRequest::InvenUnsetEffect(INT EquipSlot, INT InvenSlot)
{
	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid() || InvenSlot < 0 || InvenSlot >= MAX_EFFECT_INVENTORY_SIZE)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffSet, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	CInventory &Inven = _StateController->PlayerInfo.Inven;

	if ((EquipSlot < 0 || EquipSlot >= MAX_EFFECTSET_SIZE) && InvenSlot >= 0 && InvenSlot < MAX_EFFECT_INVENTORY_SIZE)
	{
		EquipSlot = Inven.GetEffectSlot(_StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[InvenSlot].item_sn);
	}

	if (EquipSlot >= 0 && EquipSlot < MAX_EFFECTSET_SIZE)
	{
		SLOT_DESC *SlotDesc = _ItemDesc().GetEffectSlot(EquipSlot);
		EFFECT_ITEM_INFO *Item = Inven.GetEffectSet(EquipSlot);

		if (SlotDesc && Item)
		{
			if (SlotDesc->defaultItem != ID_INVALID_ITEM && SlotDesc->defaultItem == Item->id)
			{
				// already equipping default item of the slot
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffSet, TEXT("cannot unset default"), TEXT(""), 0, 0);
				return;
			}

			PM::INVENTORY::EFFSET_REQ::Send(EquipSlot, SN_INVALID_ITEM);
			return;
		}
	}

	GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffSet, TEXT("invalid item"), TEXT(""), 0, 0);
}

void UavaNetRequest::InvenRepairWeapon(INT InvenSlot)
{
	if (_StateController->GetNetState() != _AN_INVENTORY && _StateController->GetNetState() != _AN_ROOM)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid() || (InvenSlot = GetInvenIndexFromSlot( InvenSlot )) < 0 )
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	ITEM_INFO &Item = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot];
	if (Item.IsEmpty())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("empty slot"), TEXT(""), 0, 0);
		return;
	}

	ITEM_DESC *ItemDesc = _ItemDesc().GetItem(Item.id);
	if (!ItemDesc)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("invalid item"), TEXT(""), 0, 0);
		return;
	}
	if (ItemDesc->gaugeType != _IGT_MAINTENANCE)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("cannot repair"), TEXT(""), 0, 0);
		return;
	}

	CInventory &Inven = _StateController->PlayerInfo.Inven;

	TMONEY Cost = Inven.GetItemRepairMoney(Item.sn);
	if (Cost == 0)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("already repaired"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->PlayerInfo.PlayerInfo.money < Cost)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("no money"), TEXT(""), 0, 0);
		return;
	}

	PM::INVENTORY::REPAIR_REQ::Send(Item.sn);
}

void UavaNetRequest::InvenRepairEquip(INT InvenSlot)
{
	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid() || (InvenSlot = GetInvenIndexFromSlot(InvenSlot)) < 0 )
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	ITEM_INFO &Item = _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[InvenSlot];
	if (Item.IsEmpty())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("empty slot"), TEXT(""), 0, 0);
		return;
	}

	ITEM_DESC *ItemDesc = _ItemDesc().GetItem(Item.id);
	if (!ItemDesc)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("invalid item"), TEXT(""), 0, 0);
		return;
	}
	if (ItemDesc->gaugeType != _IGT_MAINTENANCE)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("cannot repair"), TEXT(""), 0, 0);
		return;
	}

	CInventory &Inven = _StateController->PlayerInfo.Inven;

	TMONEY Cost = Inven.GetItemRepairMoney(Item.sn);
	if (Cost == 0)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("already repaired"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->PlayerInfo.PlayerInfo.money < Cost)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("no money"), TEXT(""), 0, 0);
		return;
	}

	PM::INVENTORY::REPAIR_REQ::Send(Item.sn);
}

void UavaNetRequest::InvenConvertRIS(INT InvenSlot)
{
	if (_StateController->GetNetState() != _AN_INVENTORY && _StateController->GetNetState() != _AN_ROOM)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_ConvertRIS, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid() || (InvenSlot = GetInvenIndexFromSlot(InvenSlot)) < 0)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_ConvertRIS, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	ITEM_INFO &Item = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot];
	if (Item.IsEmpty())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_ConvertRIS, TEXT("invalid slot"), TEXT(""), 0, 0);
		return;
	}

	ITEM_DESC *Desc = _ItemDesc().GetItem(Item.id);
	if (!Desc)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_ConvertRIS, TEXT("invalid slot"), TEXT(""), 0, 0);
		return;
	}

	if (!Desc->bRisConvertible)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_ConvertRIS, TEXT("not convertible"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->PlayerInfo.PlayerInfo.money < Desc->RisConvertiblePrice)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_ConvertRIS, TEXT("no money"), TEXT(""), 0, 0);
		return;
	}

	PM::INVENTORY::CONVERT_RIS_REQ::Send(Item.sn);
}

UBOOL UavaNetRequest::InvenGetWeaponRefundPrice(INT InvenSlot, INT &Money, INT &Cash)
{
	if (!_StateController->PlayerInfo.IsValid() || (InvenSlot = GetInvenIndexFromSlot(InvenSlot)) < 0)
	{
		Money = Cash = 0;
		return FALSE;
	}

	ITEM_INFO &Item = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot];
	if (Item.IsEmpty())
	{
		Money = Cash = 0;
		return FALSE;
	}

	CInventory &Inven = _StateController->PlayerInfo.Inven;
	Money = (INT)Inven.GetItemRefundMoney(Item.sn);
	//Cash = ;

	return TRUE;
}

UBOOL UavaNetRequest::InvenGetEquipRefundPrice(INT InvenSlot, INT &Money, INT &Cash)
{
	if (!_StateController->PlayerInfo.IsValid() || (InvenSlot = GetInvenIndexFromSlot(InvenSlot)) < 0)
	{
		Money = Cash = 0;
		return FALSE;
	}

	ITEM_INFO &Item = _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[InvenSlot];
	if (Item.IsEmpty())
	{
		Money = Cash = 0;
		return FALSE;
	}

	CInventory &Inven = _StateController->PlayerInfo.Inven;
	Money = (INT)Inven.GetItemRefundMoney(Item.sn);
	//Cash = ;

	return TRUE;
}

UBOOL UavaNetRequest::InvenGetCustomRefundPrice(INT InvenSlot, INT CustomSlot, INT &Money, INT &Cash)
{
	if (!_StateController->PlayerInfo.IsValid() || (InvenSlot = GetInvenIndexFromSlot(InvenSlot)) < 0 || CustomSlot < 0 || CustomSlot >= _CSI_MAX)
	{
		Money = Cash = 0;
		return FALSE;
	}

	ITEM_INFO &Item = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot];
	if (Item.IsEmpty())
	{
		Money = Cash = 0;
		return FALSE;
	}

	CInventory &Inven = _StateController->PlayerInfo.Inven;
	Money = Inven.GetCustomItemRefundMoney(Item.sn, (CUSTOM_SLOT_IDX)CustomSlot);
	//Cash = ;

	_LOG(TEXT("InvenGetCustomRefundPrice(): InvenSlot = %d, SN = %I64u, CustomSlot = %d, Money = %d"), InvenSlot, Item.sn, CustomSlot, Item.sn, Money);

	return TRUE;
}

INT UavaNetRequest::GetUsedItemEffect(BYTE ItemEffect)
{
	if (_StateController->PlayerInfo.IsValid())
		return _StateController->PlayerInfo.Inven.GetUsedEffect((ITEM_EFFECT_TYPE)ItemEffect);
	return 0;
}

void UavaNetRequest::ShopBuyWeapon(INT ListIndex, INT OptionIndex)
{
	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid() || ListIndex < 0)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	FavaShopItem *pShopItem = _ShopDesc().GetItemByIndex(ListIndex);
	if (!pShopItem || OptionIndex >= pShopItem->Options.Num())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("invalid"), TEXT(""), 0, 0);
		return;
	}

	ITEM_DESC *ItemDesc = static_cast<ITEM_DESC*>(pShopItem->Options(OptionIndex).pItem);

	if (!ItemDesc)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("invalid"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->PlayerInfo.PlayerInfo.level < ItemDesc->useLimitLevel)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("low level"), TEXT(""), 0, 0);
		return;
	}

	CInventory &Inven = _StateController->PlayerInfo.Inven;
	if (Inven.GetWeaponInvenSize() == MAX_INVENTORY_SIZE)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("full"), TEXT(""), 0, 0);
		return;
	}

	if (ItemDesc->priceType == _IPT_MONEY)
	{
		if (_StateController->PlayerInfo.PlayerInfo.money < ItemDesc->price)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("no money"), TEXT(""), 0, 0);
			return;
		}
	}

	PM::INVENTORY::ITEM_BUY_REQ::Send(ItemDesc->id);
}

void UavaNetRequest::ShopBuyEquip(INT ListIndex, INT OptionIndex)
{
	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid() || ListIndex < 0)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	FavaShopItem *pShopItem = _ShopDesc().GetItemByIndex(ListIndex);
	if (!pShopItem || OptionIndex >= pShopItem->Options.Num())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("invalid"), TEXT(""), 0, 0);
		return;
	}

	ITEM_DESC *ItemDesc = static_cast<ITEM_DESC*>(pShopItem->Options(OptionIndex).pItem);

	if (!ItemDesc)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("invalid"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->PlayerInfo.PlayerInfo.level < ItemDesc->useLimitLevel)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("low level"), TEXT(""), 0, 0);
		return;
	}

	CInventory &Inven = _StateController->PlayerInfo.Inven;
	if (Inven.GetEquipInvenSize() == MAX_INVENTORY_SIZE)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("full"), TEXT(""), 0, 0);
		return;
	}

	if (ItemDesc->priceType == _IPT_MONEY)
	{
		if (_StateController->PlayerInfo.PlayerInfo.money < ItemDesc->price)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("no money"), TEXT(""), 0, 0);
			return;
		}
	}

	PM::INVENTORY::ITEM_BUY_REQ::Send(ItemDesc->id);
}

void UavaNetRequest::ShopSellWeapon(INT InvenSlot)
{
	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid() || (InvenSlot = GetInvenIndexFromSlot( InvenSlot )) < 0 )
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	ITEM_INFO &Item = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot];
	if (Item.IsEmpty())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("empty slot"), TEXT(""), 0, 0);
		return;
	}

	CInventory &Inven = _StateController->PlayerInfo.Inven;
	if (!Inven.IsRefund(Item.sn))
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("cannot sell"), TEXT(""), 0, 0);
		return;
	}

	PM::INVENTORY::ITEM_REFUND_REQ::Send(Item);
}

void UavaNetRequest::ShopSellEquip(INT InvenSlot)
{
	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->PlayerInfo.IsValid() || (InvenSlot = GetInvenIndexFromSlot( InvenSlot )) < 0 )
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	ITEM_INFO &Item = _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[InvenSlot];
	if (Item.IsEmpty())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("empty slot"), TEXT(""), 0, 0);
		return;
	}

	CInventory &Inven = _StateController->PlayerInfo.Inven;
	if (!Inven.IsRefund(Item.sn))
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("cannot sell"), TEXT(""), 0, 0);
		return;
	}

	PM::INVENTORY::ITEM_REFUND_REQ::Send(Item);
}

UBOOL UavaNetRequest::WICGetCash()
{
	return _WebInClient().GetCash();
}

UBOOL UavaNetRequest::WICOpenChargeWindow()
{
	return _WebInClient().OpenChargeWindow();
}

UBOOL UavaNetRequest::WICBuyItem(INT idItem)
{
	// idItem을 SalesID로 변환
	DWORD SalesID = 0;
	if (IsEffectItem(idItem))
	{
		EFFECT_ITEM_DESC *EffectDesc = _ItemDesc().GetEffectItem(idItem);
		if (EffectDesc)
		{
			SalesID = EffectDesc->sales_id;
		}
	}
	else
	{
		ITEM_DESC *ItemDesc =  _ItemDesc().GetItem(idItem);
		if (ItemDesc)
		{
			SalesID = ItemDesc->sales_id;
		}
	}

	_LOG(TEXT("idItem = %d, SalesID = %d"), idItem, SalesID);
	if (SalesID > 0)
	{
		return _WebInClient().Buy(idItem, SalesID);
	}
	else
	{
		return FALSE;
	}
}

UBOOL UavaNetRequest::WICSendGift(INT idItem, INT idAccountTo)
{
	// idItem을 SalesID로 변환
	DWORD SalesID = 0;
	if (IsEffectItem(idItem))
	{
		EFFECT_ITEM_DESC *EffectDesc = _ItemDesc().GetEffectItem(idItem);
		if (EffectDesc)
		{
			SalesID = EffectDesc->sales_id;
		}
	}
	else
	{
		ITEM_DESC *ItemDesc =  _ItemDesc().GetItem(idItem);
		if (ItemDesc)
		{
			SalesID = ItemDesc->sales_id;
		}
	}

	_LOG(TEXT("idItem = %d, SalesID = %d"), idItem, SalesID);
	if (SalesID > 0)
	{
		return _WebInClient().SendGift(idItem, SalesID, (TID_ACCOUNT)idAccountTo);
	}
	else
	{
		return FALSE;
	}
}

UBOOL UavaNetRequest::WICOpenGiftWindow(INT idItem)
{
	// idItem을 SalesID로 변환
	DWORD SalesID = 0;
	if (IsEffectItem(idItem))
	{
		EFFECT_ITEM_DESC *EffectDesc = _ItemDesc().GetEffectItem(idItem);
		if (EffectDesc)
		{
			SalesID = EffectDesc->sales_id;
		}
	}
	else
	{
		ITEM_DESC *ItemDesc =  _ItemDesc().GetItem(idItem);
		if (ItemDesc)
		{
			SalesID = ItemDesc->sales_id;
		}
	}

	_LOG(TEXT("idItem = %d, SalesID = %d"), idItem, SalesID);
	if (SalesID > 0)
	{
		return _WebInClient().OpenGiftWindow(SalesID);
	}
	else
	{
		return FALSE;
	}
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Guild

void UavaNetRequest::GuildJoinChannel()
{
	if (!_StateController->PlayerInfo.IsValid())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_JoinChannel, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->GuildInfo.IsValid())
	{
		_LOG(TEXT("Error! You have no guild information."));
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_JoinChannel, TEXT("no guild"), TEXT(""), 0, 0);
		return;
	}
	if (!_StateController->GuildInfo.IsRegularGuild())
	{
		_LOG(TEXT("Error! It is not regular guild."));
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_JoinChannel, TEXT("not regular"), TEXT(""), 0, 0);
		return;
	}
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		_LOG(TEXT("Error! Guild channel is not connected."));
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_JoinChannel, TEXT("not connected"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->ChannelInfo.IsMyClanChannel() && _StateController->GetNetState() != _AN_CHANNELLIST)
	{
		_LOG(TEXT("Error! Already joined the guild channel."));
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_JoinChannel, TEXT("already joined"), TEXT(""), 0, 0);
		return;
	}

	_LOG(TEXT("Joining guild channel"));
	if (_StateController->GetNetState() == _AN_CHANNELLIST)
	{
		_StateController->ChannelInfo.SetFromGuild(_StateController->GuildInfo);
		PM::GUILD::LOBBY_JOIN_REQ::Send(GavaNetClient->CurrentChannelAddress.address64);
	}
	else
	{
		_StateController->AutoMoveDest.Clear();
		_StateController->AutoMoveDest.SetMoveDestTo(ID_MY_CLAN_HOME);
		_StateController->ProcAutoMove();
	}
}

void UavaNetRequest::GuildSetMotd(const FString &Motd)
{
	if (!_StateController->GuildInfo.IsValid())
	{
		_LOG(TEXT("Error! You have no guild information."));
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoClan"), TEXT("AVANET")), EChat_GuildSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Motd, TEXT("no guild"), TEXT(""), 0, 0);
		return;
	}
	if (!_StateController->GuildInfo.IsRegularGuild())
	{
		_LOG(TEXT("Error! It is not regular guild."));
		return;
	}
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		_LOG(TEXT("Error! Guild channel is not connected."));
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoConnection"), TEXT("AVANET")), EChat_GuildSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Motd, TEXT("not connected"), TEXT(""), 0, 0);
		return;
	}

	CHECK_GUILD_PRIV(PRIV_SET_MOTD);

	TCHAR *Msg = (TCHAR*)*Motd;
	_WordCensor().ReplaceChatMsg(Msg);
	if (appStrlen(Msg) == 0)
		return;

	PM::GUILD::SET_MOTD_REQ::Send(Msg);
}

void UavaNetRequest::GuildChat(const FString &ChatMsg, UBOOL bParse)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	if (!_StateController->GuildInfo.IsValid())
	{
		_LOG(TEXT("Error! You have no guild information."));
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoClan"), TEXT("AVANET")), EChat_GuildSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Chat, TEXT("no guild"), TEXT(""), 0, 0);
		return;
	}
	if (!_StateController->GuildInfo.IsRegularGuild())
	{
		_LOG(TEXT("Error! It is not regular guild."));
		return;
	}
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		_LOG(TEXT("Error! Guild channel is not connected."));
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoConnection"), TEXT("AVANET")), EChat_GuildSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Chat, TEXT("not connected"), TEXT(""), 0, 0);
		return;
	}

	if (_StateController->IsWhisperBlockedByClanMatch())
	{
		// 클랜전 진행 중이고 사망자 챗이 꺼져 있으면 클랜챗 불가
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Chat_CannotChatWhileClanMatch"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}
	if (_StateController->IsMatchProcessing())
	{
		// 대회 진행 중이면 클랜챗 불가
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Chat_CannotChatWhileMatch"), TEXT("AVANET")),
										EChat_PlayerSystem);
		return;
	}

	// parse chat commands
	if (bParse && ParseChatCommand(*ChatMsg))
	{
		return;
	}

	TCHAR *Msg = (TCHAR*)*ChatMsg;
	_WordCensor().ReplaceChatMsg(Msg);
	if (appStrlen(Msg) == 0)
		return;

	PM::GUILD::CHAT_NTF::Send(Msg);
}

void UavaNetRequest::GuildNotice(const FString &NoticeMsg)
{
	if (!_StateController->PlayerInfo.IsValid())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Notice, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->GuildInfo.IsValid())
	{
		_LOG(TEXT("Error! You have no guild information."));
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoClan"), TEXT("AVANET")), EChat_GuildSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Notice, TEXT("no guild"), TEXT(""), 0, 0);
		return;
	}
	if (!_StateController->GuildInfo.IsRegularGuild())
	{
		_LOG(TEXT("Error! It is not regular guild."));
		return;
	}
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		_LOG(TEXT("Error! Guild channel is not connected."));
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoConnection"), TEXT("AVANET")), EChat_GuildSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Notice, TEXT("not connected"), TEXT(""), 0, 0);
		return;
	}

	CHECK_GUILD_PRIV(PRIV_SEND_NOTICE);

	TCHAR *Msg = (TCHAR*)*NoticeMsg;
	_WordCensor().ReplaceChatMsg(Msg);
	if (appStrlen(Msg) == 0)
		return;

	PM::GUILD::NOTICE_NTF::Send(Msg);
}

void UavaNetRequest::GuildLeave()
{
	if (!_StateController->PlayerInfo.IsValid())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Leave, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->GuildInfo.IsValid())
	{
		_LOG(TEXT("Error! You have no guild information."));
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoClan"), TEXT("AVANET")), EChat_GuildSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Leave, TEXT("no guild"), TEXT(""), 0, 0);
		return;
	}
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		_LOG(TEXT("Error! Guild channel is not connected."));
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoConnection"), TEXT("AVANET")), EChat_GuildSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Leave, TEXT("not connected"), TEXT(""), 0, 0);
		return;
	}
	if (_StateController->ChannelInfo.IsMyClanChannel() || _StateController->ChannelInfo.IsFriendlyGuildChannel())
	{
		_LOG(TEXT("Error! You cannot leave the guild while connecting to guild-related channel."));
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_CannotLeaveAtGuildChannel"), TEXT("AVANET")), EChat_GuildSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Leave, TEXT("in guild channel"), TEXT(""), 0, 0);
		return;
	}

	_LOG(TEXT("Leaving the guild"));
	//PM::GUILD::LEAVE_REQ::Send();
	if ( !_WebInClient().GuildLeave(_StateController->PlayerInfo.PlayerInfo.guildInfo.strGuildID, _StateController->PlayerInfo.PlayerInfo.idAccount) )
	{
		_LOG(TEXT("Error! WebInClient request failed."));
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Leave, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
}

void UavaNetRequest::GuildKick(const FString &Nickname)
{
	if (!_StateController->PlayerInfo.IsValid())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Kick, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	if (!_StateController->GuildInfo.IsValid())
	{
		_LOG(TEXT("Error! You have no guild information."));
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoClan"), TEXT("AVANET")), EChat_GuildSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Kick, TEXT("no guild"), TEXT(""), 0, 0);
		return;
	}
	if (!_StateController->GuildInfo.IsChannelConnected())
	{
		_LOG(TEXT("Error! Guild channel is not connected."));
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoConnection"), TEXT("AVANET")), EChat_GuildSystem);
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Kick, TEXT("not connected"), TEXT(""), 0, 0);
		return;
	}

	CHECK_GUILD_PRIV(PRIV_KICK);

	FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(Nickname);
	if (!Info)
	{
		_LOG(TEXT("Error! Guild member not found."));

		_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_MemberNotFound"), TEXT("AVANET")), *Nickname),
										EChat_GuildSystem);

		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Kick, TEXT("member not found"), TEXT(""), 0, 0);
		return;
	}

	_LOG(TEXT("GuildKick(): Nickname = %s"), *Nickname);
	//PM::GUILD::KICK_REQ::Send(Info->PlayerInfo.idAccount);
	if ( !_WebInClient().GuildKick(_StateController->PlayerInfo.PlayerInfo.guildInfo.strGuildID, _StateController->PlayerInfo.PlayerInfo.idAccount, Info->GuildPlayerInfo.idAccount) )
	{
		_LOG(TEXT("Error! WebInClient request failed."));
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Kick, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}
}

UBOOL UavaNetRequest::GuildDoIHavePriv(INT Priv)
{
	const _GUILD_PRIV PrivMap[] = { PRIV_INVITE, PRIV_KICK, PRIV_SET_MOTD, PRIV_SEND_NOTICE, PRIV_CREATE_MATCH, PRIV_JOIN_MATCH };

	if (Priv >= 0 && Priv < ARRAY_COUNT(PrivMap))
		return _StateController->DoIHaveGuildPriv(PrivMap[Priv]);

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Misc

void UavaNetRequest::OptionSaveUserKey(const FString& UserKeyStr, const FString& OptionStr)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	if ( UserKeyStr.Len() == 0 || UserKeyStr.Len() > SIZE_CONFIGSTR1 || OptionStr.Len() > SIZE_CONFIGSTR2 )
	{
		GError->Logf(TEXT("Invalid config string!! UserKeyStr.Len() = %d, OptionStr.Len() = %d"), UserKeyStr.Len(), OptionStr.Len());
		return;
	}

	appStrcpy(_StateController->PlayerInfo.PlayerInfo.configstr, *UserKeyStr);
	appStrcpy(_StateController->PlayerInfo.PlayerInfo.configstr2, *OptionStr);

	PM::CLIENT::SET_CONFIG_NTF::Send(UserKeyStr, OptionStr);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Information

void UavaNetRequest::SelectPlayer(INT ListIndex)
{
	if (_StateController->GetNetState() == _AN_LOBBY)
	{
		// 로비 플레이어 목록
		if (_StateController->LobbyPlayerList.PlayerList.IsValidIndex(ListIndex))
		{
			_StateController->LobbyPlayerList.ListIndex = ListIndex;
			GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_PlayerList, TEXT("selected"), TEXT(""), 0, 0);

			FLobbyPlayerInfo &Player = _StateController->LobbyPlayerList.PlayerList(ListIndex);
			if (Player.IsFullInfo())
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_PlayerInfo, TEXT("ok"), TEXT(""), 0, 0);
			}
			else
			{
				PM::CHANNEL::PLAYER_INFO_REQ::Send(Player.LobbyPlayerInfo.idAccount);
			}
		}
	}
	else if (_StateController->GetNetState() == _AN_ROOM)
	{
		// 대기실 플레이어 목록
		if (_StateController->RoomInfo.IsValid() &&
			ListIndex >= 0 && ListIndex < Def::MAX_ALL_PLAYER_PER_ROOM && !_StateController->RoomInfo.PlayerList.IsEmpty(ListIndex))
		{
			_StateController->RoomInfo.PlayerList.ListIndex = ListIndex;
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_PlayerList, TEXT("selected"), TEXT(""), 0, 0);

			FRoomPlayerInfo &Player = _StateController->RoomInfo.PlayerList.PlayerList[ListIndex];
			if (Player.IsFullInfo())
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_PlayerInfo, TEXT("ok"), TEXT(""), 0, 0);
			}
			else
			{
				PM::ROOM::PLAYER_INFO_REQ::Send(Player.RoomPlayerInfo.idAccount);
			}
		}
	}
}

void UavaNetRequest::SelectGuildPlayer(INT ListIndex)
{
	if (_StateController->GuildInfo.PlayerList.PlayerList.IsValidIndex(ListIndex))
	{
		_StateController->GuildInfo.PlayerList.ListIndex = ListIndex;
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MemberList, TEXT("selected"), TEXT(""), 0, 0);

		FGuildPlayerInfo &Player = _StateController->GuildInfo.PlayerList.PlayerList(ListIndex);
		if (Player.GuildPlayerInfo.idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
		{
			// 길드 서버 보다는 채널 서버의 정보가 더 신뢰도가 높다.
			// 그러므로 내 정보의 경우, GuildPlayerInfo 쪽은 PlayerInfo로 덮어쓴다.
			Player.Set(_StateController->PlayerInfo.PlayerInfo);
			Player.GuildPlayerInfo.idChannel = _StateController->ChannelInfo.idChannel;
			Player.idChannel = _StateController->ChannelInfo.idChannel;
			Player.idRoom = _StateController->RoomInfo.RoomInfo.idRoom;
		}

		if (Player.IsOnline())
		{
			if (Player.IsFullInfo())
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_PlayerInfo, TEXT("ok"), TEXT(""), 0, 0);
			}
			else
			{
				PM::GUILD::PLAYER_INFO_REQ::Send(Player.GuildPlayerInfo.idAccount);
			}

			if (Player.IsLocationOutdated())
			{
				PM::GUILD::PLAYER_LOCATION_REQ::Send(Player.GuildPlayerInfo.idAccount);
			}
		}
	}
}

void UavaNetRequest::SelectBuddy(INT ListIndex)
{
	_LOG(TEXT("SelectBuddy() : ListIndex = %d"), ListIndex);
	if (_Communicator().BuddyList.BuddyList.IsValidIndex(ListIndex))
	{
		_Communicator().BuddyList.ListIndex = ListIndex;
		GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_List, TEXT("selected"), TEXT(""), 0, 0);

		FBuddyInfo &Buddy = _Communicator().BuddyList(ListIndex);
		_LOG(TEXT("SelectBuddy() : Buddy[%d] is %s"), ListIndex, Buddy.IsOnline() ? TEXT("online") : TEXT("offline"));
		if (Buddy.IsOnline())
		{
			RxGate::RXNERVE_ADDRESS Addr;
			if (Buddy.ServerType == FChannelInfo::CT_NORMAL)
				CreateCHMAddress(Addr, Buddy.idChannel);
			else
				CreateGCSAddress(Addr, Buddy.idChannel);

			if (Buddy.IsFullInfo())
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_PlayerInfo, TEXT("ok"), TEXT(""), 0, 0);
				_LOG(TEXT("%s [%d](%d) [%s]%s (%d/%d)%s"), (Buddy.IsBuddyBIA() ? TEXT("==") : Buddy.IsBuddyBoth() ? TEXT("<>") : TEXT(" >")),
										Buddy.idAccount, Buddy.Level, *Buddy.GuildName, *Buddy.Nickname,
										Buddy.ServerType, Buddy.idChannel, Buddy.IsOnline() ? *Buddy.GetLocation() : TEXT("Offline"));
			}
			else
			{
				// 해당 친구의 상세 정보가 없을 때
				// 친구가 접속해 있는 채널의 multicast address로 정보 요청 메시지를 보냄
				if (Buddy.ServerType == FChannelInfo::CT_NORMAL)
				{
					_LOG(TEXT("Requesting information of %s to channel server."), *Buddy.Nickname);
					PM::CHANNEL::PLAYER_INFO_REQ::Send(Buddy.idAccount, &Addr);
				}
				else if (Buddy.ServerType == FChannelInfo::CT_GUILD)
				{
					_LOG(TEXT("Requesting information of %s to guild server."), *Buddy.Nickname);
					PM::GUILD::PLAYER_INFO_REQ::Send(Buddy.idAccount, Buddy.idGuild, &Addr);
				}
				else
				{
					_LOG(TEXT("Error! Unknown server type."));
					if (_StateController->GuildInfo.IsValid())
					{
						FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(Buddy.idAccount);
						if (Info)
						{
							// 아직 친구가 채널에 입장하지는 않았지만, 같은 클랜원이므로, 클랜 서버에게 정보 요청
							PM::GUILD::PLAYER_INFO_REQ::Send(Buddy.idAccount);
						}
					}
				}
			}

			if (Buddy.ServerType == FChannelInfo::CT_NORMAL && Buddy.IsLocationOutdated())
			{
				// 일반 채널에 있는 경우 위치 정보까지 확인
				PM::CHANNEL::PLAYER_LOCATION_REQ::Send(Buddy.idAccount, &Addr);
			}
		}
	}
}

void UavaNetRequest::SelectBlock(INT ListIndex)
{
	if (_Communicator().BlockList.BuddyList.IsValidIndex(ListIndex))
	{
		_Communicator().BlockList.ListIndex = ListIndex;
		GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_List, TEXT("block selected"), TEXT(""), 0, 0);
	}
}

FString UavaNetRequest::GetCurrentChannelName()
{
	if (_StateController->ChannelInfo.IsValid())
		return _StateController->ChannelInfo.ChannelName;
	else
		return TEXT("");
}

FString UavaNetRequest::GetMapName(INT ListIndex)
{
	if (_StateController->MapList.MapList.IsValidIndex(ListIndex))
		return _StateController->MapList.MapList(ListIndex).MapName;
	else
		return TEXT("");
}


INT UavaNetRequest::GetCurrentRoomIndex()
{
	if (_StateController->GetNetState() < _AN_LOBBY || _StateController->GetNetState() > _AN_INGAME)
		return -1;

	return _StateController->RoomList.ListIndex;
}

INT UavaNetRequest::GetCurrentRoomState()
{
	// 0 = 에러 또는 선택한 방이 없음, 1 = 대기 중, 2 = 플레이 중
	if (_StateController->GetNetState() < _AN_LOBBY || _StateController->GetNetState() > _AN_INGAME)
		return Def::RIP_NONE;

	if (_StateController->RoomInfo.IsValid())
		return _StateController->RoomInfo.RoomInfo.state.playing;
	else
	{
		FRoomDispInfo *Room = _StateController->RoomList.GetSelected();
		if (Room)
			return Room->RoomInfo.state.playing;
		else
			return Def::RIP_NONE;
	}
}


UBOOL UavaNetRequest::GetCurrentRoomSetting(FavaRoomSetting &Setting)
{
	if (_StateController->GetNetState() < _AN_LOBBY || _StateController->GetNetState() > _AN_INGAME)
		return FALSE;

	FRoomDispInfo *Room = (_StateController->GetNetState() == _AN_LOBBY ? _StateController->RoomList.GetSelected() : &_StateController->RoomInfo);
	if (Room && Room->IsValid())
	{
		Room->DumpRoomInfo();
		ROOM_SETTING &setting = Room->RoomInfo.setting;

		Setting.idMap = setting.idMap;
		Setting.tkLevel = setting.tkLevel;
		Setting.autoBalance = setting.autoBalance;
		Setting.allowSpectator = setting.allowSpectator;
		Setting.allowInterrupt = setting.allowInterrupt;
		Setting.allowBackView = setting.allowBackView;
		Setting.allowGameGhostChat = setting.allowGhostChat;
		Setting.roundToWin = setting.roundToWin;
		Setting.MaxPlayer = setting.numMax;
		Setting.autoSwapTeam = setting.autoSwapTeam;
		Setting.mapOption = setting.mapOption;

		Setting.bPassword = (Room->RoomInfo.bPassword > 0 ? TRUE : FALSE);

		return TRUE;
	}
	else
		return FALSE;
}


UBOOL UavaNetRequest::GetCurrentEquipState(INT &MyTeam, INT &MyClass, INT &MyFace, INT &MyWeapon)
{
	if (!_StateController->RoomInfo.IsValid())
		return FALSE;

	FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
	if (!MyInfo)
		return FALSE;

	MyTeam = MyInfo->GetTeamID();//RoomPlayerInfo.idTeam;
	MyClass = MyInfo->RoomPlayerInfo.currentClass;
	MyFace = MyInfo->RoomPlayerInfo.faceType;
	MyWeapon = MyInfo->GetPrimaryWeaponID();
	return TRUE;
}

INT UavaNetRequest::GetMyRoomSlot()
{
	return _StateController->RoomInfo.IsValid() ? _StateController->MyRoomSlotIdx : ID_INVALID_ROOM_SLOT;
}


FString UavaNetRequest::GetWeaponIconCode(INT idItem)
{
	return GetAvaNetHandler()->eventGetWeaponIconCode(idItem);
}

FString UavaNetRequest::GetRandomRoomName()
{
	return _RoomName().GetRandomRoomName();
}

FString UavaNetRequest::GetWeaponName(INT idItem)
{
	Def::ITEM_DESC *Desc = _ItemDesc().GetItem(idItem);
	return Desc ? Desc->GetName() : TEXT("Unknown");
}

FString UavaNetRequest::GetSkillName(BYTE PlayerClass, INT SkillID)
{
	FSkillInfo *Info = _SkillDesc().GetSkillInfo(PlayerClass, SkillID);
	if (Info)
	{
		//return Localize(TEXT("SkillName"), *FString::Printf(TEXT("Name_Skill[%d][%d]"), PlayerClass, SkillID+1), TEXT("AVANET"));
		return Info->SkillName;
	}
	else
	{
		return TEXT("Unknown Skill");
	}
}

FString UavaNetRequest::GetAwardName(INT AwardID)
{
	FAwardInfo *Info = _AwardDesc().GetAwardInfo(AwardID);
	if (Info)
	{
		//return Localize(TEXT("AwardName"), *FString::Printf(TEXT("Name_Award[%d]"), AwardID), TEXT("AVANET"));
		return Info->AwardName;
	}
	else
	{
		return TEXT("Unknown Award");
	}
}

FString UavaNetRequest::GetPlayerLevelName(INT PlayerLevel)
{
	if (PlayerLevel >= 0)
	{
		return Localize(TEXT("PlayerLevelName"), *FString::Printf(TEXT("Name_Level[%d]"), PlayerLevel), TEXT("AVANET"));
	}
	else
	{
		return TEXT("None");
	}
}

UBOOL UavaNetRequest::AmIHost()
{
	return _StateController->AmIHost();
}

UBOOL UavaNetRequest::AmISpectator()
{
	return _StateController->AmISpectator();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RoomSetting Stored

FavaRoomSetting* UavaNetRequest::GetCurrentRoomSetting(INT CurrentMissionType)
{
	if( RoomSettingTyped.Num() < NMT_MAX )
		RoomSettingTyped.Add( NMT_MAX - RoomSettingTyped.Num() );

	if( RoomSettingTyped.IsValidIndex(CurrentMissionType) )
		return &RoomSettingTyped(CurrentMissionType);

	return NULL;
}

void UavaNetRequest::ResetRoomSettingsAsDefault()
{
	if( RoomSettingTyped.Num() < NMT_MAX )
		RoomSettingTyped.Add( NMT_MAX - RoomSettingTyped.Num() );

	BYTE CurrentChannelGroup = _StateController->ChannelInfo.IsValid() ? _StateController->ChannelInfo.Flag : UCHAR_MAX;
	// Initialize all settings
	for( INT Type = 0 ; Type < RoomSettingTyped.Num() ; Type++ )
	{
		FavaRoomSetting& RoomSetting = RoomSettingTyped(Type);

		INT MapIndex;
		for( MapIndex = 0 ; MapIndex < _StateController->MapList.MapList.Num() ; MapIndex++ )
		{
			if( _StateController->MapList.MapList(MapIndex).MissionType == Type &&
				!_StateController->MapList.MapList(MapIndex).bHidden &&
				_StateController->MapList.MapList(MapIndex).ExclChannelGroups.FindItemIndex(CurrentChannelGroup) == INDEX_NONE)
			{
				break;
			}
		}

		// 해당 미션타입의 방정보를 찾아서 초기화
		if( _StateController->MapList.MapList.IsValidIndex(MapIndex) )
		{
			FavaNetChannelSettingInfo &Info = _StateController->GetChannelSettingInfo();
			FMapInfo& MapInfo = _StateController->MapList.MapList(MapIndex);
			RoomSetting.idMap = MapInfo.idMap;
			RoomSetting.tkLevel = Info.DefaultTKLevel;//0;
			RoomSetting.autoBalance = Info.DefaultAutoBalance;//FALSE;
			RoomSetting.allowSpectator = Info.DefaultSpectator;//FALSE;
			RoomSetting.allowInterrupt = Info.DefaultInterrupt;//TRUE;
			RoomSetting.allowBackView = Info.DefaultBackView;//TRUE;
			RoomSetting.allowGameGhostChat = Info.DefaultGhostChat;//TRUE;
			RoomSetting.autoSwapTeam = Info.DefaultAutoSwapTeam;//FALSE;
			RoomSetting.roundToWin = MapInfo.DefaultWinCond;
			RoomSetting.MaxPlayer = (Info.DefaultMaxPlayers > 0 ? Info.DefaultMaxPlayers : MapInfo.DefaultMaxPlayer);
			RoomSetting.mapOption = MapInfo.AllowMapOption ? 1 : 0;

			//INT CurrentChannelGroup = _StateController->ChannelInfo.IsValid() ? _StateController->ChannelInfo.Flag : UCHAR_MAX;
			//switch( CurrentChannelGroup )
			//{
			//case EChannelFlag_Practice:
			//	RoomSetting.MaxPlayer = 6;/* 연습채널 기본인원, 변경불가 */
			//	break;
			//case EChannelFlag_Clan:
			//	RoomSetting.allowBackView = FALSE;
			//	RoomSetting.allowGameGhostChat = FALSE;
			//	break;
			//case EChannelFlag_AutoBalance:
			//	RoomSetting.autoBalance = TRUE;
			//	break;
			//default:
			//	break;
			//}
		}
	}
}

UavaNetRequest* UavaNetRequest::GetAvaNetRequest()
{
	return ::GetAvaNetRequest();
}


