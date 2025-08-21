//
// hostMigration.cpp
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#include "EnginePrivate.h"
#include "hostMigration.h"
//---------------------------------------------------------------------------
#ifdef EnableHostMigration
//---------------------------------------------------------------------------
// global def
//---------------------------------------------------------------------------
#ifdef EnableHmFastLoading
bool GIsHostMigrating = false;
bool GEnableHmFastLoading = false; // 20071016
#endif

hostMigration_t g_hostMigration;

TCHAR hmStateNames[][32] = {
	TEXT("hmsNotAssigned"),
	TEXT("hmsClient"),
	TEXT("hmsHost"),
	TEXT("hmsMigrating"),
	TEXT("hmsNewClientPrepare"),
	TEXT("hmsNewClient"),
	TEXT("hmsNewClientLoading"),
	TEXT("hmsNewClientLoaded"),
	TEXT("hmsNewHost"),
	TEXT("hmsHostRestoring"),
	TEXT("hmsEndGame"),
	TEXT("hmsEndOfType")
};
//---------------------------------------------------------------------------
// netCallback_t
//---------------------------------------------------------------------------
#define parseArg(val, payload, offset) \
		memcpy(&val, &payload[offset], sizeof(val)); offset+=sizeof(val);
	
#ifdef UseNativeServer
void netCallback_t::onPacketComplete(phostMigrationNet_t pSender, int type, char* payload) 
{
	switch(type) {
		case pdNtfMigratedHost: {
			unsigned int idHost;
			in_addr in;
			char hostAddr[128];
			unsigned short hostPort;
			int offset = 0;

			parseArg(idHost, payload, offset);
			parseArg(in, payload, offset);
			parseArg(hostPort, payload, offset);

			strcpy(hostAddr, inet_ntoa(in));		

			if(idHost == pSender->id) {
				g_hostMigration.gotoState(hmsNewHost);				
			}
			else {
				g_hostMigration.gotoState(hmsNewClient, (DWORD)hostAddr);				
			}
		} break;

		case pdNtfDeprive: // 제외됨
		case pdNtfEndGame: // 게임이 끝났3
			g_hostMigration.gotoState(hmsEndGame);
			break;		
	}		
}
#endif
//---------------------------------------------------------------------------
// hmData_t
//---------------------------------------------------------------------------
hmData_t::hmData_t(UObject* pObject, ULevel* pLevel) 
{
	timePrevInspect = 0;
	timeInterval = 1000; // 1sec
	index = -1;
	Location = FVector(0.0f, 0.0f, 0.0f);
	Rotation = FRotator(0, 0, 0);
	name = pObject->GetName();		
	bUpdated = false;
	bSetupPended = false;
	bTickable = false; // 20070302
	g_hostMigration.pDataInProcessing = this; // 20070302
}	
//---------------------------------------------------------------------------
// hostMigration_t
//---------------------------------------------------------------------------
// {{ 20071018 FHM 제어
#ifdef EnableHmFastLoading
void hostMigration_t::clearChannelToLoadFast(void)
{
	nChannelToLoadFast = 0;
	for(int lpp=0;lpp<MaxChannelToLoadFast;lpp++) {
		channelsToLoadFast[lpp] = 0;
	}
}

bool hostMigration_t::setChannelToLoadFast(const int idChannel)
{
	if(nChannelToLoadFast < MaxChannelToLoadFast) {
		channelsToLoadFast[nChannelToLoadFast++] = idChannel;	
		return true;
	}
	return false;
}
bool hostMigration_t::isChannelToLoadFast(const int idChannel)
{
	if(bEnableFastLoadingFromIni) {
		for(int lpp=0;lpp<nChannelToLoadFast;lpp++) {
			if(idChannel == channelsToLoadFast[lpp]) {
				return true;
			}	
		}
	}
	return false;
}
#endif
// }} 20071018 FHM 제어

// {{ 20070305 endround후 복원되는 player의 play허용
phmPlayerInfo_t hostMigration_t::findAllowedPlayerOnNextRound(const FString& playerName) 
{
	for(int idx=0;idx<allowedPlayersOnNextRound.Num();idx++) {
		phmPlayerInfo_t pPi = allowedPlayersOnNextRound(idx);
		if(pPi) {				
			if(pPi->name == playerName) {
				return pPi;
			}
		}
	}
	return 0x0;
}
//---------------------------------------------------------------------------
bool hostMigration_t::registerAllowedPlayerOnNextRound(const phmPlayerInfo_t pPiSrc) 
{
	if(pPiSrc) {
		if(!findAllowedPlayerOnNextRound(pPiSrc->name)) {
			phmPlayerInfo_t pPi = new hmPlayerInfo_t();
			if(pPi) {
				*pPi = *pPiSrc;
				allowedPlayersOnNextRound.Push(pPi);
				return true;
			}
		}
	}
	return false; // already exist
}
//---------------------------------------------------------------------------
bool hostMigration_t::removeAllowedPlayerOnNextRound(const FString& playerName) 
{
	for(int idx=0;idx<allowedPlayersOnNextRound.Num();idx++) {
		phmPlayerInfo_t pPi = allowedPlayersOnNextRound(idx);
		if(pPi) {
			if(pPi->name == playerName) {
				debugf(TEXT("[hostMigration_t] Removing %s on allowedPlayersOnNextRound"), *playerName);
				allowedPlayersOnNextRound.Remove(idx);
				return true;
			}
		}
	}
	return false; // not found
}
//---------------------------------------------------------------------------
void hostMigration_t::clearAllowedPlayersOnNextRound(void) 
{
	for(int idx=0;idx<allowedPlayersOnNextRound.Num();idx++) {
		phmPlayerInfo_t pPi = allowedPlayersOnNextRound(idx);
		if(pPi) {
			delete pPi;
			pPi = 0x0;
		}
		allowedPlayersOnNextRound.Remove(idx);
	}	
	allowedPlayersOnNextRound.Empty();
}
// }} 20070305 endround후 복원되는 player의 play허용
//---------------------------------------------------------------------------
bool hostMigration_t::restore(ULevel* pLevel, AActor* pActor)
{
	DWORD timeCurrent = timeGetTime();
	
	bool bRestored = false;

	if(bRestoreEnabled && (state == hmsNewHost || state == hmsHostRestoring)) {		
		bool bActorSpecified = pActor ? true : false;

		if(bActive) { // if(bActive && (nRemotes || nLocals)) {		
			for(int idxd=0;idxd<nodes.Num();idxd++) {
				phmData_t pData = nodes(idxd);
				if(pData && !pData->bUpdated) {
					// {{ interval
					if(timeCurrent < pData->timePrevInspect + pData->timeInterval) {						
						continue;
					}
					pData->timePrevInspect = timeCurrent;
					// }} interval
					AActor* pActorToRestore = pActor ? pActor : pData->findOnLevel(pLevel);				
					if(pActorToRestore) {
						debugf(TEXT("[hostMigration_t::restore] attempting to restore actor %s by %s"), *pActorToRestore->GetName(), *pData->name);
						if(pData->canRestore(this, pLevel)) {
							pDataInProcessing = pData; // 20070302
							if(pData->restore(pActorToRestore, pLevel)) {
								bRestored = true;
								pData->onDestroy(this);
								// delete
							}
							pDataInProcessing = 0x0; // 20070302
						}					
					}
					else { // 20061201
						// 복구하려는 actor가 level에 없3
						pDataInProcessing = pData; // 20070302
						if(pData->restore(pLevel)) {
							bRestored = true;
							pData->onDestroy(this);
							// delete
						}
						pDataInProcessing = 0x0; // 20070302
					}					
				}
			}
		}
	}
	
	if(bRestored) {
		int nValidObject = 0;
		for(int idxd=0;idxd<nodes.Num();idxd++) {
			phmData_t pData = nodes(idxd);			
			if(pData && (!pData->bUpdated||pData->bSetupPended)) {
				nValidObject++;
			}
		}

		if(!nValidObject) {
			clear();
		}
		log(TEXT("[hostMigration_t] restored : %d/%d objects"), nValidObject, nodes.Num());		
	}	
	return bRestored;
}
//---------------------------------------------------------------------------
void hostMigration_t::clear(bool bClearHostInfo)
{	
	bReelectLeaderByTime = false; // 20071024
	nPlayerBaseToRestore = 0; // 20071012 base relative 복원하려면 base부터 복원해야한다.

	if(bClearHostInfo) _tcscpy(hostName, TEXT(""));
	_tcscpy(mapName, TEXT(""));
	_tcscpy(playerName, TEXT(""));
	_tcscpy(lastValidActorName, TEXT(""));

	if(pMissionObject) {
		delete pMissionObject;
		pMissionObject = 0x0;
	}

	nTeamPlayerBackuped[0] = nTeamPlayerBackuped[1] = 0; // 20070523
	nTeamPlayerRestored[0] = nTeamPlayerRestored[1] = 0; // 20070523

	bReelectLeader = false; // 20070523
	// timeBeginRestoring = 0x0; // 20071024 초기화 시점 이동 to backup
	bReadyToRestoreGRI = false;
	bActive = false;
	nLocals = 0;
	nRemotes =0;
	bWarmupRound = false; // 20070306
	// timeGameInfoRestored = 0x0; // 20071024 초기화 시점 이동 to backup // [+] 20070308 TriggerAllOutEvent용 
	bDoNotRestoreC4 = false; // [+] 20070309 host가 c4를 갖고 나갔으면 pickup_c4를 복원하지 않는다.
	bDoNotRestoreNukeCase = false; // [+] 20070507 host가 NukeCase를 갖고 나갔으면 avaPickupProvider를 복원하지 않는다.

	for(int idx=0;idx<nodes.Num();idx++) {
		nodes.Remove(idx);
	}	
	nodes.Empty();

	debugf(TEXT("[hostMigration_t::clear] cleared : %d objects"), nodes.Num());		
}
//---------------------------------------------------------------------------
void hostMigration_t::resetRound(void)
{
	debugf(TEXT("[hostMigration_t::resetRound]"));
	// {{ 20070305 endround후 복원되는 player의 play허용
	clearAllowedPlayersOnNextRound();

	for(int idx=0;idx<nodes.Num();idx++) {
		phmData_t pData = nodes(idx);
		if(pData && !pData->bUpdated) {
			phmPlayerInfo_t pPi = pData->getPlayerInfo();
			if(pPi) {				
				registerAllowedPlayerOnNextRound(pPi);
				debugf(TEXT("[hostMigration_t::resetRound] registering player %s (%f) for next round"), *pPi->name, pPi->Score);
			}
		}
	}
	// }} 20070305 endround후 복원되는 player의 play허용

	if(state != hmsHost) {
		clear(true);
		gotoState(hmsHost);
		debugf(TEXT("[hostMigration_t::resetRound] HM returns to normal state of host."));
		timeBeginRestoring = 0x0; // 20071024
		timeGameInfoRestored = 0x0; // 20071024
	}
}
//---------------------------------------------------------------------------
bool hostMigration_t::backup(UObject* pObject, ULevel* pLevel)
{
	phmData_t pData = create(pObject, pLevel);
	if(pData) {		
		nodes.AddItem(pData);		
		debugf(TEXT("[hostMigration_t::backup] actor %s backuped"), *pObject->GetName());
		bActive = true;
		return true;
	}
	debugf(TEXT("[hostMigration_t::backup] actor %s skipped"), *pObject->GetName());	
	return false;
}
//---------------------------------------------------------------------------
bool hostMigration_t::backup(UWorld* pWorld)
{
	clear();
	timeGameInfoRestored = 0x0; // 20071024  초기화 시점이동 from clear
	timeBeginRestoring = 0x0; // 20071024  초기화 시점이동 from clear

	if(pWorld) {
		for (int idxActor = 0; idxActor < pWorld->CurrentLevel->Actors.Num(); idxActor++) {
			AActor* pActor = GWorld->CurrentLevel->Actors(idxActor);

			if(pActor) {				
				backup(pActor, GWorld->CurrentLevel);
				pDataInProcessing = 0x0; // 20070302
			}
		}
	}
	log(TEXT("[hostMigration_t] backup complete : %d objects"), nodes.Num());
	return bActive;
}
//---------------------------------------------------------------------------
void hostMigration_t::onTick(void)
{
	#ifdef UseNativeServer
	net.onTick();
	#endif
}
//---------------------------------------------------------------------------
bool hostMigration_t::init(void)
{
	if(bInit) return true;
	
	for(int idx=0;idx<types.Num();idx++) { //for(list<hmTypeDesc_t>::iterator it=types.begin();it!=types.end();it++) {
		hmTypeDesc_t& desc = types(idx);		
		//debugf(TEXT("[dEAthcURe|hostMigration_t::init] Registered type %s."), *desc.name);		
	}

	furl = FURL(0x0); // 20070213

	// {{ 20061204
	setEnableIndexPub(false);
	resetHmIndex();
	// }} 20061204

	#ifdef UseNativeServer
	char ipAddress[256] = "10.20.16.40";
	int port = 62000;

	FILE* fp = fopen("hostmigrationnet.ini", "rt");
	if(fp) {
		fscanf(fp, "%s %d", ipAddress, &port);
		fclose(fp);
	}

 	if(net.connect(ipAddress, port)) {
		net.reqJoin();
		net.ntfHardwareInfo();
		net.ntfNetInfo();		
	}
	#endif

	bInit = true; // 접속 여부와 상관없이
	return false;
}
//---------------------------------------------------------------------------
void hostMigration_t::deinit(void)
{
	bInit = false;
}
//---------------------------------------------------------------------------
void hostMigration_t::log(TCHAR* fmt, ...)
{
	va_list argPtr;
	va_start( argPtr, fmt );		
	TCHAR str[1024];
	_vsntprintf( str, 1024, fmt, argPtr );
	if(_log) _log(str);
	va_end( argPtr );
}
//---------------------------------------------------------------------------
void hostMigration_t::makeUrl(void)
{
	_stprintf(url, TEXT("unreal:%s?listen?name=%s?team=%d"), mapName, playerName, idTeam);
}
//---------------------------------------------------------------------------
void hostMigration_t::hostTraverse(TCHAR* playerName)
{
	if(playerName == 0x0) playerName = this->playerName;
	makeUrl(); // _stprintf(url, TEXT("unreal:%s?listen?name=%s?team=%d"), mapName, playerName, idTeam);	
	g_hostMigration.log(TEXT("[dEAthcURe][hostMigration_t::hostTraverse] %s"), url);
}
//---------------------------------------------------------------------------
void hostMigration_t::clientTraverse(char* addr, TCHAR* playerName)
{
	if(playerName == 0x0) playerName = this->playerName;
	TCHAR waddr[512];
	mbstowcs(waddr, addr, 511);
	_stprintf(url, TEXT("unreal:%s?name=%s?team=%d"), waddr, playerName, idTeam);	
	g_hostMigration.log(TEXT("[dEAthcURe][hostMigration_t::clientTraverse] %s"), url);
}
//---------------------------------------------------------------------------
void hostMigration_t::endGameTraverse(void)
{		
	_stprintf(url, TEXT("unreal:avaEntry.ue3?name=%s?team=%d"), playerName, idTeam);
	g_hostMigration.log(TEXT("[dEAthcURe][hostMigration_t::endGameTraverse] %s"), url);
}
//---------------------------------------------------------------------------
/*
bool hostMigration_t::parsePlayerName(const FURL& url)
{
	for( INT i=0; i<url.Op.Num(); i++ ) {
		TCHAR Temp[1024];
		appStrcpy( Temp, *url.Op(i) );
		TCHAR* Value = appStrchr(Temp,'=');
		if( Value ) {
			*Value++ = 0;
			if( appStricmp(Temp,TEXT("Name"))==0 ) {
				_tcscpy(g_hostMigration.playerName, Value);
				return true;
			}
		}
	}	
	return false;
}
//---------------------------------------------------------------------------
TCHAR* hostMigration_t::parseOption(const TCHAR* option, const FURL& url)
{
	for( INT i=0; i<url.Op.Num(); i++ ) {
		TCHAR Temp[1024];
		appStrcpy( Temp, *url.Op(i) );
		TCHAR* Value = appStrchr(Temp,'=');
		if( Value ) {
			*Value++ = 0;
			if( appStricmp(Temp,option)==0 ) {
				return Value;
			}
		}
	}	
	return 0x0;
}
*/
//---------------------------------------------------------------------------
void hostMigration_t::parseUrl(const FURL& url)
{
	const TCHAR* name = url.GetOption(TEXT("Name="),NULL); // TCHAR* name = parseOption(TEXT("Name"), url);
	if(name) _tcscpy(playerName, name);	

	const TCHAR* team = url.GetOption(TEXT("Team="),NULL); // TCHAR* team = parseOption(TEXT("Team"), url);
	if(team) _stscanf(team, TEXT("%d"), &idTeam);

	const TCHAR* map = *url.Map;
	if(map) _tcscpy(mapName, map);	

	log(TEXT("[hostMigration_t::parseUrl] playerName=%s idTeam=%d mapName=%s"), playerName, idTeam, mapName);
}
//---------------------------------------------------------------------------
phmData_t hostMigration_t::create(UObject* pObject, ULevel* pLevel)
{	
	for(int idx=0;idx<types.Num();idx++) { //for(list<hmTypeDesc_t>::iterator it=types.begin();it!=types.end();it++) {
		hmTypeDesc_t& desc = types(idx);
		if(desc.canCreate(pObject)) {
			return desc.create(this, pObject, pLevel);			
		}
	}
	return 0x0;	
}
//---------------------------------------------------------------------------
phmTypeDesc_t hostMigration_t::findType(const FString& name)
{
	for(int idx=0;idx<types.Num();idx++) { //for(list<hmTypeDesc_t>::iterator it=types.begin();it!=types.end();it++) {
		hmTypeDesc_t& desc = types(idx);
		if(desc.name == name) {
			return &desc;
		}
	}
	return 0x0;
}
//---------------------------------------------------------------------------
bool hostMigration_t::addType(const FString& name, bool (*canCreate)(UObject* pObject), phmData_t (*create)(struct hostMigration_t* pSender, UObject* pObject, ULevel* pLevel), void (*testSuite)(struct hostMigration_t* pSender, UWorld* pWorld))
{
	assert(canCreate);
	assert(create);
	assert(testSuite);

	// {{ 20070214
	if(findType(name)) {
		return false;
	}

	hmTypeDesc_t td;	
	td.name = name;
	td.canCreate = canCreate;
	td.create = create;
	td.testSuite = testSuite;
	types.Push(td); // types.push_back(td);
	
	return true;
	// }} 20070214	
}
//---------------------------------------------------------------------------
void hostMigration_t::testSuite(UWorld* pWorld)
{	
	// {{ 20070214	
	for(int idx=0;idx<types.Num();idx++) { //for(list<hmTypeDesc_t>::iterator it=types.begin();it!=types.end();it++) {
		hmTypeDesc_t& desc = types(idx);
		desc.testSuite(this, pWorld);
	}
	// }} 20070214	
}
//---------------------------------------------------------------------------
void hostMigration_t::resetHmIndex(void)
{
	hmIndexSerial = -1;
}
//---------------------------------------------------------------------------
void hostMigration_t::setHmIndices(ULevel* pLevel)
{
	assert(pLevel);

	if(state==hmsHost || state==hmsNewHost) {
		hmIndexSerial = -1;
		
		for(int idxActor=0;idxActor<pLevel->Actors.Num();idxActor++) {
			AActor* pActor = pLevel->Actors(idxActor);
			if(pActor) {
				pActor->hmIndex = ++hmIndexSerial;
				//debugf(TEXT("hostMigration_t::setHmIndices %s %s %d"), *pActor->GetName(), *pActor->GetClass()->GetName(), pActor->hmIndex);				
			}
		}		
	}
	debugf(TEXT("[hostMigration_t::setHmIndices] %d objects indexed"), hmIndexSerial+1);
	return;

	assert(pLevel);

	if(state!=hmsClient) {
		hmIndexSerial = -1;

		char fileName[512];
		sprintf(fileName, "actorIndices_0x%x.txt", pLevel);
		FILE* fp = fopen(fileName, "wt");
		if(fp) {
			for(int idxActor=0;idxActor<pLevel->Actors.Num();idxActor++) {
				AActor* pActor = pLevel->Actors(idxActor);
				if(pActor) {
					pActor->hmIndex = ++hmIndexSerial;
					//debugf(TEXT("hostMigration_t::setHmIndices %s %s %d"), *pActor->GetName(), *pActor->GetClass()->GetName(), pActor->hmIndex);
					char name[512];
					char className[512];

					wcstombs(name, *pActor->GetName(), 511);
					wcstombs(className, *pActor->GetClass()->GetName(), 511);

					fprintf(fp, "%s %s %d\n", name, className, pActor->hmIndex);
				}
			}
			fclose(fp);
		}
	}
	return;
}
//---------------------------------------------------------------------------
void hostMigration_t::setEnableIndexPub(bool bEnable)
{
	hmIndexSerial = -1;
	bIndexPubEnabled = bEnable;
}
//---------------------------------------------------------------------------
void hostMigration_t::setEnableRestore(bool bEnable)
{
	bRestoreEnabled = bEnable;
	/*
	// {{ 20070904 
	DWORD timeBegin = timeGetTime();
	for(int idx=0;idx<nodes.Num();idx++) {
		phmData_t pData = nodes(idx);
		if(pData) {
			pData->timePrevInspect = timeBegin;
		}
	}	
	// }} 20070904 
	*/
}
//---------------------------------------------------------------------------
phmPlayerBase_t hostMigration_t::findPlayerData(const TCHAR* playerName)
{
	for(int idx=0;idx<nodes.Num();idx++) {
		phmData_t pData = nodes(idx);
		if(pData && !pData->bUpdated && pData->isPlayer(playerName)) { // [+] 20070306 bUpdated 조건추가
			return (phmPlayerBase_t)pData;
		}		
	}
	return 0x0;
}
//---------------------------------------------------------------------------
void hostMigration_t::actorsListToFile(ULevel* pLevel, char* title, char* fileName)
{
	int nActorFromFile = 0;
	int nNullActor = 0;
	int navaKBreakable = 0;

	if(pLevel) {
		FILE* fp = fopen(fileName, "at");
		if(fp) {			
			for(int lpp=0;lpp<pLevel->Actors.Num();lpp++) {
				AActor* pActor = pLevel->Actors(lpp);
				if(pActor) {
					nActorFromFile++;
					char mbName[256];
					wcstombs(mbName, *pActor->GetName(), 255);
					if(0) { // if(pActor->bDisabledForHostMigration) {
						fprintf(fp, "[%s] %4d %4d %s (bDisabledForHostMigration)\n", title, nActorFromFile, lpp, mbName);
					}
					else {
						fprintf(fp, "[%s] %4d %4d %s\n", title, nActorFromFile, lpp, mbName);
					}

					if(pActor->GetClass()->GetName() == TEXT("avaKBreakable")) {
						navaKBreakable++;
					}
				}
				else {
					fprintf(fp, "[%s]???? %4d null\n", title, lpp);
					nNullActor++;
				}
			}
			fprintf(fp, "*total %d valid actors, %d null actors %d avaKBreakable\n\n", nActorFromFile, nNullActor, navaKBreakable);

			fclose(fp);
		}		
	}
}
//---------------------------------------------------------------------------
bool hostMigration_t::findLastValidActor(ULevel* pLevel)
{
	bool bFound = false;
	if(pLevel) {
		for(int lpp=0;lpp<pLevel->Actors.Num();lpp++) {
			AActor* pActor = pLevel->Actors(lpp);
			if(pActor) {
				_tcscpy(lastValidActorName, *pActor->GetName());				
				bFound = true;
			}
		}
	}
	actorsListToFile(pLevel, "after findLastValidActor");
	return bFound;
}
//---------------------------------------------------------------------------
bool hostMigration_t::removeNonBuiltinActors(ULevel* pLevel)
{	
	bool bRemoved = false;
	if(pLevel) {
		for(int lpp=0;lpp<pLevel->Actors.Num();lpp++) {
			AActor* pActor = pLevel->Actors(lpp);
			if(pActor) {				
				if(!_tcscmp(lastValidActorName, *pActor->GetName())) {
					int nIdxToRemove = lpp+1;
					
					for(int lpd=pLevel->Actors.Num()-1;lpd>lpp;lpd--) {
						AActor* pActorToDestroy = pLevel->Actors(lpd);
						if(pActorToDestroy) {
							debugf(TEXT("[dEAthcURe] hostMigration_t::removeNonBuiltinActors/DestroyActor %d:%s"), lpd, *pActorToDestroy->GetFullName());							
							GWorld->DestroyActor(pActorToDestroy);
						}
					}										
					//pLevel->Actors.Remove(nIdxToRemove, pLevel->Actors.Num()-nIdxToRemove); // 제대로 삭제되나 확인
					bRemoved = true;
					break;
				}
			}
		}
	}
	actorsListToFile(pLevel, "after removeNonBuiltinActors");
	return bRemoved;
}
//---------------------------------------------------------------------------
void hostMigration_t::reinitBuiltinActors(ULevel* pLevel)
{	
	if(pLevel) {
		for(int lpp=0;lpp<pLevel->Actors.Num();lpp++) {
			AActor* pActor = pLevel->Actors(lpp);
			if(pActor) {				
				pActor->eventHmReinit();				
			}
		}
	}
}
//---------------------------------------------------------------------------
bool hostMigration_t::gotoState(hmState_t state, DWORD param1, DWORD param2)
{		
	switch(state) {
		case hmsNewHost: {
			#ifdef UseNativeServer
			g_hostMigration.hostTraverse();
			#else						
			const TCHAR* Option=furl.GetOption(TEXT("Name="),NULL);
			if(Option) {
				_tcscpy(playerName, Option);
				debugf(TEXT("[hostMigration_t::gotoState|hmsNewHost] playerName parsed=%s"), playerName);
			}
			else {
				debugf(TEXT("hostMigration_t::gotoState|hmsNewHost] parsing playerName failed"));
			}			
			#endif
			this->state = state;
		} break;

		case hmsNewClient: {
			#ifdef UseNativeServer
			char* hostAddr = (char*)param1;
			g_hostMigration.clientTraverse(hostAddr);
			#else
			const TCHAR* Option=furl.GetOption(TEXT("Name="),NULL);
			if(Option) {
				_tcscpy(playerName, Option);
				debugf(TEXT("[hostMigration_t::gotoState|hmsNewClient] playerName parsed=%s"), playerName);
			}
			else {
				debugf(TEXT("hostMigration_t::gotoState|hmsNewClient] parsing playerName failed %s"), *furl.String());
			}			
			#endif
			this->state = state;
		} break;

		case hmsMigrating:
			#ifdef UseNativeServer
			if(!net.ntfLinkLost()) return false;
			#endif
			this->state = hmsMigrating;
			break;

		case hmsEndGame:
			endGameTraverse();
			this->state = state;
			break;

		// {{ 20070509 호스트 이전중 악의적으로 한참 후에 접속하는 사용자를 spectator처리
		case hmsHostRestoring:
			timeBeginRestoring = timeGetTime();
			this->state = state;
			break;
		// }} 20070509 호스트 이전중 악의적으로 한참 후에 접속하는 사용자를 spectator처리

		default:
			if(this->state == hmsNewHost) {
				if(state == hmsHostRestoring) {
					this->state = state;
				}
			}			
			// {{ 20070518 
			else if(this->state == hmsNewClient || this->state == hmsNewClientPrepare) {
				if(state != hmsNotAssigned) {
					this->state = state;
				}
			}
			// {{ 20070518 
			else {
				this->state = state;
			}
	}

	debugf(TEXT(" "));
	debugf(TEXT("          ********** hostMigration_t::gotoState %s"), hmStateNames[(int)this->state]);
	debugf(TEXT(" "));
	return true;
}
//---------------------------------------------------------------------------
// {{ 20070222
void hostMigration_t::setHostName(TCHAR* pName, int teamIdx)
{
	if(pName) {
		_tcscpy(hostName, pName);
		hostTeamIdx = teamIdx;
		log(TEXT("[hostMigration_t::setHostName] Host name = %s %d"), hostName, hostTeamIdx);
	}
}
// }} 20070222
//---------------------------------------------------------------------------
void hostMigration_t::tick(ULevel* pLevel)
{
	static DWORD timeInterval = 1000; // 1 sec
	static DWORD timeNext = 0;
	DWORD timeCurr = timeGetTime();
	
	restore(pLevel); // [+] 20070827 이건 무조건
	if(timeCurr > timeNext) {	

		bool bProcessed = false;
		for(int idxd=0;idxd<nodes.Num();idxd++) {
			phmData_t pData = nodes(idxd);
			if(pData && pData->bTickable) {
				if(pData->tick()) {
					pData->bTickable = false;
					bProcessed = true;
				}
			}
		}

		if(bProcessed) {
			int nValidObject = 0;
			for(int idxd=0;idxd<nodes.Num();idxd++) {
				phmData_t pData = nodes(idxd);
				if(pData && (!pData->bUpdated||pData->bSetupPended)) {
					nValidObject++;
				}
			}

			if(!nValidObject) {
				clear();
			}
			log(TEXT("[hostMigration_t] ticked : %d/%d objects"), nValidObject, nodes.Num());		
			// clear과정을 정리할것
		}	

		// {{ 20070308 한팀이 다 나갔는지 check
		if(timeGameInfoRestored) {
			static DWORD timeTriggerAllOutEvent = 20000; // 20070624 15sec -> 20sec // 20070309 너무 빠른 감이 없지 않다. 10 -> 20?
			if(bEnableCheckAllOut && timeGameInfoRestored && timeCurr >= timeGameInfoRestored + timeTriggerAllOutEvent) { // 20071024 bEnableCheckAllOut 추가
				debugf(TEXT("[hostMigration_t::tick] Checking all out..."));
				extern bool _hm_checkAllOut(void);
				if(_hm_checkAllOut()) {
					clear();
					debugf(TEXT("[hostMigration_t::tick] All out detected."));
				}
				bool _hm_checkAllDead(void);
				if(_hm_checkAllDead()) {
					clear();
					debugf(TEXT("[hostMigration_t::tick] All dead detected."));
				}
				timeGameInfoRestored = 0x0;
			}
		}
		// }} 20070308 한팀이 다 나갔는지 check

		// {{ 20071024 HM 60초 후에 분대장을 재선출해본다.
		if(bReelectLeaderByTime && timeBeginRestoring && timeCurr - timeBeginRestoring >= timeoutLogin) {
			extern void _hm_electLeader(AGameInfo* pGi, int teamIdx);
			_hm_electLeader(GWorld ? GWorld->GetGameInfo(): 0x0, 0);
			_hm_electLeader(GWorld ? GWorld->GetGameInfo(): 0x0, 1);
			bReelectLeaderByTime = false;
		}
		// }} 20071024 HM 60초 후에 분대장을 재선출해본다.
		timeNext = timeCurr + timeInterval;
	}
}
//---------------------------------------------------------------------------
// {{ 20070509 호스트 이전중 악의적으로 한참 후에 접속하는 사용자를 spectator처리
bool hostMigration_t::checkDelayedLogin(phmPlayerBase_t pPlayerBase)
{
	DWORD timeCurrent = timeGetTime();
	if(timeBeginRestoring && timeCurrent - timeBeginRestoring >= timeoutLogin) {
		return true;
	}
	return false;
}
// }} 20070509 호스트 이전중 악의적으로 한참 후에 접속하는 사용자를 spectator처리
//---------------------------------------------------------------------------
hostMigration_t::hostMigration_t(void)
: state(hmsNotAssigned)
, bInit(false)
,_log(0x0)
, idTeam(-1)
, hmIndexSerial(-1)
, bIndexPubEnabled(false)
, bRestoreEnabled(false)
, bWarmupRound(false)
{	
	bEnableCheckAllOut = true; // 20071024 모두 나갔는지 체크하는 기능의 활성여부
	bReelectLeaderByTime = false; // 20071024

	// {{ 20071018 FHM 제어
	#ifdef EnableHmFastLoading
	bEnableFastLoadingFromIni = false;
	clearChannelToLoadFast();
	#endif
	// }} 20071018 FHM 제어

	nPlayerBaseToRestore = 0; // 20071012 base relative 복원하려면 base부터 복원해야한다.
	pMissionObject = 0x0;
	nTeamPlayerBackuped[0] = nTeamPlayerBackuped[1] = 0; // 20070523
	nTeamPlayerRestored[0] = nTeamPlayerRestored[1] = 0; // 20070523

	bReelectLeader = false; // 20070523 host가 쌍안경을 갖고 나갔으면 60초 후에 분대장을 재선출한다.
	// {{ 20070509 호스트 이전중 악의적으로 한참 후에 접속하는 사용자를 spectator처리
	timeoutLogin = 60000;
	timeBeginRestoring = 0x0;
	// }} 20070509 호스트 이전중 악의적으로 한참 후에 접속하는 사용자를 spectator처리

	bDoNotRestoreC4 = false; // [+] 20070309 host가 c4를 갖고 나갔으면 pickup_c4를 복원하지 않는다.
	bDoNotRestoreNukeCase = false; // [+] 20070507 host가 NukeCase를 갖고 나갔으면 avaPickupProvider를 복원하지 않는다.
	timeGameInfoRestored = 0; // 20070308
	RemainingTime = 0;
	ElapsedTime = 0;
	_tcscpy(hostName, TEXT(""));
	_tcscpy(mapName, TEXT(""));
	_tcscpy(playerName, TEXT(""));
	_tcscpy(lastValidActorName, TEXT(""));

	pDataInProcessing = 0x0; // 20070302
	bReadyToRestoreGRI = false;
	bActive = false;
	nLocals = 0;
	nRemotes = 0;	

	#ifdef UseNativeServer
	net.setCallback(&netcb);	
	#endif

	extern void hmRegisterTypes(void);
	hmRegisterTypes();
}
//---------------------------------------------------------------------------
hostMigration_t::~hostMigration_t(void)
{
}
//---------------------------------------------------------------------------
void dEAthcUReTest(UObject* pObject)
{
	// {{ 20070319 test
	if(GWorld && GWorld->CurrentLevel) {
		int idx = 0;
		AActor* pGi = GWorld->CurrentLevel->findActorByClassName(TEXT("avaSWGame"), idx);
		if(pGi) {
			for (TFieldIterator<UProperty> It(pGi->GetClass()); It; ++It) {
				if (It->PropertyFlags & CPF_HmSerialize) {			
					debugf(TEXT("%s"), *It->GetName());
					
					UField* pField = pGi->FindObjectField(*It->GetName());
					if(pField) {						
						UProperty* pProperty = Cast<UProperty>(pField);
						if(pProperty) {
							debugf(TEXT("AActor:0x%x, UProperty:0x%x ArrayDim=%d ElementSize=%d PropertyFlags=%d Offset=%d"), 
								pGi, pProperty, pProperty->ArrayDim, pProperty->ElementSize, pProperty->PropertyFlags, pProperty->Offset);
							
							BYTE* pAddr = (BYTE*)pGi + pProperty->Offset + 0 * pProperty->ElementSize; //BASE + OFFSET + INDEX * SIZE
							UPropertyValue propertyValue;
							if(pProperty->GetPropertyValue(pAddr, propertyValue)) {
								debugf(TEXT("retrieving property value succeeded"));
								UBoolProperty* pBoolProperty = Cast<UBoolProperty>(pProperty);
								if(pBoolProperty) {
									debugf(TEXT("%s: bool %s"), *pField->GetName(), propertyValue.BoolValue == TRUE ? TEXT("true") : TEXT("false"));
								}
								UIntProperty* pIntProperty = Cast<UIntProperty>(pProperty);
								if(pIntProperty) {
									debugf(TEXT("%s: int %d"), *pField->GetName(), propertyValue.IntValue);

									propertyValue.IntValue = 0;
									if(pProperty->SetPropertyValue(pAddr, propertyValue)) {
										debugf(TEXT("setting property value succeeded"));
									}
								}
							}	
							// x)serialize에서는 CopyCompleteValue를 사용하면 될듯							
						}
					}					
				}
			}
		}
	}
	// }} 20070319 test

	return;// test할때만 열어두3

	static UWorld* pWorld = 0x0;
	UObject* pOuter1 = 0x0;
	UObject* pOuter2 = 0x0;

	UWorld* ptmpWorld = Cast<UWorld>(pObject);	
	if(ptmpWorld) {
		pWorld = ptmpWorld;
		if(pWorld->PersistentLevel) {
			int lpf=0;
		}
	}
	if(pObject->GetClass()->GetName() == TEXT("avaKBreakable")) {		
		int lpf=0;
		FString outerClassName = pObject->GetOuter()->GetClass()->GetName();
		if(outerClassName == TEXT("Level")) {
			int lpf=0;
			char name[256];
			
			pOuter1 = pObject->GetOuter(); // must be ULevel
			if(pOuter1) {
				wcstombs(name, *pOuter1->GetClass()->GetName(), 255);
				pOuter2 = pOuter1->GetOuter(); // UWorld?
				if(pOuter2) {
					wcstombs(name, *pOuter2->GetClass()->GetName(), 255);
					int lpf=0;
				}
			}
			/*
			AActor* pActor = pWorld->PersistentLevel->findActorByClassName(TEXT("avaKBreakable"), lpf);
			if(pActor) {
				int lpf=0;
			}
			*/
		}
		if(outerClassName == TEXT("Package")) {
			int lpf=0;
		}
	}
}
//---------------------------------------------------------------------------
void _logToFile(char* fileName, TCHAR* fmt, ...)
{
	if(fileName == 0x0) {
		fileName = "dEAthcURe.log";
	}
	va_list argPtr;
	va_start( argPtr, fmt );		
	TCHAR str[1024];
	_vsntprintf( str, 1024, fmt, argPtr );

	FILE* fp = fopen(fileName, "at");
	if(fp) {	
		_ftprintf(fp, TEXT("%s\n"), str);
		fclose(fp);
	}	
	va_end( argPtr );
}
//---------------------------------------------------------------------------
/* 20070321 hmSerialize sample
static void hmSerialize(FArchive& Ar)
{
	// {{ 20070321 hmserialize script tag support
	for (TFieldIterator<UProperty> It(GetClass()); It; ++It) {
		if (It->PropertyFlags & CPF_HmSerialize) {
			debugf(TEXT("[%s::hmSerialize] %s"), *GetClass()->GetName(), *It->GetName());
			UField* pField = FindObjectField(*It->GetName());
			if(pField) {
				UProperty* pProperty = Cast<UProperty>(pField);
				if(pProperty) {
					BYTE* pAddr = (BYTE*)this + pProperty->Offset;

					UPropertyValue propertyValue;
					UStrProperty* pStrProperty = Cast<UStrProperty>(pProperty);
					if(pStrProperty) {
						if ( Ar.IsLoading() ) {
							FString str;
							Ar << str;
							propertyValue.StringValue = &str;
							pStrProperty->SetPropertyValue(pAddr, propertyValue);
							//debugf(TEXT("** Loading string property %s"), **propertyValue.StringValue);
						}
						else {
							pStrProperty->GetPropertyValue(pAddr, propertyValue);							
							Ar << *propertyValue.StringValue;
							//debugf(TEXT("** Saving string property %s"), **propertyValue.StringValue);
						}						
						continue;
					}
					UNameProperty* pNameProperty = Cast<UNameProperty>(pProperty);
					if(pNameProperty) {
						if ( Ar.IsLoading() ) {
							//debugf(TEXT("** Loading name property skipped"));
						}
						else {
							//Ar << *propertyValue.NameValue;
							//pNameProperty->GetPropertyValue(pAddr, propertyValue);
							//debugf(TEXT("** Saving name property skipped"));
						}
						continue;
					}
					UBoolProperty* pBoolProperty = Cast<UBoolProperty>(pProperty);
					if(pBoolProperty) {
						if ( Ar.IsLoading() ) {
							Ar << propertyValue.BoolValue;

							pBoolProperty->SetPropertyValue(pAddr, propertyValue);
							//debugf(TEXT("** Loading ubool property %s"), propertyValue.BoolValue?TEXT("true"):TEXT("false"));						
						}
						else {
							pBoolProperty->GetPropertyValue(pAddr, propertyValue);
							Ar << propertyValue.BoolValue;
							//debugf(TEXT("** Saving ubool property %s"), propertyValue.BoolValue?TEXT("true"):TEXT("false"));						
						}
						continue;
					}
					
					Ar.ByteOrderSerialize( pAddr, pProperty->ArrayDim * pProperty->ElementSize);
					if (Ar.IsLoading() ) debugf(TEXT("  loaded %s %dx%d"), *It->GetName(), pProperty->ArrayDim, pProperty->ElementSize);
					else debugf(TEXT("  saved %s %dx%d"), *It->GetName(), pProperty->ArrayDim, pProperty->ElementSize);
				}
				else debugf(TEXT("[%s::hmSerialize] casting UField to UProperty failed"), *GetClass()->GetName());
			}
			else debugf(TEXT("[%s::hmSerialize] finding field %s failed"), *GetClass()->GetName(), *It->GetName());
		}
	}	
	// }} 20070321 hmserialize script tag support
}
static void hmSerialize_CpfNet(FArchive& Ar)
{
	// 이렇게하면 안됨. 복원하지 말아야될 데이터도 있음

	for (TFieldIterator<UProperty> It(GetClass()); It; ++It) {
		if (It->PropertyFlags & CPF_Net) {
			debugf(TEXT("[%s::hmSerialize] %s"), *GetClass()->GetName(), *It->GetName());
			UField* pField = FindObjectField(*It->GetName());
			if(pField) {
				UProperty* pProperty = Cast<UProperty>(pField);
				if(pProperty) {
					UByteProperty* pByteProp = Cast<UByteProperty>(pField);
					UIntProperty* pIntProp = Cast<UIntProperty>(pField);
					UBoolProperty* pBoolProp = Cast<UBoolProperty>(pField);
					UFloatProperty* pFloatProp = Cast<UFloatProperty>(pField);
					UStrProperty* pStrProp = Cast<UStrProperty>(pField);
					UNameProperty* pNameProp = Cast<UNameProperty>(pField);//test					
					UStructProperty* pStructProp = Cast<UStructProperty>(pField);//test

					if(pNameProp) debugf(TEXT("UNameProperty %s"), *It->GetName());
					if(pStructProp ) debugf(TEXT("UStructProperty %s"), *It->GetName());						

					if(pByteProp||pIntProp||pBoolProp||pFloatProp||pStrProp||pNameProp||pStructProp) {					
						BYTE* pAddr = (BYTE*)this + pProperty->Offset;
						Ar.ByteOrderSerialize( pAddr, pProperty->ArrayDim * pProperty->ElementSize);
						if (Ar.IsLoading() ) debugf(TEXT("  loaded %s %dx%d"), *It->GetName(), pProperty->ArrayDim, pProperty->ElementSize);
						else debugf(TEXT("  saved %s %dx%d"), *It->GetName(), pProperty->ArrayDim, pProperty->ElementSize);
					}
					else debugf(TEXT("[%s::hmSerialize] casting UField to UProperty failed"), *GetClass()->GetName());
				}
				else debugf(TEXT("[%s::hmSerialize] Not a intrinsic value type"));
			}
			else debugf(TEXT("[%s::hmSerialize] finding field %s failed"), *GetClass()->GetName(), *It->GetName());
		}
	}
}
*/
#endif