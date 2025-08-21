//
// hmDataTypes.cpp
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#include "avaGame.h"
#include "hmDataTypes.h"
#ifdef EnableHostMigration
//---------------------------------------------------------------------------
// global func def
//---------------------------------------------------------------------------
void _hm_electLeader(AGameInfo* pGi, int teamIdx)
{
	debugf(TEXT("[dEAthcURe|_hm_reelectLeader]"));
	if(pGi) {
		AavaGame* pGame = Cast<AavaGame>(pGi);
		if(pGame) {
			pGame->eventHmElectSquadLeader(TRUE, teamIdx);
		}
	}	
}
//---------------------------------------------------------------------------
void _hm_ignoreMoveInput(ULevel* pLevel)
{
	if(pLevel) {
		for(int idx=0;idx<pLevel->Actors.Num();idx++) {
			AActor* pActor = pLevel->Actors(idx);
			if(pActor) {
				AavaPlayerController* pPc = Cast<AavaPlayerController>(pActor);
				if(pPc) {
					pPc->eventServer_IgnoreMoveInput(TRUE);
					debugf(TEXT("[dEAthcURe|_hm_ignoreMoveInput] move input ignored for %s"), *pActor->GetName());
				}

				AavaVehicle *pV = Cast<AavaVehicle>(pActor);
				if(pV) {
					pV->setPhysics(PHYS_None, NULL, FVector(0,0,1));
					debugf(TEXT("[dEAthcURe|_hm_ignoreMoveInput] set physics PHYS_None Vehicle %s"), *pActor->GetName());
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
void _hm_checkGameGoOn(AGameInfo* pGi)
{
	if(pGi) {
		AavaGame* pGame = Cast<AavaGame>(pGi);
		if(pGame) {
			FILE* fp = fopen("IgnoreCheckGameGoOn.txt", "rt");
			if(fp) {
				pGame->bIgnoreCheckGameGoOn = TRUE;
				debugf(TEXT("[dEAthcURe|_hm_checkGameGoOn] pGame->bIgnoreCheckGameGoOn = TRUE"));
				fclose(fp);
			}
		}
	}
}
//---------------------------------------------------------------------------
// {{ 20070309 c4를 해당 player가 가지고 있는지 check
APawn* _hm_getPawn(ULevel* pLevel, APlayerReplicationInfo* pPRI) 
{
	if(pLevel && pPRI) {		
		for(int idx=0;idx<pLevel->Actors.Num();idx++) {
			AActor* pActor = pLevel->Actors(idx);
			if(pActor) {
				APawn* pPawn = Cast<APawn>(pActor);
				if(pPawn && pPawn->PlayerReplicationInfo == pPRI) {
					return pPawn;
				}
			}
		}
	}
	return 0x0;
}
//---------------------------------------------------------------------------
AActor* _hm_checkPlayerHasSomething(ULevel* pLevel, APlayerReplicationInfo* pPRI, FString className)
{
	APawn* pPawn = _hm_getPawn(pLevel, pPRI);
	if(pPawn) {
		for(int idx=0;idx<pLevel->Actors.Num();idx++) {
			AActor* pActor = pLevel->Actors(idx);
			if(pActor && pActor->Instigator == pPawn) {
				FString actorClassName = pActor->GetClass()->GetName();
				if(actorClassName == className) {
					return pActor;
				}
			}
		}
	}
	return 0x0;
}
// }} 20070309 c4를 해당 player가 가지고 있는지 check
//---------------------------------------------------------------------------
// {{ 20070308
bool _hm_checkAllOut(void)
{
	if(GWorld && GWorld->CurrentLevel) {

		AavaGame* pGameInfo = Cast<AavaGame>(GWorld->GetGameInfo());
		if(pGameInfo)
		{
			return !pGameInfo->eventCheckGameGoOn( NULL );
		}
		//int teamNum[2];

		//teamNum[0] = 0;
		//teamNum[1] = 0;

		//for(int idx=0;idx<GWorld->CurrentLevel->Actors.Num();idx++) {
		//	AActor* pActor = GWorld->CurrentLevel->Actors(idx);		
		//	if(pActor) {
		//		APlayerController* pPc = Cast<APlayerController>(pActor);
		//		// {{ 20071112 ai character 제외
		//		if(pPc && pPc->PlayerReplicationInfo && pPc->PlayerReplicationInfo->Team) {
		//			int teamIdx = pPc->PlayerReplicationInfo->Team->TeamIndex;
		//			if(teamIdx == 0 || teamIdx == 1) {
		//				teamNum[teamIdx]++;
		//			}
		//		}
		//		// }} 20071112 ai character 제외
		//		/*APlayerReplicationInfo* pPRI = Cast<APlayerReplicationInfo>(pActor);
		//		if(pPRI && pPRI->Team) {
		//			if(pPRI->Team->TeamIndex == 0 || pPRI->Team->TeamIndex == 1) {
		//				teamNum[pPRI->Team->TeamIndex]++;
		//			}
		//		}*/
		//	}
		//}
		//debugf(TEXT("t0=%d t1=%d"), teamNum[0], teamNum[1]);

		//if(teamNum[0]==0 || teamNum[1]==0) {			
		//	AGameInfo* pGameInfo = GWorld->GetGameInfo();
		//	if(pGameInfo) {
		//		AavaGame* pavaGameInfo = Cast<AavaGame>(pGameInfo);
		//		if(pavaGameInfo) {
		//			pavaGameInfo->eventTriggerAllOutEvent(teamNum[0]==0 ? 0:1);
		//			return true;
		//		}
		//	}
		//}
	}
	return false;
}
//---------------------------------------------------------------------------
bool _hm_checkAllDead(void)
{
	if(GWorld && GWorld->CurrentLevel) {
		// {{ bReinforecment==true일때는 skip
		AWorldInfo* pWorldInfo = GWorld->GetWorldInfo();
		if(pWorldInfo && pWorldInfo->GRI) {
			AavaGameReplicationInfo *pGRI = Cast<AavaGameReplicationInfo>( pWorldInfo->GRI );
			if(pGRI) {
				if(pGRI->bReinforcement) {
					debugf(TEXT("[_hm_checkAllDead] bReinforcement=true"));
					return false;
				}
			}
		}		
		// }} bReinforecment==true일때는 skip

		for(int idt=0;idt<2;idt++) {
			bool bNotDeadPlayerExist = false;
			for(int idx=0;idx<GWorld->CurrentLevel->Actors.Num();idx++) {
				AActor* pActor = GWorld->CurrentLevel->Actors(idx);		
				if(pActor) {
					APlayerReplicationInfo* pPRI = Cast<APlayerReplicationInfo>(pActor);
					if(pPRI && pPRI->Team) {
						if(pPRI->Team->TeamIndex == idt) {	
							if(!(pPRI->bIsSpectator && !pPRI->bOnlySpectator)) {
								bNotDeadPlayerExist = true;
							}
						}
					}
				}
			}
			if(bNotDeadPlayerExist == false) {
				AGameInfo* pGameInfo = GWorld->GetGameInfo();
				if(pGameInfo) {
					AavaGame* pavaGameInfo = Cast<AavaGame>(pGameInfo);
					if(pavaGameInfo) {
						debugf(TEXT("[dEAthcURe|_hm_checkAllDead] all dead on team%d"), idt);
						pavaGameInfo->eventTriggerMassacreEvent(idt); // [!] 20070516 // pavaGameInfo->eventHmEndRound();
						return true;
					}
				}
			}
		}		
	}
	return false;
}
// }} 20070308
//---------------------------------------------------------------------------
// {{ 20070306
bool _hm_setupGameInfo(AGameInfo* pGi)
{
	AavaGame* pGame = Cast<AavaGame>(pGi);
	if(pGame) {
		pGame->bMigratedHost = TRUE;
		pGame->bWarmupRound = g_hostMigration.bWarmupRound ? TRUE : FALSE;
		return true;
	}
	return false;
}
// }} 20070306
//---------------------------------------------------------------------------
// {{ 20070305
bool _hm_updatePlayerInfo(APlayerReplicationInfo* pPRI, phmPlayerInfo_t pPi)
{
	AavaPlayerReplicationInfo* pAavaPRI = Cast<AavaPlayerReplicationInfo>(pPRI);
	if(pAavaPRI && pPi) {
		pAavaPRI->Score = pPi->Score;
		pAavaPRI->Deaths = pPi->Deaths;
		pAavaPRI->AttackPoint = pPi->AttackPoint;
		pAavaPRI->DefencePoint = pPi->DefencePoint;
		pAavaPRI->LeaderPoint = pPi->LeaderPoint;
		pAavaPRI->TacticsPoint = pPi->TacticsPoint;		
		return true;
	}
	return false;
}
// }} 20070305
//---------------------------------------------------------------------------
bool _hm_isGameFinished(void)
{
	if(GWorld) {
		int idxStart = 0;
		AavaGameReplicationInfo* pGRI = Cast<AavaGameReplicationInfo>(GWorld->CurrentLevel->findActorByClassName(TEXT("avaGameReplicationInfo"), idxStart));
		debugf(TEXT("[_hm_isGameFinished] AavaGameReplicationInfo* pGRI = 0x%x"), pGRI);
		if(pGRI) {		
			debugf(TEXT("[_hm_isGameFinished] AavaGameReplicationInfo::nWinTeam=%d"), pGRI->nWinTeam);
			if(pGRI->nWinTeam == 0 || pGRI->nWinTeam == 1) {
				return true;
			}
		}
		else {
			debugf(TEXT("[_hm_isGameFinished] invalid AavaGameReplicationInfo"));
		}
	}
	else {
		debugf(TEXT("[_hm_isGameFinished] invalid GWorld"));
	}
	return false;
}
//---------------------------------------------------------------------------
// hmPlayer_t
//---------------------------------------------------------------------------
AActor* hmPlayer_t::findOnLevel(ULevel* pLevel) 
{
	if(pLevel) {
		for(int idx=0;idx<pLevel->Actors.Num();idx++) {
			AActor* pActor = pLevel->Actors(idx);			
			if(bRestorePRI) {
				AavaPlayerReplicationInfo* pPRI = Cast<AavaPlayerReplicationInfo>(pActor);
				if(pPRI && pPRI->PlayerName == playerName) {
					return pPRI;
				}
			}
			else if(bRestorePawn) {
				AavaPawn * pPawn = Cast<AavaPawn>(pActor);
				if(pPawn && pPawn->PlayerReplicationInfo && pPawn->PlayerReplicationInfo->PlayerName == playerName) {
					// {{ 20071011 base relative
					if(-1 != hmIdxBase && g_hostMigration.nPlayerBaseToRestore) {
						debugf(TEXT("[dEAthcURe] Restoring %s pended because of the base %d"), *pPawn->GetFullName(), g_hostMigration.nPlayerBaseToRestore);
						return 0x0;
					}
					// }} 20071011 base relative
					return pPawn;
				}
			}			
		}
	}
	return 0x0;
}
//---------------------------------------------------------------------------
void hmPlayer_t::backupWeaponModifier(FArchive& Ar, AavaWeapon* pWeapon)
{	
	// [+] 20070212
	// 10 x { id, #FAttachedItem x CommonAttachedItems, #FExtraMesh x CommonExtraMeshes}
	for(int lpp=0;lpp<10;lpp++) { // 10개로 h에 박혀있다	
		if(pWeapon->WeaponModifiers[lpp]) {
			UavaModifier* pModifier = Cast<UavaModifier>(pWeapon->WeaponModifiers[lpp]->ClassDefaultObject);
			if(pModifier) {
				Ar << pModifier->Id;
				debugf(TEXT("--Modifier id:%d"), pModifier->Id);
				// {{ saving pModifier->CommonAttachedItems
				int nFAttachedItem = pModifier->CommonAttachedItems.Num();
				Ar << nFAttachedItem;
				for(int lpi=0;lpi<nFAttachedItem;lpi++) {
					struct FAttachedItem item = pModifier->CommonAttachedItems(lpi);
					FString meshName, primarySocket, secondarySocket;
					meshName = item.MeshName; Ar << meshName;									
					primarySocket = item.PrimarySocket.ToString(); Ar << primarySocket;
					secondarySocket = item.SecondarySocket.ToString(); Ar << secondarySocket;
					Ar << item.MaxVisibleDistance;		
					debugf(TEXT("---- CommonAttachedItems[%d] meshName=%s primarySocket=%s secondarySocket=%s MaxVisibleDistance=%f"), 
						lpi, *meshName, *primarySocket, *secondarySocket, item.MaxVisibleDistance);
				}
				// }} saving pModifier->CommonAttachedItems

				// {{ saving pModifier->CommonExtraMeshes
				int nFExtraMesh = pModifier->CommonExtraMeshes.Num();
				Ar << nFExtraMesh;
				for(int lpm=0;lpm<nFExtraMesh;lpm++) {
					struct FExtraMesh extraMesh = pModifier->CommonExtraMeshes(lpm);
					FString meshName;
					meshName = extraMesh.MeshName; Ar << meshName;
					Ar << extraMesh.MaxVisibleDistance;
					debugf(TEXT("---- CommonExtraMeshes[%d] meshName=%s MaxVisibleDistance=%f"), 
						lpm, *meshName, extraMesh.MaxVisibleDistance);
				}
				// }} saving pModifier->CommonExtraMeshes
			}
			else { // pModifier가 0x0이면 id에 -1
				int nid = -1;
				Ar << nid;
				debugf(TEXT("casting pWeapon->WeaponModifiers[%d] to UavaModifier failed."), lpp);
			}
		}		
		else { // pWeapon->WeaponModifiers[lpp]가 0x0이면 id에 -1
			int nid = -1;
			Ar << nid;
			debugf(TEXT("invalid or empty pWeapon->WeaponModifiers[%d]"), lpp);
		}
	}
}
//---------------------------------------------------------------------------
void hmPlayer_t::restoreWeaponModifier(FArchive& Ar, AavaWeapon* pWeapon, APawn* pPawn) //void hmPlayer_t::restoreWeaponModifier(FArchive& Ar, AavaWeapon* pWeapon, AavaPawn* pPawn)
{
	// [+] 20070212
	// 10 x { id, #FAttachedItem x CommonAttachedItems, #FExtraMesh x CommonExtraMeshes}
	bool bModifierUpdated = false;
	for(int lpp=0;lpp<10;lpp++) { // 10개로 h에 박혀있다	
		int Id;
		Ar << Id;	

		if(Id !=-1) {
			debugf(TEXT("Serializer is loading UavaModifier %d"), Id);
			// {{ loading pModifier->CommonAttachedItems
			int nFAttachedItem;			
			Ar << nFAttachedItem;

			for(int lpi=0;lpi<nFAttachedItem;lpi++) {
				FString MeshName, PrimarySocket, SecondarySocket;
				FLOAT MaxVisibleDistance; 

				Ar << MeshName;
				Ar << PrimarySocket;
				Ar << SecondarySocket;
				Ar << MaxVisibleDistance;

				debugf(TEXT("Serializer is loading FAttachedItem:%d %s %s %s %f"), lpi, *MeshName, *PrimarySocket, *SecondarySocket, MaxVisibleDistance);
			}
			// }} loading pModifier->CommonAttachedItems

			// {{ loading pModifier->CommonExtraMeshes
			int nFExtraMesh;			
			Ar << nFExtraMesh;

			for(int lpm=0;lpm<nFExtraMesh;lpm++) {
				FString MeshName;
				FLOAT MaxVisibleDistance; 

				Ar << MeshName;		
				Ar << MaxVisibleDistance;

				debugf(TEXT("Serializer is loading FExtraMesh:%d %s %f"), lpm, *MeshName, MaxVisibleDistance);
			}
			// }} loading pModifier->CommonExtraMeshes		

			// {{ create modifier via script
			if(pWeapon->eventHmAddWeaponModifier(Id) == TRUE) {
				bModifierUpdated = true;
			}
			// }} create modifier via script
		}
	}
	//if(bModifierUpdated) { // [-] 20070529 
	pWeapon->eventHmWeaponModifierDone(Cast<AavaPawn>(pPawn));
	//} // [-] 20070529 
}
//---------------------------------------------------------------------------
void hmPlayer_t::backup(UObject* pObject, ULevel* pLevel) 
{
	__super::backup(pObject, pLevel);	

	AavaPlayerReplicationInfo* pPRI = Cast<AavaPlayerReplicationInfo>(pObject);
	if(pPRI == 0x0) return;

	debugf(TEXT("[dEAthcURe|hmPlayer_t::backup] BACKUP player %s"), *pPRI->PlayerName);

	// {{ 20070305 game player info
	pi.name = pPRI->PlayerName;
	pi.Score = pPRI->Score;
	pi.Deaths = pPRI->Deaths;
	pi.AttackPoint = pPRI->AttackPoint;
	pi.DefencePoint = pPRI->DefencePoint;
	pi.LeaderPoint = pPRI->LeaderPoint;
	pi.TacticsPoint = pPRI->TacticsPoint;	
	// }} 20070305 game player info

	// {{ 20070523
	if(pPRI->Team) {
		int teamIdx = pPRI->Team->TeamIndex;
		if(teamIdx ==0 || teamIdx == 1) {
			g_hostMigration.nTeamPlayerBackuped[teamIdx]++;
		}
	}
	// }} 20070523

	if(pPRI->bIsSpectator && !pPRI->bOnlySpectator) {
		bDead = true;
	}

	bRestorePRI = true;
	FMemoryWriter PRIWriter( PRIBytes, TRUE );
	pPRI->hmSerialize(PRIWriter);

	// {{ 20070530 dEAthcURe|HM
	for(int lpp=0;lpp<3;lpp++) { // 3개로 avaGameClasses.h에 박혀있음
		int bRestoreCri = 0;
		if(pPRI->avaCRI[lpp]) {
			bRestoreCri = 1;
			PRIWriter << bRestoreCri;
			debugf(TEXT("[hmPlayer_t::backup] serializing avaCRI[%d]"), lpp);
			pPRI->avaCRI[lpp]->hmSerialize(PRIWriter);
		}
		else {
			debugf(TEXT("[hmPlayer_t::backup] avaCRI[%d] invalid."), lpp);
			PRIWriter << bRestoreCri;
		}
	}
	// }} 20070530 dEAthcURe|HM
	
	AavaPlayerReplicationInfo* pPriTest = pPRI;
	AavaPawn* pPawn = 0x0;
	for(int idx=0;idx<pLevel->Actors.Num();idx++) {
		AActor* pActor = pLevel->Actors(idx);
		if(pActor) {
			AavaPawn* _pPawn = Cast<AavaPawn>(pActor);
			if(_pPawn && _pPawn->PlayerReplicationInfo == pPRI) {
				pPawn = _pPawn;
				break;
			}
			// {{ 20070831 dEAthcURe|HM
			AavaVehicle* _pVehicle = Cast<AavaVehicle>(pActor);
			if(_pVehicle && _pVehicle->PassengerPRI == pPRI) {
				if(_pVehicle->Seats.Num()>=2 && _pVehicle->Seats(1).SeatPawn) {
					pPawn = Cast<AavaPawn>(_pVehicle->Seats(1).SeatPawn->Driver);
					debugf(TEXT("pawn %s"), *pPawn->GetName());				
					break;
				}
			}
			// }} 20070831 dEAthcURe|HM
		}
	}		


	bRestorePawn = pPawn!=0x0;

	if(bRestorePawn) {
		debugf(TEXT("[dEAthcURe|hmPlayer_t::backup] at %f,%f,%f"), pPawn->Location.X, pPawn->Location.Y, pPawn->Location.Z);

		if(pPawn->bHasHelmet) {
			debugf(TEXT("[dEAthcURe|test] backup with helmet"));
		}
		else {
			debugf(TEXT("[dEAthcURe|test] backup without helmet"));
		}

		this->Location = pPawn->Location;
		this->Rotation = pPawn->Rotation;

		FMemoryWriter pawnWriter( pawnBytes, TRUE );
		pPawn->eventOnHmBackup(); // [+] 20070411
		pPawn->hmSerialize(pawnWriter);

		// {{ 20070207 dEAthcURe
		if(pPawn->CurrentWeapon && !(Cast<AavaThrowableWeapon>(pPawn->CurrentWeapon) && pPawn->CurrentWeapon->AmmoCount==0)) {			
			debugf(TEXT("[dEAthcURe|test] CurrentWeapon %s"), *pPawn->CurrentWeapon->GetName());
			FString currentWeaponName = pPawn->CurrentWeapon->GetClass()->GetPathName();
			pawnWriter << currentWeaponName;
		}
		else {
			debugf(TEXT("[dEAthcURe|test] no CurrentWeapon"));
			FString currentWeaponName = FString(TEXT("None:("));
			pawnWriter << currentWeaponName;
		}
		// }} 20070207 dEAthcURe

		// pPawn을 instigator로 가지고 있는 inventory를 찾아서 serialize한다.
		//// 먼저 갯수부터.
		int idx = 0;
		pi.nInventory = 0;
		for(idx=0;idx<pLevel->Actors.Num();idx++) {
			AActor* pActor = pLevel->Actors(idx);
			if(pActor) {
				AavaWeapon* pWeapon = Cast<AavaWeapon>(pActor);
				if(pWeapon && pWeapon->Instigator == pPawn) {
					if(!(Cast<AavaThrowableWeapon>(pWeapon) && pWeapon->AmmoCount == 0)) { // [+] 20070309 던지는 무기 개수가 0이면 백업 안함
						FString className = pWeapon->GetClass()->GetPathName();
						if(className.Len()) {
							pi.nInventory++;
						}
					}
				}
			}
		}
		pawnWriter << pi.nInventory;
		
		//// 다음 AavaWeapon
		for(idx=0;idx<pLevel->Actors.Num();idx++) {
			AActor* pActor = pLevel->Actors(idx);
			if(pActor) {
				AavaWeapon* pWeapon = Cast<AavaWeapon>(pActor);				
				if(pWeapon && pWeapon->Instigator == pPawn) {
					if(!(Cast<AavaThrowableWeapon>(pWeapon) && pWeapon->AmmoCount == 0)) { // [+] 20070309 던지는 무기 개수가 0이면 백업 안함
						FString className = pWeapon->GetClass()->GetPathName();
						if(className.Len()) {
							pawnWriter << className;
							debugf(TEXT("[dEAthcURe|hmPlayer_t::backup] BACKUP AavaWeapon %s (%s) : %d"), *pWeapon->GetName(), *className, pWeapon->AmmoCount);
							pWeapon->eventOnHmBackup(); // [+] 20070411
							pWeapon->hmSerialize(pawnWriter); // [+] 20070411

							// {{ modifier 탐색
							for(int lpp=0;lpp<10;lpp++) { // 10개로 h에 박혀있다
								UavaModifier* pModifier = Cast<UavaModifier>(pWeapon->WeaponModifiers[lpp]);						
								if(pModifier) {							
									debugf(TEXT("UavaModifier id=%d"), pModifier->Id);
									for(int lpi=0;lpi<pModifier->CommonAttachedItems.Num();lpi++) {
										struct FAttachedItem attachedItem = pModifier->CommonAttachedItems(lpi);
										debugf(TEXT("[modifier:%d] MeshName=%s, psock=%s, ssock=%s, vdist:%f"), lpi,									
											*attachedItem.MeshName, 
											attachedItem.PrimarySocket.GetName(), attachedItem.SecondarySocket.GetName(),									
											attachedItem.MaxVisibleDistance);
									}							
								}								
							}
							// }} modifier 탐색				
							backupWeaponModifier(pawnWriter, pWeapon); // [+] 20070212
						}
					} // [+] 20070309 던지는 무기 개수가 0이면 백업 안함
				}
			}
		}

		// {{ 20071011 base relative
		if(pPawn->Base) {
			AInterpActor* pInterpActor = Cast<AInterpActor>(pPawn->Base);
			if(pInterpActor) {
				hmIdxBase = pInterpActor->hmId;			
				relativeLocation = pPawn->Location - pInterpActor->Location;
				//debugf(TEXT("hmIdx of base=%d %s"), hmIdxBase, *pInterpActor->GetFullName());
			}
			// {{ 20071025 vehicle base
			else {
				hmIdxBase = -2;
				/*AavaVehicle* pVehicle = Cast<AavaVehicle>(pPawn->Base);
				if(pVehicle) {
					hmIdxBase = pVehicle->hmId;			
					relativeLocation = pPawn->Location - pVehicle->Location;
					debugf(TEXT("hmIdx of base=%d %s"), hmIdxBase, *pVehicle->GetFullName());
				}*/
			}
			// }} 20071025 vehicle base
		}
		// }} 20071011 base relative
	}	
	return;
}
//---------------------------------------------------------------------------
#define eyp	3.0f // 10.0f 에서 PlayerController/const MAXPOSITIONERRORSQUARED = 12.0;에 따라 다운시킴
#define eypNonPlayer 1.0f	// eyp을 적용하면 rigid body들이 떨어지다 넘어진다. eyp보다 작은 값 적용
bool hmPlayer_t::restore(UObject* pObject, ULevel* pLevel) 
{
	// {{ [!] 20070301
	if(bRestorePRI) {
		AavaPlayerReplicationInfo* pPRI = Cast<AavaPlayerReplicationInfo>(pObject);
		if(pPRI) {
			if(pPRI && pPRI->PlayerName == playerName) {
				//debugf(TEXT("[dEAthcURe|hmPlayer_t::restore] {{RESTORE PRI for %s at %f,%f,%f"), *pPRI->PlayerName, pPRI->Location.X, pPRI->Location.Y, pPRI->Location.Z);

				FMemoryReader PRIReader( PRIBytes, TRUE );
				pPRI->hmSerialize(PRIReader);

				// {{ 20070530 dEAthcURe|HM
				for(int lpp=0;lpp<3;lpp++) { // 3개로 avaGameClasses.h에 박혀있음
					int bRestoreCri = 0;
					PRIReader << bRestoreCri;
					if(bRestoreCri) {
						if(pPRI->avaCRI[lpp]) {
							debugf(TEXT("[hmPlayer_t::restore] unserializing avaCRI[%d]"), lpp);
							pPRI->avaCRI[lpp]->hmSerialize(PRIReader);
						}
						else {
							debugf(TEXT("[hmPlayer_t::restore] invalid avaCRI[%d]"), lpp);
							break; // cri가 invalid하면 버린다.
						}
					}
					else {
						debugf(TEXT("[hmPlayer_t::restore] restoring avaCRI[%d] skipped."), lpp);
					}
				}
				// }} 20070530 dEAthcURe|HM

				debugf(TEXT("[dEAthcURe|hmPlayer_t::restore] RESTORE PRI for %s at %f,%f,%f"), *pPRI->PlayerName, pPRI->Location.X, pPRI->Location.Y, pPRI->Location.Z);

				// {{ 20061117 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
				pPRI->Location.Z += eyp;
				// }} 20061117 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.

				// {{ 20070310 bHost세팅
				if(pPRI->PlayerName == g_hostMigration.playerName) {
					pPRI->bHost = TRUE;
					debugf(TEXT("PRI of player %s is set to host"), *pPRI->PlayerName);
				}
				// }} 20070310 bHost세팅			

				pPRI->eventOnHmRestore(); // 20070326

				bRestorePRI = false;
				g_hostMigration.bReadyToRestoreGRI = true; // testdd				
				return true;
			}
		}
	}
	else if(bRestorePawn) {
		AavaPawn* pPawn = Cast<AavaPawn>(pObject);
		if(pPawn && pPawn->PlayerReplicationInfo && pPawn->PlayerReplicationInfo->PlayerName == playerName && pPawn->InvManager) {
			//debugf(TEXT("[dEAthcURe] {{RESTORE Pawn for %s at %f,%f,%f"), *pPawn->PlayerReplicationInfo->PlayerName, pPawn->Location.X, pPawn->Location.Y, pPawn->Location.Z);

			FMemoryReader pawnReader( pawnBytes, TRUE );
			pPawn->hmSerialize(pawnReader);

			debugf(TEXT("[dEAthcURe] RESTORE Pawn for %s at %f,%f,%f"), *pPawn->PlayerReplicationInfo->PlayerName, pPawn->Location.X, pPawn->Location.Y, pPawn->Location.Z);

			// {{ 20070208 dEAthcURe
			FString currentWeaponName;
			pawnReader << currentWeaponName;
			debugf(TEXT("[dEAthcURe|hmPlayer_t::restore] current weapon = %s"), *currentWeaponName);
			// }} 20070208 dEAthcURe

			if(pPawn->bHasHelmet) {
				debugf(TEXT("[dEAthcURe|test] RESTORE with helmet"));
			}
			else {
				debugf(TEXT("[dEAthcURe|test] RESTORE without helmet"));
			}

			pPawn->Location.Z += eyp;		

			// {{ 20061117 controller강제세팅
			if(pPawn->Controller) {
				pPawn->Controller->Location = pPawn->Location;
				pPawn->Controller->Rotation = pPawn->Rotation;
			}

			if(pPawn->PlayerReplicationInfo) {
				pPawn->PlayerReplicationInfo->Location = pPawn->Location;
				pPawn->PlayerReplicationInfo->Rotation = pPawn->Rotation;
			}
			// }} 20061117 controller강제세팅	

			int nInv = 0;
			pawnReader << nInv;			
			
			bool bSetCurrentWeapon = false; // [+] 20070309
			for(int lpp=0;lpp<nInv;lpp++) {
				FString className;
				pawnReader << className;

				UBOOL bActivate = className == currentWeaponName ? TRUE : FALSE;

				debugf(TEXT("[dEAthcURe] RESTORE AInventory:%d %s %s"), lpp, *className, bActivate ? TEXT("*current*") : TEXT(""));

				// AavaWeapon* pWeapon = Cast<AavaWeapon>(pPawn->eventHmAddWeapon(className, bActivate));
				AavaWeapon* pWeapon = Cast<AavaWeapon>(pPawn->eventHmAddWeaponBegin(className)); 
				if(pWeapon) {
					pWeapon->hmSerialize(pawnReader); // [+] 20070411
					pWeapon->eventOnHmRestore(); // [+] 20070411

					restoreWeaponModifier(pawnReader, pWeapon, pPawn); // [+] 20070212 dEAthcURe
					pPawn->eventHmAddWeaponEnd(pWeapon, bActivate);
					debugf(TEXT("[dEAthcURe|hmPlayer_t::restore] Pawn %s has a weapon %s"), *pPawn->GetName(), *className); // debugf(TEXT("[dEAthcURe|hmPlayer_t::restore] Pawn %s has a weapon %s %d %d"), *pPawn->GetName(), *className, ammoCount, reloadCnt);
					if(bActivate) bSetCurrentWeapon = true; // [+] 20070309
				}
				else {
					debugf(TEXT("[dEAthcURe|hmPlayer_t::restore] creating weapon %s for pawn %s failed."), *className, *pPawn->GetName());
				}				
			}			

			// {{ 2007309
			if(bSetCurrentWeapon == false) {
				if(pPawn->InvManager) {
					pPawn->InvManager->eventHmSwitchToBestWeapon();
				}
			}
			// }} 2007309

			// {{ 20070523 새로운 분대장 임명
			if(pPawn->PlayerReplicationInfo->Team) {
				int teamIdx = pPawn->PlayerReplicationInfo->Team->TeamIndex;
				if(teamIdx ==0 || teamIdx == 1) {
					g_hostMigration.nTeamPlayerRestored[teamIdx]++;

					if(g_hostMigration.bReelectLeader && teamIdx == g_hostMigration.hostTeamIdx && g_hostMigration.nTeamPlayerRestored[teamIdx] == g_hostMigration.nTeamPlayerBackuped[teamIdx]) {
						_hm_electLeader(GWorld ? GWorld->GetGameInfo(): 0x0, teamIdx);
						g_hostMigration.bReelectLeader = false;
					}
				}
			}				
			// }} 20070523 새로운 분대장 임명

			// {{ 20071011 base relative
			if(-2 == hmIdxBase) {
				pPawn->setPhysics(PHYS_Falling, NULL, FVector(0,0,1));
			} else
			if(-1 != hmIdxBase) {
				for(int lpp=0;lpp<pLevel->Actors.Num();lpp++) {
					AInterpActor* pInterpActor = Cast<AInterpActor>(pLevel->Actors(lpp));
					if(pInterpActor && pInterpActor->hmId == hmIdxBase) {
						pPawn->Location = pInterpActor->Location + relativeLocation;
						// {{ 20061117 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
						pPawn->Location.Z += eyp;
						// }} 20061117 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
						pPawn->setPhysics(PHYS_Falling, NULL, FVector(0,0,1));
						pPawn->SetBase(pInterpActor); // 20071015
						debugf(TEXT("[dEAthcURe|HM] Player restored on %s"), *pInterpActor->GetFullName());
						break;
					}		
					// {{ 20071025 vehicle base
					//else {						
					//	AavaVehicle* pVehicle = Cast<AavaVehicle>(pLevel->Actors(lpp));
					//	if(pVehicle && pVehicle->hmId == hmIdxBase) {
					//		pPawn->Location = pVehicle->Location + relativeLocation;
					//		// {{ 20061117 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
					//		pPawn->Location.Z += eyp;
					//		// }} 20061117 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
					//		pPawn->setPhysics(PHYS_Falling, NULL, FVector(0,0,1));
					//		pPawn->SetBase(pVehicle); // 20071015
					//		debugf(TEXT("[dEAthcURe|HM] Player restored on %s"), *pVehicle->GetFullName());
					//		break;
					//	}
					//}
					// }} 20071025 vehicle base
				}								
			}
			// }} 20071011 base relative

			pPawn->eventOnHmRestore(); // 20070326

			bRestorePawn = false;
			bUpdated = true;			
			return true;
		}
	}
	return false;
	// }} [!] 20070301
	if(bRestorePRI) {
		AavaPlayerReplicationInfo* pPRI = Cast<AavaPlayerReplicationInfo>(pObject);
		if(pPRI && pPRI->PlayerName == playerName) {
			debugf(TEXT("[dEAthcURe] {{RESTORE PRI for %s %f %f %f"), *pPRI->PlayerName, pPRI->Location.X, pPRI->Location.Y, pPRI->Location.Z);

			FMemoryReader PRIReader( PRIBytes, TRUE );
			pPRI->hmSerialize(PRIReader);

			debugf(TEXT("[dEAthcURe] }}RESTORE PRI for %s %f %f %f"), *pPRI->PlayerName, pPRI->Location.X, pPRI->Location.Y, pPRI->Location.Z);

			// {{ 20061117 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
			pPRI->Location.Z += eyp;
			// }} 20061117 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.

			bRestorePRI = false;
			g_hostMigration.bReadyToRestoreGRI = true; // testdd
			return true;
		}
	}
	else if(bRestorePawn) {
		AavaPawn* pPawn = Cast<AavaPawn>(pObject);
		if(pPawn && pPawn->PlayerReplicationInfo->PlayerName == playerName && pPawn->InvManager) {

			debugf(TEXT("[dEAthcURe] RESTORE Pawn for %s"), *pPawn->PlayerReplicationInfo->PlayerName);			

			FMemoryReader pawnReader( pawnBytes, TRUE );
			pPawn->hmSerialize(pawnReader);

			// {{ 20070208 dEAthcURe
			FString currentWeaponName;
			pawnReader << currentWeaponName;			
			// }} 20070208 dEAthcURe

			if(pPawn->bHasHelmet) {
				debugf(TEXT("[dEAthcURe|test] RESTORE with helmet"));
			}
			else {
				debugf(TEXT("[dEAthcURe|test] RESTORE without helmet"));
			}

			pPawn->Location.Z += eyp;		

			// {{ 20061117 controller강제세팅
			if(pPawn->Controller) {
				pPawn->Controller->Location = pPawn->Location;
				pPawn->Controller->Rotation = pPawn->Rotation;
			}

			if(pPawn->PlayerReplicationInfo) {
				pPawn->PlayerReplicationInfo->Location = pPawn->Location;
				pPawn->PlayerReplicationInfo->Rotation = pPawn->Rotation;
			}
			// }} 20061117 controller강제세팅	

			int nInv = 0;
			pawnReader << nInv;

			TArray<AWeapon*> processedWeapons;
			
			for(int lpp=0;lpp<nInv;lpp++) {
				FString className;
				pawnReader << className;

				int ammoCount;
				pawnReader << ammoCount;

				// {{ 20061123
				int reloadCnt;
				pawnReader << reloadCnt;
				// }} 20061123

				debugf(TEXT("[dEAthcURe] RESTORE AInventory %d:%s"), lpp, *className);

				if(pPawn->InvManager) {
					bool bRestored = false;

					for(AInventory* pInv=pPawn->InvManager->InventoryChain;pInv;pInv=pInv->Inventory) {
						AavaWeapon* pWeapon = Cast<AavaWeapon>(pInv);
						if(pWeapon) {
							if(pWeapon->GetClass()->GetPathName() == className) {
								debugf(TEXT("[dEAthcURe|hmPlayer_t::restore] Pawn %s already has a weapon %s "), *pPawn->GetName(), *className);
								pWeapon->AmmoCount = ammoCount;
								// {{ 20061123
								AavaWeap_BaseGun* pGun = Cast<AavaWeap_BaseGun>(pWeapon);
								if(pGun) {
									pGun->ReloadCnt = reloadCnt;
								}
								// }} 20061123
								bRestored = true;
								processedWeapons.Push(pWeapon);
								restoreWeaponModifier(pawnReader, pWeapon, pPawn); // [+] 20070212 dEAthcURe
								break;
							}
						}
					}
					if(!bRestored) {
						debugf(TEXT("[dEAthcURe|hmPlayer_t::restore] Pawn %s does not have a weapon %s, create new one"), *pPawn->GetName(), *className);										
						AavaWeapon* pWeapon = Cast<AavaWeapon>(pPawn->eventHmAddWeapon(className, false));
						if(pWeapon) {
							pWeapon->AmmoCount = ammoCount;
							// {{ 20061123
							AavaWeap_BaseGun* pGun = Cast<AavaWeap_BaseGun>(pWeapon);
							if(pGun) {
								pGun->ReloadCnt = reloadCnt;
							}			
							// }} 20061123
							processedWeapons.Push(pWeapon);
							restoreWeaponModifier(pawnReader, pWeapon, pPawn); // [+] 20070212 dEAthcURe
						}				
					}			
				}
				else {
					debugf(TEXT("[dEAthcURe|hmPlayer_t::restore] No InvManager on %s"), *pPawn->GetName());		
				}		
			}

			// {{ 20061106 dEAthcURe BACKUP항목에 없는 weapon의 삭제
			// 좀더 쉽게 갈수도 있겠으나 script 수정을 최소화하기 위해 이와 같이 구현하였3	
			AInventory* pInv=pPawn->InvManager->InventoryChain;
			while(pInv) {
				bool bExist = false;
				for(int idx=0;idx<processedWeapons.Num();idx++) { // for(list<AWeapon*>::iterator it=processedWeapons.begin();it!=processedWeapons.end();it++) {
					AWeapon* pWeapon = processedWeapons(idx);
					if(pWeapon == pInv) {
						bExist = true;
						break;
					}
				}
				if(bExist == false) {
					debugf(TEXT("[dEAthcURe|hmPlayer_t::restore] --Default weapon %s destroyed"), *pInv->GetClass()->GetName());
					AavaWeapon* pWeapon = Cast<AavaWeapon>(pInv);
					pWeapon->eventDestroyed();
					pInv = pPawn->InvManager->InventoryChain;
				}
				else {
					debugf(TEXT("[dEAthcURe|hmPlayer_t::restore] --Weapon %s granted"), *pInv->GetClass()->GetName());
					pInv = pInv->Inventory;
				}
			}	
			// }} 20061106 dEAthcURe BACKUP항목에 없는 weapon의 삭제			

			AavaPlayerController* pCtrl = Cast<AavaPlayerController>(pPawn->Controller);
			if(pCtrl) {
				pCtrl->eventHmSetCurrentWeapon(currentWeaponName);
			}			

			bRestorePawn = false;
			bUpdated = true;
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------------
void hmPlayer_t::testSuite(phostMigration_t pSender, UWorld* pWorld)
{
	return;
	if(pWorld) {
		for (int idxActor = 0; idxActor < pWorld->CurrentLevel->Actors.Num(); idxActor++) {
			AActor* pActor = GWorld->CurrentLevel->Actors(idxActor);	
			if(pActor) {
				// pri는 pawn이 없어도 존재한다.
				// playercontroller는 client입장에서 자기거 하나밖에 없다 hm기준이 못됨				
			}
		}
	}
}
//---------------------------------------------------------------------------
hmPlayer_t::hmPlayer_t(UObject* pObject, ULevel* pLevel) 
: hmPlayerBase_t(pObject, pLevel) 
{
	timeInterval = 0;

	// {{ 20071011 base relative
	hmIdxBase = -1;	
	// }} 20071011 base relative

	backup(pObject, pLevel);

	AavaPlayerReplicationInfo* pPRI = Cast<AavaPlayerReplicationInfo>(pObject);
	if(pPRI) {
		_tcscpy(playerName, *pPRI->PlayerName);
	}
}
//---------------------------------------------------------------------------
// hmLocalPlayer_t
//---------------------------------------------------------------------------
void hmLocalPlayer_t::backup(UObject* pObject, ULevel* pLevel)
{
	__super::backup(pObject, pLevel);
}
//---------------------------------------------------------------------------
bool hmLocalPlayer_t::restore(UObject* pObject, ULevel* pLevel)
{
	return __super::restore(pObject, pLevel);
}
//---------------------------------------------------------------------------
bool hmLocalPlayer_t::canCreate(UObject* pObject)
{
	AavaPlayerReplicationInfo* pPRI = Cast<AavaPlayerReplicationInfo>(pObject);
	if(pPRI) {
		BYTE role = pPRI->Role;
		TCHAR name[32];
		_tcscpy(name, *pPRI->PlayerName);
		if(pPRI->Role == ROLE_AutonomousProxy || pPRI->Role == ROLE_Authority) {
			// {{ 20070222 이전 호스트는 backup하지 않는다.
			if(!_tcscmp(name, g_hostMigration.hostName)) {
				g_hostMigration.log(TEXT("[hmLocalPlayer_t::canCreate] Backup player data canceled because of prev host %s"), name);

				AActor* pActor = 0x0;

				// {{ 20070309 host가 c4를 갖고 나갔으면 pickup_c4를 복원하지 않는다.
				pActor = _hm_checkPlayerHasSomething(GWorld->CurrentLevel, pPRI, TEXT("avaWeap_C4"));
				if(GWorld && pActor) {					
					APawn* pPawn = _hm_getPawn(GWorld->CurrentLevel, pPRI);
					if(pPawn) {						
						hmDroppingPickup_t::create(pActor, pPawn->Location, pPawn->Rotation);
					}
					if(g_hostMigration.pMissionObject == 0x0) {
						g_hostMigration.bDoNotRestoreC4 = true;
					}
				}
				// }} 20070309 host가 c4를 갖고 나갔으면 pickup_c4를 복원하지 않는다.

				// {{ 20070507 host가 NukeCase를 갖고 나갔으면 avaPickupProvider를 복원하지 않는다.
				pActor = _hm_checkPlayerHasSomething(GWorld->CurrentLevel, pPRI, TEXT("avaWeap_NukeCase"));
				if(GWorld && pActor) {					
					APawn* pPawn = _hm_getPawn(GWorld->CurrentLevel, pPRI);
					if(pPawn) {
						hmDroppingPickup_t::create(pActor, pPawn->Location, pPawn->Rotation);
					}
					if(g_hostMigration.pMissionObject == 0x0) {
						g_hostMigration.bDoNotRestoreNukeCase = true;
					}
				}
				// }} 20070507 host가 NukeCase를 갖고 나갔으면 avaPickupProvider를 복원하지 않는다.

				// {{ 20070523 host가 쌍안경을 갖고 나갔으면 60초 후에 분대장을 재선출한다.
				if(GWorld && _hm_checkPlayerHasSomething(GWorld->CurrentLevel, pPRI, TEXT("avaWeap_Binocular"))) {
					g_hostMigration.bReelectLeader = true;
				}
				// }} 20070523 host가 쌍안경을 갖고 나갔으면 60초 후에 분대장을 재선출한다.
				return false;
			}
			// }} 20070222 이전 호스트는 backup하지 않는다.
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmLocalPlayer_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmLocalPlayer_t(pObject, pLevel); // with controller
}
//---------------------------------------------------------------------------
void hmLocalPlayer_t::onDestroy(phostMigration_t pSender)
{
	pSender->nLocals--;
	pSender->bReadyToRestoreGRI = true;
}
//---------------------------------------------------------------------------
hmLocalPlayer_t::hmLocalPlayer_t(UObject* pObject, ULevel* pLevel)
: hmPlayer_t(pObject, pLevel)
{
}
//---------------------------------------------------------------------------
// hmRemotePlayer_t
//---------------------------------------------------------------------------
void hmRemotePlayer_t::backup(UObject* pObject, ULevel* pLevel) 
{
	__super::backup(pObject, pLevel);		
}
//---------------------------------------------------------------------------
bool hmRemotePlayer_t::restore(UObject* pObject, ULevel* pLevel) 
{
	return __super::restore(pObject, pLevel);		
}
//---------------------------------------------------------------------------
bool hmRemotePlayer_t::canCreate(UObject* pObject)
{
	AavaPlayerReplicationInfo* pPRI = Cast<AavaPlayerReplicationInfo>(pObject);
	if(pPRI) {
		if(pPRI->Role == ROLE_SimulatedProxy) {
			// {{ 20070222 이전 호스트는 backup하지 않는다.
			if(!_tcscmp(*pPRI->PlayerName, g_hostMigration.hostName)) {
				g_hostMigration.log(TEXT("[hmRemotePlayer_t::canCreate] Backup player data canceled because of prev host %s"), *pPRI->PlayerName);

				AActor* pActor = 0x0;

				// {{ 20070309 host가 c4를 갖고 나갔으면 pickup_c4를 복원하지 않는다.
				pActor = _hm_checkPlayerHasSomething(GWorld->CurrentLevel, pPRI, TEXT("avaWeap_C4"));
				if(GWorld && pActor) {					
					APawn* pPawn = _hm_getPawn(GWorld->CurrentLevel, pPRI);
					if(pPawn) {
						hmDroppingPickup_t::create(pActor, pPawn->Location, pPawn->Rotation);
					}
					if(g_hostMigration.pMissionObject == 0x0) {
						g_hostMigration.bDoNotRestoreC4 = true;
					}
				}
				// }} 20070309 host가 c4를 갖고 나갔으면 pickup_c4를 복원하지 않는다.

				// {{ 20070507 host가 NukeCase를 갖고 나갔으면 avaPickupProvider를 복원하지 않는다.
				pActor = _hm_checkPlayerHasSomething(GWorld->CurrentLevel, pPRI, TEXT("avaWeap_NukeCase"));
				if(GWorld && pActor) {					
					APawn* pPawn = _hm_getPawn(GWorld->CurrentLevel, pPRI);
					if(pPawn) {
						hmDroppingPickup_t::create(pActor, pPawn->Location, pPawn->Rotation);
					}
					if(g_hostMigration.pMissionObject == 0x0) {
						g_hostMigration.bDoNotRestoreNukeCase = true;
					}
				}
				// }} 20070507 host가 NukeCase를 갖고 나갔으면 avaPickupProvider를 복원하지 않는다.

				// {{ 20070523 host가 쌍안경을 갖고 나갔으면 60초 후에 분대장을 재선출한다.
				if(GWorld && _hm_checkPlayerHasSomething(GWorld->CurrentLevel, pPRI, TEXT("avaWeap_Binocular"))) {
					g_hostMigration.bReelectLeader = true;
				}
				// }} 20070523 host가 쌍안경을 갖고 나갔으면 60초 후에 분대장을 재선출한다.
				return false;
			}
			// }} 20070222 이전 호스트는 backup하지 않는다.
			return true;
		}
	}
	return false;	
}
//---------------------------------------------------------------------------
phmData_t hmRemotePlayer_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nRemotes++;
	return new hmRemotePlayer_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
void hmRemotePlayer_t::onDestroy(phostMigration_t pSender)
{
	pSender->nRemotes--;
}
//---------------------------------------------------------------------------
hmRemotePlayer_t::hmRemotePlayer_t(UObject* pObject, ULevel* pLevel) 
: hmPlayer_t(pObject, pLevel) 
{		
}
//---------------------------------------------------------------------------
// hmActor_t
//---------------------------------------------------------------------------
void hmActor_t::backup(UObject* pObject, ULevel* pLevel) 
{
	AActor* pActor = Cast<AActor>(pObject);
	if(pActor) {
		__super::backup(pActor, pLevel);

		debugf(TEXT("[dEAthcURe|hmActor_t] BACKUP %s %s(hmIndex:%d) at (%f %f %f)"), 
			*pActor->GetClass()->GetName(), *pActor->GetName(), pActor->hmIndex, pActor->Location.X, pActor->Location.Y, pActor->Location.Z);
		
		FMemoryWriter MemoryWriter( Bytes, TRUE );
		pActor->hmSerialize(MemoryWriter);
	}
}
//---------------------------------------------------------------------------
bool hmActor_t::restore(UObject* pObject, ULevel* pLevel) 
{
	AActor* pActor = Cast<AActor>(pObject);
	if(pActor) {
		FMemoryReader MemoryReader( Bytes, TRUE );
		pActor->hmSerialize(MemoryReader);

		// {{ 20070522
		if(pActor->GetClass()->GetName() == TEXT("KActor")) {
			Location.Z += eyp;
			debugf(TEXT("[dEAthcURe|hmActor_t::restore] correcting z for %s"), *pActor->GetName());
		}
		// }} 20070522

		pActor->eventOnHmRestore(); // 20070326
		
		debugf(TEXT("[dEAthcURe|hmActor_t] RESTORE existing %s %s(hmIndex:%d) at (%f %f %f)"), 
			*pActor->GetClass()->GetName(), *pActor->GetName(), pActor->hmIndex, pActor->Location.X, pActor->Location.Y, pActor->Location.Z);

		bUpdated = true;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool hmActor_t::canCreate(UObject* pObject)
{
	FString className = pObject->GetClass()->GetName();
	if(className == TEXT("KActor")) return true;	
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmActor_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmActor_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmActor_t::hmActor_t(UObject* pObject, ULevel* pLevel) 
: hmData_t(pObject, pLevel) 
{	
	backup(pObject, pLevel);
}
//---------------------------------------------------------------------------
// hmPickup_t
//---------------------------------------------------------------------------
AActor* hmPickup_t::findOnLevel(ULevel* pLevel) 
{
	if(pLevel) {
		// {{ 20061205 index값이 valid하면 index를 가지고 찾을수 있3
		AActor* pActor = __super::findOnLevel(pLevel);
		if(pActor) {
			return pActor;
		}
		// }} 20061205 index값이 valid하면 index를 가지고 찾을수 있3

		int idxStart = 0;
		while(idxStart<pLevel->Actors.Num()) {
			AavaPickup* pPickup = Cast<AavaPickup>(pLevel->findActorByClassName(TEXT("avaPickup"), idxStart));
			if(pPickup==0x0) {
				break;
			}
			debugf(TEXT("%f %f %f - %f %f %f"), Location.X, Location.Y, Location.Z, pPickup->Location.X, pPickup->Location.Y, pPickup->Location.Z);
			debugf(TEXT("%s %s"), *className, *inventoryClassName);
			FVector diff = Location - pPickup->Location;
			if(diff.Size2D() < 10.0f) {
				return pPickup;
			}			
		}
	}
	return 0x0; // test for restore
}
//---------------------------------------------------------------------------
void hmPickup_t::backup(UObject* pObject, ULevel* pLevel) 
{
	__super::backup(pObject, pLevel);

	AavaPickup* pPickup = Cast<AavaPickup>(pObject);
	if(pPickup == 0x0) return;				

	debugf(TEXT("[dEAthcURe] BACKUP AavaPickup %s at (%f %f %f)"), *pPickup->GetName(), pPickup->Location.X, pPickup->Location.Y, pPickup->Location.Z);
	
	FMemoryWriter MemoryWriter( Bytes, TRUE );
	pPickup->hmSerialize(MemoryWriter);	

	// non-serialized data here	
	className = pPickup->GetClass()->GetPathName();
	debugf(TEXT("[hmPickup_t::backup] className=%s"), *className);
	inventoryClassName = pPickup->InventoryClass->GetPathName();
	debugf(TEXT("[hmPickup_t::backup] inventoryClassName=%s"), *inventoryClassName);

	// {{ 20070309 backup weapon inventory
	int bWeapon = 0;
	AavaWeapon* pWeapon = 0x0;
	if(pPickup->Inventory) {
		pWeapon = Cast<AavaWeapon>(pPickup->Inventory);
		if(pWeapon) {
			bWeapon = 1;
		}
	}
		
	MemoryWriter << bWeapon;

	if(pWeapon) {
		FString className = pWeapon->GetClass()->GetPathName();		
		MemoryWriter << className;
		debugf(TEXT("[hmPickup_t::backup] serializing weapon %s"), *className);
		MemoryWriter << pWeapon->AmmoCount;
		// {{ 20061123
		AavaWeap_BaseGun* pGun = Cast<AavaWeap_BaseGun>(pWeapon);
		if(pGun) {			
			MemoryWriter << pGun->ReloadCnt;
		}
		else { // gun이든 아니든 기록한다.
			BYTE zero =0; // [!] 20070624 int -> byte
			MemoryWriter << zero;
		}
		// }} 20061123
		hmPlayer_t::backupWeaponModifier(MemoryWriter, pWeapon);		
	}	
	// }} 20070309 backup weapon inventory
}
//---------------------------------------------------------------------------
bool hmPickup_t::restore(UObject* pObject, ULevel* pLevel) 
{
	if(pLevel == 0x0) return false;
	
	AavaPickup* pPickup = Cast<AavaPickup>(pObject);
	if(pPickup == 0x0) return false;	

	// {{ [!] 20070507 위치이동 type checing후 [+] 20070309 host가 c4를 갖고 나갔으면 pickup_c4를 복원하지 않는다.
	if(g_hostMigration.bDoNotRestoreC4 && className == TEXT("avaRules.avaPickUp_C4")) return true;	
	// }} [!] 20070507 위치이동 type checing후 [+] 20070309 host가 c4를 갖고 나갔으면 pickup_c4를 복원하지 않는다.

	// {{ [+] 20070507 host가 NukeCase를 갖고 나갔으면 avaPickupProvider를 복원하지 않는다.	
	if(g_hostMigration.bDoNotRestoreNukeCase && inventoryClassName == TEXT("avaRules.avaWeap_NukeCase")) {
		debugf(TEXT("[hmPickup_t::restore] Restoring avaGame.avaPickupProvider for avaRules.avaWeap_NukeCase *canceled*"));
		return true;
	}	
	// }} [+] 20070507 host가 NukeCase를 갖고 나갔으면 avaPickupProvider를 복원하지 않는다.

	FMemoryReader MemoryReader( Bytes, TRUE );
	pPickup->hmSerialize(MemoryReader);

	// {{ 20070309 backup weapon inventory
	int bWeapon = 0;
	MemoryReader << bWeapon;

	if(bWeapon) {
		FString className;
		MemoryReader << className;

		BYTE ammoCount = 0; // [!] 20070624 int -> byte
		MemoryReader << ammoCount;
		
		BYTE reloadCnt = 0; // [!] 20070624 int -> byte
		MemoryReader << reloadCnt;

		debugf(TEXT("[hmPickup_t::restore] weapon class %s %d %d"), *className, ammoCount, reloadCnt);

		AavaPlayerController* pCtrl = Cast<AavaPlayerController>(pLevel->findActor(APlayerController::StaticClass()));
		if(pCtrl) {		
			AActor* pActor = pCtrl->eventHmSpawnActor(className, Location, Rotation);
			if(pActor) {
				AavaWeapon* pWeapon = Cast<AavaWeapon>(pActor);
				if(pWeapon) {
					pWeapon->AmmoCount = ammoCount;
					AavaWeap_BaseGun* pGun = Cast<AavaWeap_BaseGun>(pWeapon);
					if(pGun) {
						pGun->ReloadCnt = reloadCnt;
					}
					hmPlayer_t::restoreWeaponModifier(MemoryReader, pWeapon);
					pPickup->Inventory = pWeapon;
				}				
			}
		}
	}
	else {
		debugf(TEXT("[hmPickup_t::restore] non weapon class"));
	}
	// }} 20070309 backup weapon inventory

	if(pPickup->bShutDown) {
		pPickup->eventHmShutdown();
	}

	if(pLevel) {
		// non-serialized data here		
	}			

	pPickup->eventOnHmRestore(); // 20070326
	
	debugf(TEXT("[dEAthcURe] RESTORE check index %d %d"), index, pObject->GetNetIndex());
	debugf(TEXT("[dEAthcURe] RESTORE AavaPickup %s at (%f %f %f)"), *pPickup->GetName(), pPickup->Location.X, pPickup->Location.Y, pPickup->Location.Z);

	bUpdated = true;
	return true;
}
//---------------------------------------------------------------------------
bool hmPickup_t::restore(ULevel* pLevel) 
{	
	if(pLevel == 0x0) return false;

	// {{ [+] 20070309 host가 c4를 갖고 나갔으면 pickup_c4를 복원하지 않는다.
	if(g_hostMigration.bDoNotRestoreC4 && className == TEXT("avaRules.avaPickUp_C4")) return true;	
	// }} [+] 20070309 host가 c4를 갖고 나갔으면 pickup_c4를 복원하지 않는다.

	// {{ [+] 20070507 host가 NukeCase를 갖고 나갔으면 avaPickupProvider를 복원하지 않는다.	
	if(g_hostMigration.bDoNotRestoreNukeCase && inventoryClassName == TEXT("avaRules.avaWeap_NukeCase")) {
		debugf(TEXT("[hmPickup_t::restore] Restoring avaGame.avaPickupProvider for avaRules.avaWeap_NukeCase *canceled*"));
		return true;
	}	
	// }} [+] 20070507 host가 NukeCase를 갖고 나갔으면 avaPickupProvider를 복원하지 않는다.

	AavaPlayerController* pCtrl = Cast<AavaPlayerController>(pLevel->findActor(APlayerController::StaticClass()));
	if(pCtrl) {
		// {{ 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
		Location.Z += eypNonPlayer;
		// }} 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.

		AActor* pActor = pCtrl->eventHmSpawnActor(className, Location, Rotation);
		if(pActor) {
			FMemoryReader MemoryReader( Bytes, TRUE );
			pActor->hmSerialize(MemoryReader);

			AavaPickup* pPickup = Cast<AavaPickup>(pActor);
			if(pPickup) {
				// {{ 20070309 backup weapon inventory
				int bWeapon = 0;
				MemoryReader << bWeapon;

				if(bWeapon) {
					FString className;
					MemoryReader << className;

					BYTE ammoCount = 0; // [!] 20070624 int -> byte
					MemoryReader << ammoCount;
					
					BYTE reloadCnt = 0; // [!] 20070624 int -> byte
					MemoryReader << reloadCnt;

					debugf(TEXT("[hmPickup_t::restore] weapon class %s %d %d"), *className, ammoCount, reloadCnt);
					
					AActor* pActor = pCtrl->eventHmSpawnActor(className, Location, Rotation);
					if(pActor) {
						AavaWeapon* pWeapon = Cast<AavaWeapon>(pActor);
						if(pWeapon) {
							pWeapon->AmmoCount = ammoCount;
							AavaWeap_BaseGun* pGun = Cast<AavaWeap_BaseGun>(pWeapon);
							if(pGun) {
								pGun->ReloadCnt = reloadCnt;
							}
							hmPlayer_t::restoreWeaponModifier(MemoryReader, pWeapon);
							pPickup->Inventory = pWeapon;
						}
						
					}				
				}
				else {
					debugf(TEXT("[hmPickup_t::restore] non weapon class"));
				}
				// }} 20070309 backup weapon inventory
			
				//BYTE* pti = &pPickup->TeamIdx;
				debugf(TEXT("TeamIdx=%d"), pPickup->TeamIdx);	
				pPickup->eventHmSetupPickup(inventoryClassName);
				
				debugf(TEXT("[dEAthcURe] RESTORE %s %s at (%f %f %f)"), *pActor->GetClass()->GetName(), *pActor->GetName(), pPickup->Location.X, pPickup->Location.Y, pPickup->Location.Z);

				if(pPickup->bDynamicSpawned) {
					debugf(TEXT("[dEAthcURe|hmPickup_t::restore] bDynamicSpawned"));
				}
				else {
					debugf(TEXT("[dEAthcURe|hmPickup_t::restore] !bDynamicSpawned"));
					//pPickup->bDynamicSpawned = TRUE; //? 20070208 test [-]
				}
			}

			pActor->eventOnHmRestore(); // 20070326
		}
		else {
			debugf(TEXT("[dEAthcURe] Spawning %s failed"), *className);
		}

		bUpdated = true;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool hmPickup_t::canCreate(UObject* pObject)
{
	AavaPickup* pPickup = Cast<AavaPickup>(pObject);
	if(pPickup) {
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmPickup_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmPickup_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmPickup_t::hmPickup_t(UObject* pObject, ULevel* pLevel) 
: hmData_t(pObject, pLevel) 
{
	backup(pObject, pLevel);	
}
// {{ [+] 20070503
//---------------------------------------------------------------------------
// hmPickupProvider_t for avaPickupProvider/TriggerVolume/Volume/Brush/Actor
//---------------------------------------------------------------------------
void hmPickupProvider_t::backup(UObject* pObject, ULevel* pLevel)
{
	__super::backup(pObject, pLevel);

	AavaPickupProvider* pPickupProvider = Cast<AavaPickupProvider>(pObject);
	if(pPickupProvider == 0x0) return;

	debugf(TEXT("[dEAthcURe] BACKUP AavaPickupProvider %s(hmIndex:%d) at (%f %f %f)"), *pPickupProvider->GetName(), pPickupProvider->hmIndex, pPickupProvider->Location.X, pPickupProvider->Location.Y, pPickupProvider->Location.Z);
	
	FMemoryWriter MemoryWriter( Bytes, TRUE );
	pPickupProvider->hmSerialize(MemoryWriter);

	className = pPickupProvider->GetClass()->GetPathName(); // [+] 20070228
	debugf(TEXT("[hmPickupProvider_t::backup] className=%s"), *className);

	if(pPickupProvider->InventoryClass) {
		inventoryClassName = pPickupProvider->InventoryClass->GetPathName(); // [+] 20070507
		debugf(TEXT("[hmPickupProvider_t::backup] inventoryClassName=%s"), *inventoryClassName);
	}
}
//---------------------------------------------------------------------------
bool hmPickupProvider_t::restore(UObject* pObject, ULevel* pLevel) 
{
	AavaPickupProvider* pPickupProvider = Cast<AavaPickupProvider>(pObject);
	if(pPickupProvider == 0x0) return false;

	// {{ [+] 20070507 host가 NukeCase를 갖고 나갔으면 avaPickupProvider를 복원하지 않는다.	
	if(g_hostMigration.bDoNotRestoreNukeCase && inventoryClassName == TEXT("avaRules.avaWeap_NukeCase")) {
		debugf(TEXT("[hmPickupProvider_t::restore] Restoring avaGame.avaPickupProvider for avaRules.avaWeap_NukeCase *canceled*"));
		return true;
	}	
	// }} [+] 20070507 host가 NukeCase를 갖고 나갔으면 avaPickupProvider를 복원하지 않는다.

	// {{ [!] 20070507 위치이동 type checing후 [+] 20070309 host가 c4를 갖고 나갔으면 pickup_c4를 복원하지 않는다.
	if(g_hostMigration.bDoNotRestoreC4 && className == TEXT("avaRules.avaPickUp_C4")) return true;	
	// }} [!] 20070507 위치이동 type checing후 [+] 20070309 host가 c4를 갖고 나갔으면 pickup_c4를 복원하지 않는다.

	debugf(TEXT("[hmPickupProvider_t::restore] className=%s"), *className);

	FMemoryReader MemoryReader( Bytes, TRUE );
	pPickupProvider->hmSerialize(MemoryReader);			

	pPickupProvider->eventOnHmRestore(); // 20070326
	
	debugf(TEXT("[dEAthcURe] RESTORE existing AavaPickupProvider %s at (%f %f %f)"), *pPickupProvider->GetName(), pPickupProvider->Location.X, pPickupProvider->Location.Y, pPickupProvider->Location.Z);

	bUpdated = true;
	return true;
}
//---------------------------------------------------------------------------
bool hmPickupProvider_t::canCreate(UObject* pObject)
{
	AavaPickupProvider* pPickupProvider = Cast<AavaPickupProvider>(pObject);
	if(pPickupProvider) {
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmPickupProvider_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmPickupProvider_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmPickupProvider_t::hmPickupProvider_t(UObject* pObject, ULevel* pLevel)
: hmData_t(pObject, pLevel) 
{
	backup(pObject, pLevel);
}
// }} [+] 20070503
/* 20070214 작업보류
//---------------------------------------------------------------------------
// hmPickupC4_t for AavaPickup_C4
//---------------------------------------------------------------------------
AActor* hmPickupC4_t::findOnLevel(ULevel* pLevel) 
{
	if(pLevel) {
		// {{ 20061205 index값이 valid하면 index를 가지고 찾을수 있3
		AActor* pActor = __super::findOnLevel(pLevel);
		if(pActor) {
			return pActor;
		}
		// }} 20061205 index값이 valid하면 index를 가지고 찾을수 있3

		int idxStart = 0;
		while(idxStart<pLevel->Actors.Num()) {
			AavaPickup* pPickup = Cast<AavaPickup>(pLevel->findActorByClassName(TEXT("avaPickup"), idxStart));
			if(pPickup==0x0) {
				break;
			}
			debugf(TEXT("%f %f %f - %f %f %f"), Location.X, Location.Y, Location.Z, pPickup->Location.X, pPickup->Location.Y, pPickup->Location.Z);
			debugf(TEXT("%s %s"), *className, *inventoryClassName);
			FVector diff = Location - pPickup->Location;
			if(diff.Size2D() < 10.0f) {
				return pPickup;
			}			
		}
	}
	return 0x0; // test for restore
}
//---------------------------------------------------------------------------
void hmPickupC4_t::backup(UObject* pObject, ULevel* pLevel) 
{
	__super::backup(pObject, pLevel);

	AavaPickup* pPickup = Cast<AavaPickup>(pObject);
	if(pPickup == 0x0) return;				

	debugf(TEXT("[dEAthcURe] BACKUP AavaPickup %s at (%f %f %f)"), *pPickup->GetName(), pPickup->Location.X, pPickup->Location.Y, pPickup->Location.Z);
	
	FMemoryWriter MemoryWriter( Bytes, TRUE );
	pPickup->hmSerialize(MemoryWriter);	

	// non-serialized data here	
	className = pPickup->GetClass()->GetPathName();
	debugf(TEXT("[hmPickupC4_t::backup] className=%s"), *className);
	inventoryClassName = pPickup->InventoryClass->GetPathName();
	debugf(TEXT("[hmPickupC4_t::backup] inventoryClassName=%s"), *inventoryClassName);
	//Location = pPickup->Location;
    //Rotation = pPickup->Rotation;
}
//---------------------------------------------------------------------------
bool hmPickupC4_t::restore(UObject* pObject, ULevel* pLevel) 
{
	AavaPickup* pPickup = Cast<AavaPickup>(pObject);
	if(pPickup == 0x0) return false;	

	FMemoryReader MemoryReader( Bytes, TRUE );
	pPickup->hmSerialize(MemoryReader);

	if(pPickup->bShutDown) {
		pPickup->eventHmShutdown();
	}

	if(pLevel) {
		// non-serialized data here		
	}			
	
	debugf(TEXT("[dEAthcURe] RESTORE check index %d %d"), index, pObject->GetNetIndex());
	debugf(TEXT("[dEAthcURe] RESTORE AavaPickup %s at (%f %f %f)"), *pPickup->GetName(), pPickup->Location.X, pPickup->Location.Y, pPickup->Location.Z);

	bUpdated = true;
	return true;
}
//---------------------------------------------------------------------------
bool hmPickupC4_t::restore(ULevel* pLevel) 
{	
	AavaPlayerController* pCtrl = Cast<AavaPlayerController>(GWorld->CurrentLevel->findActor(APlayerController::StaticClass()));
	if(pCtrl) {
		// {{ 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
		Location.Z += eypNonPlayer;
		// }} 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.

		AActor* pActor = pCtrl->eventHmSpawnActor(className, Location, Rotation);
		if(pActor) {
			FMemoryReader MemoryReader( Bytes, TRUE );
			pActor->hmSerialize(MemoryReader);
			
			AavaPickup* pPickup = Cast<AavaPickup>(pActor); assert(pPickup);
			BYTE* pti = &pPickup->TeamIdx;
			debugf(TEXT("TeamIdx=%d"), pPickup->TeamIdx);	
			pPickup->eventHmSetupPickup(inventoryClassName);
			
			debugf(TEXT("[dEAthcURe] RESTORE %s %s at (%f %f %f)"), *pActor->GetClass()->GetName(), *pActor->GetName(), pPickup->Location.X, pPickup->Location.Y, pPickup->Location.Z);

			if(pPickup->bDynamicSpawned) {
				debugf(TEXT("[dEAthcURe|hmPickupC4_t::restore] bDynamicSpawned"));
			}
			else {
				debugf(TEXT("[dEAthcURe|hmPickupC4_t::restore] !bDynamicSpawned"));
				//pPickup->bDynamicSpawned = TRUE; //? 20070208 test [-]
			}
		}
		else {
			debugf(TEXT("[dEAthcURe] Spawning %s failed"), *className);
		}

		bUpdated = true;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool hmPickupC4_t::canCreate(UObject* pObject)
{
	AavaPickup* pPickup = Cast<AavaPickup>(pObject);
	if(pPickup) {
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmPickupC4_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmPickupC4_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmPickupC4_t::hmPickupC4_t(UObject* pObject, ULevel* pLevel) 
: hmData_t(pObject, pLevel) 
{
	backup(pObject, pLevel);	
}
*/
//---------------------------------------------------------------------------
// hmKActor_t for AavaKActor/AKActor/ADynamicSMActor/AActor
//---------------------------------------------------------------------------
void hmKActor_t::backup(UObject* pObject, ULevel* pLevel)
{
	__super::backup(pObject, pLevel);

	AavaKActor* pKActor = Cast<AavaKActor>(pObject);
	if(pKActor == 0x0) return;

	debugf(TEXT("[dEAthcURe] BACKUP AavaKActor %s(hmIndex:%d) at (%f %f %f)"), *pKActor->GetName(), pKActor->hmIndex, pKActor->Location.X, pKActor->Location.Y, pKActor->Location.Z);
	
	FMemoryWriter MemoryWriter( Bytes, TRUE );
	pKActor->hmSerialize(MemoryWriter);

	className = pKActor->GetClass()->GetPathName(); // [+] 20070228
}
//---------------------------------------------------------------------------
bool hmKActor_t::restore(UObject* pObject, ULevel* pLevel) 
{
	AavaKActor* pKActor = Cast<AavaKActor>(pObject);
	if(pKActor == 0x0) return false;

	FMemoryReader MemoryReader( Bytes, TRUE );
	pKActor->hmSerialize(MemoryReader);		

	// {{ [+] 20070228
	if(pKActor->bShutDown) {
		pKActor->eventHmShutdown();
	}
	else {
		// {{ 20061117 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
		Location.Z += eyp;
		// }} 20061117 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
	}
	// }} [+] 20070228

	pKActor->eventOnHmRestore(); // 20070326
	
	debugf(TEXT("[dEAthcURe] RESTORE existing AavaKActor %s at (%f %f %f)"), *pKActor->GetName(), pKActor->Location.X, pKActor->Location.Y, pKActor->Location.Z);

	bUpdated = true;
	return true;
}
//---------------------------------------------------------------------------
bool hmKActor_t::canCreate(UObject* pObject)
{
	AavaKActor* pKActor = Cast<AavaKActor>(pObject);
	if(pKActor) {
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmKActor_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmKActor_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmKActor_t::hmKActor_t(UObject* pObject, ULevel* pLevel)
: hmData_t(pObject, pLevel) 
{
	backup(pObject, pLevel);
}
//---------------------------------------------------------------------------
// pKActorSpawnable for AKActorSpawnable/AKActor/ADynamicSMActor/AActor
//---------------------------------------------------------------------------
void hmKActorSpawnable_t::backup(UObject* pObject, ULevel* pLevel)
{
	__super::backup(pObject, pLevel);

	AKActorSpawnable* pKActorSpawnable = Cast<AKActorSpawnable>(pObject);
	if(pKActorSpawnable == 0x0) return;

	debugf(TEXT("[dEAthcURe] BACKUP AKActorSpawnable %s(hmIndex:%d) at (%f %f %f)"), *pKActorSpawnable->GetName(), pKActorSpawnable->hmIndex, pKActorSpawnable->Location.X, pKActorSpawnable->Location.Y, pKActorSpawnable->Location.Z);
	
	FMemoryWriter MemoryWriter( Bytes, TRUE );
	pKActorSpawnable->hmSerialize(MemoryWriter);

	className = pKActorSpawnable->GetClass()->GetPathName();
}
//---------------------------------------------------------------------------
bool hmKActorSpawnable_t::restore(UObject* pObject, ULevel* pLevel) 
{
	AKActorSpawnable* pKActorSpawnable = Cast<AKActorSpawnable>(pObject);
	if(pKActorSpawnable == 0x0) return false;

	FMemoryReader MemoryReader( Bytes, TRUE );
	pKActorSpawnable->hmSerialize(MemoryReader);

	pKActorSpawnable->eventOnHmRestore(); // 20070326
	
	debugf(TEXT("[dEAthcURe] RESTORE AKActorSpawnable %s at (%f %f %f)"), *pKActorSpawnable->GetName(), pKActorSpawnable->Location.X, pKActorSpawnable->Location.Y, pKActorSpawnable->Location.Z);

	bUpdated = true;
	return true;
}
//---------------------------------------------------------------------------
bool hmKActorSpawnable_t::restore(ULevel* pLevel) 
{
	// 20070228 작업보류,
	AavaPlayerController* pCtrl = Cast<AavaPlayerController>(GWorld->CurrentLevel->findActor(APlayerController::StaticClass()));
	if(pCtrl) {
		// {{ 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
		Location.Z += eypNonPlayer;
		// }} 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.

		AActor* pActor = pCtrl->eventHmSpawnActor(className, Location, Rotation);
		if(pActor) {
			/*
			UavaKActorFactory* pActorFactory = Cast<UavaKActorFactory>(UavaKActorFactory::StaticClass());
			if(pActorFactory) {
				AActor* pActor = pActorFactory->CreateActor(pActor->Location, pActor->Rotation, 0x0);
				if(pActor) {
					//pActor->
				}		
			}
			*/

			FMemoryReader MemoryReader( Bytes, TRUE );
			pActor->hmSerialize(MemoryReader);			

			pActor->eventOnHmRestore(); // 20070326
			
			debugf(TEXT("[dEAthcURe] RESTORE %s %s at (%f %f %f)"), *pActor->GetClass()->GetName(), *pActor->GetName(), pActor->Location.X, pActor->Location.Y, pActor->Location.Z);
		}
		else {
			debugf(TEXT("[dEAthcURe] Spawning %s failed"), *className);
		}

		bUpdated = true;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool hmKActorSpawnable_t::canCreate(UObject* pObject)
{
	AKActorSpawnable* pKActorSpawnable = Cast<AKActorSpawnable>(pObject);
	if(pKActorSpawnable) {
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmKActorSpawnable_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmKActorSpawnable_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmKActorSpawnable_t::hmKActorSpawnable_t(UObject* pObject, ULevel* pLevel)
: hmData_t(pObject, pLevel) 
{
	backup(pObject, pLevel);
}
//---------------------------------------------------------------------------
// hmKBreakable_t
//---------------------------------------------------------------------------
void hmKBreakable_t::backup(UObject* pObject, ULevel* pLevel)
{
	__super::backup(pObject, pLevel);

	AavaKBreakable* pKBreakable = Cast<AavaKBreakable>(pObject);
	if(pKBreakable == 0x0) return;

	debugf(TEXT("[dEAthcURe] BACKUP AavaKBreakable %s(hmIndex:%d) at (%f %f %f)"), *pKBreakable->GetName(), pKBreakable->hmIndex, pKBreakable->Location.X, pKBreakable->Location.Y, pKBreakable->Location.Z);
	
	FMemoryWriter MemoryWriter( Bytes, TRUE );
	pKBreakable->hmSerialize(MemoryWriter);		
}
//---------------------------------------------------------------------------
UObject* _test_pBreakable = 0x0;
bool hmKBreakable_t::restore(UObject* pObject, ULevel* pLevel) 
{
	AavaKBreakable* pKBreakable = Cast<AavaKBreakable>(pObject);
	if(pKBreakable == 0x0) return false;

	FMemoryReader MemoryReader( Bytes, TRUE );
	pKBreakable->hmSerialize(MemoryReader);

	if(pKBreakable->bShutDown) {
		_test_pBreakable = pObject; // test
		//GIsUTracing = TRUE;
		pKBreakable->eventHmShutdown();				
	}

	pKBreakable->eventOnHmRestore(); // 20070326
	
	debugf(TEXT("[dEAthcURe] RESTORE AavaKBreakable %s at (%f %f %f)"), *pKBreakable->GetName(), pKBreakable->Location.X, pKBreakable->Location.Y, pKBreakable->Location.Z);

	bUpdated = true;
	return true;
}
//---------------------------------------------------------------------------
bool hmKBreakable_t::canCreate(UObject* pObject)
{
	AavaKBreakable* pKBreakable = Cast<AavaKBreakable>(pObject);
	if(pKBreakable) {
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmKBreakable_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmKBreakable_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmKBreakable_t::hmKBreakable_t(UObject* pObject, ULevel* pLevel)
: hmData_t(pObject, pLevel) 
{
	backup(pObject, pLevel);
}
//---------------------------------------------------------------------------
// hmProjectile_t
//---------------------------------------------------------------------------
AActor* hmProjectile_t::findOnLevel(ULevel* pLevel) 
{
	if(pLevel) {
		int idxStart = 0;
		while(idxStart<pLevel->Actors.Num()) {
			AavaProjectile* pProjectile = Cast<AavaProjectile>(pLevel->findActorByClassName(TEXT("avaProjectile"), idxStart));
			if(pProjectile==0x0) {
				break;
			}
			debugf(TEXT("%f %f %f - %f %f %f"), Location.X, Location.Y, Location.Z, pProjectile->Location.X, pProjectile->Location.Y, pProjectile->Location.Z);			
			FVector diff = Location - pProjectile->Location;
			if(diff.Size() < 10.0f) {
				return pProjectile; // 아마 이걸로 걸리는건 없을듯
			}			
		}
	}
	return 0x0; // test for restore
}
//---------------------------------------------------------------------------
void hmProjectile_t::backup(UObject* pObject, ULevel* pLevel) 
{
	__super::backup(pObject, pLevel);

	AavaProjectile* pProjectile = Cast<AavaProjectile>(pObject);
	if(pProjectile == 0x0) return;				

	debugf(TEXT("[dEAthcURe] BACKUP AavaProjectile %s at (%f %f %f)"), *pProjectile->GetName(), pProjectile->Location.X, pProjectile->Location.Y, pProjectile->Location.Z);
	
	FMemoryWriter MemoryWriter( Bytes, TRUE );
	pProjectile->hmSerialize(MemoryWriter);	

	// non-serialized data here	
	className = pProjectile->GetClass()->GetPathName();
	debugf(TEXT("[hmProjectile_t::backup] className=%s"), *className);		
	//Location = pProjectile->Location;
    //Rotation = pProjectile->Rotation;
}
//---------------------------------------------------------------------------
bool hmProjectile_t::restore(UObject* pObject, ULevel* pLevel) 
{
	AavaProjectile* pProjectile = Cast<AavaProjectile>(pObject);
	if(pProjectile == 0x0) return false;	

	FMemoryReader MemoryReader( Bytes, TRUE );
	pProjectile->hmSerialize(MemoryReader);	

	if(pLevel) {
		// non-serialized data here		
	}

	pProjectile->eventOnHmRestore(); // 20070326
	
	debugf(TEXT("[dEAthcURe] RESTORE AavaProjectile %s at (%f %f %f)"), *pProjectile->GetName(), pProjectile->Location.X, pProjectile->Location.Y, pProjectile->Location.Z);

	bUpdated = true;
	return true;
}
//---------------------------------------------------------------------------
bool hmProjectile_t::restore(ULevel* pLevel) 
{	
	AavaPlayerController* pCtrl = Cast<AavaPlayerController>(GWorld->CurrentLevel->findActor(APlayerController::StaticClass()));
	if(pCtrl) {
		// {{ 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
		Location.Z += eypNonPlayer;
		// }} 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.

		AActor* pActor = pCtrl->eventHmSpawnActor(className, Location, Rotation);
		if(pActor) {
			FMemoryReader MemoryReader( Bytes, TRUE );
			pActor->hmSerialize(MemoryReader);		

			pActor->eventOnHmRestore(); // 20070326
			
			debugf(TEXT("[dEAthcURe] RESTORE %s %s at (%f %f %f)"), *pActor->GetClass()->GetName(), *pActor->GetName(), pActor->Location.X, pActor->Location.Y, pActor->Location.Z);
		}
		else {
			debugf(TEXT("[dEAthcURe] Spawning %s failed"), *className);
		}

		bUpdated = true;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool hmProjectile_t::canCreate(UObject* pObject)
{
	AavaProjectile* pProjectile = Cast<AavaProjectile>(pObject);
	if(pProjectile) {
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmProjectile_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmProjectile_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmProjectile_t::hmProjectile_t(UObject* pObject, ULevel* pLevel) 
: hmData_t(pObject, pLevel) 
{
	backup(pObject, pLevel);
}
//---------------------------------------------------------------------------
// hmKProjectile_t
//---------------------------------------------------------------------------
AActor* hmKProjectile_t::findOnLevel(ULevel* pLevel) 
{
	if(pLevel) {
		int idxStart = 0;
		while(idxStart<pLevel->Actors.Num()) {
			AavaKProjectile* pKProjectile = Cast<AavaKProjectile>(pLevel->findActorByClassName(TEXT("AavaKProjectile"), idxStart));
			if(pKProjectile==0x0) {
				break;
			}
			debugf(TEXT("%f %f %f - %f %f %f"), Location.X, Location.Y, Location.Z, pKProjectile->Location.X, pKProjectile->Location.Y, pKProjectile->Location.Z);			
			FVector diff = Location - pKProjectile->Location;
			if(diff.Size() < 10.0f) {
				return pKProjectile; // 아마 이걸로 걸리는건 없을듯
			}			
		}
	}
	return 0x0; // test for restore
}
//---------------------------------------------------------------------------
void hmKProjectile_t::backup(UObject* pObject, ULevel* pLevel) 
{
	__super::backup(pObject, pLevel);

	AavaKProjectile* pKProjectile = Cast<AavaKProjectile>(pObject);
	if(pKProjectile == 0x0) return;				

	debugf(TEXT("[dEAthcURe] BACKUP AavaKProjectile %s at (%f %f %f)"), *pKProjectile->GetName(), pKProjectile->Location.X, pKProjectile->Location.Y, pKProjectile->Location.Z);
	
	FMemoryWriter MemoryWriter( Bytes, TRUE );
	pKProjectile->hmSerialize(MemoryWriter);	

	// non-serialized data here	
	className = pKProjectile->GetClass()->GetPathName();
	debugf(TEXT("[hmProjectile_t::backup] className=%s"), *className);		
	//Location = pKProjectile->Location;
    //Rotation = pKProjectile->Rotation;
}
//---------------------------------------------------------------------------
bool hmKProjectile_t::restore(UObject* pObject, ULevel* pLevel) 
{
	AavaKProjectile* pKProjectile = Cast<AavaKProjectile>(pObject);
	if(pKProjectile == 0x0) return false;	

	FMemoryReader MemoryReader( Bytes, TRUE );
	pKProjectile->hmSerialize(MemoryReader);	

	if(pLevel) {
		// non-serialized data here		
	}

	pKProjectile->eventOnHmRestore(); // 20070326
	
	debugf(TEXT("[dEAthcURe] RESTORE AavaProjectile %s at (%f %f %f)"), *pKProjectile->GetName(), pKProjectile->Location.X, pKProjectile->Location.Y, pKProjectile->Location.Z);

	bUpdated = true;
	return true;
}
//---------------------------------------------------------------------------
bool hmKProjectile_t::restore(ULevel* pLevel) 
{	
	AavaPlayerController* pCtrl = Cast<AavaPlayerController>(GWorld->CurrentLevel->findActor(APlayerController::StaticClass()));
	if(pCtrl) {
		// {{ 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
		Location.Z += eypNonPlayer;
		// }} 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.

		AActor* pActor = pCtrl->eventHmSpawnActor(className, Location, Rotation);
		if(pActor) {
			FMemoryReader MemoryReader( Bytes, TRUE );
			pActor->hmSerialize(MemoryReader);		

			pActor->eventOnHmRestore(); // 20070326
			
			debugf(TEXT("[dEAthcURe] RESTORE %s %s at (%f %f %f)"), *pActor->GetClass()->GetName(), *pActor->GetName(), pActor->Location.X, pActor->Location.Y, pActor->Location.Z);
		}
		else {
			debugf(TEXT("[dEAthcURe] Spawning %s failed"), *className);
		}

		bUpdated = true;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool hmKProjectile_t::canCreate(UObject* pObject)
{
	AavaKProjectile* pKProjectile = Cast<AavaKProjectile>(pObject);
	if(pKProjectile) {
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmKProjectile_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmKProjectile_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmKProjectile_t::hmKProjectile_t(UObject* pObject, ULevel* pLevel, bool bBackup) 
: hmData_t(pObject, pLevel) 
{
	if(bBackup) {
		backup(pObject, pLevel);
	}
}
//---------------------------------------------------------------------------
// hmProj_C4_t
//---------------------------------------------------------------------------
AActor* hmProj_C4_t::findOnLevel(ULevel* pLevel) 
{
	if(pLevel) {
		int idxStart = 0;
		while(idxStart<pLevel->Actors.Num()) {
			AavaProj_C4* pC4 = Cast<AavaProj_C4>(pLevel->findActorByClassName(TEXT("avaProj_C4"), idxStart));
			if(pC4==0x0) {
				break;
			}
			debugf(TEXT("%f %f %f - %f %f %f"), Location.X, Location.Y, Location.Z, pC4->Location.X, pC4->Location.Y, pC4->Location.Z);			
			FVector diff = Location - pC4->Location;
			if(diff.Size() < 10.0f) {
				return pC4; // 아마 이걸로 걸리는건 없을듯
			}			
		}
	}
	return 0x0; // test for restore
}
//---------------------------------------------------------------------------
void hmProj_C4_t::backup(UObject* pObject, ULevel* pLevel) 
{
	AavaProj_C4* pC4 = Cast<AavaProj_C4>(pObject);
	if(pC4 == 0x0) return;				

	__super::backup(pObject, pLevel);

	debugf(TEXT("[dEAthcURe] BACKUP AavaProj_C4 %s at (%f %f %f)"), *pC4->GetName(), pC4->Location.X, pC4->Location.Y, pC4->Location.Z);
}
//---------------------------------------------------------------------------
bool hmProj_C4_t::restore(ULevel* pLevel) 
{	
	AavaPlayerController* pCtrl = Cast<AavaPlayerController>(GWorld->CurrentLevel->findActor(APlayerController::StaticClass()));
	if(pCtrl) {
		// {{ 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
		Location.Z += eypNonPlayer;
		// }} 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.

		AActor* pActor = pCtrl->eventHmSpawnActor(className, Location, Rotation);
		if(pActor) {
			debugf(TEXT("[dEAthcURe] Spawning %s -> 0x%x"), *className, pActor);

			FMemoryReader MemoryReader( Bytes, TRUE );
			pActor->hmSerialize(MemoryReader);		

			pActor->eventOnHmRestore(); // 20070326
			
			debugf(TEXT("[dEAthcURe] RESTORE %s %s at (%f %f %f)"), *pActor->GetClass()->GetName(), *pActor->GetName(), pActor->Location.X, pActor->Location.Y, pActor->Location.Z);		
		}
		else {
			debugf(TEXT("[dEAthcURe] Spawning %s failed"), *className);
		}

		bUpdated = true;
		if(bSetupPended) { // [20070302] pri를 찾기위해 pending한다.
			bTickable = true;			
			return false;
		}
	}
	return false;
}
//---------------------------------------------------------------------------
bool hmProj_C4_t::tick(void)
{
	int idxStart = 0;
	AActor* pActor = GWorld->CurrentLevel->findActorByClassName(TEXT("avaProj_C4"), idxStart);	
	if(pProj_C4 && pActor != pProj_C4) {
		bSetupPended = false;
		return true;
	}

	idxStart = 0;
	AavaPlayerReplicationInfo * pPRI = 0x0;
	do {
		pPRI = Cast<AavaPlayerReplicationInfo>(GWorld->CurrentLevel->findActorByClassName(TEXT("avaPlayerReplicationInfoEx"), idxStart));
		if(pPRI) {			
			if(pPRI->PlayerName == playerNameSetted) {
				AavaProj_C4* pC4 = (AavaProj_C4*)pProj_C4;
				if(pC4) {
					pC4->SettedPRI = pPRI;
					pC4->InstigatorController = Cast<AController>(pPRI->Owner); // [+] 20070907 폭설자가 kill score먹도록
					debugf(TEXT("Setting instigators Setter %s Instigator %s"), *pPRI->GetName(), pC4->InstigatorController ? *pC4->InstigatorController->GetName() : TEXT("unknown"));
				} // debug else
				bSetupPended = false;
				return true;
			}
		}		
	} while(pPRI);
	return false;
}
//---------------------------------------------------------------------------
bool hmProj_C4_t::canCreate(UObject* pObject)
{
	AavaProj_C4* pC4 = Cast<AavaProj_C4>(pObject);
	if(pC4) {
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmProj_C4_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmProj_C4_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmProj_C4_t::hmProj_C4_t(UObject* pObject, ULevel* pLevel) 
: hmKProjectile_t(pObject, pLevel, false) 
{
	pProj_C4 = 0x0;
	playerNameSetted = "";
	bSetupSettedPlayerPended = false;
	backup(pObject, pLevel);	
}
//---------------------------------------------------------------------------
// hmGameInfo_t
//---------------------------------------------------------------------------
AActor* hmGameInfo_t::findOnLevel(ULevel* pLevel) 
{
	if(pLevel) {
		for(int idx=0;idx<pLevel->Actors.Num();idx++) {
			AActor* pActor = pLevel->Actors(idx);				
			if(pActor && pActor->IsA(AGameReplicationInfo::StaticClass())) {				
				return pActor;
			}
		}
	}
	return 0x0;
}
//---------------------------------------------------------------------------
void hmGameInfo_t::backup(UObject* pObject, ULevel* pLevel) 
{
	__super::backup(pObject, pLevel);
	AavaGameReplicationInfo* pGameReplicationInfo = Cast<AavaGameReplicationInfo>(pObject);
	if(pGameReplicationInfo == 0x0) return;				

	debugf(TEXT("[dEAthcURe] BACKUP AavaGameReplicationInfo %s"), *pGameReplicationInfo->GetName());
	
	FMemoryWriter MemoryWriter( Bytes, TRUE );
	pGameReplicationInfo->hmSerialize(MemoryWriter);

	// {{ 20070306
	if(pGameReplicationInfo->bWarmupRound) {
		g_hostMigration.bWarmupRound = true;
	}
	// }} 20070306

	for(int lpp=0;lpp<2;lpp++) {
		int bBackupTeam;
		if(pGameReplicationInfo->Teams[lpp]) {
			bBackupTeam = 1;
			MemoryWriter << bBackupTeam;
			pGameReplicationInfo->Teams[lpp]->hmSerialize(MemoryWriter);
		}
		else {
			bBackupTeam = 0;
			MemoryWriter << bBackupTeam;
			debugf(TEXT("[dEAthcURe|hmGameInfo_t::backup] Invalid team info %d"), lpp);
		}
	}

	g_hostMigration.RemainingTime = pGameReplicationInfo->RemainingTime;
	g_hostMigration.ElapsedTime = pGameReplicationInfo->ElapsedTime;
	debugf(TEXT("[dEAthcURe|hmGameInfo_t::backup] RemainingTime=%d ElapsedTime=%d"), g_hostMigration.RemainingTime, g_hostMigration.ElapsedTime);
}
//---------------------------------------------------------------------------
bool hmGameInfo_t::restore(UObject* pObject, ULevel* pLevel) 
{
	AavaGameReplicationInfo* pGRI = Cast<AavaGameReplicationInfo>(pObject);
	if(pGRI == 0x0) return false;

	debugf(TEXT("[dEAthcURe] RESTORE AavaGameReplicationInfo %s"), *pGRI->GetName());

	FMemoryReader MemoryReader( Bytes, TRUE );
	pGRI->hmSerialize(MemoryReader);	

	for(int lpp=0;lpp<2;lpp++) {
		int bBackupTeam;
		MemoryReader << bBackupTeam;
		if(bBackupTeam && pGRI->Teams[lpp]) {
			pGRI->Teams[lpp]->hmSerialize(MemoryReader);
			pGRI->Teams[lpp]->eventOnHmRestore(); // 20070326
		}
		else {
			debugf(TEXT("[dEAthcURe|hmGameInfo_t::restore] Invalid team info %d"), lpp);
		}		
	}

	if(pLevel) {
		pGameInfo = Cast<AavaGame>(pLevel->findActor(AavaGame::StaticClass()));
		if(pGameInfo) {
			if(!g_hostMigration.bWarmupRound) { // [!] 20070306 웜업라운드일때는 시간을 복원하지 않는다.
				pGameInfo->RemainingTime = g_hostMigration.RemainingTime;
				pGameInfo->ElapsedTime = g_hostMigration.ElapsedTime;
				debugf(TEXT("[hmGameInfo_t::restore] RemainingTime=%d ElapsedTime=%d"), pGameInfo->RemainingTime, pGameInfo->ElapsedTime);
			}
			
			debugf(TEXT("[hmGameInfo_t::restore] bHmRoundEnd=%d, nWinTeam=%d"), pGRI->bHmRoundEnd, pGRI->nWinTeam);
			if(pGRI->bHmRoundEnd || pGRI->nWinTeam == 0 || pGRI->nWinTeam == 1) { 
				// [!] 20070303 조건수정
				bTickable = true;
				timeEndRound = timeGetTime() + 10000; // 20070305 시간수정
			}
		}			
	}

	pGRI->eventOnHmRestore(); // 20070326

	// {{ 20070524 dEAthcURe|HM
	if(g_hostMigration.pMissionObject) {
		g_hostMigration.pMissionObject->restore(pLevel);
	}
	// }} 20070524 dEAthcURe|HM

	g_hostMigration.bReelectLeaderByTime = true; // 20071024

	g_hostMigration.timeGameInfoRestored = timeGetTime(); // [+] 20070308 TriggerAllOutEvent용
	debugf(TEXT("GameInfo restored at %d"), g_hostMigration.timeGameInfoRestored);
	bUpdated = true;
	return true;
}
//---------------------------------------------------------------------------
bool hmGameInfo_t::tick(void)
{
	if(timeGetTime() > timeEndRound) {
		debugf(TEXT("[hmGameInfo_t::restore] before calling pGameInfo->eventHmEndRound()"));
		pGameInfo->eventHmEndRound();
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool hmGameInfo_t::canCreate(UObject* pObject)
{
	AavaGameReplicationInfo* pGameReplicationInfo = Cast<AavaGameReplicationInfo>(pObject);
	if(pGameReplicationInfo) {
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmGameInfo_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmGameInfo_t(pObject, pLevel);	
}
//---------------------------------------------------------------------------
bool hmGameInfo_t::canRestore(phostMigration_t pSender, ULevel* pLevel)
{
	if(pSender->bReadyToRestoreGRI) {
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
void hmGameInfo_t::onDestroy(phostMigration_t pSender)
{
	pSender->nLocals--;
}
//---------------------------------------------------------------------------
hmGameInfo_t::hmGameInfo_t(UObject* pObject, ULevel* pLevel) 
: hmData_t(pObject, pLevel) 
{
	timeInterval = 0;

	backup(pObject, pLevel);

	AavaGameReplicationInfo* pGameReplicationInfo = Cast<AavaGameReplicationInfo>(pObject);
	if(pGameReplicationInfo) {
		name = pGameReplicationInfo->GetName();
	}	
	pGameInfo = 0x0;
}
// {{ [+] 20070419
//---------------------------------------------------------------------------
// hmKismetState_t for AavaKismetState/AActor
//---------------------------------------------------------------------------
void hmKismetState_t::backup(UObject* pObject, ULevel* pLevel)
{
	__super::backup(pObject, pLevel);

	AavaKismetState* pKismetState = Cast<AavaKismetState>(pObject);
	if(pKismetState == 0x0) return;

	debugf(TEXT("[dEAthcURe] BACKUP AavaKismetState %s(hmIndex:%d)"), *pKismetState->GetName(), pKismetState->hmIndex);
	
	FMemoryWriter MemoryWriter( Bytes, TRUE );
	pKismetState->hmSerialize(MemoryWriter);
}
//---------------------------------------------------------------------------
bool hmKismetState_t::restore(UObject* pObject, ULevel* pLevel) 
{
	AavaKismetState* pKismetState = Cast<AavaKismetState>(pObject);
	if(pKismetState == 0x0) return false;

	FMemoryReader MemoryReader( Bytes, TRUE );
	pKismetState->hmSerialize(MemoryReader);

	pKismetState->eventOnHmRestore(); // 20070326
	
	debugf(TEXT("[dEAthcURe] RESTORE existing AavaKismetState %s"), *pKismetState->GetName());

	bUpdated = true;
	return true;
}
//---------------------------------------------------------------------------
bool hmKismetState_t::canCreate(UObject* pObject)
{
	AavaKismetState* pKismetState = Cast<AavaKismetState>(pObject);
	if(pKismetState) {
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmKismetState_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmKismetState_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmKismetState_t::hmKismetState_t(UObject* pObject, ULevel* pLevel)
: hmData_t(pObject, pLevel) 
{
	backup(pObject, pLevel);
}
//---------------------------------------------------------------------------
// }} [+] 20070419
// {{ [+] 20070516
//---------------------------------------------------------------------------
// hmShatterGlassActor_t for AavaShatterGlassActor/AActor
//---------------------------------------------------------------------------
void hmShatterGlassActor_t::backup(UObject* pObject, ULevel* pLevel)
{
	__super::backup(pObject, pLevel);

	AavaShatterGlassActor* pShatterGlassActor = Cast<AavaShatterGlassActor>(pObject);
	if(pShatterGlassActor == 0x0) return;

	debugf(TEXT("[dEAthcURe] BACKUP AavaShatterGlassActor %s(hmIndex:%d)"), *pShatterGlassActor->GetName(), pShatterGlassActor->hmIndex);
	
	FMemoryWriter MemoryWriter( Bytes, TRUE );
	pShatterGlassActor->hmSerialize(MemoryWriter);
}
//---------------------------------------------------------------------------
bool hmShatterGlassActor_t::restore(UObject* pObject, ULevel* pLevel) 
{
	AavaShatterGlassActor* pShatterGlassActor = Cast<AavaShatterGlassActor>(pObject);
	if(pShatterGlassActor == 0x0) return false;

	FMemoryReader MemoryReader( Bytes, TRUE );
	pShatterGlassActor->hmSerialize(MemoryReader);

	pShatterGlassActor->eventOnHmRestore(); // 20070326
	
	debugf(TEXT("[dEAthcURe] RESTORE existing AavaShatterGlassActor %s"), *pShatterGlassActor->GetName());

	bUpdated = true;
	return true;
}
//---------------------------------------------------------------------------
bool hmShatterGlassActor_t::canCreate(UObject* pObject)
{
	AavaShatterGlassActor* pShatterGlassActor = Cast<AavaShatterGlassActor>(pObject);
	if(pShatterGlassActor) {
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmShatterGlassActor_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmShatterGlassActor_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmShatterGlassActor_t::hmShatterGlassActor_t(UObject* pObject, ULevel* pLevel)
: hmData_t(pObject, pLevel) 
{
	backup(pObject, pLevel);
}
//---------------------------------------------------------------------------
// {{ [+] 20070723
//---------------------------------------------------------------------------
// hmMatineeActor_t for AMatineeActor/AActor
//---------------------------------------------------------------------------
AActor* hmMatineeActor_t::findOnLevel(ULevel* pLevel) 
{			
	for(int idxActor=0;idxActor<pLevel->Actors.Num();idxActor++) {
		AActor* pActor = pLevel->Actors(idxActor);
		if(pActor) {
			AMatineeActor* pMatineeActor = Cast<AMatineeActor>(pActor);
			if(pMatineeActor && pMatineeActor->InterpAction && pMatineeActor->InterpAction->bHostMigration && pMatineeActor->InterpAction->hmId == hmId) {
				debugf(TEXT("[dEAthcURe] findOnLevel AMatineeActor hmId %d (expecting %d)"), pMatineeActor->InterpAction->hmId, hmId);
				return pActor;
			}			
		}		
	}
	if(!bMatineeHmTriggered) {
		AWorldInfo* pWorldInfo = GWorld->GetWorldInfo();
		if(pWorldInfo) {
			USequence* pGameSequence = pWorldInfo->GetGameSequence();
			if(pGameSequence) {
				USequence* pRootSequence = pGameSequence->GetRootSequence();
				if(pRootSequence) {
					TArray<USeqAct_Interp*> Interps;
					pRootSequence->FindSeqObjectsByClass(USeqAct_Interp::StaticClass(), (TArray<USequenceObject*>&)Interps);
					
					for ( INT EventIndex = 0; EventIndex < Interps.Num(); EventIndex++ ) {
						USeqAct_Interp* pInterp = Interps(EventIndex);
						if(pInterp) {
							if(pInterp->bHostMigration) {
								if(pInterp->hmId == hmId) {
									pInterp->InputLinks(0).bHasImpulse = TRUE;
									INT aIdx = pGameSequence->DelayedActivatedOps.AddZeroed();
									pGameSequence->DelayedActivatedOps(aIdx).Op = pInterp;
									bMatineeHmTriggered = true;
									return 0x0;
								}
							}
						}
					}
				}
			}
		}		
	}
	return 0x0;
}
//---------------------------------------------------------------------------
void hmMatineeActor_t::backup(UObject* pObject, ULevel* pLevel)
{
	__super::backup(pObject, pLevel);

	AMatineeActor* pMatineeActor = Cast<AMatineeActor>(pObject);
	if(pMatineeActor == 0x0) return;

	if(pMatineeActor && pMatineeActor->InterpAction) {
		hmId = pMatineeActor->InterpAction->hmId;
	}

	debugf(TEXT("[dEAthcURe] BACKUP AMatineeActor %s(hmId:%d)"), *pMatineeActor->GetName(), hmId);
	
	FMemoryWriter MemoryWriter( Bytes, TRUE );
	pMatineeActor->hmSerialize(MemoryWriter);

	g_hostMigration.nPlayerBaseToRestore++; // 20071012 base relative 복원하려면 base부터 복원해야한다.
}
//---------------------------------------------------------------------------
bool hmMatineeActor_t::restore(UObject* pObject, ULevel* pLevel) 
{	
	AMatineeActor* pMatineeActor = Cast<AMatineeActor>(pObject);
	if(pMatineeActor == 0x0) return false;

	FMemoryReader MemoryReader( Bytes, TRUE );
	pMatineeActor->hmSerialize(MemoryReader);

	pMatineeActor->eventOnHmRestore(); // 20070326	
	
	debugf(TEXT("[dEAthcURe] RESTORE existing AMatineeActor %s"), *pMatineeActor->GetName());
	g_hostMigration.nPlayerBaseToRestore = g_hostMigration.nPlayerBaseToRestore > 0 ? g_hostMigration.nPlayerBaseToRestore-1 : 0; // 20071012 base relative 복원하려면 base부터 복원해야한다.

	bUpdated = true;
	return true;
}
//---------------------------------------------------------------------------
bool hmMatineeActor_t::canCreate(UObject* pObject)
{
	AMatineeActor* pMatineeActor = Cast<AMatineeActor>(pObject);
	if(pMatineeActor && pMatineeActor->InterpAction && pMatineeActor->InterpAction->bHostMigration) {
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmMatineeActor_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmMatineeActor_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmMatineeActor_t::hmMatineeActor_t(UObject* pObject, ULevel* pLevel)
: hmData_t(pObject, pLevel) 
{
	hmId = -1;
	bMatineeHmTriggered = false;
	backup(pObject, pLevel);
}
// }} 20070723
// {{ 20070801
//---------------------------------------------------------------------------
// hmVehicle_t for AavaVehicle/AavaVehicleBase/ASVehicle/AVehicle/APawn/AActor
//---------------------------------------------------------------------------
AActor* hmVehicle_t::findOnLevel(ULevel* pLevel) 
{	
	for(int idxActor=0;idxActor<pLevel->Actors.Num();idxActor++) {
		AActor* pActor = pLevel->Actors(idxActor);
		if(pActor) {
			AavaVehicle* pVehicle = Cast<AavaVehicle>(pActor);
			if(pVehicle && pVehicle->IsValid()) {
				debugf(TEXT("[hmVehicle_t::findOnLevel] %d %d"), pVehicle->hmId, hmId);
				if(pVehicle->hmId == hmId) {
					return pActor;
				}
			}			
		}
	}

	/*
	if(!bVehicleFactoryEventTriggered) {
		AWorldInfo* pWorldInfo = GWorld->GetWorldInfo();
		if(pWorldInfo) {
			USequence* pGameSequence = pWorldInfo->GetGameSequence();
			if(pGameSequence) {
				USequence* pRootSequence = pGameSequence->GetRootSequence();
				if(pRootSequence) {
					TArray<UavaSeqEvent_VehicleFactory*> VehicleFactories;
					pRootSequence->FindSeqObjectsByClass(UavaSeqEvent_VehicleFactory::StaticClass(), (TArray<USequenceObject*>&)VehicleFactories);
					
					for ( INT EventIndex = 0; EventIndex < VehicleFactories.Num(); EventIndex++ ) {
						UavaSeqEvent_VehicleFactory* pVehicleFactory = VehicleFactories(EventIndex);
						if(pVehicleFactory) {							
							if(pVehicleFactory->hmId == hmId) {
								pVehicleFactory->eventActivated();								
								bVehicleFactoryEventTriggered = true;
								return 0x0;
							}							
						}
					}
				}
			}
		}		
	}
	*/
	
	return 0x0;
}
//---------------------------------------------------------------------------
void hmVehicle_t::backup(UObject* pObject, ULevel* pLevel)
{
	__super::backup(pObject, pLevel);

	AavaVehicle* pVehicle = Cast<AavaVehicle>(pObject);
	if(pVehicle == 0x0) return;

	hmId = pVehicle->hmId;
	debugf(TEXT("[dEAthcURe] BACKUP AavaVehicle %s(hmId:%d)"), *pVehicle->GetName(), pVehicle->hmId);
	
	FMemoryWriter MemoryWriter( Bytes, TRUE );
	pVehicle->hmSerialize(MemoryWriter);

	APawn* pPawn = 0x0;
	if(pVehicle->Seats.Num()>=2 && pVehicle->Seats(1).SeatPawn) {
		pPawn = Cast<APawn>(pVehicle->Seats(1).SeatPawn);
	}
	if(pPawn) {
		FMemoryWriter MemoryWriter( WeaponBytes, TRUE );		
		// {{ 20070905 player에서 가져옴

		// pPawn을 instigator로 가지고 있는 inventory를 찾아서 serialize한다.
		//// 먼저 갯수부터.
		int idx = 0;
		int nInventory = 0;
		for(idx=0;idx<pLevel->Actors.Num();idx++) {
			AActor* pActor = pLevel->Actors(idx);
			if(pActor) {
				AavaWeapon* pWeapon = Cast<AavaWeapon>(pActor);
				if(pWeapon && pWeapon->Instigator == pPawn) {
					if(!(Cast<AavaThrowableWeapon>(pWeapon) && pWeapon->AmmoCount == 0)) { // [+] 20070309 던지는 무기 개수가 0이면 백업 안함
						FString className = pWeapon->GetClass()->GetPathName();
						if(className.Len()) {
							nInventory++;
						}
					}
				}
			}
		}
		MemoryWriter << nInventory;
		debugf(TEXT("BACKUP nInv = %d"), nInventory);
		
		//// 다음 AavaWeapon
		for(idx=0;idx<pLevel->Actors.Num();idx++) {
			AActor* pActor = pLevel->Actors(idx);
			if(pActor) {
				AavaWeapon* pWeapon = Cast<AavaWeapon>(pActor);				
				if(pWeapon && pWeapon->Instigator == pPawn) {
					if(!(Cast<AavaThrowableWeapon>(pWeapon) && pWeapon->AmmoCount == 0)) { // [+] 20070309 던지는 무기 개수가 0이면 백업 안함
						FString className = pWeapon->GetClass()->GetPathName();
						if(className.Len()) {
							MemoryWriter << className;
							debugf(TEXT("[dEAthcURe|hmVehicle_t::backup] BACKUP AavaWeapon %s (%s) : %d"), *pWeapon->GetName(), *className, pWeapon->AmmoCount);
							pWeapon->eventOnHmBackup(); // [+] 20070411
							pWeapon->hmSerialize(MemoryWriter); // [+] 20070411

							// {{ modifier 탐색
							for(int lpp=0;lpp<10;lpp++) { // 10개로 h에 박혀있다
								UavaModifier* pModifier = Cast<UavaModifier>(pWeapon->WeaponModifiers[lpp]);						
								if(pModifier) {							
									debugf(TEXT("UavaModifier id=%d"), pModifier->Id);
									for(int lpi=0;lpi<pModifier->CommonAttachedItems.Num();lpi++) {
										struct FAttachedItem attachedItem = pModifier->CommonAttachedItems(lpi);
										debugf(TEXT("[modifier:%d] MeshName=%s, psock=%s, ssock=%s, vdist:%f"), lpi,									
											*attachedItem.MeshName, 
											attachedItem.PrimarySocket.GetName(), attachedItem.SecondarySocket.GetName(),									
											attachedItem.MaxVisibleDistance);
									}							
								}								
							}
							// }} modifier 탐색				
							hmPlayer_t::backupWeaponModifier(MemoryWriter, pWeapon); // [+] 20070212
						}
					} // [+] 20070309 던지는 무기 개수가 0이면 백업 안함
				}
			}
		}
		// }} 20070905 player에서 가져옴

		g_hostMigration.nPlayerBaseToRestore++; // 20071012 base relative 복원하려면 base부터 복원해야한다.
	}	
}
//---------------------------------------------------------------------------
bool hmVehicle_t::restore(UObject* pObject, ULevel* pLevel) 
{
	AavaVehicle* pVehicle = Cast<AavaVehicle>(pObject);
	if(pVehicle == 0x0) return false;

	FMemoryReader MemoryReader( Bytes, TRUE );
	pVehicle->hmSerialize(MemoryReader);

	APawn* pPawn = 0x0;
	if(pVehicle->Seats.Num()>=2 && pVehicle->Seats(1).SeatPawn) {
		pPawn = Cast<APawn>(pVehicle->Seats(1).SeatPawn);
	}
	if(pPawn) {

		// {{ 20070907 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
		pPawn->Location.Z += eyp;
		// }} 20070907약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.

		FMemoryReader MemoryReader( WeaponBytes, TRUE );		

		// {{ 20070905 player에서 가져옴
		int nInv = -1;
		MemoryReader << nInv;		
		debugf(TEXT("RESTORE nInv = %d"), nInv);
		
		bool bSetCurrentWeapon = false; // [+] 20070309
		for(int lpp=0;lpp<nInv;lpp++) {
			FString className;
			MemoryReader << className;		

			for(int idx=0;idx<pLevel->Actors.Num();idx++) {
				AActor* pActor = pLevel->Actors(idx);
				if(pActor) {
					AavaWeapon* pWeapon = Cast<AavaWeapon>(pActor);
					if(pWeapon && pWeapon->Instigator == pPawn && pWeapon->GetClass()->GetPathName() == className) {
						pWeapon->hmSerialize(MemoryReader); // [+] 20070411
						pWeapon->eventOnHmRestore(); // [+] 20070411
						hmPlayer_t::restoreWeaponModifier(MemoryReader, pWeapon, pPawn); // [+] 20070212 dEAthcURe
						debugf(TEXT("[dEAthcURe::hmVehicle_t] RESTORE AInventory:%d %s ammoCount=%d"), lpp, *className, pWeapon->AmmoCount);
						break;
					}
				}
			}			
		}		
		// }} 20070905 player에서 가져옴
	}

	pVehicle->eventOnHmRestore(); // 20070326
	
	debugf(TEXT("[dEAthcURe] RESTORE existing AavaVehicle %s"), *pVehicle->GetName());

	g_hostMigration.nPlayerBaseToRestore = g_hostMigration.nPlayerBaseToRestore > 0 ? g_hostMigration.nPlayerBaseToRestore-1 : 0; // 20071012 base relative 복원하려면 base부터 복원해야한다.

	g_hostMigration.bReadyToRestoreGRI = true; // 20071221 dEAthcURe|HM 탱크가 복구되도 GRI복구하도록

	bUpdated = true;
	return true;
}
//---------------------------------------------------------------------------
bool hmVehicle_t::canCreate(UObject* pObject)
{
	AavaVehicle* pVehicle = Cast<AavaVehicle>(pObject);
	if(pVehicle) {
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
phmData_t hmVehicle_t::create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel)
{
	pSender->nLocals++;
	return new hmVehicle_t(pObject, pLevel);
}
//---------------------------------------------------------------------------
hmVehicle_t::hmVehicle_t(UObject* pObject, ULevel* pLevel)
: hmData_t(pObject, pLevel) 
{
	hmId = -1;
	timeInterval = 0; // 매턴 복구
	bVehicleFactoryEventTriggered = false;
	backup(pObject, pLevel);
}
// }} 20070801
//---------------------------------------------------------------------------
// hmDroppingPickup_t
//---------------------------------------------------------------------------
bool hmDroppingPickup_t::backup(AActor* pActor, FVector Location, FRotator Rotation)
{
	AavaWeapon* pavaWeapon = Cast<AavaWeapon>(pActor);
	if(pavaWeapon) {
		pickupClassName = pavaWeapon->PickUpClass->GetPathName();
		this->Location = Location;
		this->Rotation = Rotation;
		this->bJustAddAmmo = pavaWeapon->PickUpAddAmmo;
		this->bDrawInRadar = pavaWeapon->bDrawInRadar;
		this->Lifetime = pavaWeapon->PickUpLifeTime;
		this->IndicatorType = pavaWeapon->IndicatorType;
		this->TeamIdx = pavaWeapon->PickUpTeamNum;

		inventoryClassName = pavaWeapon->GetClass()->GetPathName();				
		debugf(TEXT("[hmMissionObjectHostOwned_t::backup] serializing weapon %s"), *inventoryClassName);

		FMemoryWriter MemoryWriter( Bytes, TRUE );
		pavaWeapon->hmSerialize(MemoryWriter);		
		hmPlayer_t::backupWeaponModifier(MemoryWriter, pavaWeapon);

		return true;
	}	
	return false;
}
//---------------------------------------------------------------------------
bool hmDroppingPickup_t::restore(ULevel* pLevel)
{
	AavaPlayerController* pCtrl = Cast<AavaPlayerController>(pLevel->findActor(APlayerController::StaticClass()));
	if(pCtrl) {
		// {{ 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.
		Location.Z += eypNonPlayer;
		// }} 20070302 약간 위로 올려야 바닥을 뚫고 떨어지지 않는다.

		AActor* pActor = pCtrl->eventHmSpawnActor(pickupClassName, Location, Rotation);
		if(pActor) {
			AavaPickup* pPickup = Cast<AavaPickup>(pActor);
			if(pPickup) {				
				pPickup->bJustAddAmmo = bJustAddAmmo;
				pPickup->bDrawInRadar = bDrawInRadar;
				pPickup->Lifetime = Lifetime;
				pPickup->IndicatorType = IndicatorType;
				pPickup->TeamIdx = TeamIdx;
				debugf(TEXT("[hmDroppingPickup_t] bJustAddAmmo=%d bDrawInRadar=%d Lifetime=%f IndicatorType=%d TeamIdx=%d"), bJustAddAmmo, bDrawInRadar, Lifetime, IndicatorType, TeamIdx);
				pPickup->eventHmSetupPickup(inventoryClassName);				

				AActor* pActor = pCtrl->eventHmSpawnActor(inventoryClassName, Location, Rotation);
				if(pActor) {
					AavaWeapon* pWeapon = Cast<AavaWeapon>(pActor);
					if(pWeapon) {
						FMemoryReader MemoryReader( Bytes, TRUE );
						pWeapon->hmSerialize(MemoryReader);						
						hmPlayer_t::restoreWeaponModifier(MemoryReader, pWeapon);
						pPickup->Inventory = pWeapon;

						pActor->eventOnHmRestore();
					}					
				}
				
				debugf(TEXT("[dEAthcURe|hmDroppingPickup_t::restore] %s %s at (%f %f %f)"), *pActor->GetClass()->GetName(), *pActor->GetName(), pPickup->Location.X, pPickup->Location.Y, pPickup->Location.Z);

				if(pPickup->bDynamicSpawned) {
					debugf(TEXT("[dEAthcURe|hmDroppingPickup_t::restore] bDynamicSpawned"));
				}
				else {
					debugf(TEXT("[dEAthcURe|hmDroppingPickup_t::restore] !bDynamicSpawned"));					
				}
				pActor->eventOnHmRestore();
				return true;
			}			
		}		
	}
	debugf(TEXT("[dEAthcURe] Spawning %s failed"), *pickupClassName);
	return false;
}
//---------------------------------------------------------------------------
bool hmDroppingPickup_t::create(AActor* pActor, FVector Location, FRotator Rotation)
{
	if(g_hostMigration.pMissionObject == 0x0) {
		phmDroppingPickup_t pDp = new hmDroppingPickup_t();
		if(pDp) {
			bool bSuccess = pDp->backup(pActor, Location, Rotation);
			if(bSuccess) {
				g_hostMigration.pMissionObject = pDp;
			}
			else {
				delete pDp;
			}
			return bSuccess;
		}
	}
	return false;
}
//---------------------------------------------------------------------------
// }} [+] 20070419
#endif