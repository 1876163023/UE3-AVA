//=============================================================================
// Copyright 2005 Epic Games - All Rights Reserved.
// Confidential.
//=============================================================================
#include "PrecompiledHeaders.h"
#include "avaGame.h"
#include "UnNet.h"
#include "avaNetClient.h" // [+] 20070531 dEAthcURe|HM

// {{ 20061219 dEAthcURe|HM
#ifdef EnableHostMigration
#include "hmMacro.h"
#endif
// }} 20061219 dEAthcURe|HM

static inline UBOOL NEQ(const FTakeHitInfo& A, const FTakeHitInfo& B, UPackageMap* Map, UActorChannel* Channel)
{
	return (A.bDamage != B.bDamage || A.HitLocation != B.HitLocation || A.Momentum != B.Momentum || A.DamageType != B.DamageType || A.HitBoneIndex != B.HitBoneIndex);
}

static inline UBOOL NEQ(const FKOTH3TeamStatus& A, const FKOTH3TeamStatus& B, UPackageMap* Map, UActorChannel* Channel)
{
	return (A.NumPlayersInside != B.NumPlayersInside || A.TimeRemains != B.TimeRemains);
}

static inline UBOOL NEQ(const FSceneStateDataType& lhs, const FSceneStateDataType& rhs, UPackageMap* Map, UActorChannel* Channel)
{
	UBOOL bNeq = FALSE;
	for( INT i = 0 ; i < ARRAY_COUNT(lhs.OpenStatus) ; i++)
	{
		if( lhs.OpenStatus[i].bOpen != rhs.OpenStatus[i].bOpen )
		{
			bNeq = TRUE;
		}
	}
	return (lhs.SceneName != rhs.SceneName) || bNeq;
}

// {{ 20070423 dEAthcURe|HM hmIntVar
int AavaGameReplicationInfo::findHmIntVar(FString hmIntVarName)
{
	int nItem = sizeof(HmIntVars) / sizeof(FString);

	//debugf(TEXT("sizeof(HmIntVars) / sizeof(FString)=%d"), nItem);
	for(int lpp=0;lpp<nItem;lpp++) {
		TCHAR varName[512];
		int value;
		if(_stscanf(*HmIntVars[lpp], TEXT("%s %d"), varName, &value)==2) {
			debugf(TEXT("findHmIntVar %d %s %d"), lpp, varName, value);
			if(hmIntVarName == varName) {
				return lpp;
			}
		}
	}
	return -1;	
}
bool AavaGameReplicationInfo::setHmIntVar(FString hmIntVarName, int value)
{
	resetHmIntVar(hmIntVarName); // [+] 20070430
	if(findHmIntVar(hmIntVarName)==-1) {
		int nItem = sizeof(HmIntVars) / sizeof(FString);
		for(int lpp=0;lpp<nItem;lpp++) {
			if(HmIntVars[lpp] == TEXT("")) {				
				HmIntVars[lpp] = FString::Printf(TEXT("%s %d"), *hmIntVarName, value);

				if(GWorld) {
					AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
					if(WorldInfo) {
						NetUpdateTime = WorldInfo->TimeSeconds - 1.0f;
					}
				}
				debugf(TEXT("   <---- setHmIntVar HmIntVars[%d] %s"), lpp, *HmIntVars[lpp]);
				return true;
			}
		}
	}
	return false;	
}
bool AavaGameReplicationInfo::resetHmIntVar(FString hmIntVarName)
{
	int idx = findHmIntVar(hmIntVarName);
	
	if(idx!=-1) {
		debugf(TEXT("   <---- resetHmIntVar HmIntVars[%d] %s"), idx, *HmIntVars[idx]);
		HmIntVars[idx] = TEXT("");
		if(GWorld) {
			AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
			if(WorldInfo) {
				NetUpdateTime = WorldInfo->TimeSeconds - 1.0f;
			}
		}		
		return true;
	}	
	return false;	
}
// }} 20070423 dEAthcURe|HM hmIntVar

// {{ 20061219 dEAthcURe|HM
#ifdef EnableHostMigration
void AavaGameReplicationInfo::hmSerialize(FArchive& Ar)
{
	Super::hmSerialize(Ar);

	/* test disable 20070323 검증후삭제할것
	if ( Ar.IsLoading() ) {
		_hms_defLoading;
		_hms_loadValue(bHmRoundEnd); // BITFIELD bHmRoundEnd:1;
		_hms_loadValue(bWarmupRound); // BITFIELD bWarmupRound:1;
		_hms_loadValue(bPracticeRound); // BITFIELD bPracticeRound:1;
		//_hms_loadValue(bEnableEjectBullet3rd); // BITFIELD bEnableEjectBullet3rd:1;
		_hms_loadValue(bShowMissionPos); // BITFIELD bShowMissionPos:1;
		_hms_loadValue(bReinforcement); // BITFIELD bReinforcement:1;
	}
	else {
		_hms_defSaving;
		_hms_saveValue(bHmRoundEnd); // // BITFIELD bHmRoundEnd:1;
		_hms_saveValue(bWarmupRound); // BITFIELD bWarmupRound:1;
		_hms_saveValue(bPracticeRound); // BITFIELD bPracticeRound:1;
		//_hms_saveValue(bEnableEjectBullet3rd); // BITFIELD bEnableEjectBullet3rd:1;
		_hms_saveValue(bShowMissionPos); // BITFIELD bShowMissionPos:1;
		_hms_saveValue(bReinforcement); // BITFIELD bReinforcement:1;
	}

	Ar << DogTagPackCnt; // [+] 20070228
	Ar << nWinTeam; // [+] 20070303
	Ar << CurrentRound; // [!] 20070306
    Ar << CurrentRoundTimeLimit; // [!] 20070306
	Ar << ElapsedSyncTime; // [+] 20070306
	*/

	if ( Ar.IsLoading() ) {
		debugf(TEXT("[AavaGameReplicationInfo::hmSerialize] loading nWinTeam=%d, bHmRoundEnd=%d"), nWinTeam, bHmRoundEnd);		
	}
	else {
		debugf(TEXT("[AavaGameReplicationInfo::hmSerialize] saving nWinTeam=%d, bHmRoundEnd=%d"), nWinTeam, bHmRoundEnd);
	}
	return; //? 이걸 살려두면 new client player가 dead body로 나온다 // [+] 20070213 test

    Ar << MissionIndicatorIdx;
    Ar << MissionTime;
    Ar << MissionTimeRate;
    Ar << MissionMaxTime;
    Ar << MissionTargetTime;
    Ar << MissionTimeDir;
    Ar << MissionReplTmpTime;
    Ar << ReplicatedMissionTime;
    Ar << WinCondition;	
    Ar << FlagState[0];
	Ar << FlagState[1];
    Ar << MissionType;
    Ar << AttackTeam;
    Ar << RoundWinner;
	/*Ar << KOTH3[0];
	Ar << KOTH3[1];*/
    
    //Ar << ReinforcementCount;	//?
	for(int lpp=0;lpp<4;lpp++) {
		Ar << CurrentWaypoint[lpp];
	}	
}
#endif
// }} 20061219 dEAthcURe|HM

INT* AavaGameReplicationInfo::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);

	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	

	DOREP(avaGameReplicationInfo,bWarmupRound);
	DOREP(avaGameReplicationInfo,bPracticeRound);	
	DOREP(avaGameReplicationInfo,CurrentRound);
	DOREP(avaGameReplicationInfo,bReinforcement);
	DOREP(avaGameReplicationInfo,bAllowThirdPersonCam);
	DOREP(avaGameReplicationInfo,DominanceTeamIdx);
	DOREP(avaGameReplicationInfo,MissionIndicatorIdx);
	DOREP(avaGameReplicationInfo,MissionTimeDir);
	DOREP(avaGameReplicationInfo,MissionTargetTime);
	DOREP(avaGameReplicationInfo,ReplicatedMissionTime);
	DOREP(avaGameReplicationInfo,MissionTimeRate);
	DOREP(avaGameReplicationInfo,MissionMaxTime);
	DOREP(avaGameReplicationInfo,TargetMissionTime);
	DOREPARRAY(avaGameReplicationInfo, CurrentWaypoint);
	DOREP(avaGameReplicationInfo,WinCondition);
	DOREPARRAY(avaGameReplicationInfo, ReinforcementFreq);
	DOREP(avaGameReplicationInfo,BaseScore);
	DOREP(avaGameReplicationInfo,ElapsedSyncTime);
	DOREP(avaGameReplicationInfo,DogTagPackCnt);
	DOREP(avaGameReplicationInfo,bHmRoundEnd); // 20070302 dEAthcURe|HM	
	DOREP(avaGameReplicationInfo,nWinTeam);
	DOREP(avaGameReplicationInfo,FriendlyFireType);
	DOREP(avaGameReplicationInfo,AttackTeam);
	DOREP(avaGameReplicationInfo,bRoundTimeOver);
	DOREPARRAY(avaGameReplicationInfo,HmEvents); // 20070420 dEAthcURe|HM	
	DOREPARRAY(avaGameReplicationInfo,HmVariables); // 20070420 dEAthcURe|HM	
	DOREPARRAY(avaGameReplicationInfo,HmIntVars); // 20070423 dEAthcURe|HM	
	DOREPARRAY(avaGameReplicationInfo,MissionHelp);
	DOREPARRAY(avaGameReplicationInfo,MissionCheckPoint);
	DOREP(avaGameReplicationInfo, bEnableGhostChat);
	DOREP(avaGameReplicationInfo, bSwapRule);
	DOREP(avaGameReplicationInfo, bSwappedTeam);
	//DOREP(avaGameReplicationInfo,MatchMode);
	DOREPARRAY(avaGameReplicationInfo,KOTH3);

	if (bNetInitial)
	{
		DOREP(avaGameReplicationInfo,MissionType);
		DOREPARRAY(avaGameReplicationInfo,OptionalSceneData);
		DOREP(avaGameReplicationInfo,OptionalSceneCount);
	}
	return Ptr;
}

INT* AavaPawn::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);
	
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if ( bNetDirty )
	{
		if ( bNetOwner )
		{			
			DOREP(avaPawn,bHasHelmet);
			DOREP(avaPawn,TouchedPickup_Count);
			DOREP(avaPawn,BombVolume);
			DOREPARRAY(avaPawn,TouchedPickUp_Rep);
			DOREP(avaPawn,TouchedPP);
		}
		else
		{
			DOREP(avaPawn,RemoteViewYaw);
			DOREP(avaPawn,bIsDash);
		}
		DOREP(avaPawn,CurrentWeapon);		
		DOREP(avaPawn,ImpactLocation0);
		DOREP(avaPawn,ImpactLocation1);
		DOREP(avaPawn,ImpactLocation2);
		DOREP(avaPawn,ImpactLocation3);
		DOREP(avaPawn,ImpactLocation4);
		DOREP(avaPawn,ImpactLocation5);
		DOREP(avaPawn,ImpactLocation6);
		DOREP(avaPawn,ImpactLocation7);
		DOREP(avaPawn,ImpactDirection);
		DOREP(avaPawn,BloodSpurtFlags);		
		DOREP(avaPawn,ReloadAnimPlayCount);
		DOREP(avaPawn,PreReloadAnimPlayCount);
		DOREP(avaPawn,PostReloadAnimPlayCount);
		DOREP(avaPawn,PullPinAnimPlayCount);
		DOREP(avaPawn,MountSilencerPlayCount);
		DOREP(avaPawn,UnMountSilencerPlayCount);
		DOREP(avaPawn,WeaponState);
		DOREP(avaPawn,NightvisionActivated);
		DOREP(avaPawn,WeaponZoomChange);
		DOREP(avaPawn,TOH_Momentum);
		DOREP(avaPawn,TOH_Play);
		DOREP(avaPawn,Armor_Stomach);
		DOREP(avaPawn,HealthMax);	
		DOREP(avaPawn,ArmorMax);	
		DOREPARRAY(avaPawn,PossessedWeapon);
		// Yaw 각도 제한 Replication Property
		DOREP(avaPawn,bLimitYawAngle);
		DOREP(avaPawn,MaxLimitYawAngle);
		DOREP(avaPawn,MinLimitYawAngle);
		DOREP(avaPawn,bLimitPitchAngle);
		DOREP(avaPawn,MinLimitPitchAngle);
		DOREP(avaPawn,MaxLimitPitchAngle);
		// 중화기 거치 여부 Flag
		DOREP(avaPawn,HeavyWeaponType);
		DOREP(avaPawn,GripHeavyWeapon);
		DOREP(avaPawn,DogTagCnt);
		if (WorldInfo->TimeSeconds < LastTakeHitTimeout)
		{
			DOREP(avaPawn,LastTakeHitInfo);
			DOREP(avaPawn,bLastTakeHitVisibility);
		}		
		DOREP(avaPawn,bTargetted);
		DOREP(avaPawn,TargettedCnt);
		DOREP(avaPawn,StressLevel);
		DOREP(avaPawn,EncryptKey);
	}

	return Ptr;
}

// {{ 20070212 dEAthcURe|HM
class AWorldInfo* AavaWeapon::GetWorldInfo()
{
	return GWorld ? GWorld->GetWorldInfo() : NULL;
}

// }} 20070212 dEAthcURe|HM

INT* AavaWeapon::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);
	
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if ( bNetDirty )
	{
		DOREP(avaWeapon,AmmoCount);
		DOREPARRAY(avaWeapon,WeaponModifiers);
		DOREP(avaWeapon,ModifiersRepDone);
		DOREP(avaWeapon,PickUpLifeTime);
		DOREP(avaWeapon,MaintenanceRate);
		DOREP(avaWeapon,PickUpTeamNum);
		DOREP(avaWeapon,bDrawInRadar);
		DOREP(avaWeapon,RadarIconCode);
	}

	return Ptr;
}

INT* AavaWeaponPickupFactory::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);
	
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if (bNetInitial)
	{
		DOREP(avaWeaponPickupFactory,WeaponPickupClass);
	}
	
	return Ptr;
}


INT* AavaGameObjective::GetOptimizedRepList(BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel)
{
	Ptr = Super::GetOptimizedRepList(Recent, Retire, Ptr, Map, Channel);

	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);
	if (bNetDirty)
	{
		DOREP(avaGameObjective,DefenderTeamIndex);
		DOREP(avaGameObjective,bUnderAttack);
	}

	return Ptr;
}

//static inline UBOOL NEQ(const struct FPlayerInfoStruct& A,const struct FPlayerInfoStruct& B,UPackageMap* Map,UActorChannel* Channel)
//{
//	INT i,j;
//	for ( i = 0 ; i < ARRAY_COUNT(A.PlayerModifiers) ; ++i )
//		if ( A.PlayerModifiers[i] != B.PlayerModifiers[i] )	return TRUE;
//
//	for ( i = 0 ; i < ARRAY_COUNT(A.WeaponInfos) ; ++i )
//	{
//		for ( j = 0 ; j < ARRAY_COUNT(A.WeaponInfos[0].WeaponModifiers) ; ++ j )
//		{
//			if ( A.WeaponInfos[i].WeaponModifiers[j] != B.WeaponInfos[i].WeaponModifiers[j] )
//				return TRUE;
//		}
//		if ( A.WeaponInfos[i].WeaponClass != B.WeaponInfos[i].WeaponClass )	
//			return TRUE;
//	}
//	return FALSE;
//}

void AavaGame::FullscreenMovie_PlayMovie( const FString& MovieFilename )
{
	if (GFullScreenMovie)
	{
		// Play movie and block on playback.
		GFullScreenMovie->GameThreadPlayMovie(MM_PlayOnceFromStream, *MovieFilename);
		GFullScreenMovie->GameThreadWaitForMovie();
		GFullScreenMovie->GameThreadStopMovie();
	}
}

void AavaGame::FullscreenMovie_StopMovie()
{
	if (GFullScreenMovie)
	{
		// Play movie and block on playback.
		GFullScreenMovie->GameThreadStopMovie();
	}
}
// {{ dEAthcURe|HM
#ifdef EnableHostMigration
void AavaGame::HmClear(UBOOL bMigratedHost)
{
	g_hostMigration.resetRound(); // [+] 20070305 dEAthcURe

	/* [-] 20070305
	if(g_hostMigration.state != hmsHost) {
		g_hostMigration.clear(true);
		g_hostMigration.gotoState(hmsHost);
		debugf(TEXT("[AavaGame::HmClear] HM returns to normal state of host."));
	}	
	*/

	int CurrentRound = 0;
	AWorldInfo* pWorldInfo = GWorld->GetWorldInfo();
	if(pWorldInfo && pWorldInfo->GRI) {
		AavaGameReplicationInfo *pGRI = Cast<AavaGameReplicationInfo>( pWorldInfo->GRI );
		if(pGRI) {
			CurrentRound = pGRI->CurrentRound;
			debugf(TEXT("bWarmupround=%d bMigratedHost=%d CurrentRound=%d"), bWarmupRound?1:0, bMigratedHost?1:0, CurrentRound);
		}
	}

	
	// {{ 20070531 dEAthcURe|HM 라운드가 끝나면 나갈수있다.
	if(GavaNetClient && (bMigratedHost || CurrentRound>1)) {
		GavaNetClient->timeHostBegin = GCurrentTime - 120.0f;
		debugf(TEXT("[HM|AavaGame::HmClear] TimeBeginHost = %f"), GavaNetClient->timeHostBegin);
	}	
	// }} 20070531 dEAthcURe|HM 라운드가 끝나면 나갈수있다.
}

void AavaGame::hmSerialize(FArchive& Ar)
{
	//Autogenerated code by HmSerializeGenerator/generateSerializer.rb

	Super::hmSerialize(Ar);

	/*
	if (Ar.IsLoading() ) {
		_hms_defLoading;
		_hms_loadValue(bStartedCountDown); // BITFIELD bStartedCountDown:1;
		_hms_loadValue(bTournament); // BITFIELD bTournament:1;
		_hms_loadValue(bAllowSpectateEnemy); // BITFIELD bAllowSpectateEnemy:1;
		_hms_loadValue(bBeautyMode); // BITFIELD bBeautyMode:1;
		_hms_loadValue(bDemoMode); // BITFIELD bDemoMode:1;
		_hms_loadValue(bOverTimeBroadcast); // BITFIELD bOverTimeBroadcast:1;
		_hms_loadValue(bQuickStart); // BITFIELD bQuickStart:1;
		_hms_loadValue(bWarmupRound); // BITFIELD bWarmupRound:1;
		_hms_loadValue(bWeaponStay); // BITFIELD bWeaponStay:1;
		_hms_loadValue(bPlayerBecameActive); // BITFIELD bPlayerBecameActive:1;
		_hms_loadValue(bForceRespawn); // BITFIELD bForceRespawn:1;
		_hms_loadValue(bMustHaveMultiplePlayers); // BITFIELD bMustHaveMultiplePlayers:1;
		_hms_loadValue(bSkipPlaySound); // BITFIELD bSkipPlaySound:1;
		_hms_loadValue(bAllowMPGameSpeed); // BITFIELD bAllowMPGameSpeed:1;
		_hms_loadValue(bTeamScoreRounds); // BITFIELD bTeamScoreRounds:1;
		_hms_loadValue(bMustJoinBeforeStart); // BITFIELD bMustJoinBeforeStart:1;
		_hms_loadValue(bWaitForNetPlayers); // BITFIELD bWaitForNetPlayers:1;
		_hms_loadValue(bPlayersMustBeReady); // BITFIELD bPlayersMustBeReady:1;
		_hms_loadValue(bTempForceRespawn); // BITFIELD bTempForceRespawn:1;
		_hms_loadValue(bSoaking); // BITFIELD bSoaking:1;
		_hms_loadValue(bFinalStartup); // BITFIELD bFinalStartup:1;		
	}
	else {
		_hms_defSaving;
		_hms_saveValue(bStartedCountDown); // BITFIELD bStartedCountDown:1;
		_hms_saveValue(bTournament); // BITFIELD bTournament:1;
		_hms_saveValue(bAllowSpectateEnemy); // BITFIELD bAllowSpectateEnemy:1;
		_hms_saveValue(bBeautyMode); // BITFIELD bBeautyMode:1;
		_hms_saveValue(bDemoMode); // BITFIELD bDemoMode:1;
		_hms_saveValue(bOverTimeBroadcast); // BITFIELD bOverTimeBroadcast:1;
		_hms_saveValue(bQuickStart); // BITFIELD bQuickStart:1;
		_hms_saveValue(bWarmupRound); // BITFIELD bWarmupRound:1;
		_hms_saveValue(bWeaponStay); // BITFIELD bWeaponStay:1;
		_hms_saveValue(bPlayerBecameActive); // BITFIELD bPlayerBecameActive:1;
		_hms_saveValue(bForceRespawn); // BITFIELD bForceRespawn:1;
		_hms_saveValue(bMustHaveMultiplePlayers); // BITFIELD bMustHaveMultiplePlayers:1;
		_hms_saveValue(bSkipPlaySound); // BITFIELD bSkipPlaySound:1;
		_hms_saveValue(bAllowMPGameSpeed); // BITFIELD bAllowMPGameSpeed:1;
		_hms_saveValue(bTeamScoreRounds); // BITFIELD bTeamScoreRounds:1;
		_hms_saveValue(bMustJoinBeforeStart); // BITFIELD bMustJoinBeforeStart:1;
		_hms_saveValue(bWaitForNetPlayers); // BITFIELD bWaitForNetPlayers:1;
		_hms_saveValue(bPlayersMustBeReady); // BITFIELD bPlayersMustBeReady:1;
		_hms_saveValue(bTempForceRespawn); // BITFIELD bTempForceRespawn:1;
		_hms_saveValue(bSoaking); // BITFIELD bSoaking:1;
		_hms_saveValue(bFinalStartup); // BITFIELD bFinalStartup:1;
	}
	*/

	/* test disable 20070322 검증후삭제할것
	Ar << RemainingTime; // INT RemainingTime;
	Ar << ElapsedTime; // INT ElapsedTime;
	*/

	/*
	Ar << ResetTimeDelay; // INT ResetTimeDelay;
	Ar << NumRounds; // INT NumRounds;
	Ar << NetWait; // INT NetWait;
	Ar << FemaleBackupNameOffset; // INT FemaleBackupNameOffset;
	Ar << EnemyRosterName; // FStringNoInit EnemyRosterName;
	Ar << WinCondition; // INT WinCondition;
	Ar << PlayerKills; // INT PlayerKills;	
	Ar << DesiredPlayerCount; // INT DesiredPlayerCount;
	Ar << WarmupTime; // INT WarmupTime;
	Ar << CallSigns; // FStringNoInit CallSigns[15];
	Ar << PlayerReplicationInfoClassName; // FStringNoInit PlayerReplicationInfoClassName;
	Ar << NRFPawnClassName; // FStringNoInit NRFPawnClassName[3];
	Ar << StartupStageCount; // INT StartupStageCount[8];	
	Ar << nWinTeam; // INT nWinTeam;
	Ar << EUPawnClassName; // FStringNoInit EUPawnClassName[3];
	Ar << GameStatsClass; // FStringNoInit GameStatsClass;
	Ar << FemaleBackupNames; // FStringNoInit FemaleBackupNames[32];
	Ar << PlayerDeaths; // INT PlayerDeaths;
	Ar << WarmupRemaining; // INT WarmupRemaining;
	Ar << RestartWait; // INT RestartWait;
	Ar << ServerSkillLevel; // INT ServerSkillLevel;
	Ar << RulesMenuType; // FStringNoInit RulesMenuType;
	Ar << EndMessageCounter; // INT EndMessageCounter;
	Ar << EndMessageWait; // INT EndMessageWait;
	Ar << SpawnProtectionTime; // FLOAT SpawnProtectionTime;
	Ar << StartupStage; // BYTE StartupStage;
	Ar << LastTaunt; // INT LastTaunt[2];
	Ar << MaleBackupNameOffset; // INT MaleBackupNameOffset;
	Ar << DefaultEnemyRosterClass; // FStringNoInit DefaultEnemyRosterClass;
	Ar << ResetCountDown; // INT ResetCountDown;
	Ar << GameUMenuType; // FStringNoInit GameUMenuType;
	Ar << MapPrefix; // FStringNoInit MapPrefix;
	Ar << MaleBackupNames; // FStringNoInit MaleBackupNames[32];
	Ar << MinNetPlayers; // INT MinNetPlayers;
	Ar << Acronym; // FStringNoInit Acronym;
	Ar << EndTime; // FLOAT EndTime;
	Ar << DefaultMaxLives; // INT DefaultMaxLives;
	Ar << AdjustedDifficulty; // FLOAT AdjustedDifficulty;
	Ar << SpawnAllowTime; // INT SpawnAllowTime;
	Ar << LateEntryLives; // INT LateEntryLives;
	Ar << EndTimeDelay; // FLOAT EndTimeDelay;
	Ar << PlayerControllerClassName; // FStringNoInit PlayerControllerClassName;
	Ar << CountDown; // INT CountDown;
	Ar << Description; // FStringNoInit Description;
	*/

	//{{other values
	// class ANavigationPoint* LastPlayerStartSpot;
	// class ANavigationPoint* LastStartSpot;
	// class AActor* EndGameFocus;
	// class AavaTeamInfo* EnemyRoster;
	// TArrayNoInit<class UClass*> DefaultInventory;
	// class UClass* VictoryMessageClass;
	// TArrayNoInit<struct FGameTypePrefix> MapPrefixes;
	// TArrayNoInit<struct FGameTypePrefix> CustomMapPrefixes;
	// class AavaGameStats* GameStats;
	// TArrayNoInit<struct FTextStyleInfo> TextStyleData;
	//}}other values
}

void AavaPlayerReplicationInfo::hmSerialize(FArchive& Ar)
{
	// check 20070207 from nop
	//Autogenerated code by HmSerializeGenerator/generateSerializer.rb

	Super::hmSerialize(Ar);

	/* test disable 20070323 검증후삭제할것
	if (Ar.IsLoading() ) {
		_hms_defLoading;
		//_hms_loadValue(bInitLastInfo); // BITFIELD bInitLastInfo:1;
		_hms_loadValue(bFetchWeaponSniperMan); // BITFIELD bFetchWeaponSniperMan:1;
		_hms_loadValue(bFetchWeaponPointMan); // BITFIELD bFetchWeaponPointMan:1;
		_hms_loadValue(bReadyInGame); // BITFIELD bReadyInGame:1;
		_hms_loadValue(bFetchPointMan); // BITFIELD bFetchPointMan:1;
		_hms_loadValue(bFetchRifleMan); // BITFIELD bFetchRifleMan:1;
		_hms_loadValue(bWaypointTeamFlag); // BITFIELD bWaypointTeamFlag:1;
		_hms_loadValue(bSquadLeader); // BITFIELD bSquadLeader:1;
		_hms_loadValue(bFetchWeaponRifleMan); // BITFIELD bFetchWeaponRifleMan:1;
		_hms_loadValue(bFetchCharacter); // BITFIELD bFetchCharacter:1;
		_hms_loadValue(bFetchSniper); // BITFIELD bFetchSniper:1;
	}
	else {
		_hms_defSaving;
		//_hms_saveValue(bInitLastInfo); // BITFIELD bInitLastInfo:1;
		_hms_saveValue(bFetchWeaponSniperMan); // BITFIELD bFetchWeaponSniperMan:1;
		_hms_saveValue(bFetchWeaponPointMan); // BITFIELD bFetchWeaponPointMan:1;
		_hms_saveValue(bReadyInGame); // BITFIELD bReadyInGame:1;
		_hms_saveValue(bFetchPointMan); // BITFIELD bFetchPointMan:1;
		_hms_saveValue(bFetchRifleMan); // BITFIELD bFetchRifleMan:1;
		_hms_saveValue(bWaypointTeamFlag); // BITFIELD bWaypointTeamFlag:1;
		_hms_saveValue(bSquadLeader); // BITFIELD bSquadLeader:1;
		_hms_saveValue(bFetchWeaponRifleMan); // BITFIELD bFetchWeaponRifleMan:1;
		_hms_saveValue(bFetchCharacter); // BITFIELD bFetchCharacter:1;
		_hms_saveValue(bFetchSniper); // BITFIELD bFetchSniper:1;
	}

	Ar << GaugeMax; // INT GaugeMax;
	Ar << DefencePoint; // FLOAT DefencePoint;
	Ar << WhenDeadStr; // FStringNoInit WhenDeadStr;
	Ar << LastTeam; // INT LastTeam;
	Ar << AttackPoint; // FLOAT AttackPoint;
	Ar << StatusStr; // FStringNoInit StatusStr;
	Ar << SpectatorStr; // FStringNoInit SpectatorStr;
	Ar << WaypointTeamindex; // INT WaypointTeamindex;
	Ar << LastClass; // INT LastClass;
	Ar << SniperItem; // FStringNoInit SniperItem;
	Ar << WeaponRifleMan; // FStringNoInit WeaponRifleMan;
	Ar << TacticsPoint; // FLOAT TacticsPoint;
	Ar << WeaponSniperMan; // FStringNoInit WeaponSniperMan;
	Ar << RoundLoseScore; // FLOAT RoundLoseScore;
	Ar << WeaponPointMan; // FStringNoInit WeaponPointMan;
	Ar << CharacterItem; // FStringNoInit CharacterItem;
	Ar << PointManItem; // FStringNoInit PointManItem;
	Ar << PlayerClassID; // INT PlayerClassID;
	Ar << GaugeCur; // INT GaugeCur;
	Ar << LeaderPoint; // FLOAT LeaderPoint;
	Ar << PlayerClassStr[0];
	Ar << PlayerClassStr[1];
	Ar << PlayerClassStr[2]; // Ar << PlayerClassStr; // FStringNoInit PlayerClassStr[3];
	Ar << RoundWinScore; // FLOAT RoundWinScore;
	Ar << Level; // INT Level;
	Ar << AccountID; // INT AccountID;
	Ar << RifleManItem; // FStringNoInit RifleManItem;
	Ar << CurrentSpawnClassID; // INT CurrentSpawnClassID;
	Ar << TeamKillCnt; // [+] 20070222 dEAthcURe|HM
	*/

	//{{other values
	// class AavaPlayerModifierInfo* avaPMI;
	// class AavaClassReplicationInfo* avaCRI[3];
	//}}other values

	// {{ 20070308 gauge초기화
	if (Ar.IsLoading() ) {
		GaugeCur = 0;
		GaugeMax = 0;
	}
	// }} 20070308 gauge초기화
}
#endif
// }} dEAthcURe|HM

INT* AavaPlayerReplicationInfo::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);
	
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);
	if (bNetDirty)
	{
		DOREP(avaPlayerReplicationInfo,bReplicatedByNewHost); // [+] 20070523 dEAthcURe|HM
		DOREP(avaPlayerReplicationInfo,GaugeMax);		
		DOREP(avaPlayerReplicationInfo,GaugeCur);		
		DOREP(avaPlayerReplicationInfo,PlayerClassID);	
		DOREP(avaPlayerReplicationInfo,bSquadLeader);
		//DOREP(avaPlayerReplicationInfo,CharacterItem);
		//DOREP(avaPlayerReplicationInfo,PointManItem);
		//DOREP(avaPlayerReplicationInfo,RifleManItem);
		//DOREP(avaPlayerReplicationInfo,SniperItem);
		//DOREP(avaPlayerReplicationInfo,WeaponPointMan);
		//DOREP(avaPlayerReplicationInfo,WeaponRifleMan);
		//DOREP(avaPlayerReplicationInfo,WeaponSniperMan);
		DOREP(avaPlayerReplicationInfo,AccountID);
		DOREP(avaPlayerReplicationInfo,Level);
		DOREP(avaPlayerReplicationInfo,AttackPoint);
		DOREP(avaPlayerReplicationInfo,DefencePoint);
		DOREP(avaPlayerReplicationInfo,LeaderPoint);
		DOREP(avaPlayerReplicationInfo,TacticsPoint);
		DOREP(avaPlayerReplicationInfo,RoundWinScore);
		DOREP(avaPlayerReplicationInfo,RoundLoseScore);
		DOREP(avaPlayerReplicationInfo,StatusStr);
		DOREP(avaPlayerReplicationInfo,TeamKillCnt);
		DOREP(avaPlayerReplicationInfo,CurrentSpawnClassID);
		DOREPARRAY(avaPlayerReplicationInfo,avaCRI);
		DOREP(avaPlayerReplicationInfo,bHost);
		DOREP(avaPlayerReplicationInfo,RoundTopKillCnt);
		DOREP(avaPlayerReplicationInfo,RoundTopHeadShotKillCnt);
		DOREP(avaPlayerReplicationInfo,RoundNoDamageCnt);
		DOREP(avaPlayerReplicationInfo,RoundTopScoreCnt);
		DOREP(avaPlayerReplicationInfo,RoundNoDeathCnt);
		DOREP(avaPlayerReplicationInfo,TopLevelKillCnt);
		DOREP(avaPlayerReplicationInfo,HigherLevelKillCnt);
		DOREP(avaPlayerReplicationInfo,BulletMultiKillCnt);
		DOREP(avaPlayerReplicationInfo,GrenadeMultiKillCnt);
		DOREP(avaPlayerReplicationInfo,HelmetDropCnt);
		DOREP(avaPlayerReplicationInfo,bSilentLogIn);
		DOREP(avaPlayerReplicationInfo,LeaderScore);
		DOREP(avaPlayerReplicationInfo,LastDeathTime);
		DOREP(avaPlayerReplicationInfo,CurrentUseActionType);
	}
	return Ptr;
}

INT* AavaLinkedReplicationInfo::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);
	
	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if (bNetDirty)
	{
		DOREP(avaLinkedReplicationInfo,NextReplicationInfo);
	}

	return Ptr;
}

INT* AavaPickupFactory::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);
	return Ptr;
}

//-----------------------------------------------------------------------------
//	avaWeaponPawn
//-----------------------------------------------------------------------------
INT* AavaWeaponPawn::GetOptimizedRepList(BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel)
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);

	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if (bNetDirty)
	{
		DOREP(avaWeaponPawn,MySeatIndex);
		DOREP(avaWeaponPawn,MyVehicle);
		DOREP(avaWeaponPawn,MyVehicleWeapon);
	}

	return Ptr;
}

//-----------------------------------------------------------------------------
//	avaVehicleWeapon
//-----------------------------------------------------------------------------
INT* AavaVehicleWeapon::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);

	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if (bNetInitial)// && bNetOwner)
	{
		DOREP(avaVehicleWeapon,SeatIndex);
		DOREP(avaVehicleWeapon,MyVehicle);
	}

	return Ptr;
}

//-----------------------------------------------------------------------------
//	avaVehicle
//-----------------------------------------------------------------------------
INT* AavaVehicle::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);

	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if (bNetDirty)
	{
		if (!bNetOwner)
		{
			DOREP(avaVehicle,WeaponRotation);
		}
		DOREP(avaVehicle,hmId); // 20070801 dEAthcURe|HM
		DOREP(avaVehicle,AIScriptedRouteIndex);
		DOREP(avaVehicle,bTeamLocked);
		DOREP(avaVehicle,Team);
		DOREP(avaVehicle,bDeadVehicle);
		DOREP(avaVehicle,bPlayingSpawnEffect);
		DOREP(avaVehicle,bIsDisabled);
		DOREP(avaVehicle,SeatMask);
		DOREP(avaVehicle,PassengerPRI);
		DOREP(avaVehicle,ForceMovementDirection);
		DOREPARRAY(avaVehicle,PlayerNames);
		if (WorldInfo->TimeSeconds < LastTakeHitTimeout)
		{
			DOREP(avaVehicle,LastTakeHitInfo);
		}
		if (KillerController == NULL || Map->CanSerializeObject(KillerController))
		{
			DOREP(avaVehicle,KillerController);
		}
	}
	return Ptr;
}

//-----------------------------------------------------------------------------
//	avaVehicleFactory
//-----------------------------------------------------------------------------
INT* AavaVehicleFactory::GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel )
{
	Ptr = Super::GetOptimizedRepList(Recent,Retire,Ptr,Map,Channel);

	checkSlow(StaticClass()->ClassFlags & CLASS_NativeReplication);	
	if (bNetDirty)
	{
		DOREP(avaVehicleFactory,bHasLockedVehicle);
		if (bReplicateChildVehicle)
		{
			DOREP(avaVehicleFactory,ChildVehicle);
		}
	}

	return Ptr;
}


//--------------------------------------------------------------
