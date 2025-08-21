/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaGame

	Name: avaNetHandler.cpp

	Description: Implementation of avaNetHandler

***/
#include "avaGame.h"

#include "avaNetClient.h"
#include "avaNetPrivate.h"
#include "avaNetClasses.h"
#include "avaNetStateController.h"
#include "avaNetEventHandler.h"
#include "avaMsgSend.h"
#include "avaStaticData.h"
#include "avaCommunicator.h"
#include "ComDef/Def.h"
#include "ComDef/Inventory.h"
#include "ComDef/WordCensor.h"

#include "avaTransactions.h"

IMPLEMENT_CLASS(UavaNetHandler);

FavaPlayerScoreInfo::FavaPlayerScoreInfo()
{
	appMemset(this, 0, sizeof(FavaPlayerScoreInfo));
	idAccount = Def::ID_INVALID_ACCOUNT;
}

// 임시로 avaNetRequest.cpp에 만든 함수를 갖다 씁니다.
extern INT GetInvenIndexFromSlot( INT InvenSlot );

UavaNetHandler::UavaNetHandler()
{
}


FString UavaNetHandler::GetConfigString()
{
	if (_StateController->PlayerInfo.IsValid())
		return _StateController->PlayerInfo.PlayerInfo.configstr;
	else
		return TEXT("");
}

FString UavaNetHandler::GetConfigString2()
{
	if (_StateController->PlayerInfo.IsValid())
		return _StateController->PlayerInfo.PlayerInfo.configstr2;
	else
		return TEXT("");
}

// {{ 20070531 dEAthcURe|HM host exit조건
UBOOL UavaNetHandler::CanExitGame()
{
	if (!_StateController->AmIHost()) { // 호스트가 아니면 잡지않는다.		
		return TRUE;
	}

	if(!GavaNetClient) {
		return TRUE; // avanetclient가 invalid하면 나갈 수 있다.
	}
	
	if(GavaNetClient->timeHostBegin + 120.0f <= GCurrentTime) { // 호스트가 시작된 이후 2분이 지나면 나갈수있다.
		return TRUE;
	}
	return FALSE;	
}
// }} 20070531 dEAthcURe|HM host exit조건

void UavaNetHandler::LeaveRoom(BYTE Reason)
{
	GetAvaNetRequest()->LeaveRoom(Reason);
}

void UavaNetHandler::KickSlowLoadingPlayers(INT CutOffPerc)
{
	_LOG(TEXT("KickSlowLoadingPlayers()"));

	for (INT i = RoomStartingPlayerList.Num() - 1; i >= 0; --i)
	{
		if (RoomStartingPlayerList(i).LoadingProgress <= CutOffPerc)
		{
			//GetAvaNetRequest()->RoomKickPlayer(RoomStartingPlayerList(i).NickName, 1);
			RoomStartingPlayerList.Remove(i);
			//ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("cancel"), TEXT(""), i, 0);
			ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("skip check"), TEXT(""), i, 0);
		}
	}
}


void UavaNetHandler::CheckLoadingTimeOut()
{
	if (_StateController->GetNetState() == _AN_INGAME && _StateController->AmIHost() && RoomStartingPlayerList.Num() > 0)
	{
		DOUBLE CurTime = appSeconds();

		_LOG(TEXT("CheckLoadingTimeOut() --> HOST"));
		for (INT i = RoomStartingPlayerList.Num() - 1; i >= 0; --i)
		{
			FavaRoomPlayerInfo &Info = RoomStartingPlayerList(i);
			if ( (Info.LoadingStepCount == 0 && CurTime - Info.UpdateTime > 90) ||
				(Info.LoadingStepCount > 0 && CurTime - Info.UpdateTime > 30) )
			{
				// remove the client who had no progress after the last check time
				_LOG(TEXT("CheckLoadingTimeOut() --> Skipping [%d]%d from loading check; CurTime = %d, UpdateTime = %d"),
									RoomStartingPlayerList(i).AccountID, *RoomStartingPlayerList(i).NickName,
									(DWORD)CurTime, (DWORD)Info.UpdateTime);
				RoomStartingPlayerList.Remove(i);
				ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("skip check"), TEXT(""), i, 0);
			}
		}

		if (RoomStartingPlayerList.Num() > 0)
		{
			LoadingCheckTime = appSeconds() + 30.0;
		}
		else
		{
			LoadingCheckTime = 0.0;
			LoadingCutOffPerc = -1;
		}
	}
	else
	{
		LoadingCheckTime = 0.0;
		LoadingCutOffPerc = -1;
	}

	return;
}

void UavaNetHandler::StartLoadingCheck()
{
	DOUBLE CurTime = appSeconds();

	for (INT i = 0; i < RoomStartingPlayerList.Num(); ++i)
	{
		RoomStartingPlayerList(i).UpdateTime = CurTime;
	}

	GetAvaNetHandler()->LoadingCheckTime = CurTime + LOADING_CHECK_TIME;
	GetAvaNetHandler()->LoadingCutOffPerc = 0;
}

void UavaNetHandler::CollectLastResultInfo()
{
	if (/*_StateController->LastResultInfo.InfoLevel == 3 || */!_StateController->RoomInfo.IsValid())
		return;
	//if (!_StateController->LastResultInfo.bNeedInfo)
	//	return;

	_LOG(TEXT("LastResultInfo::CollectLastResultInfo()"));

	//_StateController->LastResultInfo.RoomResultInfo.Empty();

	if (GWorld && GWorld->GetWorldInfo())
	{
		if (GWorld->URL.Map == FURL::DefaultMap)
		{
			_LOG(TEXT("Cannot collect result info from default map game"));
			return;
		}

		AavaGameReplicationInfo *GRI = Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI );
		if (GRI)
		{
			INT BaseIdx = 0;
			for (INT Team = 0; Team < 2; ++Team)
			{
				_StateController->LastResultInfo.TeamScore[Team] = (GRI->Teams[Team] ? GRI->Teams[Team]->Score : 0);

				for (INT i = 0; i < GRI->PRIArray.Num(); ++i)
				{
					AavaPlayerReplicationInfo *PRI = Cast<AavaPlayerReplicationInfo>(GRI->PRIArray(i));
					if (PRI)
					{
						FRoomPlayerInfo *Player = _StateController->RoomInfo.PlayerList.Find(PRI->AccountID);
						if (!Player)
						{
							_LOG(TEXT("Player not found! AccountID = %d"), PRI->AccountID);
							continue;
						}

						//_LOG(TEXT("Player %d; %s"), Player->RoomPlayerInfo.idSlot, Player->RoomPlayerInfo.nickname);

						if (Player->RoomPlayerInfo.idSlot < 0 || Player->RoomPlayerInfo.idSlot >= MAX_PLAYER_PER_ROOM)
							continue;
						if ((Team == 0 && Player->RoomPlayerInfo.idSlot >= MAX_PLAYER_PER_ROOM / 2) ||
							(Team == 1 && Player->RoomPlayerInfo.idSlot < MAX_PLAYER_PER_ROOM / 2))
							continue;

						FLastResultInfo::FPlayerResultInfo *Result = _StateController->LastResultInfo.FindPlayer(Player->RoomPlayerInfo.idSlot);
						if (!Result)
						{
							Result = new(_StateController->LastResultInfo.RoomResultInfo) FLastResultInfo::FPlayerResultInfo;
							check(Result);
						}

						Result->idSlot = Player->RoomPlayerInfo.idSlot;
						//Result->idAccount = PRI->AccountID;
						Result->Level = PRI->Level;
						Result->Nickname = PRI->PlayerName;
						Result->Score = PRI->Score;//PRI->AttackPoint + PRI->DefencePoint + PRI->LeaderPoint + PRI->TacticsPoint;
						Result->Death = PRI->Deaths;
						Result->xp = -1;
						Result->SupplyPoint = -1;

						Result->bPcBang = Player->RoomPlayerInfo.pcBang > 0;
					}
				}
			}

			_StateController->LastResultInfo.Sort();

			_StateController->LastResultInfo.InfoLevel = 3;
			return;
		}
	}
	_LOG(TEXT("LastResultInfo::CollectLastResultInfo() : failed."));
}

EMsgCategory GAVANET_MsgCat;
INT GAVANET_MsgID;
FString GAVANET_Param1;
FString GAVANET_Param2;
INT GAVANET_Param3, GAVANET_Param4;

void UavaNetHandler::ProcMessage(EMsgCategory MsgCat, INT MsgID, const FString &Param1, const FString &Param2, INT Param3, INT Param4)
{
	check( IsInGameThread() );

	GAVANET_MsgCat = MsgCat;
	GAVANET_MsgID = MsgID;
	GAVANET_Param1 = Param1;
	GAVANET_Param2 = Param2;
	GAVANET_Param3 = Param3;
	GAVANET_Param4 = Param4;

	switch (MsgCat)
	{
	case EMsg_Client:
		eventProcClientMessage(MsgID, Param1, Param2, Param3, Param4);
		break;

	case EMsg_Channel:
		eventProcChannelMessage(MsgID, Param1, Param2, Param3, Param4);
		break;

	case EMsg_Room:
		eventProcRoomMessage(MsgID, Param1, Param2, Param3, Param4);
		break;

	case EMsg_Game:
		eventProcGameMessage(MsgID, Param1, Param2, Param3, Param4);
		break;

	case EMsg_Inventory:
		eventProcInventoryMessage(MsgID, Param1, Param2, Param3, Param4);
		break;

	case EMsg_Admin:
		eventProcAdminMessage(MsgID, Param1, Param2, Param3, Param4);
		break;

	case EMsg_Buddy:
		eventProcBuddyMessage(MsgID, Param1, Param2, Param3, Param4);
		break;

	case EMsg_Guild:
		eventProcGuildMessage(MsgID, Param1, Param2, Param3, Param4);
		break;

	case EMsg_Option:
		eventProcOptionMessage(MsgID, Param1, Param2, Param3, Param4);
		break;
	}
}


UBOOL UavaNetHandler::IsPlayerAdult()
{
	return _StateController->PlayerInfo.Age >= 18;
}


FString UavaNetHandler::GetConnectResult()
{
	FString Result = _StateController->LastConnectResult;

	if (Result != TEXT("already connected"))
		_StateController->LastConnectResult = TEXT("");

	return Result;
}

void UavaNetHandler::ListChannel()
{
	GetAvaNetRequest()->ListChannel();
}


void UavaNetHandler::ListRoom()
{
	GetAvaNetRequest()->ListRoom();
}


void _SetPlayerScore(Def::PLAYER_RESULT_INFO &Info, FavaPlayerScoreInfo &ScoreInfo)
{
	appMemset(&Info, 0, sizeof(Def::PLAYER_RESULT_INFO));
	Info.idAccount = ScoreInfo.idAccount;
	Info.gameWin = ScoreInfo.GameWin;
	Info.roundWin = ScoreInfo.RoundWin;
	Info.roundDefeat = ScoreInfo.RoundDefeat;
	Info.disconnectCount = ScoreInfo.DisconnectCount;
	//Info.killCount = ScoreInfo.KillCount;
	Info.deathCount = ScoreInfo.DeathCount;
	Info.score.attacker = Clamp(ScoreInfo.Score.Attacker, 0, USHRT_MAX);
	Info.score.defender = Clamp(ScoreInfo.Score.Defender, 0, USHRT_MAX);
	Info.score.leader = Clamp(ScoreInfo.Score.Leader, 0, USHRT_MAX);
	Info.score.tactic = Clamp(ScoreInfo.Score.Tactic, 0, USHRT_MAX);
	//Info.supplyPoint = ScoreInfo.SupplyPoint;

	for (INT i = 0; i < Def::_CLASS_MAX; ++i)
	{
		Info.classResultInfo[i].playTime = ScoreInfo.ClassScoreInfo[i].PlayTime;
		Info.classResultInfo[i].playRound = ScoreInfo.ClassScoreInfo[i].PlayRound;
		Info.classResultInfo[i].sprintTime = ScoreInfo.ClassScoreInfo[i].SprintTime;
		Info.classResultInfo[i].killCount = ScoreInfo.ClassScoreInfo[i].KillCount;
		Info.classResultInfo[i].headshotCount = ScoreInfo.ClassScoreInfo[i].HeadshotCount;
		Info.classResultInfo[i].headshotKillCount = ScoreInfo.ClassScoreInfo[i].HeadshotKillCount;
		Info.classResultInfo[i].takenDamage = ScoreInfo.ClassScoreInfo[i].TakenDamage;

		for (INT j = 0; j < 4; ++j)
		{
			Info.classResultInfo[i].weaponDamage[j] = ScoreInfo.ClassScoreInfo[i].WeaponDamage[j];
			Info.classResultInfo[i].weaponKillCount[j] = ScoreInfo.ClassScoreInfo[i].WeaponKillCount[j];
		}
	}

	Info.roundTopKillCount = ScoreInfo.RoundTopKillCount;
	Info.roundTopHeadshotKillCount = ScoreInfo.RoundTopHeadshotKillCount;
	Info.topLevelKillCount = ScoreInfo.TopLevelKillCount;
	Info.higherLevelKillCount = ScoreInfo.HigherLevelKillCount;
	Info.bulletMultiKillCount = ScoreInfo.BulletMultiKillCount;
	Info.grenadeMultiKillCount = ScoreInfo.GrenadeMultiKillCount;
	Info.noDamageWinCount = ScoreInfo.NoDamageWinCount;

	Info.topScoreCount = ScoreInfo.TopScoreCount;
	Info.noDeathRoundCount = ScoreInfo.NoDeathRoundCount;

	for (INT i = 0; i < 4; ++i)
	{
		Info.weaponFireCount[i] = ScoreInfo.WeaponFireCount[i];
		Info.weaponHitCount[i] = ScoreInfo.WeaponHitCount[i];
		Info.weaponHeadshotCount[i] = ScoreInfo.WeaponHeadshotCount[i] ;
	}

	Info.helmetDropCount = ScoreInfo.HelmetDropCount;

	Info.teamKillCount = ScoreInfo.TeamKillCount;

	Info.currentClass = ScoreInfo.CurrentClass;
	Info.bLeader = (ScoreInfo.bLeader ? 1 : 0);

#if !FINAL_RELEASE
	_LOG(TEXT("Player result info; idAccount = %d %s"), Info.idAccount, Info.bLeader ? TEXT(" (Leader)") : TEXT(""));
	_LOG(TEXT("  GameWin = %d, RoundWin = %d, RoundDefeat = %d, DeathCount = %d, Score = %d/%d/%d/%d"),
				Info.gameWin, Info.roundWin, Info.roundDefeat, Info.deathCount, Info.score.attacker, Info.score.defender, Info.score.leader, Info.score.tactic);
	_LOG(TEXT("  Pointman;"));
	_LOG(TEXT("    PlayTime = %d, PlayRound = %d, SprintTime = %d, KillCount = %d, TakenDamage = %d, HeadshotCount = %d, HeadshotKillCount = %d"),
				Info.classResultInfo[0].playTime, Info.classResultInfo[0].playRound, Info.classResultInfo[0].sprintTime,
				Info.classResultInfo[0].killCount, Info.classResultInfo[0].takenDamage,
				Info.classResultInfo[0].headshotCount, Info.classResultInfo[0].headshotKillCount);
	_LOG(TEXT("    SMGDamage = %d, SMGKillCount = %d, PistolDamage = %d, PistolKillCount = %d, KnifeDamage = %d, KnifeKillCount = %d, GrenadeDamage = %d, GrenadeKillCount = %d"),
				Info.classResultInfo[0].weaponDamage[3], Info.classResultInfo[0].weaponKillCount[3],
				Info.classResultInfo[0].weaponDamage[1], Info.classResultInfo[0].weaponKillCount[1],
				Info.classResultInfo[0].weaponDamage[0], Info.classResultInfo[0].weaponKillCount[0],
				Info.classResultInfo[0].weaponDamage[2], Info.classResultInfo[0].weaponKillCount[2]);
	_LOG(TEXT("  Rifleman;"));
	_LOG(TEXT("    PlayTime = %d, PlayRound = %d, SprintTime = %d, KillCount = %d, TakenDamage = %d, HeadshotCount = %d, HeadshotKillCount = %d"),
				Info.classResultInfo[1].playTime, Info.classResultInfo[1].playRound, Info.classResultInfo[1].sprintTime,
				Info.classResultInfo[1].killCount, Info.classResultInfo[1].takenDamage,
				Info.classResultInfo[1].headshotCount, Info.classResultInfo[1].headshotKillCount);
	_LOG(TEXT("    ARDamage = %d, ARKillCount = %d, PistolDamage = %d, PistolKillCount = %d, KnifeDamage = %d, KnifeKillCount = %d, GrenadeDamage = %d, GrenadeKillCount = %d"),
				Info.classResultInfo[1].weaponDamage[3], Info.classResultInfo[1].weaponKillCount[3],
				Info.classResultInfo[1].weaponDamage[1], Info.classResultInfo[1].weaponKillCount[1],
				Info.classResultInfo[1].weaponDamage[0], Info.classResultInfo[1].weaponKillCount[0],
				Info.classResultInfo[1].weaponDamage[2], Info.classResultInfo[1].weaponKillCount[2]);
	_LOG(TEXT("  Sniper;"));
	_LOG(TEXT("    PlayTime = %d, PlayRound = %d, SprintTime = %d, KillCount = %d, TakenDamage = %d, HeadshotCount = %d, HeadshotKillCount = %d"),
				Info.classResultInfo[2].playTime, Info.classResultInfo[2].playRound, Info.classResultInfo[2].sprintTime,
				Info.classResultInfo[2].killCount, Info.classResultInfo[2].takenDamage,
				Info.classResultInfo[2].headshotCount, Info.classResultInfo[2].headshotKillCount);
	_LOG(TEXT("    SRDamage = %d, SRKillCount = %d, PistolDamage = %d, PistolKillCount = %d, KnifeDamage = %d, KnifeKillCount = %d, GrenadeDamage = %d, GrenadeKillCount = %d"),
				Info.classResultInfo[2].weaponDamage[3], Info.classResultInfo[2].weaponKillCount[3],
				Info.classResultInfo[2].weaponDamage[1], Info.classResultInfo[2].weaponKillCount[1],
				Info.classResultInfo[2].weaponDamage[0], Info.classResultInfo[2].weaponKillCount[0],
				Info.classResultInfo[2].weaponDamage[2], Info.classResultInfo[2].weaponKillCount[2]);
	for (INT i = 0; i < 4; ++i)
	{
		_LOG(TEXT("WeaponFireCount[%d] = %d, WeaponHitCount[%d] = %d, WeaponHeadshotCount[%d] = %d"),
					i, Info.weaponFireCount[i], i, Info.weaponHitCount[i], i, Info.weaponHeadshotCount[i]);
	}
	_LOG(TEXT("RoundTopKillCount = %d, RoundTopHeadshotKillCount = %d, TopLevelKillCount = %d, HigherLevelKillCount = %d"),
				Info.roundTopKillCount, Info.roundTopHeadshotKillCount, Info.topLevelKillCount, Info.higherLevelKillCount);
	_LOG(TEXT("BulletMultiKillCount = %d, GrenadeMultiKillCount = %d, TopScoreCount = %d"),
				Info.bulletMultiKillCount, Info.roundTopHeadshotKillCount, Info.topLevelKillCount);
	_LOG(TEXT("NoDamageWinCount = %d, NoDeathRoundCount = %d, HelmetDropCount = %d, TeamKillCount = %d"),
				Info.noDamageWinCount, Info.noDeathRoundCount, Info.helmetDropCount, Info.teamKillCount);
#endif
}


void UavaNetHandler::UpdatePlayerScore(FavaPlayerScoreInfo ScoreInfo)
{
	// 연습 채널은 업데이트하지 않음
	if (_StateController->GetChannelSetting(EChannelSetting_UpdatePlayerScore) == 0)
		return;

	Def::PLAYER_RESULT_INFO Info;

	_SetPlayerScore(Info, ScoreInfo);

	PM::GAME::UPDATE_SCORE_NTF::Send(Info);
}

void UavaNetHandler::UpdateGameState(INT RoundCount, BYTE GameState)
{
	switch (GameState)
	{
	case EGS_GameBegin:
		break;

	case EGS_GameEnd:
		_LOG(TEXT("EGS_GameEnd"));
		//CollectLastResultInfo();
		break;

	case EGS_RoundBegin:
		//RoomStartingPlayerList.Empty();
		if (_StateController->AmIHost())
			PM::GAME::UPDATE_STATE_NTF::Send((BYTE)RoundCount);
		break;

	case EGS_RoundEnd:
		//RoomStartingPlayerList.Empty();
		_LOG(TEXT("EGS_RoundEnd"));
		//CollectLastResultInfo();
		break;
	}
}

void UavaNetHandler::EndGame(const TArray<struct FavaPlayerScoreInfo> &ScoreInfoList, INT AvgHostPing)
{
	ReportEndGame(AvgHostPing);
}

void UavaNetHandler::ReportGameResult(const TArray<FavaPlayerScoreInfo> &ScoreInfoList, INT TeamScoreEU, INT TeamScoreNRF)
{
	_LOG(TEXT("TeamScoreEU = %d, TeamScoreNRF = %d"), TeamScoreEU, TeamScoreNRF);

	//CollectLastResultInfo();

	// 연습 채널은 업데이트하지 않음
	if (_StateController->GetChannelSetting(EChannelSetting_UpdatePlayerScore) == 0)
		return;

	if (_StateController->AmIHost())
	{
		TArray<Def::PLAYER_RESULT_INFO> InfoList;

		for (INT n = 0; n < ScoreInfoList.Num(); ++n)
		{
			FavaPlayerScoreInfo &ScoreInfo = (FavaPlayerScoreInfo&)ScoreInfoList(n);
			FRoomPlayerInfo *Player = _StateController->RoomInfo.PlayerList.Find(ScoreInfo.idAccount);
			if (!Player || Player->GetTeamID() == RT_SPECTATOR)
				continue;

			Def::PLAYER_RESULT_INFO *pInfo = new(InfoList) Def::PLAYER_RESULT_INFO;
			check(pInfo);
			_SetPlayerScore(*pInfo, ScoreInfo);
		}

		INT TeamScore[2];
		TeamScore[0] = TeamScoreEU;//(TeamScoreEU >= 0 ? TeamScoreEU : _StateController->LastResultInfo.TeamScore[0]);
		TeamScore[1] = TeamScoreNRF;//(TeamScoreNRF >= 0 ? TeamScoreNRF : _StateController->LastResultInfo.TeamScore[1]);

		if (InfoList.Num() > 0)
			PM::GAME::UPDATE_SCORE_NTF::Send(InfoList, TeamScore, TRUE);
	}
}

void UavaNetHandler::ReportEndGame(INT AvgHostPing)
{
	if (!_StateController->AmIHost())
	{
		_LOG(TEXT("I am not host; so don't need to report end game"));
		return;
	}

	_LOG(TEXT("ReportEndGame(); AvgHostPing = %d"), AvgHostPing);

	_LOG(TEXT("Returning to avaEntry..."));

	if (_StateController->GetNetState() < _AN_ROOM)
	{
		GEngine->SetClientTravel(TEXT("avaEntry.ut3"), TRAVEL_Absolute);
		//GEngine->Exec(TEXT("DISCONNECT"));
		return;
	}

	PM::GAME::END_NTF::Send(AvgHostPing);
}


void UavaNetHandler::ReportGameStat(FavaStatLog &StatLog)
{
	// 연습 채널은 업데이트하지 않음
	if (_StateController->GetChannelSetting(EChannelSetting_UpdateStatLog) == 0)
		return;

	FMapInfo *Map = _StateController->GetCurrentMap();
	if (!Map)
	{
		_LOG(TEXT("Error! Current map info not found."));
		return;
	}
	if (Map->bHidden || !Map->bStatLog)
	{
		_LOG(TEXT("This map is set to skip sending stat log."));
		return;
	}

	if (_StateController->RoomInfo.IsValid() && _StateController->AmIHost())
	{
		if (StatLog.RoundPlayLogs.Num() == 0 ||
			(StatLog.RoundPlayLogs.Num() == 1 && StatLog.RoundPlayLogs(0).RoundTime < 120))
		{
			_LOG(TEXT("Too short game. This log is not need to send."));
			return;
		}
		if (StatLog.RoundPlayLogs.Num() > 50)
		{
			_LOG(TEXT("Error! Too many rounds. This log should not be sent."));
			return;
		}

		STAT_GAME_SCORE_LOG statGameScore[2];
		TArray<STAT_ROUND_PLAY_LOG> statRoundPlay;
		TArray<STAT_WEAPON_LOG> statRoundWeapon;
		TArray<STAT_KILL_LOG> statKill;

		for (INT i = 0; i < 2; ++i)
		{
			statGameScore[i].killCount = StatLog.GameScoreLogs[i].KillCount;
			statGameScore[i].suicideCount = StatLog.GameScoreLogs[i].SuicideCount;
			statGameScore[i].headshotKillCount = StatLog.GameScoreLogs[i].HeadshotKillCount;
			statGameScore[i].score.attacker = StatLog.GameScoreLogs[i].Score.Attacker;
			statGameScore[i].score.defender = StatLog.GameScoreLogs[i].Score.Defender;
			statGameScore[i].score.leader = StatLog.GameScoreLogs[i].Score.Leader;
			statGameScore[i].score.tactic = StatLog.GameScoreLogs[i].Score.Tactic;
			statGameScore[i].friendlyFireCount = StatLog.GameScoreLogs[i].FriendlyFireCount;
			statGameScore[i].friendlyKillCount = StatLog.GameScoreLogs[i].FriendlyKillCount;
			statGameScore[i].spawnCount[0] = StatLog.GameScoreLogs[i].SpawnCount[0];
			statGameScore[i].spawnCount[1] = StatLog.GameScoreLogs[i].SpawnCount[1];
			statGameScore[i].spawnCount[2] = StatLog.GameScoreLogs[i].SpawnCount[2];

			_LOG(TEXT("statGameScore[%d].killCount = %d, statGameScore[%d].suicideCount = %d, statGameScore[%d].headshotKillCount = %d"),
					i, statGameScore[i].killCount, i, statGameScore[i].suicideCount, i, statGameScore[i].headshotKillCount);
		}

		for (INT i = 0; i < StatLog.RoundPlayLogs.Num(); ++i)
		{
			FavaRoundPlayLog &RoundPlayLog = StatLog.RoundPlayLogs(i);
			STAT_ROUND_PLAY_LOG *pStat = new(statRoundPlay) STAT_ROUND_PLAY_LOG;
			check(pStat);

			pStat->winTeam = RoundPlayLog.Winner;
			pStat->winType = RoundPlayLog.WinType;
			pStat->startTime = RoundPlayLog.StartTime;
			pStat->roundTime = RoundPlayLog.RoundTime;
			pStat->playerCount[0] = RoundPlayLog.PlayerCount[0];
			pStat->playerCount[1] = RoundPlayLog.PlayerCount[1];

			_LOG(TEXT("Round = %d, winTeam = %d, winType = %d"), i, pStat->winTeam, pStat->winType);

			for (INT j = 0; j < RoundPlayLog.WeaponLogs.Num(); ++j)
			{
				FavaWeaponLog &WeaponLog = RoundPlayLog.WeaponLogs(j);
				TID_ITEM idWeapon = ID_INVALID_ITEM;

				for (INT k = 0; k < WeaponIDList.Num(); ++k)
				{
					if (WeaponIDList(k).Weapon == WeaponLog.Weapon)
					{
						idWeapon = WeaponIDList(k).ItemId;
						break;
					}
				}

				if (idWeapon != ID_INVALID_ITEM)
				{
					ITEM_DESC *Desc = _ItemDesc().GetItem(idWeapon);
					if (!Desc)
					{
						_LOG(*FString::Printf(TEXT("ReportGameStat: Unknown weapon id detected! : name = %s, id = %d"), *WeaponLog.Weapon->GetPureName().ToString(), idWeapon));
						continue;
					}

					if (!Desc->statLog)
						continue;

					STAT_WEAPON_LOG *pStatWeapon = new(statRoundWeapon) STAT_WEAPON_LOG;
					check(pStatWeapon);

					pStatWeapon->round = i;
					pStatWeapon->idWeapon = idWeapon;
					pStatWeapon->usedCount = WeaponLog.UsedCount;
					pStatWeapon->fireCount = WeaponLog.FireCount;
					pStatWeapon->hitCount_Head = WeaponLog.HitCount_Head;
					pStatWeapon->hitCount_Body = WeaponLog.HitCount_Body;
					pStatWeapon->hitCount_Stomach = WeaponLog.HitCount_Stomach;
					pStatWeapon->hitCount_LeftArm = WeaponLog.HitCount_LeftArm;
					pStatWeapon->hitCount_RightArm = WeaponLog.HitCount_RightArm;
					pStatWeapon->hitCount_LeftLeg = WeaponLog.HitCount_LeftLeg;
					pStatWeapon->hitCount_RightLeg = WeaponLog.HitCount_RightLeg;
					//pStatWeapon->multiHitCount = WeaponLog.MultiHitCount;
					pStatWeapon->hitDistance = WeaponLog.HitDistance;
					pStatWeapon->hitDamage = WeaponLog.HitDamage;
					pStatWeapon->killCount[0] = WeaponLog.KillCount[0];
					pStatWeapon->killCount[1] = WeaponLog.KillCount[1];
					pStatWeapon->killCount[2] = WeaponLog.KillCount[2];
					pStatWeapon->headshotKillCount = WeaponLog.HeadshotKillCount;
					pStatWeapon->multiKillCount = WeaponLog.MultiKillCount;
				}
			}
		}

		for (INT i = 0; i < StatLog.KillLogs.Num(); ++i)
		{
			FavaKillLog &KillLog = StatLog.KillLogs(i);
			TID_ITEM idWeapon = ID_INVALID_ITEM;

			for (INT k = 0; k < WeaponIDList.Num(); ++k)
			{
				if (WeaponIDList(k).Weapon == KillLog.Weapon)
				{
					idWeapon = WeaponIDList(k).ItemId;
					break;
				}
			}

			if (idWeapon != ID_INVALID_ITEM)
			{
				STAT_KILL_LOG *pStat = new(statKill) STAT_KILL_LOG;
				check(pStat);

				pStat->idWeapon = idWeapon;

				pStat->killTime = KillLog.KillTime;
				pStat->killerLocX = KillLog.KillerLocation.X;
				pStat->killerLocY = KillLog.KillerLocation.Y;
				pStat->killerLocZ = KillLog.KillerLocation.Z;
				pStat->victimLocX = KillLog.VictimLocation.X;
				pStat->victimLocY = KillLog.VictimLocation.Y;
				pStat->victimLocZ = KillLog.VictimLocation.Z;
			}
		}

		PM::GAME::REPORT_STAT_NTF::Send(statGameScore, statRoundPlay, statRoundWeapon, statKill);

		_LOG(TEXT("Game statistics data reported; RoundPlayLogs = %d, WeaponLogs = %d, KillLogs = %d"), statRoundPlay.Num(), statRoundWeapon.Num(), statKill.Num());
	}
}


void UavaNetHandler::LeaveGame()
{
	GetAvaNetRequest()->LeaveGame();
}

void UavaNetHandler::DisconnectFromGame(INT NextState)
{
	if (_StateController->GetNetState() == _AN_INGAME)
	{
		GetAvaNetHandler()->RoomStartingPlayerList.Empty();

		if (LoadingCheckTime > 0.0)
		{
			_LOG(TEXT("Canceling the loading progress."));

			LoadingCheckTime = 0.0;
			LoadingCutOffPerc = -1;

			CavaNetEventHandler::NoticedProgress = -1;
			CavaNetEventHandler::NoticedTime = 0.0;

			PM::GAME::LOADING_PROGRESS_NTF::Send(255, -1);

			_StateController->GoToState(NextState);

			GEngine->SetClientTravel(TEXT("?closed"), TRAVEL_Absolute);
		}
		else
		{
			_StateController->GoToState(NextState);

			_LOG(TEXT("Disconnecting from game..."));
			GEngine->Exec(TEXT("DISCONNECT"));
		}
	}
}

void UavaNetHandler::VoteForHostBan()
{
	PM::GAME::HOST_BAN_NTF::Send();
}

void UavaNetHandler::AddChatMsg(const FString &ChatMsg, BYTE MsgType)
{
	_StateController->ChatMsgList.Add((FString&)ChatMsg, (INT)MsgType);
}

UBOOL UavaNetHandler::FilterChatMsg(FString &ChatMsg)
{
	if (ChatMsg.Len() > 0)
		return _WordCensor().ReplaceChatMsg((TCHAR*)*ChatMsg);
	else
		return FALSE;
}

UBOOL UavaNetHandler::ParseChatCommand( const FString& Cmd )
{
	return GetAvaNetRequest()->ParseChatCommand( Cmd );
}

void UavaNetHandler::GetWrappedString(TArray<FString>& out_strings,const FString& inMsg,INT MsgType,INT BoundSize/*=300*/)
{
	AavaGameReplicationInfo *GRI = GetWorldInfo() ? Cast<AavaGameReplicationInfo>(GetWorldInfo()->GRI) : NULL;
	UUISceneClient *SceneClient = GEngine && GEngine->GameViewport && GEngine->GameViewport->UIController ? Cast<UUISceneClient>(GEngine->GameViewport->UIController->SceneClient ) : NULL;
	UUISkin* ActiveSkin = SceneClient ? Cast<UUISkin>(SceneClient->ActiveSkin) : NULL;

	if( GRI == NULL || SceneClient == NULL || ActiveSkin == NULL )
	{
		out_strings.AddItem(inMsg);
		return;
	}

	
	FName StyleTagFound;
	for( INT i = 0 ; i < GRI->TextStyleData.Num() ; i++ )
	{
		FTextStyleInfo& StyleInfo = GRI->TextStyleData(i);
		if( StyleInfo.Id == MsgType)
		{
			StyleTagFound = FName(*StyleInfo.StyleTag);
			break;
		}
	}

	UUIStyle* Style = ActiveSkin->FindStyle( StyleTagFound );
	UUIStyle_Data* StyleData = Style ? Style->GetStyleForStateByClass(UUIState_Enabled::StaticClass()) : NULL;
	UUIStyle_Text* TextStyleData = StyleData ? Cast<UUIStyle_Text>(StyleData) : NULL;
	UUIStyle_Combo* ComboStyleData = StyleData ? Cast<UUIStyle_Combo>(StyleData) : NULL;
	if( ComboStyleData )
		TextStyleData = Cast<UUIStyle_Text>(ComboStyleData->TextStyle.GetStyleData());

	if( TextStyleData == NULL )
	{
		out_strings.AddItem(inMsg);
		return;
	}

	TArray<FWrappedStringElement> WrappedStringElements;
	FRenderParameters WrapStringParameters(0,0,BoundSize, 1000.f, TextStyleData->StyleFont);
	UUIString::WrapString( WrapStringParameters, 0, *inMsg, WrappedStringElements );

	for( INT i = 0 ; i < WrappedStringElements.Num() ; i++ )
		out_strings.AddItem(WrappedStringElements(i).Value);
}

UBOOL UavaNetHandler::AmIAdmin()
{
	return _StateController->AmIAdmin();
}

UBOOL UavaNetHandler::IsStealthMode()
{
	return (AmIAdmin() && _StateController->StealthMode);
}

UBOOL UavaNetHandler::IsInClanLobby()
{
	return _StateController->ChannelInfo.IsMyClanChannel();
}

BYTE UavaNetHandler::GetCurrentChannelFlag()
{
	return _StateController->ChannelInfo.IsValid() ? _StateController->ChannelInfo.Flag : UCHAR_MAX;
}

INT UavaNetHandler::GetChannelSetting(BYTE Setting)
{
	return _StateController->GetChannelSetting(Setting);
}

UBOOL UavaNetHandler::IsInPcBang()
{
	return _StateController->PlayerInfo.IsPcBang();
}

FString UavaNetHandler::GetMyNickname()
{
	return _StateController->PlayerInfo.IsValid() ? _StateController->PlayerInfo.PlayerInfo.nickname : TEXT("");
}

FString UavaNetHandler::GetMyClanName()
{
	return _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild != ID_INVALID_GUILD ? _StateController->PlayerInfo.PlayerInfo.guildInfo.guildName : TEXT("");
}

INT UavaNetHandler::GetMyAccountID()
{
	return _StateController->PlayerInfo.PlayerInfo.idAccount;
}

BYTE UavaNetHandler::GetMyBestChannelFlag()
{
	INT MyLevel = _StateController->PlayerInfo.PlayerInfo.level;
	EChannelFlag ChannelFlag = EChannelFlag_Trainee;

	if( MyLevel < _LEV_SERGEANT)
		ChannelFlag = EChannelFlag_Trainee;
	else
		ChannelFlag = EChannelFlag_Normal;

	return ChannelFlag;
}

INT UavaNetHandler::GetCurrentRoomState()
{
	return GetAvaNetRequest()->GetCurrentRoomState();
}

UBOOL UavaNetHandler::GetCurrentEquipState(INT &MyTeam, INT &MyClass, INT &MyFace, INT &MyWeapon)
{
	return GetAvaNetRequest()->GetCurrentEquipState(MyTeam, MyClass, MyFace, MyWeapon);
}

INT UavaNetHandler::GetHostAccountID()
{
	if (_StateController->RoomInfo.IsValid() && !_StateController->RoomInfo.PlayerList.IsEmpty(_StateController->RoomInfo.HostIdx))
	{
		return _StateController->RoomInfo.PlayerList.PlayerList[_StateController->RoomInfo.HostIdx].RoomPlayerInfo.idAccount;
	}
	return ID_INVALID_ACCOUNT;
}

UBOOL UavaNetHandler::AmIHost()
{
	return GetAvaNetRequest()->AmIHost();
}

UBOOL UavaNetHandler::AmISpectator()
{
	return (_StateController->RoomInfo.IsValid() && _StateController->AmISpectator());
}

UBOOL UavaNetHandler::AmIReady()
{
	return (_StateController->RoomInfo.IsValid() && _StateController->GetMyRoomPlayerInfo() && _StateController->GetMyRoomPlayerInfo()->RoomPlayerInfo.bReady);
}

UBOOL UavaNetHandler::IsCountingDown()
{
	return _StateController->IsCountingDown();
}

/*! @brief 에러 메시지를 얻기 위해서 IsGameStartableEx()함수로 옮겼습니다.(2007/03/12 고광록)
	@param ErrorMsg
		에러 메시지(MarkupString) -> 에러 메세지 타입("invalid room","needtoready","")으로 수정
*/
UBOOL UavaNetHandler::IsGameStartableEx(FString& ErrorTypeStr, UBOOL bCheckHost)
{
	FRoomInfo &Room= _StateController->RoomInfo;
	FMapInfo *MapInfo = _StateController->GetCurrentMap();
	if (!Room.IsValid() || !MapInfo)
	{
		//ErrorMsg = TEXT("<strings:avaNET.UIPopUpMsg.Msg_Game_FailedToStartGame_InvalidRoom>");
		ErrorTypeStr = TEXT("InvalidRoom");
		return FALSE;
	}

	if (_StateController->IsCountingDown() && !AmIHost())
	{
		ErrorTypeStr = TEXT("DontAllowInterrupt");
		return FALSE;
	}

	if (_StateController->ChannelInfo.IsMatchChannel())
	{
		// 대회 채널에서 심판 권한 이상은 제한 체크 없이 시작 가능
		if (_StateController->PlayerInfo.GetCurrentChannelMaskLevel() >= _CML_REFREE)
			return TRUE;

		// 선수 권한을 가진 플레이어가 게임 시작 시에는 정상적으로 상황 체크
	}

//#if FINAL_RELEASE
//	UBOOL bSkipMinPlayerCheck = (_StateController->GetChannelSetting(EChannelSetting_SkipMinPlayerCheck) > 0);
//#else
	UBOOL bSkipMinPlayerCheck = TRUE;
//#endif

	INT ReadyCnt[2] = { 0, 0 };
	for (INT i = 0; i < MAX_PLAYER_PER_ROOM; ++i)
	{
		if ( !Room.PlayerList.IsEmpty(i) )
		{
			if (Room.HostIdx == i || Room.PlayerList.PlayerList[i].IsReady())
				++ReadyCnt[i / MAX_PLAYER_PER_TEAM];
		}
	}

	if ((!bCheckHost || _StateController->AmIHost()) && !Room.IsPlaying())
	{
		if (Room.RoomInfo.setting.allowInterrupt == 0)
		{
			for (INT i = 0; i < MAX_PLAYER_PER_ROOM; ++i)
			{
				if (Room.PlayerList.IsEmpty(i) || Room.HostIdx == i)
					continue;
				if (Room.PlayerList.PlayerList[i].RoomPlayerInfo.bReady != _READY_WAIT)
				{
					//ErrorMsg = TEXT("<strings:avaNET.UIPopUpMsg.Msg_Game_FailedToStartGame_NeedToReady>");
					ErrorTypeStr = TEXT("NeedToReady");
					return FALSE;
				}
			}
		}
		if( !bSkipMinPlayerCheck)
		{
			INT NumPerTeam = (MapInfo->MissionType == _MAP_TRAINING ? 1 : 3);
			if( Room.RoomInfo.setting.autoBalance > 0 || MapInfo->MissionType == _MAP_TRAINING)
			{
				// at least 6(2) players needed to be ready
				if (ReadyCnt[0] + ReadyCnt[1] < NumPerTeam * 2)
				{
					//ErrorMsg = TEXT("<strings:avaNET.UIPopUpMsg.Msg_Game_FailedToStartGame_TotalMinPlayers>");
					ErrorTypeStr = TEXT("TotalMinPlayers");
					return FALSE;
				}
			}
			else
			{
				// each team must have at least 3 players to be ready
				if (ReadyCnt[0] < NumPerTeam || ReadyCnt[1] < NumPerTeam)
				{
					//ErrorMsg = TEXT("<strings:avaNET.UIPopUpMsg.Msg_Game_FailedToStartGame_EachTeamMinPlayers>");
					ErrorTypeStr = TEXT("EachTeamMinPlayers");
					return FALSE;
				}
			}
		}

		if( !IsBalancedRoomPlayers( ErrorTypeStr ) )
		{
			ErrorTypeStr = ErrorTypeStr;
			return FALSE;
		}

		return TRUE;
	}
	else if (!_StateController->AmIHost() && Room.IsPlaying() && Room.RoomInfo.setting.allowInterrupt > 0)
	{
#ifdef _DONT_START_AT_HOST_LOADING
		FRoomPlayerInfo *Host = !Room.PlayerList.IsEmpty(Room.HostIdx) ? &Room.PlayerList.PlayerList[Room.HostIdx] : NULL;
		if (Host && Host->RoomPlayerInfo.bReady == _READY_LOADING)
		{
			// 방장이 로딩 중
			//ErrorMsg = TEXT("<strings:avaNET.UIPopUpMsg.Msg_Game_FailedToStartGame_HostLoading>");
			ErrorTypeStr = TEXT("HostLoading");
			return FALSE;
		}
#endif
		// 난입하려는 팀의 인원이 상대 팀보다 2명이상 많으면 난입할 수 없다
		if( !IsBalancedRoomPlayers( ErrorTypeStr ) )
		{
			ErrorTypeStr = ErrorTypeStr	;
			return FALSE;
		}

		return TRUE;
	}

	ErrorTypeStr = TEXT("DontAllowInterrupt");

	return FALSE;
}

//! 에러 메시지를 얻기 위해서 IsGameStartableEx()함수로 옮겼습니다.(2007/03/12 고광록)
UBOOL UavaNetHandler::IsGameStartable(UBOOL bCheckHost)
{
	FString errorMsg;
	return IsGameStartableEx(errorMsg, bCheckHost);
}

UBOOL UavaNetHandler::IsBalancedRoomPlayers( FString& ErrorTypeStr )
{
	if( !_StateController->RoomInfo.IsValid() )
	{
		ErrorTypeStr = TEXT("");
		return FALSE;
	}
	
	FRoomInfo &Room= _StateController->RoomInfo;

	UBOOL bSkipUnbalanceCheck = _StateController->GetChannelSetting(EChannelSetting_RoomSkipBalanceCheck) > 0 ||
								GetCurrentMapMissionType() == NMT_MilitaryDrill;

	INT InGamePlayerCnt[2] = { 0, 0 };
	INT PlayerCnt[2] = { 0, 0 };
	INT ReadyCnt[2] = { 0, 0 };
	FRoomPlayerInfo* MyRoomPlayerInfo = _StateController->GetMyRoomPlayerInfo();
	BYTE MyTeamID = MyRoomPlayerInfo ? MyRoomPlayerInfo->GetTeamID() : RT_NONE;

	for (INT i = 0; i < MAX_PLAYER_PER_ROOM; ++i)
	{
		if ( !Room.PlayerList.IsEmpty(i) )
		{
			++PlayerCnt[i / MAX_PLAYER_PER_TEAM];
			if (Room.HostIdx == i || Room.PlayerList.PlayerList[i].IsReady())
				++ReadyCnt[i / MAX_PLAYER_PER_TEAM];
			if ( Room.PlayerList.PlayerList[i].IsPlaying() )
				++InGamePlayerCnt[i / MAX_PLAYER_PER_TEAM];
		}
	}

	if( _StateController->AmIHost() )
	{		
		if( !bSkipUnbalanceCheck )
		{
			// 호스트 일때 양팀의 레디인원차가 3명이상 차이나면 시작할 수 없다.
			if( !Room.RoomInfo.setting.autoBalance )
			{
				if ( Abs( ReadyCnt[0] - ReadyCnt[1] ) > 2 )
				{
					ErrorTypeStr = TEXT("UnbalanceWhenStart");
					return FALSE;
				}
			}
		}
	}
	else
	{
		if( !bSkipUnbalanceCheck )
		{
			if( ((MyTeamID == RT_EU && (InGamePlayerCnt[0] - InGamePlayerCnt[1]) > 1) ||
				(MyTeamID == RT_NRF && (InGamePlayerCnt[1] - InGamePlayerCnt[0]) > 1)) )
			{
				ErrorTypeStr = TEXT("UnbalanceWhenInterrupt");
				return FALSE;
			}
		}
	}
	return TRUE;
}

UBOOL UavaNetHandler::IsSpectatorAllowed()
{
	return (_StateController->RoomInfo.IsValid() && _StateController->RoomInfo.RoomInfo.setting.allowSpectator > 0);
}

void UavaNetHandler::GetRoomStartPlayerList(TArray<INT>& IDList)
{
	if (_StateController->RoomInfo.IsValid())
	{
		IDList = _StateController->RoomInfo.PlayerList.StartPlayerList;
	}
	else
	{
		_LOG(TEXT("GetRoomStartPlayerList(): Room info is not valid."));
	}
}

//void UavaNetHandler::GetRoomStartingPlayerList(TArray<FavaRoomPlayerInfo>& PlayerList)
//{
//	if (_StateController->RoomInfo.IsValid())
//	{
//		PlayerList = *((TArray<FavaRoomPlayerInfo>*)&(_StateController->RoomInfo.PlayerList.StartingPlayerList));
//	}
//	else
//	{
//		_LOG(TEXT("GetRoomStartingPlayerList(): Room info is not valid."));
//	}
//}

FString UavaNetHandler::GetCurrentMapFileName()
{
	FMapInfo *Info = _StateController->GetCurrentMap();
	if (Info)
	{
		return Info->FileName;
	}

	return TEXT("");
}

BYTE UavaNetHandler::GetCurrentMapMissionType()
{
	FMapInfo *Info = _StateController->GetCurrentMap();

	return Info ? Info->MissionType :NMT_MAX;
}


FString UavaNetHandler::GetWeaponName(INT idItem)
{
	return GetAvaNetRequest()->GetWeaponName(idItem);
}


INT UavaNetHandler::GetAvailableWeaponsBySlot(INT idSlot, TArray<INT>& ItemList)
{
	CavaItemDescData *pDescData = (CavaItemDescData*)(_ItemDesc().GetDescData());
	if (!pDescData)
		return 0;

	return pDescData->GetAvailableWeaponsBySlot(idSlot, ItemList);
}

INT UavaNetHandler::GetAvailableEquipsBySlot(INT idSlot, TArray<INT>& ItemList)
{
	CavaItemDescData *pDescData = (CavaItemDescData*)(_ItemDesc().GetDescData());
	if (!pDescData)
		return 0;

	return pDescData->GetAvailableEquipsBySlot(idSlot, ItemList);
}

INT UavaNetHandler::GetAvailableEffectsBySlot(INT idSlot, TArray<INT>& ItemList)
{
	CavaItemDescData *pDescData = (CavaItemDescData*)(_ItemDesc().GetDescData());
	if (!pDescData)
		return 0;

	return pDescData->GetAvailableEffectsBySlot(idSlot, ItemList);
}


INT UavaNetHandler::GetMyRoomSlot()
{
	return GetAvaNetRequest()->GetMyRoomSlot();
}

INT UavaNetHandler::GetPlayerRoomSlot(INT idAccount)
{
	if (!_StateController->RoomInfo.IsValid())
		return -1;
	for (INT i = 0; i < MAX_ALL_PLAYER_PER_ROOM; ++i)
	{
		if (_StateController->RoomInfo.PlayerList.PlayerList[i].RoomPlayerInfo.idAccount == idAccount)
			return i;
	}

	return -1;
}

UBOOL UavaNetHandler::GetRoomInfo(INT RoomListIndex,INT& RoomID,INT& nPassword,FString& RoomName,FString& HostName)
{
	if( ! _StateController->RoomList.RoomList.IsValidIndex(RoomListIndex) )
		return FALSE;

	const Def::ROOM_INFO& RoomInfo = _StateController->RoomList.RoomList(RoomListIndex).RoomInfo;
	RoomID = RoomInfo.idRoom;
	nPassword = RoomInfo.bPassword;
	RoomName = RoomInfo.roomName;
	HostName = RoomInfo.hostName;

	return TRUE;
}

UBOOL UavaNetHandler::IsMatchRoom()
{
	FRoomInfo &Room = _StateController->RoomInfo;
	return Room.IsValid() && Room.RoomInfo.state.bMatch > 0 && _StateController->ChannelInfo.IsMatchChannel();
}

INT UavaNetHandler::GetCurrentChannelMaskLevel(INT idAccount /*= 0*/)
{
	if (!_StateController->PlayerInfo.IsValid())
		return _CML_NONE;
	return _StateController->PlayerInfo.GetCurrentChannelMaskLevel();
}

INT UavaNetHandler::CheckMyLocation()
{
	if(GavaNetClient)
		return _StateController->CheckMyLocation();
	else
		return 0;
}

UBOOL UavaNetHandler::IsGameResultValid(UBOOL bFull)
{
	return (bFull ? _StateController->LastResultInfo.IsFullResult() : _StateController->LastResultInfo.IsValid());
}


FString UavaNetHandler::GetURLString(INT idAccount, const FString &Option)
{
	return _StateController->GetURLString(idAccount, Option);
}

FString UavaNetHandler::GetMyURLString(const FString &Option)
{
	return _StateController->GetURLString(ID_INVALID_ACCOUNT, Option);
}

UBOOL UavaNetHandler::IsPlayerInGame(INT idAccount)
{
	UBOOL Result = FALSE;

	do
	{
		if (!GIsGame && GIsEditor)
		{
			Result = TRUE;
			break;
		}
		if (!GavaNetClient->CurrentConnection())
		{
			Result = TRUE;
			break;
		}
		if (!_StateController->RoomInfo.IsValid())
		{
			Result = TRUE;
			break;
		}

		FRoomPlayerInfo *Info = _StateController->RoomInfo.PlayerList.Find(idAccount);

		//if (Info && Info->RoomPlayerInfo.bReady == _READY_PLAYING)
		//	return TRUE;
		Result = (Info != NULL);
	}
	while (0);

	_LOG(TEXT("UavaNetHandler::IsPlayerInGame(), Result = %s"), Result ? TEXT("TRUE") : TEXT("FALSE"));

	return Result;
}

FString UavaNetHandler::GetSkillName(BYTE PlayerClass,INT SkillID)
{
	return GetAvaNetRequest()->GetSkillName(PlayerClass, SkillID);
}

FString UavaNetHandler::GetAwardName(INT AwardID)
{
	return GetAvaNetRequest()->GetAwardName(AwardID);
}

FString UavaNetHandler::GetPlayerLevelName(INT PlayerLevel)
{
	return GetAvaNetRequest()->GetPlayerLevelName(PlayerLevel);
}

UBOOL UavaNetHandler::GetItemDesc(INT ItemId, FString& ItemName, BYTE& LevelLimit,BYTE& GaugeType,INT& Price,INT& RepairPrice,INT& RebuildPrice,FString& LiteralDesc, FString& IconCode, INT& Customizable)
{
	if( _StateController == NULL )
		return FALSE;

	ITEM_DESC *Desc = _ItemDesc().GetItem(ItemId);
	CUSTOM_ITEM_DESC *cDesc = (Desc == NULL) ? _ItemDesc().GetCustomItem(ItemId) : NULL;
	TCHAR szIconCode[] = { '\0' , '\0' };

	if( Desc == NULL && cDesc == NULL)
		return FALSE;

	extern FString GetIconCodeStringExt(const ITEM_DESC* ItemDesc, const CUSTOM_ITEM_DESC* cItemDesc = NULL);

	if( Desc != NULL)
	{
		LevelLimit = Desc->useLimitLevel;
		GaugeType = Desc->gaugeType;
		Price = Desc->priceType == _IPT_MONEY || Desc->priceType == _IPT_CASH ? Desc->price : 0;
		RepairPrice = Desc->maintenancePrice;
		RebuildPrice = Desc->RisConvertiblePrice;
		ItemName = Desc->GetName();
		LiteralDesc = Desc->GetDescription();
		
		//szIconCode[0] = Desc->itemIconCode;
		//IconCode = szIconCode;
		IconCode = GetIconCodeStringExt( Desc, NULL );

		Customizable = (Desc->slotType & _EP_WEAP_CUSTOMIZABLE);
	}
	else if( cDesc != NULL )
	{
		LevelLimit = cDesc->useLimitLevel;
		GaugeType = cDesc->gaugeType;
		Price = cDesc->priceType == _IPT_MONEY || cDesc->priceType == _IPT_CASH ? cDesc->price : 0;
		RepairPrice = 0;
		RebuildPrice = 0;
		ItemName = cDesc->GetName();
		LiteralDesc = cDesc->GetDescription();

		//szIconCode[0] = cDesc->itemIconCode;
		//IconCode = szIconCode;
		IconCode = GetIconCodeStringExt( NULL, cDesc );

		Customizable = FALSE;
	}

	return TRUE;
}

UBOOL UavaNetHandler::GetEffectItemDesc(INT ItemId, INT& GaugeType, INT& EffectType, INT& EffectValue, FString& ItemName, FString& ItemDesc, FString& IconStr)
{
	if( _StateController == NULL )
		return FALSE;

	EFFECT_ITEM_DESC *Desc = _ItemDesc().GetEffectItem(ItemId);

	if( Desc == NULL )
		return FALSE;

	extern FString GetSupportItemIcon( BYTE ItemType, INT EffectValue = 0 );

	if( Desc != NULL)
	{
		GaugeType = Desc->gaugeType;
		EffectType = Desc->effectInfo.effectType;
		EffectValue = Desc->effectInfo.effectValue;
		ItemName = Desc->GetName();
		ItemDesc = Desc->GetDescription();
		IconStr = GetSupportItemIcon( EffectType );
	}

	return TRUE;
}

UBOOL UavaNetHandler::GetItemId(BYTE IndexType,INT ListIndex,INT& ItemID)
{
	if( _StateController == NULL || ListIndex < 0)
		return FALSE;

	switch(IndexType)
	{
	case 0: /*AVANET_LISTINDEX_WeaponInven*/
		if( 10000 < ListIndex )
			return FALSE;
		ItemID = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[ListIndex%100].id;
		return TRUE;
	case 1: /*AVANET_LISTINDEX_EquipInven*/
		if( ListIndex < 10000)
			return FALSE;
		ItemID = _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[ListIndex-10000].id;
		return TRUE;
	case 2: /*AVANET_LISTINDEX_ItemList*/
		ItemID = -1;
		{
			FavaShopItem *pItem = _ShopDesc().GetItemByIndex(ListIndex);
			if (pItem)
				ItemID = pItem->GetDefaultItemID();
		}
		return (ItemID >= 0);
	case 3: /*AVANET_LISTINDEX_CustomItemList*/
		ItemID = -1;
		{
			FavaShopItem *pItem = _ShopDesc().GetCustomItemByIndex(ListIndex);
			if (pItem)
				ItemID = pItem->GetDefaultItemID();
		}
		return (ItemID >= 0);
	default:	 return FALSE;
	}
	/*return FALSE;*/
}

UBOOL UavaNetHandler::GetCustomItemID(INT InvenSlot,INT CustomSlotIndex,INT& ItemID)
{
	if( _StateController == NULL 
		|| InvenSlot >= 10000
		|| (InvenSlot = GetInvenIndexFromSlot(InvenSlot)) == INDEX_NONE 
		|| !(0 <= CustomSlotIndex && CustomSlotIndex < _CSI_MAX) )
		return FALSE;

	TSN_ITEM ItemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot].sn;
	CInventory &Inven = _StateController->PlayerInfo.Inven;
	CUSTOM_ITEM_INFO* cItemInfo = Inven.GetCustomInvenToSlot( ItemSN, (Def::CUSTOM_SLOT_IDX)CustomSlotIndex);

	ItemID = cItemInfo ? cItemInfo->id : INDEX_NONE;
	return cItemInfo != NULL;
}


UBOOL UavaNetHandler::GetWeaponRIS(INT InvenSlot, BYTE& bIsRISConvertible, INT& RemodelPrice)
{
	if(_StateController == NULL || 
		!_StateController->PlayerInfo.IsValid() ||
		(InvenSlot >= 10000 || InvenSlot < 0) )
		return FALSE;

	ITEM_DESC *ItemDesc = _ItemDesc().GetItem( _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot % 100].id );
	if( ItemDesc == NULL )
		return FALSE;

	bIsRISConvertible = ItemDesc->bRisConvertible;
	RemodelPrice = ItemDesc->RisConvertiblePrice;
	return TRUE;
}

UBOOL UavaNetHandler::GetPlayerInfo(BYTE& Level, BYTE& LastClass, BYTE& LastTeam, INT& exp, INT& SupplyPoint, INT& Cash, INT& Money, INT& idClanMark)
{
	if( _StateController == NULL || 
		!_StateController->PlayerInfo.IsValid() )
		return FALSE;

	Level = _StateController->PlayerInfo.PlayerInfo.level;
	LastClass = _StateController->PlayerInfo.PlayerInfo.currentClass;
	LastTeam = _StateController->PlayerInfo.PlayerInfo.lastTeam;
	exp = _StateController->PlayerInfo.PlayerInfo.xp;
	SupplyPoint = _StateController->PlayerInfo.PlayerInfo.supplyPoint;
	Cash = _StateController->PlayerInfo.Cash;
	Money = _StateController->PlayerInfo.PlayerInfo.money;
	idClanMark = _StateController->PlayerInfo.GetClanMarkID();

	return TRUE;
}

INT UavaNetHandler::GetPlayerTeamIndex(INT idAccount)
{
	if (!_StateController->RoomInfo.IsValid())
		return 255;

	FRoomPlayerInfo *Info = _StateController->RoomInfo.PlayerList.Find(idAccount);
	if (!Info)
		return 255;

	return Info->GetTeamID();
}

INT UavaNetHandler::GetClanMarkID(INT idAccount)
{
	if (!_StateController->RoomInfo.IsValid())
		return -1;

	FRoomPlayerInfo *Info = _StateController->RoomInfo.PlayerList.Find(idAccount);
	if (!Info)
		return -1;

	return Info->GetClanMarkID();
}

UBOOL UavaNetHandler::GetWeaponRepairInfo(INT InvenSlot, INT& RepairPrice, BYTE& bAfford)
{
	if( _StateController == NULL || 
		!_StateController->PlayerInfo.IsValid() ||
		(InvenSlot = GetInvenIndexFromSlot(InvenSlot)) < 0)
			return FALSE;

	const ITEM_INFO& ItemInfo = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot%100];
	
	CInventory& Inven = _StateController->PlayerInfo.Inven;
	RepairPrice = Inven.GetItemRepairMoney(ItemInfo.sn);
	TMONEY Money = _StateController->PlayerInfo.PlayerInfo.money;
	bAfford = Money >= (TMONEY)RepairPrice;

	ITEM_DESC* ItemDesc = _ItemDesc().GetItem(ItemInfo.id);

	if ( ItemDesc )
	{
		// Primary Weapon이고 영구 아이템인 경우에만 TRUE.
		if ( (ItemDesc->slotType & _EP_WEAP_PRIMARY) && (ItemDesc->gaugeType == _IGT_MAINTENANCE) )
			return TRUE;
	}

	return FALSE;
}

UBOOL UavaNetHandler::GetEquipRepairInfo(INT InvenSlot, INT& RepairPrice, BYTE& bAfford)
{
	if( _StateController == NULL || 
		!_StateController->PlayerInfo.IsValid() ||
		(InvenSlot = GetInvenIndexFromSlot(InvenSlot)) < 0)
		return FALSE;

	const ITEM_INFO& ItemInfo = _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[InvenSlot];

	CInventory Inven;
	Inven.Init( &_ItemDesc(), &_StateController->PlayerInfo.PlayerInfo.itemInfo);
	RepairPrice = Inven.GetItemRepairMoney(ItemInfo.sn);
	bAfford = _StateController->PlayerInfo.PlayerInfo.money >= (TMONEY)RepairPrice;

	return TRUE;
}

UBOOL UavaNetHandler::GetCustomCompInfo(INT InvenSlot,INT CustomListIndex,INT& AlterSlot,INT& AlterItemID,INT& CompPrice)
{
	if( _StateController == NULL || 
		!_StateController->PlayerInfo.IsValid() ||
		(InvenSlot = GetInvenIndexFromSlot(InvenSlot)) < 0 )
		return FALSE;

	const ITEM_INFO* ItemInfo = &_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot];
	CUSTOM_ITEM_DESC* cItemDesc = _ItemDesc().GetCustomItemByIndex( CustomListIndex );

	if( ItemInfo == NULL || cItemDesc == NULL )
		return FALSE;

	CInventory& Inven = _StateController->PlayerInfo.Inven;
	CUSTOM_ITEM_INFO* cItemInfo = Inven.GetCustomInvenToSlot(ItemInfo->sn, cItemDesc->customType);
	
	CompPrice = Inven.GetCustomItemRefundMoney(ItemInfo->sn, cItemDesc->customType);
	AlterSlot = (INT)cItemDesc->customType;
	AlterItemID = cItemInfo ? cItemInfo->id : -1;

	return TRUE;
}

UBOOL UavaNetHandler::DoIHaveItem(INT ListIndex)
{
	if (!_StateController->PlayerInfo.IsValid())
		return FALSE;

	FavaShopItem *pItem = _ShopDesc().GetItemByIndex(ListIndex);
	if (!pItem)
		return FALSE;

	CInventory &Inven = _StateController->PlayerInfo.Inven;
	return (Inven.GetFirstItem(pItem->GetDefaultItemID()) != NULL);
}

UBOOL UavaNetHandler::DoIHaveCustomItemInSlot( INT InvenSlot, INT SlotIndex)
{
	if( !_StateController->PlayerInfo.IsValid() 
		|| (InvenSlot = GetInvenIndexFromSlot(InvenSlot)) == INDEX_NONE 
		|| !(0 <= SlotIndex && SlotIndex < _CSI_MAX) )
		return FALSE;

	CInventory &Inven = _StateController->PlayerInfo.Inven;
	TSN_ITEM ItemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[ InvenSlot ].sn;
	return Inven.GetCustomInvenToSlot( ItemSN, (Def::CUSTOM_SLOT_IDX)SlotIndex ) != NULL;
}

UBOOL UavaNetHandler::DoIHaveCustomItem(INT ListIndex)
{
	if (!_StateController->PlayerInfo.IsValid())
		return FALSE;

	CUSTOM_ITEM_DESC *pDesc = _ItemDesc().GetCustomItemByIndex(ListIndex);
	if (!pDesc)
		return FALSE;

	for (INT i = 0; i < MAX_CUSTOM_INVENTORY_SIZE; ++i)
	{
		if (_StateController->PlayerInfo.PlayerInfo.itemInfo.customWeapon[i].id == pDesc->id)
			return TRUE;
	}

	return FALSE;
}

/*! @brief 친구인지 유무를 얻는다.
	@param FriendPlayerName
		친구 유무를 알아볼 이름.
	@param ListIndex
		BuddyList의 인덱스.
	@param BuddyType
		친구의 종류.
		enum _BuddyConst
		{
			BT_BUDDY_ONESIDE = 1,
			BT_BUDDY_BOTH = 2,
			BT_BUDDY_BIA = 5,
			BT_BUDDY_OTHER = 1,
			BT_BLOCK = 9,

			BUDDY_ACCEPT = 0,
			BUDDY_REJECT = 110,
		};
*/
UBOOL UavaNetHandler::HaveFriendPlayerNamed( const FString& FriendPlayerName, INT &ListIndex, INT &BuddyType )
{
	FBuddyInfo *pBuddyInfo = _Communicator().BuddyList.Find( FriendPlayerName, ListIndex );
	if ( pBuddyInfo )
	{
		BuddyType = pBuddyInfo->BuddyType;
		return TRUE;
	}

	return FALSE;
}

UBOOL UavaNetHandler::HaveBlockedPlayerNamed( const FString& BlockedPlayerName, INT &ListIndex )
{
	return _Communicator().BlockList.Find( BlockedPlayerName, ListIndex ) != NULL;
}

UBOOL UavaNetHandler::HaveClanMemberNamed( const FString& ClanMemberName, INT &ListIndex )
{
	FGuildPlayerInfo* GuildPlayerInfo = _StateController->GuildInfo.PlayerList.Find(ClanMemberName);
	return GuildPlayerInfo != NULL;
}

UBOOL UavaNetHandler::IsBIAPlayer( const FString& PlayerName )
{
	FBuddyInfo* BuddyInfo = _Communicator().GetBIA();
	return BuddyInfo ? BuddyInfo->Nickname == PlayerName : FALSE;
}

UBOOL UavaNetHandler::HasOwnClan( const FString& PlayerName, FString& ItsOwnClanName)
{
	return FALSE;
}


UBOOL UavaNetHandler::GetLobbyPlayerName(INT LobbyPlayerListIndex,FString& PlayerName)
{
	UBOOL bValidIndex = _StateController->LobbyPlayerList.PlayerList.IsValidIndex( LobbyPlayerListIndex );
	FString BuddyName = bValidIndex ? _StateController->LobbyPlayerList.PlayerList(LobbyPlayerListIndex).LobbyPlayerInfo.nickname : TEXT("");
	return bValidIndex;
}

UBOOL UavaNetHandler::GetFriendPlayerName(INT BuddyListIndex,FString& PlayerName)
{
	UBOOL bValidIndex = _Communicator().BuddyList.BuddyList.IsValidIndex( BuddyListIndex );
	FString BuddyName = bValidIndex ? _Communicator().BuddyList(BuddyListIndex).Nickname : TEXT("");
	return bValidIndex;
}

UBOOL UavaNetHandler::GetBlockedPlayerName(INT BlockListIndex,FString& PlayerName)
{
	UBOOL bValidIndex = _Communicator().BlockList.BuddyList.IsValidIndex( BlockListIndex );
	FString BuddyName = bValidIndex ? _Communicator().BlockList(BlockListIndex).Nickname : TEXT("");
	return bValidIndex;
}

UBOOL UavaNetHandler::GetSelectedLobbyPlayerInfo(FString& NickName,FString& GuildName,INT& Level,INT& WinCount,INT& DefeatCount,INT& DisconnectCount,INT& KillCount,INT& DeathCount)
{
	FLobbyPlayerInfo* PlayerInfo = _StateController->PlayerInfo.IsValid() ? _StateController->LobbyPlayerList.GetSelected() : NULL;
	if( PlayerInfo == NULL )
		return FALSE;

	NickName = PlayerInfo->LobbyPlayerInfo.nickname;
	GuildName = PlayerInfo->PlayerInfo.guildName;
	Level = PlayerInfo->PlayerInfo.level;
	WinCount = PlayerInfo->PlayerInfo.gameWin;
	DefeatCount = PlayerInfo->PlayerInfo.gameDefeat;
	DisconnectCount = PlayerInfo->PlayerInfo.disconnectCount;
	DeathCount = PlayerInfo->PlayerInfo.deathCount;
	KillCount = PlayerInfo->PlayerInfo.killCount;

	return TRUE;
}

UBOOL UavaNetHandler::GetSelectedFriendPlayerInfo(FString& NickName,FString& GuildName,INT& Level,INT& WinCount,INT& DefeatCount,INT& DisconnectCount,INT& KillCount,INT& DeathCount)
{
	FBuddyInfo* BuddyInfo = _Communicator().BuddyList.GetSelected();
	if( BuddyInfo == NULL )
		return FALSE;

	NickName = BuddyInfo->Nickname;
	GuildName = BuddyInfo->GuildName;
	Level = BuddyInfo->Level;
	WinCount = BuddyInfo->GameWin;
	DefeatCount = BuddyInfo->GameDefeat;
	DisconnectCount = BuddyInfo->DisconnectCount;
	DeathCount = BuddyInfo->DeathCount;
	KillCount = BuddyInfo->KillCount;

	return TRUE;
}

UBOOL UavaNetHandler::GetSelectedBlockedPlayerInfo(FString& NickName,FString& GuildName,INT& Level,INT& WinCount,INT& DefeatCount,INT& DisconnectCount,INT& KillCount,INT& DeathCount)
{
	FBuddyInfo* BuddyInfo = _Communicator().BlockList.GetSelected();
	if( BuddyInfo == NULL )
		return FALSE;

	NickName = BuddyInfo->Nickname;
	GuildName = BuddyInfo->GuildName;
	Level = BuddyInfo->Level;
	WinCount = BuddyInfo->GameWin;
	DefeatCount = BuddyInfo->GameDefeat;
	DisconnectCount = BuddyInfo->DisconnectCount;
	DeathCount = BuddyInfo->DeathCount;
	KillCount = BuddyInfo->KillCount;

	return TRUE;
}

UBOOL UavaNetHandler::GetSelectedGuildPlayerInfo(FString& NickName,FString& GuildName,INT& Level,INT& WinCount,INT& DefeatCount,INT& DisconnectCount,INT& KillCount,INT& DeathCount)
{
	FGuildPlayerInfo *GuildPlayerInfoOuter = _StateController->GuildInfo.IsValid() ? _StateController->GuildInfo.PlayerList.GetSelected() : NULL;
	Def::GUILD_PLAYER_INFO *GuildPlayerinfo = GuildPlayerInfoOuter ? &GuildPlayerInfoOuter->GuildPlayerInfo : NULL;
	if( GuildPlayerInfoOuter == NULL || GuildPlayerinfo == NULL )
		return FALSE;

	NickName = GuildPlayerinfo->nickname;
	GuildName = _StateController->GuildInfo.GuildInfo.name;
	Level = GuildPlayerinfo->level;
	WinCount = GuildPlayerInfoOuter->GameWin;
	DefeatCount = GuildPlayerInfoOuter->GameDefeat;
	DisconnectCount = GuildPlayerInfoOuter->DisconnectCount;
	DeathCount = GuildPlayerInfoOuter->DeathCount;
	KillCount = GuildPlayerInfoOuter->KillCount;

	return TRUE;
}

UBOOL UavaNetHandler::CheckWeaponRefundCond(INT InvenSlot)
{
	if( !_StateController->PlayerInfo.IsValid() ||
		(InvenSlot = GetInvenIndexFromSlot(InvenSlot)) == INDEX_NONE)
		return FALSE;

	return _StateController->PlayerInfo.Inven.IsRefund(_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot].sn);
}

UBOOL UavaNetHandler::CheckEquipRefundCond(INT InvenSlot)
{
	if( !_StateController->PlayerInfo.IsValid() ||
		(InvenSlot = GetInvenIndexFromSlot(InvenSlot)) == INDEX_NONE)
		return FALSE;

	
	return _StateController->PlayerInfo.Inven.IsRefund(_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[InvenSlot].sn);
}

UBOOL UavaNetHandler::CheckCustomRefundCond(INT InvenSlot,INT CustomSlotIndex)
{
	if( !_StateController->PlayerInfo.IsValid() ||
		(InvenSlot = GetInvenIndexFromSlot(InvenSlot)) == INDEX_NONE ||
		!(0 <= CustomSlotIndex && CustomSlotIndex < _CSI_MAX))
		return FALSE;

	CInventory& Inven = _StateController->PlayerInfo.Inven;
	ITEM_INFO& ItemInfo = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenSlot];

	CUSTOM_ITEM_INFO* cItemInfo = Inven.GetCustomInvenToSlot( ItemInfo.sn, (Def::CUSTOM_SLOT_IDX)CustomSlotIndex );
	CUSTOM_ITEM_DESC* cItemDesc = cItemInfo ? _ItemDesc().GetCustomItem(cItemInfo->id) : NULL;
	if( cItemDesc )
	{
		debugf(TEXT("cItemDesc Name = %s, IsDefault = %s"), cItemDesc->GetName(), cItemDesc->isDefaultItem ? GTrue : GFalse);
		debugf(TEXT("IsRefund Result = %s"), Inven.IsRefund( ItemInfo.sn, (Def::CUSTOM_SLOT_IDX)CustomSlotIndex ) ? GTrue : GFalse);
	}

	return Inven.IsRefund( ItemInfo.sn, (Def::CUSTOM_SLOT_IDX)CustomSlotIndex );
}

/*! @brief 캐쉬 금액을 Refresh하는 함수.
	@note
		Console Conmmand = net wic cash
	@remark
		웹에서 금액을 받아오고 난 후에 발생하는 이벤트.
		성공 - (EMsg_Inventory_GetCash, "ok", "", cash, 0)
		실패 - (EMsg_Inventory_GetCash, "failed", "", 0, 0)
		
		웹 실패 - (EMsg_Inventory_GetCash, "bad web result", "", 0, 0)
*/
UBOOL UavaNetHandler::WICGetCash()
{
	return GetAvaNetRequest()->WICGetCash();
}

/*! @brief 캐쉬를 충전하기 위한 윈도우를 열어준다.
	@note
		Console Command = net wic charge
	@remark
		윈도우가 닫이는 경우 발생하는 이벤트.
		(EMsg_Inventory_Charge, "ok", "", 0, 0)

		충전창이 닫히고 난 후에는 WICGetCash()함수를 호출해서 refresh할 수 있다.
*/
UBOOL UavaNetHandler::WICOpenChargeWindow()
{
	return GetAvaNetRequest()->WICOpenChargeWindow();
}

/*! @brief 캐쉬 아이템 구매 함수.
	@param idItem
		아이템 아이디.
	@remark
		처리 후 호출되는 이벤트.
		성공 - (EMsg_Inventory_EffBuy, "web ok", "", cash, 0)
		실패 - (EMsg_Inventory_EffBuy, "web failed", "", 0, 0)

		결제 후 게임 서버에서 아이템을 지급받고 나면 발생하는 이벤트.
		(EMsg_Inventory_EffBuy, "ok", "", 0, 0)
		(EMsg_Inventory_EffBuy, "ok", "effect", EquipSlot, InvenSlot)
		(EMsg_Inventory_EffBuy, "ok", "equip", EquipSlot, InvenSlot )
		(EMsg_Inventory_EffBuy, "ok", "weapon", EquipSlot, InvenSlot)

		(EMsg_Inventory_EffBuy, "failed", "", 0, 0) -> 구매 확인 실패
		(EMsg_Inventory_EffBuy, "invalid", "", 0, 0) -> 존재하지 않는 아이템을 구매하려 한 경우

		웹 실패 - (EMsg_Inventory_EffBuy, "bad web result", "", 0, 0)

	@note
		참고로, 아이템 구매의 경우, 현금이 걸려 있는 매우 민감한 사안이므로, 
		기존의 다른 request와는 달리 타임 아웃 처리하지 않을 생각입니다.
		기존의 게임 아이템 구매는 타임 아웃 처리를 하므로, 잘못하면 중복 구매가 되는 일이 있었지요.

		하지만 유료 아이템의 경우, 결제를 요청했는데 네트웍 장애 등의 이유로 
		웹에서 어떤 응답이라도 없으면, 그 이후에는 아이템 구매가 불가능해질겁니다.
		게임을 종료하고 다시 접속해야 하지요.
*/
UBOOL UavaNetHandler::WICBuyItem(INT idItem)
{
	return GetAvaNetRequest()->WICBuyItem(idItem);
}

UBOOL UavaNetHandler::WICSendGift(INT idItem, INT idAccountTo)
{
	return GetAvaNetRequest()->WICSendGift(idItem, idAccountTo);
}

UBOOL UavaNetHandler::WICOpenGiftWindow(INT idItem)
{
	return GetAvaNetRequest()->WICOpenGiftWindow(idItem);
}

INT UavaNetHandler::BeginTransaction( const FString& SessionName )
{
	return Trans ? Trans->Begin(*SessionName) : INDEX_NONE;
}

UBOOL UavaNetHandler::UndoTransaction()
{
	if( !Trans )
		return FALSE;

	if( Trans->IsActive() )
		Trans->End();

	UBOOL bResult = Trans->Undo();
	Trans->Reset(TEXT("Common Reset"));

	return bResult;
}

INT UavaNetHandler::EndTransaction()
{
	return Trans && Trans->IsActive() ? Trans->End() : INDEX_NONE;
}

void UavaNetHandler::GetTransactionObjects( TArray<UObject*>& OutObjects)
{
	if( Trans )
		Trans->GetTransactionObjects( OutObjects );
}

FString UavaNetHandler::CheckRoomKickedState()
{
	_LOG(TEXT("CheckRoomKickedState() : State = %d, Reason = %d"), _StateController->GetNetState(), _StateController->PlayerInfo.RoomKickedReason);
	FString Reason;
	if (_StateController->GetNetState() >= _AN_LOBBY && _StateController->PlayerInfo.RoomKickedReason != 255)
	{
		switch (_StateController->PlayerInfo.RoomKickedReason)
		{
		case KR_INVALID_LEVEL:
			Reason = TEXT("invalid level");
			_StateController->GoToState(_AN_CHANNELLIST);
			break;
		case KR_INVALID_CLAN:
			Reason = TEXT("invalid clan");
			_StateController->GoToState(_AN_CHANNELLIST);
			break;
		case KR_INVALID_SD:
			Reason = TEXT("invalid sd");
			_StateController->GoToState(_AN_CHANNELLIST);
			break;
		case KR_PCBANG_ONLY:
			Reason = TEXT("pcbang only");
			_StateController->GoToState(_AN_CHANNELLIST);
			break;
		default:
			break;
		}

		_StateController->PlayerInfo.RoomKickedReason = 255;
	}
	_LOG(TEXT("CheckRoomKickedState() : Reason = %s"), *Reason);
	return Reason;
}

FString UavaNetHandler::GetLastWhisperedPlayerName()
{
	return _Communicator().LastWhisperedPlayer;
}

INT UavaNetHandler::GetBIAAccountID()
{
	FBuddyInfo *BIA = _Communicator().GetBIA();
	return BIA ? BIA->idAccount : ID_INVALID_ACCOUNT;
}

void UavaNetHandler::ProcHostCrash()
{
	// vote to ban crashed host.
	//if (!AmISpectator()) // 20070227 dEAthcURe|HM test disable
	//	VoteForHostBan();

	// _StateController->GoToState(_AN_ROOM);  // [-] 20070212 dEAthcURe|HM
}


UBOOL UavaNetHandler::ProcCountDown()
{
	return _StateController->ProcCountDown();
}

void UavaNetHandler::SwapTeamInGame()
{
	GetAvaNetRequest()->RoomSwapTeam(RR_GAMEREQ);
}


void UavaNetHandler::ClearRTNotice()
{
	_StateController->RTNotice = TEXT("");
}

INT UavaNetHandler::GetCurrentTickerCount()
{
	return _TickerMsg().Num();
}

FLOAT UavaNetHandler::GetChatOffDue()
{
	return _StateController->ChatOffDue;
}

UBOOL UavaNetHandler::CanMyClanJoinSelectedRoom()
{
	if (_StateController->GetNetState() != _AN_LOBBY || !_StateController->ChannelInfo.IsFriendlyGuildChannel())
		return FALSE;
	if (_StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild == ID_INVALID_GUILD)
		return FALSE;

	TID_GUILD idGuild = _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild;
	FRoomDispInfo *Room = _StateController->RoomList.GetSelected();
	if (!Room)
		return FALSE;

	return (Room->RoomInfo.idGuild[0] == idGuild || Room->RoomInfo.idGuild[0] == ID_INVALID_GUILD ||
			Room->RoomInfo.idGuild[1] == idGuild || Room->RoomInfo.idGuild[1] == ID_INVALID_GUILD);
}

void UavaNetHandler::GetCurrentRoomsClanNames(FString& EUName, FString& NRFName)
{
	if (!_StateController->RoomInfo.IsValid() || !_StateController->ChannelInfo.IsFriendlyGuildChannel())
	{
		EUName = TEXT("");
		NRFName = TEXT("");
		return;
	}

	int EUIndex  = _StateController->RoomInfo.PlayerList.FindFirstValidSlot(RT_EU);
	int NRFIndex = _StateController->RoomInfo.PlayerList.FindFirstValidSlot(RT_NRF);

	if ( EUIndex != -1 )
		EUName = _StateController->RoomInfo.PlayerList.PlayerList[EUIndex].RoomPlayerInfo.guildName;
	if ( NRFIndex != -1 )
		NRFName = _StateController->RoomInfo.PlayerList.PlayerList[NRFIndex].RoomPlayerInfo.guildName;
}


UBOOL UavaNetHandler::IsPendingPopUpMsg()
{
	return FALSE;//_StateController->PendingPopUpMsgs.Num() > 0;
}

//void UavaNetHandler::PushPopUpMsg(BYTE MsgType, const FString& PopUpMsg, const FString& NextScene, FName NextUIEventName )
//{
//	_StateController->PendingPopUpMsgs.PushPopUpMsg(MsgType, PopUpMsg, NextScene, NextUIEventName );
//}
//
//UBOOL UavaNetHandler::PopFirstPopUpMsg(FavaPopUpMsgInfo& Info)
//{
//	return _StateController->PendingPopUpMsgs.PopFirstPopUpMsg(Info.MsgType, Info.PopUpMsg, Info.NextScene, Info.NextUIEventName );
//}
//
//void UavaNetHandler::ClearPopUpMsg()
//{
//	_StateController->PendingPopUpMsgs.ClearPopUpMsg();
//}

void UavaNetHandler::ReportVoteNew(BYTE Command, INT idCaller, const TArray<INT> &VoterList, INT param1, INT param2)
{
	if (AmIHost() && _StateController->IsStatePlaying() && Command == EVC_Kick && param1 == _StateController->PlayerInfo.PlayerInfo.idAccount)
	{
		// 방장이 게임 중 강퇴 당하면, 방장 자신의 전적을 서버에 보고
		eventSendPlayerResult(param1);
	}

	PM::GAME::REPORT_VOTE_NTF::Send(Command, idCaller, VoterList, param1, param2);
}

UBOOL UavaNetHandler::IsVoteAvailable()
{
	return _StateController->IsVoteAvailable();
}


class AWorldInfo* UavaNetHandler::GetWorldInfo()
{
	return ( GWorld && GWorld->IsValidWorldInfo() ) ? GWorld->GetWorldInfo() : NULL;
}

UClass* UavaNetHandler::GetGameInfoClass()
{
	AWorldInfo* WorldInfo = GetWorldInfo();

	if( WorldInfo )
	{
		if( WorldInfo->Game )
			return WorldInfo->Game->GetClass();
		else if( WorldInfo->GRI && WorldInfo->GRI->GameClass )
			return WorldInfo->GRI->GameClass;
	}

	return NULL;
}

INT UavaNetHandler::GetNetVersion()
{
	return GBuiltFromChangeList;
}


UavaNetHandler *GavaNetHandler = NULL;

UavaNetHandler* UavaNetHandler::GetAvaNetHandler()
{
	if (!GavaNetHandler)
	{
		GavaNetHandler = ConstructObject<UavaNetHandler>(UavaNetHandler::StaticClass(), INVALID_OBJECT, NAME_None, RF_Keep);
		check(GavaNetHandler);

		// initialize
		GavaNetHandler->eventOnInit();

		GavaNetHandler->Trans =  new UavaTransBuffer( 8 * 1024 );
	}

	return GavaNetHandler;
}

void UavaNetHandler::OptionSaveUserKey(const FString& UserKeyStr, const FString& OptionStr)
{
	GetAvaNetRequest()->OptionSaveUserKey( UserKeyStr, OptionStr );
}

// {{ 20070212 dEAthcURe|HM 나중에소스정리
void NtfLinkLost(void)
{
	if(GavaNetHandler) {
		GavaNetHandler->ProcHostCrash();
		debugf(TEXT("[dEAthcURe|NtfLinkLost|HM] Host link lost notified to server"));
	}
	else {
		debugf(TEXT("[dEAthcURe|NtfLinkLost|HM] Host link lost notification failed due to null avaNetHandler"));
	}
}
// }} 20070212 dEAthcURe|HM 나중에소스정리

FString UavaNetHandler::GetClanMarkPkgNameFromID(INT ID, UBOOL bSmall, UBOOL bMarkup)
{
	static const INT CLANMARK_SIZE     = 26;
	static const INT CLANMARK_PER_PAGE = CLANMARK_SIZE * CLANMARK_SIZE;	//! 1 page has 26*26 clanmarks.
	static const INT CLANMARK_BASEPAGE = 1;
	static TCHAR *ClanMarkStringTemplates[] =
	{
		TEXT("avaClanMarkLarge%02d.%05d"),
		TEXT("avaClanMarkLarge%02d.%05d"),
		TEXT("<Images:avaClanMarkLarge%02d.%05d; U=0 V=0 XL=128 YL=128>"),
		TEXT("<Images:avaClanMark.Small%02d; UL=19 VL=19 DimX=26 DimY=26 TileNum=675 Period=0 Repeat=0 Offset=%d>"),
	};

	if ( ID < 0 )
		return FString(TEXT(""));

	INT PageNum		= ID / CLANMARK_PER_PAGE + CLANMARK_BASEPAGE;	// 676 means 26 * 26
	INT Index		= ID % CLANMARK_PER_PAGE;
	INT TmplIndex	= (INT)bMarkup * 2 + (INT)bSmall;

	// 해당하는 ClanMarkString으로 리턴해 준다.
	return FString::Printf(ClanMarkStringTemplates[TmplIndex], PageNum, Index);
}


// 대기방 테스트용 플레이어 추가
//void UavaNetHandler::AddDummyPlayer(const FString& DummyPlayerName, UBOOL bReady, BYTE TeamID )
//{
//	static INT Account = 100;
//
//	if( _StateController->IsStateInRoom() )
//	{
//		Def::ROOM_PLAYER_INFO RoomPlayerInfo;
//
//		RoomPlayerInfo.bReady = !bReady ? _READY_NONE : _StateController->RoomInfo.RoomInfo.state.playing == RIP_PLAYING ? _READY_PLAYING : _READY_WAIT;
//		debugf(TEXT("bReady = %s, Playing = %s"), bReady ? GTrue : GFalse, _StateController->RoomInfo.RoomInfo.state.playing == RIP_PLAYING ? GTrue : GFalse);
//		appStrncpy( RoomPlayerInfo.nickname, *DummyPlayerName,SIZE_NICKNAME);
//		INT StartSlot = (TeamID == RT_NRF) ? MAX_PLAYER_PER_TEAM : (TeamID == RT_SPECTATOR) ? MAX_PLAYER_PER_ROOM : 0;
//		INT MaxSlotSize = (TeamID == RT_NRF || TeamID == RT_EU) ? MAX_PLAYER_PER_TEAM : (TeamID == RT_SPECTATOR) ? MAX_SPECTATOR_PER_ROOM : 0;
//		INT CurrentSlot = INDEX_NONE;
//		for( INT i = StartSlot ; i < (StartSlot + MaxSlotSize) ; i++ )
//		{
//			if( _StateController->RoomInfo.PlayerList.IsEmpty(i) )
//			{
//				CurrentSlot = i;
//				break;
//			}
//		}
//
//		if( CurrentSlot == INDEX_NONE )
//			return;
//
//		RoomPlayerInfo.idSlot = CurrentSlot;
//		RoomPlayerInfo.idAccount = Account++;
//		RoomPlayerInfo.idTeam = TeamID;
//		RoomPlayerInfo.hostRating = 100;
//		
//		_StateController->RoomInfo.AddPlayer(RoomPlayerInfo);
//	}
//}
