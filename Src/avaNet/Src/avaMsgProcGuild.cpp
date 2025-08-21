/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaMsgProcGuild.cpp

	Description: Implementation of message processors

***/
#include "avaNet.h"

#define ASSERT check

#include "ComDef/Def.h"

using namespace Def;

#include "ComDef/MsgDef.h"
#include "ComDef/MsgDefGuild.h"

#include "avaNetClient.h"
#include "avaMsgProc.h"
#include "avaMsgSend.h"
#include "avaNetStateController.h"
#include "avaCommunicator.h"
#include "avaWebInClient.h"

#include "UnUIMarkupResolver.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Guild

// Ŭ�� ����
void PM::GUILD::INFO_NTF::Proc(_LPMSGBUF pData)
{
	TMSG msg(pData);

	//_StateController->GuildInfo.GuildInfo = msg.Data().info;
	appMemcpy(&_StateController->GuildInfo.GuildInfo, &msg.Data().info, sizeof(GUILD_INFO));
	_StateController->GuildInfo.Dump();

	if (_StateController->GuildInfo.GuildInfo.idGuild != ID_INVALID_GUILD)
	{
		// PlayerInfo�� Ŭ�� ���� ����
		_StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild = _StateController->GuildInfo.GuildInfo.idGuild;
		appStrcpy(_StateController->PlayerInfo.PlayerInfo.guildInfo.strGuildID, _StateController->GuildInfo.GuildInfo.strGuild);
		appStrcpy(_StateController->PlayerInfo.PlayerInfo.guildInfo.guildName, _StateController->GuildInfo.GuildInfo.name);
		_StateController->PlayerInfo.PlayerInfo.guildInfo.idGuildMark = _StateController->GuildInfo.GuildInfo.idGuildMark;
		_StateController->PlayerInfo.PlayerInfo.guildInfo.guildLevel = _StateController->GuildInfo.GuildInfo.level;
	}

	// MOTD ���
	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_Motd"), TEXT("AVANET")),
													_StateController->GuildInfo.GuildInfo.motd),
									EChat_GuildSystem);

	GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Info, TEXT(""), TEXT(""), 0, 0);
	GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_PlayerInfo, TEXT(""), TEXT(""), 0, 0);
}

void PM::GUILD::MEMBER_LIST_NTF::Proc(_LPMSGBUF pData)
{
	if ( !_StateController->GuildInfo.IsValid() )
		return;

	TMSG msg(pData);
	PM::_MsgBuffer<GUILD_PLAYER_INFO> MemberList(msg, msg.Data().members);
	WORD Count = MemberList.GetCount();

	//_StateController->GuildInfo.PlayerList.Clear();

	if (Count > 0)
	{
		for (WORD i = 0; i < Count; ++i)
		{
			FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Add(MemberList[i]);
			if (Info)
			{
				if (Info->GuildPlayerInfo.idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
				{
					// it's me
					Info->Set(_StateController->PlayerInfo.PlayerInfo);
					Info->idChannel = _StateController->ChannelInfo.idChannel;
				}
				else
				{
					FBuddyInfo *Buddy = _Communicator().BuddyList.Find(Info->GuildPlayerInfo.idAccount);
					if (Buddy && Buddy->Nickname != Info->GuildPlayerInfo.nickname)
					{
						// �ش� Ŭ������ ģ�� ��Ͽ� ������, ģ�� ��� ���� �г��Ӱ� Ŭ���� ��� ���� �г����� �ٸ� ��
						// ģ�� ����� �г����� �켱�Ѵ�.
						Info->UpdateNickname(Buddy->Nickname, FALSE);

						// �ٸ� Ŭ�����鿡�� �˷���
						PM::GUILD::NICKNAME_UPDATE_NTF::Send(Info->GuildPlayerInfo.idAccount, Buddy->Nickname);
					}
				}
			}
		}
	}

	_StateController->GuildInfo.PlayerList.DumpPlayerList();
	_LOG(TEXT("Guild Master = [%d]%s"), _StateController->GuildInfo.GuildInfo.idMaster, *_StateController->GuildInfo.GetMasterName());

	// ���� ���� ��� ������� MemberCount ����
	_StateController->GuildInfo.RefreshMemberCount();

	GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MemberList, TEXT("ok"), TEXT(""), 0, 0);
}

void PM::GUILD::LOBBY_JOIN_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(LOBBY_JOIN_REQ);

	if (_StateController->GetNetState() != _AN_CHANNELLIST)
		return;

	TMSG msg(pData);

	if (msg.Data().result == RC_OK)
	{
		_LOG(TEXT("Guild channel joined."));

		//_StateController->LobbyPlayerList.Add(_StateController->PlayerInfo.PlayerInfo);

		GavaNetClient->Settings.Set(CFG_LASTCHANNEL, *appItoa(_StateController->ChannelInfo.idChannel));

		// Ŭ�� ������ ������ ä�� ���� ����
		_StateController->ChannelInfo.SetFromGuild(_StateController->GuildInfo);
		_StateController->GoToState(_AN_LOBBY);

		// ���� ���� ��ġ ����
		_WebInClient().ChannelSet(_StateController->ChannelInfo.ChannelNameShort);

		// �ڵ� �̵� ���̾����� ó����
		if ( !_StateController->ProcAutoMove() )
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_JoinChannel, TEXT("ok"), TEXT(""), 0, 0);
		}
	}
	else
	{
		_StateController->StopAutoMove();
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_JoinChannel, TEXT("failed"), TEXT(""), 0, 0);
	}
}


void PM::GUILD::LOBBY_JOIN_NTF::Proc(_LPMSGBUF pData)
{
	// ��� ���� -> CHANNEL::LOBBY_JOIN_NTF�� ��ü

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	_LOG(TEXT("Player joined..."));

	FLobbyPlayerInfo *Info = _StateController->LobbyPlayerList.Add(def.playerInfo);
	if (Info)
	{
		_StateController->SyncPlayerLevel(def.playerInfo.idAccount, def.playerInfo.level);

		Info->DumpPlayerInfo();

		//GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_LobbyJoin, TEXT(""), TEXT(""), def.playerInfo.idAccount, 0);
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_PlayerList, TEXT("done"), TEXT(""), 0, 0);
	}
}

void PM::GUILD::LOBBY_LEAVE_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(LOBBY_LEAVE_REQ);

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

		// ���� ���� ��ġ ���� (ä�� ���� ��)
		//_WebInClient().ChannelSet(Localize(TEXT("Channel"), TEXT("Text_Loc_ChannelList"), TEXT("AVANET")));
	}
	else
	{
		_StateController->StopAutoMove();

		//GavaNetClient->GetEventHandler()->Error(GavaNetClient->CurrentConnection(), 0);
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Leave, TEXT("failed"), TEXT(""), 0, 0);
	}
}

void PM::GUILD::LOBBY_LEAVE_NTF::Proc(_LPMSGBUF pData)
{
	// ��� ���� -> CHANNEL::LOBBY_LEAVE_NTF�� ��ü

	if (_StateController->GetNetState() != _AN_LOBBY)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

#if !FINAL_RELEASE
	FLobbyPlayerInfo *Info = _StateController->LobbyPlayerList.Find(def.idAccount);
	if (Info)
		_LOG(TEXT("[%d]%s left..."), Info->PlayerInfo.idAccount, Info->PlayerInfo.nickname);
#endif

	_StateController->LobbyPlayerList.Remove(def.idAccount);

	GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_PlayerList, TEXT("done"), TEXT(""), 0, 0);
	//GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_LobbyLeave, TEXT(""), TEXT(""), def.idAccount, 0);
}

void PM::GUILD::LOBBY_CHAT_NTF::Proc(_LPMSGBUF pData)
{
	if (_StateController->GetNetState() != _AN_LOBBY)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	//FLobbyPlayerInfo *Player = _StateController->LobbyPlayerList.Find(def.idAccount);
	//if (!Player)
	//	return;

	PM::_MsgString ChatMsg(msg, def.chatmsg);
	FString ChatStr = FString::Printf(TEXT("%s : %s"), def.nickname/*Player->PlayerInfo.nickname*/, (LPCWSTR)ChatMsg);

	_StateController->LogChatConsole(*ChatStr);
}

void PM::GUILD::SET_MOTD_ANS::Proc(_LPMSGBUF pData)
{
	if ( !_StateController->GuildInfo.IsValid() )
		return;

	TMSG msg(pData);

	if (msg.Data().result == RC_GUILD_NO_PRIV)
	{
		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoPriv"), TEXT("AVANET")),
										EChat_GuildSystem);
		return;
	}
}

void PM::GUILD::MOTD_NTF::Proc(_LPMSGBUF pData)
{
	// Ŭ�� MOTD ���� �޽���

	if ( !_StateController->GuildInfo.IsValid() )
		return;

	TMSG msg(pData);

	PM::_MsgString Motd(msg, msg.Data().motd);

	_StateController->GuildInfo.SetMotd((LPCWSTR)Motd);

	GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Motd, TEXT("ok"), TEXT(""), 0, 0);

	FString FormattedMsg = FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_Motd"), TEXT("AVANET")),
																	_StateController->GuildInfo.GuildInfo.motd);
	_StateController->LogChatConsoleNoMatch(*FormattedMsg, EChat_GuildSystem);
}


void PM::GUILD::CHAT_NTF::Proc(_LPMSGBUF pData)
{
	// Ŭ�� ê

	if ( !_StateController->GuildInfo.IsValid() )
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FString Nickname;
	FGuildPlayerInfo *Player = _StateController->GuildInfo.PlayerList.Find(def.idAccount);
	if (Player)
		Nickname = Player->GuildPlayerInfo.nickname;
	else if (_StateController->PlayerInfo.PlayerInfo.idAccount == def.idAccount)
		Nickname = _StateController->PlayerInfo.PlayerInfo.nickname;
	else
		Nickname = TEXT("???");

	PM::_MsgString ChatMsg(msg, def.chatmsg);
	FString ChatStr = FString::Printf(TEXT("%s : %s"), *Nickname, (LPCWSTR)ChatMsg);
	_StateController->LogChatConsoleNoMatch(*ChatStr, EChat_GuildChat);
}


void PM::GUILD::WHISPER_NTF::Proc(_LPMSGBUF pData)
{
	// ��� ���� -> CHANNEL::WHISPER_NTF�� ��ü

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (_Communicator().BlockList.Find(msg.Data().idAccount))
		return;

	_Communicator().LastWhisperedPlayer = def.nickname;
	if (_Communicator().LastWhisperedPlayer.Len() == 0)
		return;

	PM::_MsgString ChatMsg(msg, def.chatmsg);
	FString ChatStr = FString::Printf(TEXT("[From %s] %s"), def.nickname, (LPCWSTR)ChatMsg);

	_StateController->LogChatConsole(*ChatStr, EChat_Whisper);
}


void PM::GUILD::PLAYER_INFO_ANS::Proc(_LPMSGBUF pData)
{
	// Ŭ���� �Ǵ� Ŭ�� ������ �پ��ִ� ģ���� ����

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (def.result == RC_OK)
	{
		_Communicator().SyncPlayerInfo(def.playerInfo);
		if (_StateController->GuildInfo.IsValid())
			_StateController->GuildInfo.SyncPlayerInfo(def.playerInfo);
		//_LOG(TEXT("idAccount = %d, idGuildMark = %d"), def.playerInfo.idAccount, def.playerInfo.idGuildMark);
		//FGuildPlayerInfo *GuildPlayer = _StateController->GuildInfo.PlayerList.Find(def.playerInfo.idAccount);
		//if (GuildPlayer)
		//{
		//	GuildPlayer->Set(def.playerInfo);
		//	GuildPlayer->idChannel = ID_MY_CLAN_HOME;
		//	GuildPlayer->DumpPlayerInfo();

		//	_StateController->SyncPlayerInfo(*GuildPlayer);

		//	GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Guild_PlayerInfo, TEXT("ok"), TEXT(""), 0, 0);
		//}
		//else
		//{
		//	_LOG(TEXT("GuildPlayerInfo not found!"));
		//}

		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_PlayerInfo, TEXT("ok"), TEXT(""), 0, 0);
		return;
	}

	GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Guild_PlayerInfo, TEXT("failed"), TEXT(""), 0, 0);
}


void PM::GUILD::NOTICE_NTF::Proc(_LPMSGBUF pData)
{
	// Ŭ�� ����

	if ( !_StateController->GuildInfo.IsValid() )
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FGuildPlayerInfo *Player = _StateController->GuildInfo.PlayerList.Find(def.idAccount);
	FString Nickname = (Player ? Player->GuildPlayerInfo.nickname : TEXT("???"));

	PM::_MsgString ChatMsg(msg, def.chatmsg);
	FString ChatStr = FString::Printf(TEXT("%s : %s"), *Nickname, (LPCWSTR)ChatMsg);
	_StateController->LogChatConsoleNoMatch(*ChatStr, EChat_GuildNotice);
}


void PM::GUILD::LOGIN_NTF::Proc(_LPMSGBUF pData)
{
	// Ŭ���� ���� �˸�

	if ( !_StateController->GuildInfo.IsValid() )
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(def.idAccount);
	if (Info)
	{
		_LOG(TEXT("clan member login; Nickname = %s, idChannel = %d"), Info->GuildPlayerInfo.nickname, def.idChannel);
		Info->GuildPlayerInfo.idChannel = def.idChannel;
		Info->DumpPlayerInfo();
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MemberState, TEXT("online"), Info->GuildPlayerInfo.nickname, 0, 0);

		FString Msg = FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_Connected"), TEXT("AVANET")),
														Info->GuildPlayerInfo.nickname);
		_StateController->LogChatConsoleNoMatch(*Msg, EChat_GuildSystem);
	}
}


void PM::GUILD::LOGOUT_NTF::Proc(_LPMSGBUF pData)
{
	// Ŭ���� ���� ���� �˸�

	if ( !_StateController->GuildInfo.IsValid() )
		return;

	TMSG msg(pData);

	FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(msg.Data().idAccount);
	if (Info)
	{
		Info->SetOffline();
		Info->DumpPlayerInfo();
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MemberState, TEXT("offline"), TEXT(""), 0, 0);

		FString Msg = FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_DIsconnected"), TEXT("AVANET")),
														Info->GuildPlayerInfo.nickname);
		_StateController->LogChatConsoleNoMatch(*Msg, EChat_GuildSystem);
	}
}


void PM::GUILD::JOIN_NTF::Proc(_LPMSGBUF pData)
{
	// Ŭ���� ���� �޽���

	if ( !_StateController->GuildInfo.IsValid() )
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FString Msg = FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_Joined"), TEXT("AVANET")),
													def.playerInfo.nickname);
	_StateController->LogChatConsoleNoMatch(*Msg, EChat_GuildSystem);

	_StateController->GuildInfo.PlayerList.Add(def.playerInfo);
	_StateController->GuildInfo.RefreshMemberCount();

	FPlayerDispInfo *Info = _StateController->FindPlayerFromList(def.playerInfo.idAccount);
	if (Info)
	{
		appStrcpy(Info->PlayerInfo.guildName, _StateController->PlayerInfo.PlayerInfo.guildInfo.guildName);
		Info->PlayerInfo.idGuildMark = _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuildMark;
		if (_StateController->IsStateInRoom())
		{
			FRoomPlayerInfo *Player = (FRoomPlayerInfo*)Info;
			Player->RoomPlayerInfo.idGuildMark = _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuildMark;
			appStrcpy(Player->RoomPlayerInfo.guildName, _StateController->PlayerInfo.PlayerInfo.guildInfo.guildName);
		}
	}

	FBuddyInfo *Buddy = _Communicator().BuddyList.Find(def.playerInfo.idAccount);
	if (Buddy)
	{
		Buddy->GuildName = _StateController->PlayerInfo.PlayerInfo.guildInfo.guildName;
		Buddy->idGuild = _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild;
		Buddy->idGuildMark = _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuildMark;
	}

	GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MemberList, TEXT("ok"), TEXT(""), 0, 0);
}

void PM::GUILD::LEAVE_NTF::Proc(_LPMSGBUF pData)
{
	// Ŭ���� Ż�� �޽���

	if ( !_StateController->GuildInfo.IsValid() )
		return;

	TMSG msg(pData);

	TID_ACCOUNT idAccount = msg.Data().idAccount;
	//FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(msg.Data().idAccount);
	//if (Info)
	//	_StateController->LogChatConsole(*FString::Printf(Localize(TEXT("ChatConsoleMessage"), TEXT("Text_GuildKicked"), TEXT("AVANET")), EChat_GuildSystem),
	//								Info->PlayerInfo.nickname);

	if (idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
	{
		_LOG(TEXT("You left the guild."));
		_StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild = ID_INVALID_GUILD;
		appStrcpy(_StateController->PlayerInfo.PlayerInfo.guildInfo.guildName, TEXT(""));
		_StateController->GuildInfo.Clear();

		FString Msg = Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_Left"), TEXT("AVANET"));
		_StateController->LogChatConsoleNoMatch(*Msg, EChat_GuildSystem);

		if (_StateController->ChannelInfo.IsNormalChannel())
		{
			if (_StateController->ChannelInfo.IsFriendlyGuildChannel())
			{
				// ���� ���� ��ġ�� ģ�� Ŭ���� ä���� ���, ��� ä�� ������� ���ư��� ��.
				if (_StateController->IsStatePlaying())
				{
					// ���� ���� ���̶�� ���� ���� �� ä�� ��� ȭ������ �Ѿ
					//_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_YouKickedAtMatch"), TEXT("AVANET")), EChat_GuildSystem);
				}
				else
				{
					// ��� ä�� ��� ȭ������ �Ѿ
					_StateController->GoToState(_AN_CHANNELLIST);
					if (_StateController->IsStateInRoom())
						GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Leave, TEXT("ok"), TEXT(""), 0, 0);
					else
						GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Leave, TEXT("ok"), TEXT(""), 0, 0);
				}
			}
			else
			{
				// ���� ���� ��ġ�� �Ϲ� ä���� ���, ä�� �޽����� �뺸�� ��.
				GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_PlayerInfo, TEXT(""), TEXT(""), 0, 0);
				GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MemberList, TEXT("ok"), TEXT(""), 0, 0);
			}
		}
		else if (_StateController->ChannelInfo.IsMyClanChannel())
		{
			// ���� ���� ��ġ�� �� Ŭ�� Ȩ�� ���, ������ ������ ����.
			GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_PlayerInfo, TEXT(""), TEXT(""), 0, 0);
		}
	}
	else
	{
		_LOG(TEXT("A member left the guild. (idAccount = %d)"), idAccount);
		// ��� ��Ͽ��� ����
		_StateController->GuildInfo.PlayerList.Remove(idAccount);
		_StateController->GuildInfo.RefreshMemberCount();

		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MemberList, TEXT("ok"), TEXT(""), 0, 0);

		FPlayerDispInfo *Player = _StateController->FindPlayerFromList(msg.Data().idAccount);
		if (Player)
		{
			// ���� ���� ���� �κ� ���ǿ� �ִٸ�, �÷��̾� �������� Ŭ�� �̸� ����
			appStrcpy(Player->PlayerInfo.guildName, TEXT(""));
		}

		FBuddyInfo *Buddy = _Communicator().BuddyList.Find(msg.Data().idAccount);
		if (Buddy)
		{
			// ģ���� ��ϵǾ� �ִٸ�, ģ�� �������� Ŭ�� �̸� ����
			Buddy->idGuild = ID_INVALID_GUILD;
			Buddy->GuildName = TEXT("");
			GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_List, TEXT("ok"), TEXT(""), _Communicator().BuddyList.Num(), 0);
		}
	}
}

void PM::GUILD::KICK_NTF::Proc(_LPMSGBUF pData)
{
	// Ŭ���� �߹� �޽���

	if ( !_StateController->PlayerInfo.IsValid() || !_StateController->GuildInfo.IsValid() )
		return;

	TMSG msg(pData);

	if (_StateController->PlayerInfo.PlayerInfo.idAccount == msg.Data().idAccount)
	{
		// you are kicked
		_LOG(TEXT("You are kicked from your guild."));

		_StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild = ID_INVALID_GUILD;
		appStrcpy(_StateController->PlayerInfo.PlayerInfo.guildInfo.guildName, TEXT(""));
		_StateController->GuildInfo.Clear();

		FString Msg = Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_YouKicked"), TEXT("AVANET"));
		_StateController->LogChatConsoleNoMatch(*Msg, EChat_GuildSystem);

		if (_StateController->ChannelInfo.IsNormalChannel())
		{
			if (_StateController->ChannelInfo.IsFriendlyGuildChannel())
			{
				// ���� ���� ��ġ�� ģ�� Ŭ���� ä���� ���, ��� ä�� ������� ���ư��� ��.
				if (_StateController->IsStatePlaying())
				{
					// ���� ���� ���̶�� ���� ���� �� ä�� ��� ȭ������ �Ѿ
					//_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_YouKickedAtMatch"), TEXT("AVANET")), EChat_GuildSystem);
				}
				else
				{
					// ��� ä�� ��� ȭ������ �Ѿ
					_StateController->GoToState(_AN_CHANNELLIST);
				}

				GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Kick, TEXT("you"), TEXT(""), 0, 0);
			}
			else
			{
				if (_StateController->GetNetState() > _AN_CHANNELLIST)
				{
					// ���� ���� ��ġ�� �Ϲ� ä���� ���, ä�� �޽����� �뺸�� ��.
					GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_PlayerInfo, TEXT(""), TEXT(""), 0, 0);
					GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MemberList, TEXT("ok"), TEXT(""), 0, 0);
				}
				else
				{
					// ä�� ��� ȭ���̶�� �˾����� ǥ��
					GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_PlayerInfo, TEXT(""), TEXT(""), 0, 0);
					GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Kick, TEXT("you"), TEXT(""), 0, 0);
				}
			}
		}
		else if (_StateController->ChannelInfo.IsMyClanChannel())
		{
			// ���� ���� ��ġ�� �� Ŭ�� Ȩ�� ���, ������ ������ ����.
			GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_PlayerInfo, TEXT(""), TEXT(""), 0, 0);
			GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Kick, TEXT("you"), TEXT(""), 0, 0);
		}
	}
	else
	{
		FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(msg.Data().idAccount);
		if (Info)
		{
			FString Msg = FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_Kicked"), TEXT("AVANET")),
															Info->GuildPlayerInfo.nickname);
			_StateController->LogChatConsoleNoMatch(*Msg, EChat_GuildSystem);
		}

		// ��� ��Ͽ��� ����
		_StateController->GuildInfo.PlayerList.Remove(msg.Data().idAccount);
		_StateController->GuildInfo.RefreshMemberCount();
		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MemberList, TEXT("ok"), TEXT(""), 0, 0);

		FPlayerDispInfo *Player = _StateController->FindPlayerFromList(msg.Data().idAccount);
		if (Player)
		{
			// ���� ���� ���� �κ� ���ǿ� �ִٸ�, �÷��̾� �������� Ŭ�� �̸� ����
			appStrcpy(Player->PlayerInfo.guildName, TEXT(""));
		}

		FBuddyInfo *Buddy = _Communicator().BuddyList.Find(msg.Data().idAccount);
		if (Buddy)
		{
			// ģ���� ��ϵǾ� �ִٸ�, ģ�� �������� Ŭ�� �̸� ����
			Buddy->idGuild = ID_INVALID_GUILD;
			Buddy->GuildName = TEXT("");
			GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_List, TEXT("ok"), TEXT(""), _Communicator().BuddyList.Num(), 0);
		}
	}
}

void PM::GUILD::GRANTGRADE_NTF::Proc(_LPMSGBUF pData)
{
	// Ŭ���� ��� ����

	if ( !_StateController->PlayerInfo.IsValid() || !_StateController->GuildInfo.IsValid() )
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (def.byRank >= SIZE_GUILD_RANK)
	{
		_LOG(TEXT("Error! Wrong idRank; idRank = %d"), def.byRank);
		return;
	}

	FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(def.idToAccount);
	if (Info)
	{
		if (Info->GuildPlayerInfo.idRank == def.byRank)
		{
			_LOG(TEXT("Guild rank is not changed."));
			return;
		}

		_LOG(TEXT("%s's guild rank is changed to %d"), Info->GuildPlayerInfo.nickname, def.byRank);

		// idRank; 2 = Ŭ����, 3 = ����, 4 = ������; 0 = ��ȸ��, 1 = �մ�; 0, 1�� ����� ���� ����� �뺸�� �ʿ䰡 ����. (���ӳ����� 0, 1�� ����� �������� ����)
		if (Info->GuildPlayerInfo.idRank >= 3 || def.byRank >= 3)
		{
			FString RankNameBefore = Localize(TEXT("ClanRankName"), *FString::Printf(TEXT("Name_Clan_Rank[%d]"), Info->GuildPlayerInfo.idRank), TEXT("AVANET"));
			FString RankNameAfter = Localize(TEXT("ClanRankName"), *FString::Printf(TEXT("Name_Clan_Rank[%d]"), def.byRank), TEXT("AVANET"));
			FString Msg = FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_RankChanged"), TEXT("AVANET")),
															Info->GuildPlayerInfo.nickname, *RankNameBefore, *RankNameAfter);
			_StateController->LogChatConsoleNoMatch(*Msg, EChat_GuildSystem);
		}

		Info->GuildPlayerInfo.idRank = def.byRank;
	}

	if (_StateController->PlayerInfo.PlayerInfo.idAccount == def.idToAccount)
	{
		_StateController->PlayerInfo.PlayerInfo.guildInfo.idRank = def.byRank;
	}

	GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_RankChanged, Info ? Info->GuildPlayerInfo.nickname : TEXT(""), TEXT(""), def.idToAccount, 0);
}

// ������ ����
void PM::GUILD::ENTRUST_MASTER_NTF::Proc(_LPMSGBUF pData)
{
	if ( !_StateController->PlayerInfo.IsValid() || !_StateController->GuildInfo.IsValid() )
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (_StateController->PlayerInfo.PlayerInfo.idAccount == def.idToAccount)
	{
		_LOG(TEXT("You are new guild master"));
		_StateController->PlayerInfo.PlayerInfo.guildInfo.idRank = 4;
	}

	FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(def.idToAccount);
	if (!Info)
		return;

	FString Msg = FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_MasterChanged"), TEXT("AVANET")),
													*_StateController->GuildInfo.GetMasterName(), Info->GuildPlayerInfo.nickname);
	_StateController->LogChatConsoleNoMatch(*Msg, EChat_GuildSystem);

	GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MasterChanged, Info->GuildPlayerInfo.nickname, TEXT(""), def.idToAccount, 0);

	_StateController->GuildInfo.GuildInfo.idMaster = def.idToAccount;
}

// �� Ŭ�� Ȩ ä���� ��Ƽĳ��Ʈ �ּ� ����
void PM::GUILD::GET_CHANNEL_ADDR_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(GET_CHANNEL_ADDR_REQ);

	if ( !_StateController->PlayerInfo.IsValid() || !_StateController->GuildInfo.IsValid() || _StateController->GuildInfo.IsChannelConnected())
	{
		_LOG(TEXT("Error! Guild information is invalid or already connected."));
		return;
	}

	TMSG msg(pData);
	DEF &def = msg.Data();

	_INT64 Zero(0);
	if (def.addr == Zero)
	{
		_LOG(TEXT("Error! Received address is zero."));
		return;
	}

	// �ּҰ� ��ȿ�ϴٸ� ��� ����; �� Ŭ�� Ȩ ä���� ������ ���¿��� �ٽ� ���� �Ǿ��� ���� ��������
	_StateController->PlayerInfo.SetRxGuildAddress(def.addr);

	GavaNetClient->CurrentGuildAddress.addrType = 0;
	GavaNetClient->CurrentGuildAddress.address64 = def.addr;

	GavaNetClient->CreateRxGateSession(GavaNetClient->CurrentGuildAddress);
}

// Ŭ�� ����� ���� ����
//void PM::GUILD::UPDATEINFO_NTF::Proc(_LPMSGBUF pData)
//{
//	if ( !_StateController->PlayerInfo.IsValid() || !_StateController->GuildInfo.IsValid())
//	{
//		_LOG(TEXT("Error! Guild information is invalid"));
//		return;
//	}
//
//	TMSG msg(pData);
//
//	_StateController->GuildInfo.PlayerList.Add(msg.Data().playerInfo);
//	_StateController->GuildInfo.RefreshMemberCount();
//
//	GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MemberList, TEXT("ok"), TEXT(""), 0, 0);
//}

// ģ�� Ŭ���� ��� �޽���
void PM::GUILD::SCORE_UPDATE_NTF::Proc(_LPMSGBUF pData)
{
	if ( !_StateController->PlayerInfo.IsValid() || !_StateController->GuildInfo.IsValid())
	{
		_LOG(TEXT("Error! Guild information is invalid"));
		return;
	}

	TMSG msg(pData);
	DEF &def = msg.Data();

	_StateController->GuildInfo.GuildInfo.totalWinCnt = def.totalWinCnt;
	_StateController->GuildInfo.GuildInfo.totalLoseCnt = def.totalLoseCnt;

	GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_UpdateScore, TEXT(""), TEXT(""), 0, 0);

	FMapInfo *MapInfo = _StateController->MapList.Find(def.idMap);
	FString MapName;
	// Get map name
	if (MapInfo)
	{
		FUIStringParser Parser;
		Parser.ScanString(MapInfo->MapName);
		TIndirectArray<FTextChunk> *Chunks = (TIndirectArray<FTextChunk>*)Parser.GetTextChunks();
		for (INT i = 0; i < Chunks->Num(); ++i)
		{
			FTextChunk &Chunk = (*Chunks)(i);

			if ( Chunk.IsMarkup() )
			{
				FString Tag, Value;
				if ( Chunk.GetDataStoreMarkup(Tag, Value) )
				{
					// Parse the markup?

					//UUIDataStore* DataStore = ResolveDataStore(*Tag);
					//if ( DataStore != NULL )
					//{
					//	FUIProviderFieldValue ResolvedValue(EC_EventParm);
					//	if ( DataStore->GetDataStoreValue(Value, ResolvedValue) )
					//	{
					//		MapName = ResolvedValue.StringValue;
					//	}
					//}

					if (Tag == TEXT("Strings"))
					{
						TCHAR *Toks[3] = {0, 0, 0};
						TCHAR *Val = (TCHAR*)*Value;

						INT Cnt = 0;
						Toks[Cnt] = wcstok(Val, TEXT("."));
						while (Toks[Cnt] != NULL && Cnt < 2)
						{
							++Cnt;
							Toks[Cnt] = wcstok(NULL, TEXT("."));
						}
						if (Toks[0] && Toks[1] && Toks[2])
						{
							MapName = Localize(Toks[1], Toks[2], Toks[0]);
						}
					}
				}
				else
				{
					MapName = Chunk.ChunkValue;
				}
			}
			else
			{
				MapName = Chunk.ChunkValue;
			}

			if (MapName.Len() > 0)
				break;
		}
	}

	if (MapName.Len() == 0)
		MapName = TEXT("???");

	FString Msg1 = FString::Printf(*Localize(TEXT("ChatConsoleMessage"),
												def.winFlag > 0 ? TEXT("Text_Clan_FriendlyMatchResultWin") : TEXT("Text_Clan_FriendlyMatchResultDefeat"),
												TEXT("AVANET")),
									def.destGuildName);
	FString Msg2 = FString::Printf(*Localize(TEXT("ChatConsoleMessage"),
												TEXT("Text_Clan_FriendlyMatchResult"),
												TEXT("AVANET")),
									*_StateController->GetTimeFromAppSec(appSeconds()),
									*MapName,
									*Localize(TEXT("avaTeamGame"), *FString::Printf(TEXT("RealTeamNames[%d]"), def.teamCode), TEXT("avaGame")),
									def.totalWinCnt, def.totalLoseCnt);

	if (_StateController->IsStatePlaying() /*&& _StateController->ChannelInfo.IsFriendlyGuildChannel()*/)
	{
		// �ΰ����� ��忡�� ǥ������ �ʴ´�. ���ǿ��� �� �� �ְ� ä�� �޽��� ��Ͽ� �̸� �־��.
		_StateController->ChatMsgList.AddWithPrefix(Msg1, EChat_GuildSystem);
		_StateController->ChatMsgList.AddWithPrefix(Msg2, EChat_GuildSystem);
	}
	else
	{
		_StateController->LogChatConsole(*Msg1, EChat_GuildSystem);
		_StateController->LogChatConsole(*Msg2, EChat_GuildSystem);
	}
}

// Ŭ������ �г����� ��������
void PM::GUILD::NICKNAME_UPDATE_NTF::Proc(_LPMSGBUF pData)
{
	if ( !_StateController->PlayerInfo.IsValid() || !_StateController->GuildInfo.IsValid())
	{
		_LOG(TEXT("Error! Guild information is invalid"));
		return;
	}

	TMSG msg(pData);
	DEF &def = msg.Data();

	FString Nickname = def.nickname;
	FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(def.idAccount);
	if (Info)
		Info->UpdateNickname(Nickname);

	FBuddyInfo *Buddy = _Communicator().BuddyList.Find(def.idAccount);
	if (Buddy && Buddy->Nickname != Nickname)
	{
		// ģ�� ��Ͽ����� �г��� �ٲ�
		Buddy->Nickname = Nickname;
		GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_List, TEXT("ok"), TEXT(""), _Communicator().BuddyList.Num(), 0);
	}
	FBuddyInfo *Block = _Communicator().BlockList.Find(def.idAccount);
	if (Block && Block->Nickname != Nickname)
	{
		// ���� ��Ͽ����� �г��� �ٲ�
		Block->Nickname = Nickname;
		GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_AddBlock, TEXT(""), TEXT(""), 0, 0);
	}

	if (_StateController->PlayerInfo.PlayerInfo.idAccount == def.idAccount && Nickname != _StateController->PlayerInfo.PlayerInfo.nickname)
	{
		// �ڱ� �ڽ��� �г����� �ٲ����
		appStrncpy(_StateController->PlayerInfo.PlayerInfo.nickname, *Nickname, SIZE_NICKNAME+1);
		GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_PlayerInfo, TEXT("ok"), TEXT(""), 0, 0);
	}
}

// Ŭ�� �̸��� �ٲ����
void PM::GUILD::CLANNAME_UPDATE_NTF::Proc(_LPMSGBUF pData)
{
	if ( !_StateController->PlayerInfo.IsValid() || !_StateController->GuildInfo.IsValid())
	{
		_LOG(TEXT("Error! Guild information is invalid"));
		return;
	}

	TMSG msg(pData);
	DEF &def = msg.Data();

	FString OldGuildName = _StateController->GuildInfo.GuildInfo.name;
	FString GuildName = def.guildName;
	if (GuildName != OldGuildName)
	{
		FString Msg = FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_UpdateClanName"), TEXT("AVANET")),
											*OldGuildName, *GuildName);
		_StateController->LogChatConsoleNoMatch(*Msg, EChat_GuildSystem);

		appStrncpy(_StateController->GuildInfo.GuildInfo.name, *GuildName, SIZE_GUILD_NAME+1);
		appStrncpy(_StateController->PlayerInfo.PlayerInfo.guildInfo.guildName, *GuildName, SIZE_GUILD_NAME+1);

		// ����� �� ���� Ŭ������ ������ �ش� �÷��̾��� Ŭ�� �̸� ���� ������Ʈ
		if (_StateController->IsStateInRoom())
		{
			for (INT i = 0; i < MAX_ALL_PLAYER_PER_ROOM; ++i)
			{
				FRoomPlayerInfo &Info = _StateController->RoomInfo.PlayerList.PlayerList[i];
				if (Info.IsValid() && OldGuildName == Info.RoomPlayerInfo.guildName)
				{
					appStrncpy(Info.RoomPlayerInfo.guildName, *GuildName, SIZE_GUILD_NAME+1);
					if (Info.IsFullInfo())
					{
						appStrncpy(Info.PlayerInfo.guildName, *GuildName, SIZE_GUILD_NAME+1);
					}
				}
			}
		}
		else if (_StateController->IsStateInLobby())
		{
			for (INT i = 0; i < _StateController->LobbyPlayerList.PlayerList.Num(); ++i)
			{
				FLobbyPlayerInfo &Info = _StateController->LobbyPlayerList.PlayerList(i);
				if (Info.IsValid() && Info.IsFullInfo() && OldGuildName == Info.PlayerInfo.guildName)
				{
					appStrncpy(Info.PlayerInfo.guildName, *GuildName, SIZE_GUILD_NAME+1);
				}
			}
		}

		// ģ�� �� ���� Ŭ������ ������ �ش� �÷��̾��� Ŭ�� �̸� ���� ������Ʈ
		for (INT i = 0; i < _Communicator().BuddyList.Num(); ++i)
		{
			FBuddyInfo &Info = _Communicator().BuddyList(i);
			if (Info.IsOnline() && OldGuildName == Info.GuildName)
			{
				Info.GuildName = GuildName;
			}
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Info, TEXT("ok"), TEXT(""), 0, 0);
		GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_PlayerInfo, TEXT("ok"), TEXT(""), 0, 0);
	}
}

void PM::GUILD::PLAYER_LOCATION_ANS::Proc(_LPMSGBUF pData)
{
	if (!_StateController->PlayerInfo.IsValid() || !_StateController->GuildInfo.IsValid())
	{
		_LOG(TEXT("Error! Guild information is invalid"));
		return;
	}

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (def.result != RC_OK)
	{
		if (!_StateController->AutoMoveDest.IsMoving() && _StateController->AutoMoveDest.idAccount == def.idAccount)
		{
			_StateController->StopAutoMove();
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Guild_PlayerInfo, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	FGuildPlayerInfo *Info = _StateController->GuildInfo.PlayerList.Find(def.idAccount);
	if (!Info)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Guild_PlayerInfo, TEXT("failed"), TEXT(""), 0, 0);
		return;
	}

	_Communicator().SyncPlayerInfo(def.idAccount, def.idChannel);
	_StateController->GuildInfo.SyncPlayerInfo(def.idAccount, def.idChannel);
	GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Guild_PlayerInfo, TEXT("ok"), TEXT(""), 0, 0);

	if (!_StateController->AutoMoveDest.IsMoving() && _StateController->AutoMoveDest.idAccount == def.idAccount)
	{
		// ���󰡱⸦ �ϴ� ���̾���
		GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_FollowPlayer, TEXT("ok"), TEXT(""), 0, 0);

		if (def.idChannel == ID_INVALID_CHANNEL)
		{
			_StateController->StopAutoMove();
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_PlayerOffline"), TEXT("AVANET")), Info->GuildPlayerInfo.nickname),
											EChat_PlayerSystem);
		}
		else
		{
			_Communicator().FollowPlayer(def.idAccount, Info->GuildPlayerInfo.nickname, _StateController->GuildInfo.GuildInfo.idGuild, def.idChannel);
		}
	}
	else
	{
		if (def.idChannel != ID_INVALID_CHANNEL && def.idChannel != ID_MY_CLAN_HOME)
		{
			// �÷��̾ CHS�� �ִٸ�, �� ��ġ���� Ȯ��
			RxGate::RXNERVE_ADDRESS Addr;
			CreateCHMAddress(Addr, def.idChannel);
			PM::CHANNEL::PLAYER_LOCATION_REQ::Send(def.idAccount, &Addr);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void PM::GUILD::Proc(_LPMSGBUF pData)
{
	MSG_HEADER *pHeader = (MSG_HEADER*)pData->GetData();

	switch (pHeader->msg_id)
	{
		CASE_MSG_PROC(INFO_NTF)
		CASE_MSG_PROC(MEMBER_LIST_NTF)
		CASE_MSG_PROC(LOBBY_JOIN_ANS)
		CASE_MSG_PROC(LOBBY_JOIN_NTF)
		CASE_MSG_PROC(LOBBY_LEAVE_ANS)
		CASE_MSG_PROC(LOBBY_LEAVE_NTF)
		CASE_MSG_PROC(LOBBY_CHAT_NTF)
		CASE_MSG_PROC(MOTD_NTF)
		CASE_MSG_PROC(CHAT_NTF)
		CASE_MSG_PROC(WHISPER_NTF)
		CASE_MSG_PROC(PLAYER_INFO_ANS)
		CASE_MSG_PROC(NOTICE_NTF)
		CASE_MSG_PROC(LOGIN_NTF)
		CASE_MSG_PROC(LOGOUT_NTF)
		CASE_MSG_PROC(JOIN_NTF)
		CASE_MSG_PROC(LEAVE_NTF)
		CASE_MSG_PROC(KICK_NTF)
		CASE_MSG_PROC(ENTRUST_MASTER_NTF)
		CASE_MSG_PROC(GRANTGRADE_NTF)
		CASE_MSG_PROC(GET_CHANNEL_ADDR_ANS)
		//CASE_MSG_PROC(UPDATEINFO_NTF)
		CASE_MSG_PROC(SCORE_UPDATE_NTF)
		CASE_MSG_PROC(NICKNAME_UPDATE_NTF)
		CASE_MSG_PROC(CLANNAME_UPDATE_NTF)
		CASE_MSG_PROC(PLAYER_LOCATION_ANS)

	default:
		_LOG(TEXT("Invalid GUILD message received. ID = %d"), pHeader->msg_id);
	}
}

void PM::GUILD::ProcTimeOut(const BYTE *Buffer, INT BufferLen)
{
	MSG_HEADER *pHeader = (MSG_HEADER*)Buffer;

	switch (pHeader->msg_id)
	{
		CASE_MSG_TIMEOUT_PROC(Guild, JoinChannel, LOBBY_JOIN_REQ)
		CASE_MSG_TIMEOUT_PROC(Guild, LeaveChannel, LOBBY_LEAVE_REQ)
		CASE_MSG_TIMEOUT_PROC(Guild, GetChannelAddr, GET_CHANNEL_ADDR_REQ)
	default:
		_LOG(TEXT("Some GUILD message timed out. ID = %d"), pHeader->msg_id);
	}
}
