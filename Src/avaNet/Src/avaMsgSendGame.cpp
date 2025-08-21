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
// Game

void PM::GAME::START_NTF::Send()
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_ROOM ||
		(_StateController->MyRoomSlotIdx == ID_INVALID_ROOM_SLOT && !_StateController->AmIStealthMode()))
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	// {{ 20070224
	#ifdef EnableP2pConn
	SOCKADDR myAddrLocal, myAddrPublic;
	memset(&myAddrLocal, 0, sizeof(SOCKADDR));
	memset(&myAddrPublic, 0, sizeof(SOCKADDR));
	// {{ 20070120 dEAthcURe #Room join시 udp addr을 얻어와 server에 보고.
	if(_StateController->AmIHost()) {
		_ahead_deinitSocket();
		if(_ahead_initSocket() && GavaNetClient) {
			if(GavaNetClient->pp2pConn) delete GavaNetClient->pp2pConn;
			GavaNetClient->pp2pConn = new p2pConn_t(GavaNetClient); // 20080213 dEAthcURe +GavaNetClient
			if(GavaNetClient->pp2pConn) {
				TCHAR wstr[256];
				mbstowcs(wstr, GavaNetClient->pp2pConn->p2psIpAddr, 255);
				debugf(TEXT("          <---------- [PM::GAME::START_NTF::Send] p2p server %s %d"), wstr, GavaNetClient->pp2pConn->p2psPort);
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
							debugf(TEXT("          <---------- [PM::GAME::START_NTF::Send] Resolving my UDP addr failed."));
						}
					}
					else {
						debugf(TEXT("          <---------- [PM::GAME::START_NTF::Send] waitForConnect failed."));
					}					
				}				
			}
		}
	}
	// }} 20070120 dEAthcURe #Room join시 udp addr을 얻어와 server에 보고.	
	#endif
	// }} 20070224

	TMSG msg;

	#ifndef EnableP2pConn	
	FInternetIpAddr Addr;
	GSocketSubsystem->GetLocalHostAddr(*GLog, Addr);
	msg.Data().hostInfo.extAddr.ipAddress = ((SOCKADDR_IN*)((SOCKADDR*)Addr))->sin_addr.S_un.S_addr;	
	#endif

	// {{ 20070224 dEAthcURe host만 유효
	#ifdef EnableP2pConn	
	if(_StateController->AmIHost() && GavaNetClient->pp2pConn) {
		msg.Data().hostInfo.intAddr.ipAddress = ((SOCKADDR_IN*)&myAddrLocal)->sin_addr.S_un.S_addr;	
		msg.Data().hostInfo.intAddr.port = ((SOCKADDR_IN*)&myAddrLocal)->sin_port;

		msg.Data().hostInfo.extAddr.ipAddress = ((SOCKADDR_IN*)&myAddrPublic)->sin_addr.S_un.S_addr;	
		msg.Data().hostInfo.extAddr.port = ((SOCKADDR_IN*)&myAddrPublic)->sin_port;

		TCHAR localstr[256], publicstr[256];
		mbstowcs(localstr, inet_ntoa(((SOCKADDR_IN*)&myAddrLocal)->sin_addr), 255);
		mbstowcs(publicstr, inet_ntoa(((SOCKADDR_IN*)&myAddrPublic)->sin_addr), 255);
		debugf(TEXT("          <---------- [PM::GAME::START_NTF::Send] msg.Data().hostInfo.~ %s:%d %s:%d"), 
			localstr, ((SOCKADDR_IN*)&myAddrLocal)->sin_port, publicstr, ((SOCKADDR_IN*)&myAddrPublic)->sin_port);
	}
	#endif
	// }} 20070224 dEAthcURe

	SEND_MSG_AUTO(msg);
}

void PM::GAME::READY_NTF::Send()
{
	CHECK_NO_CONNECTION()

	TMSG msg;
	SEND_MSG_AUTO(msg);

	// {{ 20070531 dEAthcURe|HM 호스트가 준비된 시점의 시간을 기록한다.
	if(GavaNetClient) {
		GavaNetClient->timeHostBegin = GCurrentTime;
		debugf(TEXT("[HM|PM::GAME::READY_NTF::Send] TimeBeginHost = %f"), GavaNetClient->timeHostBegin);
	}	
	// }} 20070531 dEAthcURe|HM 호스트가 준비된 시점의 시간을 기록한다.
}


void PM::GAME::JOIN_NTF::Send(UDP_HOST_INFO &addrInfo)
{
	CHECK_NO_CONNECTION()

	TMSG msg;

	msg.Data().idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
	msg.Data().hostInfo = addrInfo;

	SEND_MSG_AUTO(msg);		
	GavaNetClient->CurrentConnection()->Tick(); // [!] flush it immediately
}

void PM::GAME::UPDATE_STATE_NTF::Send(BYTE RoundCount)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INGAME)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}
	if (!_StateController->AmIHost())
	{
		PROC_MSG_SEND_ERROR(TEXT("host only"));
		return;
	}

	TMSG msg;

	msg.Data().roundCount = RoundCount;

	SEND_MSG_AUTO(msg);
}

void PM::GAME::LEAVE_NTF::Send()
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INGAME)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;

	SEND_MSG_AUTO(msg);
	GavaNetClient->CurrentConnection()->Tick(); // 20070501 dEAthcURe|HM flush it immediately
}

void PM::GAME::END_NTF::Send(INT AvgHostPing)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INGAME)// && _StateController->GetNetState() != _AN_ROOM)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}
	if (!_StateController->AmIHost())
	{
		PROC_MSG_SEND_ERROR(TEXT("host only"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	def.avgHostPing = (WORD)AvgHostPing;

	SEND_MSG_AUTO(msg);
}

void PM::GAME::UPDATE_SCORE_NTF::Send(PLAYER_RESULT_INFO &PlayerScore)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INGAME) // && _StateController->GetNetState() != _AN_ROOM)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	if (!_StateController->AmIHost())
	{
		PROC_MSG_SEND_ERROR(TEXT("host only"));
		return;
	}

	TMSG msg( TMSG::BODY_LENGTH + PM::_GetBufferSize<PLAYER_RESULT_INFO>(1) );
	DEF &def = msg.Data();
	PM::_MsgBuffer<PLAYER_RESULT_INFO> ScoreBuf(msg, def.playerScore);

	ScoreBuf.SetOffset(0);
	ScoreBuf.SetCount(1);
	ScoreBuf.SetBuffer(&PlayerScore);

	msg.Data().bGameEnd = 0;
	msg.Data().teamWinCount[0] = 0;
	msg.Data().teamWinCount[1] = 0;

	SEND_MSG_AUTO(msg);
}

void PM::GAME::UPDATE_SCORE_NTF::Send(const TArray<PLAYER_RESULT_INFO> &PlayerScore, INT TeamScore[2], UBOOL bGameEnd)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INGAME) // && _StateController->GetNetState() != _AN_ROOM)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	if (!_StateController->AmIHost())
	{
		PROC_MSG_SEND_ERROR(TEXT("host only"));
		return;
	}
	if (PlayerScore.Num() == 0)
	{
		PROC_MSG_SEND_ERROR(TEXT("no player"));
		return;
	}

	TMSG msg( TMSG::BODY_LENGTH + PM::_GetBufferSize<PLAYER_RESULT_INFO>(PlayerScore.Num()) );
	DEF &def = msg.Data();
	PM::_MsgBuffer<PLAYER_RESULT_INFO> ScoreBuf(msg, def.playerScore);

	ScoreBuf.SetOffset(0);
	ScoreBuf.SetCount(PlayerScore.Num());
	ScoreBuf.SetBuffer((PLAYER_RESULT_INFO*)PlayerScore.GetData(), PlayerScore.Num());

	msg.Data().bGameEnd = bGameEnd ? 1 : 0;
	msg.Data().teamWinCount[0] = (BYTE)TeamScore[0];
	msg.Data().teamWinCount[1] = (BYTE)TeamScore[1];

	SEND_MSG_AUTO(msg);
}

void PM::GAME::HOST_BAN_NTF::Send()
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INGAME && _StateController->GetNetState() != _AN_ROOM)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}
	if (!_StateController->RoomInfo.IsValid() || _StateController->RoomInfo.PlayerList.IsEmpty(_StateController->RoomInfo.HostIdx))
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	msg.Data().idHostAccount = _StateController->RoomInfo.PlayerList.PlayerList[_StateController->RoomInfo.HostIdx].PlayerInfo.idAccount;
	SEND_MSG_AUTO(msg);
	GavaNetClient->CurrentConnection()->Tick(); // [!] flush it immediately
}

void PM::GAME::START_COUNT_NTF::Send()
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

void PM::GAME::CANCEL_COUNT_NTF::Send()
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_ROOM)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}
	if (!_StateController->AmIHost())
	{
		PROC_MSG_SEND_ERROR(TEXT("host only"));
		return;
	}

	TMSG msg;
	SEND_MSG_AUTO(msg);
}

void PM::GAME::REPORT_STAT_NTF::Send(STAT_GAME_SCORE_LOG *GameScoreLogs,
									 TArray<STAT_ROUND_PLAY_LOG> &RoundPlayLogs,
									 TArray<STAT_WEAPON_LOG> &WeaponLogs,
									 TArray<STAT_KILL_LOG> &KillLogs)
{
	CHECK_NO_CONNECTION()

	if (!_StateController->AmIHost())
	{
		PROC_MSG_SEND_ERROR(TEXT("host only"));
		return;
	}

	TMSG msg(TMSG::BODY_LENGTH +
			PM::_GetBufferSize<STAT_ROUND_PLAY_LOG>(RoundPlayLogs.Num()) +
			PM::_GetBufferSize<STAT_WEAPON_LOG>(WeaponLogs.Num()) +
			PM::_GetBufferSize<STAT_KILL_LOG>(KillLogs.Num()));
	DEF &def = msg.Data();

	//_LOG(TEXT("Buffer Size = %d"), msg.GetBufLength());

	appMemcpy(def.gameScoreLogs, GameScoreLogs, 2 * sizeof(STAT_GAME_SCORE_LOG));

	//for (INT i = 0; i < 2; ++i)
	//{
	//	_LOG(TEXT("GameScore[%d]"), i);
	//	_LOG(TEXT("\tKillCount = %d"), def.gameScoreLogs[i].killCount);
	//	_LOG(TEXT("\tSuicideCount = %d"), def.gameScoreLogs[i].suicideCount);
	//	_LOG(TEXT("\tHeadshotKillCount = %d"), def.gameScoreLogs[i].headshotKillCount);
	//	_LOG(TEXT("\tScore.Attacker = %d"), def.gameScoreLogs[i].score.attacker);
	//	_LOG(TEXT("\tScore.Defender = %d"), def.gameScoreLogs[i].score.defender);
	//	_LOG(TEXT("\tScore.Leader = %d"), def.gameScoreLogs[i].score.leader);
	//	_LOG(TEXT("\tScore.Tactic = %d"), def.gameScoreLogs[i].score.tactic);
	//	_LOG(TEXT("\tFriendlyFireCount = %d"), def.gameScoreLogs[i].friendlyFireCount);
	//	_LOG(TEXT("\tFriendlyKillCount = %d"), def.gameScoreLogs[i].friendlyKillCount);
	//	_LOG(TEXT("\tSpawnCount[0] = %d"), def.gameScoreLogs[i].spawnCount[0]);
	//	_LOG(TEXT("\tSpawnCount[1] = %d"), def.gameScoreLogs[i].spawnCount[1]);
	//	_LOG(TEXT("\tSpawnCount[2] = %d"), def.gameScoreLogs[i].spawnCount[2]);
	//}

	PM::_MsgBuffer<STAT_ROUND_PLAY_LOG> RoundPlayBuf(msg, def.roundPlayLogs);
	RoundPlayBuf.SetOffset(0);
	RoundPlayBuf.SetCount(RoundPlayLogs.Num());
	for (INT i = 0; i < RoundPlayLogs.Num(); ++i)
	{
		RoundPlayBuf.SetBuffer(i, &RoundPlayLogs(i));

		STAT_ROUND_PLAY_LOG &RoundLog = RoundPlayBuf[i];

		//_LOG(TEXT("RoundPlayLog[%d]"), i);
		//_LOG(TEXT("\tWinner = %d"), RoundLog.winTeam);
		//_LOG(TEXT("\tWinType = %d"), RoundLog.winType);
		//_LOG(TEXT("\tStartTime = %d"), RoundLog.startTime);
		//_LOG(TEXT("\tRoundTime = %d"), RoundLog.roundTime);
	}

	PM::_MsgBuffer<STAT_WEAPON_LOG> WeaponBuf(msg, def.roundWeaponLogs);
	WeaponBuf.SetOffsetAfter(RoundPlayBuf);
	WeaponBuf.SetCount(WeaponLogs.Num());
	for (INT i = 0; i < WeaponLogs.Num(); ++i)
	{
		WeaponBuf.SetBuffer(i, &WeaponLogs(i));

		STAT_WEAPON_LOG &WeaponLog = WeaponBuf[i];

		//_LOG(TEXT("WeaponLog[%d]"), i);
		//_LOG(TEXT("\tRound = %d"), WeaponLog.round);
		//_LOG(TEXT("\tWeaponID = %d"), WeaponLog.idWeapon);
		//_LOG(TEXT("\tUsedCount = %d"), WeaponLog.usedCount);
		//_LOG(TEXT("\tFireCount = %d"), WeaponLog.fireCount);
		//_LOG(TEXT("\tHitCount_Head = %d"), WeaponLog.hitCount_Head);
		//_LOG(TEXT("\tHitCount_Body = %d"), WeaponLog.hitCount_Body);
		//_LOG(TEXT("\tHitCount_Stomach = %d"), WeaponLog.hitCount_Stomach);
		//_LOG(TEXT("\tHitCount_LeftArm = %d"), WeaponLog.hitCount_LeftArm);
		//_LOG(TEXT("\tHitCount_RightArm = %d"), WeaponLog.hitCount_RightArm);
		//_LOG(TEXT("\tHitCount_LeftLeg = %d"), WeaponLog.hitCount_LeftLeg);
		//_LOG(TEXT("\tHitCount_RightLeg = %d"), WeaponLog.hitCount_RightLeg);
		//_LOG(TEXT("\tHitDistance = %.2f"), WeaponLog.hitDistance);
		//_LOG(TEXT("\tHitDamage = %d"), WeaponLog.hitDamage);
		//_LOG(TEXT("\tKillCount[0] = %d"), WeaponLog.killCount[0]);
		//_LOG(TEXT("\tKillCount[1] = %d"), WeaponLog.killCount[1]);
		//_LOG(TEXT("\tKillCount[2] = %d"), WeaponLog.killCount[2]);
		//_LOG(TEXT("\tHeadshotKillCount = %d"), WeaponLog.headshotKillCount);
		//_LOG(TEXT("\tMultiKillCount = %d"), WeaponLog.multiKillCount);
	}

	PM::_MsgBuffer<STAT_KILL_LOG> KillBuf(msg, def.killLogs);
	KillBuf.SetOffsetAfter(WeaponBuf);
	KillBuf.SetCount(KillLogs.Num());
	for (INT i = 0; i < KillLogs.Num(); ++i)
	{
		KillBuf.SetBuffer(i, &KillLogs(i));

		STAT_KILL_LOG &KillLog = KillBuf[i];

		//_LOG(TEXT("KillLog[%d]"), i);
		//_LOG(TEXT("\tKillTime = %d"), KillLog.killTime);
		//_LOG(TEXT("\tKillerLoc = (%.2f, %.2f, %.2f)"), KillLog.killerLocX, KillLog.killerLocY, KillLog.killerLocZ);
		//_LOG(TEXT("\tVictimLoc = (%.2f, %.2f, %.2f)"), KillLog.victimLocX, KillLog.victimLocY, KillLog.victimLocZ);
	}

	//_LOG(TEXT("Dumping message buffer... %d bytes"), msg.GetBuf()->GetLength());
	//FString DumpBuf;
	//for (DWORD i = 0; i < msg.GetBuf()->GetLength(); ++i)
	//{
	//	DumpBuf += FString::Printf(TEXT("%02x "), msg.GetBuf()->GetData()[i]);

	//	if (i % 16 == 15)
	//	{
	//		_LOG(*DumpBuf);
	//		DumpBuf = TEXT("");
	//	}
	//}
	//if (DumpBuf.Len() > 0)
	//	_LOG(*DumpBuf);

	SEND_MSG_AUTO(msg);
}

#ifdef EnableClientPreloading // 20080103 dEAthcURe|LP test dd
extern bool GbClientPreloading;
#endif

void PM::GAME::LOADING_PROGRESS_NTF::Send(BYTE Progress, INT Step)
{
	CHECK_NO_CONNECTION()

#ifdef EnableClientPreloading // 20080103 dEAthcURe|LP test dd		
	if (!GbClientPreloading && _StateController->GetNetState() != _AN_INGAME)		
#else
	if (_StateController->GetNetState() != _AN_INGAME)
#endif	
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	msg.Data().progress = Progress;
	msg.Data().step = Step;

	SEND_MSG_AUTO_F(msg);
}

void PM::GAME::REPORT_VOTE_NTF::Send(DWORD Command, DWORD idCaller, const TArray<INT> &VoterList, DWORD param1, DWORD param2)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INGAME)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	def.command = Command;
	def.idCaller = idCaller;
	def.param1 = param1;
	def.param2 = param2;

	for (INT i = 0; i < MAX_PLAYER_PER_TEAM-1; ++i)
	{
		def.idVoter[i] = (i < VoterList.Num() ? VoterList(i) : ID_INVALID_ACCOUNT);
	}

	SEND_MSG_AUTO(msg);
}

void PM::GAME::CHAT_NTF::Send( const FString &ChatMsg, UBOOL Team )
{
	CHECK_NO_CONNECTION()

	if (!_StateController->PlayerInfo.IsValid())
		return;

	if (_StateController->GetNetState() != _AN_INGAME && !_StateController->RoomInfo.IsPlaying())
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg(TMSG::BODY_LENGTH + PM::_GetStringSize((WCHAR*)*ChatMsg));

	msg.Data().idAccount = _StateController->PlayerInfo.PlayerInfo.idAccount;
	msg.Data().team = Team ? 1 : 0;

	PM::_MsgString MsgString(msg, msg.Data().chatmsg);
	MsgString.SetOffset(0);
	MsgString = *ChatMsg;

	SEND_MSG_AUTO(msg);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcMsgSendErrorGame(WORD MsgID, const FString& Err)
{
	BYTE id = (MsgID & 0xff);

		using namespace PM::GAME;

		switch (id)
		{
			CASE_MSG_SEND_ERROR(Game, Start, START_NTF, Err)
			//CASE_MSG_SEND_ERROR(Game, Ready, READY_NTF, Err)
			//CASE_MSG_SEND_ERROR(Game, Join, JOIN_NTF, Err)
			//CASE_MSG_SEND_ERROR(Game, UPDATE_STATE_NTF, Err)
			CASE_MSG_SEND_ERROR(Game, Leave, LEAVE_NTF, Err)
			CASE_MSG_SEND_ERROR(Game, End, END_NTF, Err)
			CASE_MSG_SEND_ERROR(Game, ResultUpdate, UPDATE_SCORE_NTF, Err)
			//CASE_MSG_SEND_ERROR(Game, HOST_BAN_NTF, Err)
			CASE_MSG_SEND_ERROR(Game, StartCount, START_COUNT_NTF, Err)
			CASE_MSG_SEND_ERROR(Game, CancelCount, CANCEL_COUNT_NTF, Err)
			//CASE_MSG_SEND_ERROR(Game, REPORT_STAT_NTF, Err)
			//CASE_MSG_SEND_ERROR(Game, LOADING_PROGRESS_NTF, Err)
			//CASE_MSG_SEND_ERROR(Game, REPORT_VOTE_NTF, Err)
		default:
			_LOG(TEXT("Failed to send some GAME message. ID = %d"), id);
		}
}
