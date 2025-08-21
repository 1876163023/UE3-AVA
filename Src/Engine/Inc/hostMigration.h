//
// hostMigration.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifdef EnableHostMigration
#ifdef EnableHmFastLoading
extern bool GIsHostMigrating;
extern bool GEnableHmFastLoading; // 20071016
#endif

#ifndef hostMigrationH
#define hostMigrationH
//---------------------------------------------------------------------------
//#define UseNativeServer // [+] 20060212

#ifdef UseNativeServer
#include "hostMigrationNet.h"
#endif
//---------------------------------------------------------------------------
struct hmMissionObject_t {
	TArray<BYTE> Bytes;	

	FString pickupClassName;
	FString inventoryClassName;
	FVector Location;
	FRotator Rotation;

	virtual bool backup(AActor* pActor, FVector Location, FRotator Rotation) { return false;}
	virtual bool restore(ULevel* pLevel) {return false;}	
};
typedef struct hmMissionObject_t hmMissionObject_t;
typedef struct hmMissionObject_t* phmMissionObject_t;
//---------------------------------------------------------------------------
// {{ 20070305 endround �� �����ϴ� player�� pri���� ������, pri��ü�� �����Ͽ� �׽�Ʈ�غ���
struct hmPlayerInfo_t {
	FString name;
	FLOAT Score;
    FLOAT Deaths;
	FLOAT AttackPoint;
    FLOAT DefencePoint;
    FLOAT LeaderPoint;
    FLOAT TacticsPoint;
	int nInventory; // [+] 20070307	

	hmPlayerInfo_t& operator=(const hmPlayerInfo_t& pi) {
		name = pi.name;
		Score = pi.Score;
		Deaths = pi.Deaths;
		AttackPoint = pi.AttackPoint;
		DefencePoint = pi.DefencePoint;
		LeaderPoint = pi.LeaderPoint;
		TacticsPoint = pi.TacticsPoint;
		nInventory = pi.nInventory; // [+] 20070307		
		return *this;
	}

	hmPlayerInfo_t(void) {
		name = TEXT("");
		Score = 0.0f;
		Deaths =  0.0f;
		AttackPoint = 0.0f;
		DefencePoint = 0.0f;
		LeaderPoint = 0.0f;
		TacticsPoint = 0.0f;
		nInventory = 0; // [+] 20070307		
	}
};
typedef struct hmPlayerInfo_t hmPlayerInfo_t;
typedef struct hmPlayerInfo_t* phmPlayerInfo_t;
// }} 20070305 
//---------------------------------------------------------------------------
struct hmData_t {	
	int index;
	FVector Location;
    FRotator Rotation;

	TArray<BYTE> Bytes;	
	FString name;
	bool bUpdated; // serialize�Ǿ����� ����
	bool bSetupPended; // game�� ��� �����Ͱ� ����Ǿ����� ����
	DWORD timePrevInspect;
	DWORD timeInterval;

	virtual AActor* findOnLevel(ULevel* pLevel) {
		if(index!=-1) {
			for(int idxActor=0;idxActor<pLevel->Actors.Num();idxActor++) {
				AActor* pActor = pLevel->Actors(idxActor);
				if(pActor && pActor->hmIndex == index) {
					return pActor;
				}
			}			
		}
		return 0x0;
	}
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0) {
		AActor* pActor = Cast<AActor>(pObject);
		if(pActor) {
			index = pActor->hmIndex;
			Location = pActor->Location;
			Rotation = pActor->Rotation;

			pActor->eventOnHmBackup(); // 20070326
		}
	}	
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0)=0;	
	virtual bool restore(ULevel* pLevel = 0x0) {return false;}

	static bool canCreate(UObject* pObject) { return false; }
	static struct hmData_t* create(struct hostMigration_t* pSender, UObject* pObject, ULevel* pLevel) { return 0x0; }
	static void testSuite(struct hostMigration_t* pSender, UWorld* pWorld) {}

	virtual bool canRestore(struct hostMigration_t* pSender, ULevel* pLevel) { return true; }
	virtual void onDestroy(struct hostMigration_t* pSender) {} // destroy�� �ƴ϶� onAfterDeserialize�̴�

	virtual bool isPlayer(const TCHAR* name) {
		return false; // /GR���� dynamic_cast�� �ȵǼ� �̷���.. �Ѥ�^
	}

	virtual phmPlayerInfo_t getPlayerInfo(void) { // 20070305
		return 0x0;
	}

	// {{ 20070302
	bool bTickable;
	virtual bool tick(void) { // return true if to be deleted
		return false;
	}
	// }} 20070302
	hmData_t(UObject* pObject, ULevel* pLevel);
};
typedef struct hmData_t hmData_t;
typedef struct hmData_t* phmData_t;
//---------------------------------------------------------------------------
struct hmPlayerBase_t : hmData_t {
	hmPlayerInfo_t pi; // 20070305

	TCHAR playerName[64];
	bool bDead; // 20061214	
	
	bool bRestorePRI; // 20061215
	TArray<BYTE> PRIBytes;	
	
	bool bRestorePawn; // 20061215
	TArray<BYTE> pawnBytes;	

	virtual bool isPlayer(const TCHAR* name) { // /GR���� dynamic_cast�� �ȵǼ� �̷���.. �Ѥ�^		
		if(!_tcscmp(name, playerName)) {
			return true;
		}
		return false;
	}

	virtual phmPlayerInfo_t getPlayerInfo(void) { // 20070305
		return &pi;
	}

	hmPlayerBase_t(UObject* pObject, ULevel* pLevel) : hmData_t(pObject, pLevel) {
		_tcscpy(playerName, TEXT("dEAthcURe"));
		bDead = false; // 20061214
		bRestorePRI = true; // 20061215
		bRestorePawn = true; // 20061215
	}
};
typedef struct hmPlayerBase_t hmPlayerBase_t;
typedef struct hmPlayerBase_t* phmPlayerBase_t;
//---------------------------------------------------------------------------
enum hmState_t {
	hmsNotAssigned,
	hmsClient,
	hmsHost,
	hmsMigrating,
	hmsNewClientPrepare, // [+] 20070502
	hmsNewClient,
	hmsNewClientLoading,
	hmsNewClientLoaded, // [+] 20070528
	hmsNewHost,
	hmsHostRestoring,
	hmsEndGame,
	hmsEndOfType
};
//---------------------------------------------------------------------------
// callback object def
//---------------------------------------------------------------------------
#ifdef UseNativeServer
struct netCallback_t : hmNetCallback_t {
	virtual void onPacketComplete(phostMigrationNet_t pSender, int type, char* payload);		
};
#endif
//---------------------------------------------------------------------------
struct hmTypeDesc_t {	
	FString name; //
	bool (*canCreate)(UObject* pObject);
	phmData_t (*create)(struct hostMigration_t* pSender, UObject* pObject, ULevel* pLevel);
	void (*testSuite)(struct hostMigration_t* pSender, UWorld* pWorld);

	hmTypeDesc_t(void) {
		canCreate = 0x0;
		create = 0x0;
		testSuite = 0x0;
	}
};
typedef struct hmTypeDesc_t hmTypeDesc_t;
typedef struct hmTypeDesc_t* phmTypeDesc_t;

//---------------------------------------------------------------------------
struct hostMigration_t {
	bool bEnableCheckAllOut; // 20071024 ��� �������� üũ�ϴ� ����� Ȱ������
	bool bReelectLeaderByTime; // 20071024

	// {{ 20071018 FHM ����
	#ifdef EnableHmFastLoading
	#define MaxChannelToLoadFast 16
	int nChannelToLoadFast;
	int channelsToLoadFast[MaxChannelToLoadFast];

	bool bEnableFastLoadingFromIni;
	void clearChannelToLoadFast(void);
	bool setChannelToLoadFast(const int idChannel);
	bool isChannelToLoadFast(const int idChannel);
	#endif
	// }} 20071018 FHM ����

	int nPlayerBaseToRestore; // 20071012 base relative �����Ϸ��� base���� �����ؾ��Ѵ�.

	phmMissionObject_t pMissionObject;
	int nTeamPlayerBackuped[2]; // [+] 20070523 ������ ����� ���� ����̳�
	int nTeamPlayerRestored[2]; // [+] 20070523 ������ ������ ���� ����̳�

	bool bReelectLeader; // [+] 20070523 host�� �־Ȱ��� ���� �������� 60�� �Ŀ� �д����� �缱���Ѵ�.
	DWORD timeoutLogin;
	DWORD timeBeginRestoring; // 20070509 ȣ��Ʈ ������ ���������� ���� �Ŀ� �����ϴ� ����ڸ� spectatoró��
	bool bDoNotRestoreC4; // [+] 20070309 host�� c4�� ���� �������� pickup_c4�� �������� �ʴ´�.
	bool bDoNotRestoreNukeCase; // [+] 20070507 host�� NukeCase�� ���� �������� avaPickupProvider�� �������� �ʴ´�.
	DWORD timeGameInfoRestored; // [+] 20070308 TriggerAllOutEvent��
	int RemainingTime, ElapsedTime;
	bool bWarmupRound;
	int idTeam;
	
	// {{ 20070305 endround�� �����Ǵ� player�� play���
	TArray<phmPlayerInfo_t> allowedPlayersOnNextRound;
	phmPlayerInfo_t findAllowedPlayerOnNextRound(const FString& playerName);
	bool registerAllowedPlayerOnNextRound(const phmPlayerInfo_t pPiSrc);
	bool removeAllowedPlayerOnNextRound(const FString& playerName);
	void clearAllowedPlayersOnNextRound(void);
	// }} 20070305 endround�� �����Ǵ� player�� play���

	TArray<hmTypeDesc_t> types; // ���� ��ϵ� actor type�鸸 HM�� �����ȴ�.
	
	bool bInit;
	hmState_t state;

	#ifdef UseNativeServer
	netCallback_t netcb;
	hostMigrationNet_t net;
	#endif

	bool bReadyToRestoreGRI;
	bool bActive;
	int nLocals;
	int nRemotes;
	TArray<phmData_t> nodes;	

	bool restore(ULevel* pLevel, AActor* pActor = 0x0);
	void clear(bool bClearHostInfo = false);
	void resetRound(void);
	bool backup(UObject* pObject, ULevel* pLevel);
	bool backup(UWorld* pWorld);

	virtual void onTick(void);

	bool init(void);
	void deinit(void);

	void (*_log)(TCHAR* str);
	void setLogFunc(void (*func)(TCHAR* str)) { _log = func;}
	void log(TCHAR* fmt, ...);

	TCHAR mapName[512];
	TCHAR playerName[512];	

	TCHAR url[512];
	FURL furl;	
		
	void makeUrl(void);
	void hostTraverse(TCHAR* playerName = 0x0);
	void clientTraverse(char* ip, TCHAR* playerName = 0x0);
	void endGameTraverse(void);

	//bool parsePlayerName(const FURL& url);
	//TCHAR* parseOption(const TCHAR* option, const FURL& url);
	void parseUrl(const FURL& url);

	phmData_t create(UObject* pObject, ULevel* pLevel);
	phmTypeDesc_t findType(const FString& name); //
	bool addType(
		const FString& name, 
		bool (*canCreate)(UObject* pObject), 
		phmData_t (*create)(struct hostMigration_t* pSender, UObject* pObject, ULevel* pLevel), 
		void (*testSuite)(struct hostMigration_t* pSender, UWorld* pWorld));

	void testSuite(UWorld* pWorld);

	// {{ 20061204 dEAthcURe
	int hmIndexSerial;
	bool bIndexPubEnabled;	
	void resetHmIndex(void);
	void setHmIndices(ULevel* pLevel);
	void setEnableIndexPub(bool bEnable = true);
	// }} 20061204 dEAthcURe

	bool bRestoreEnabled;
	void setEnableRestore(bool bEnable = true);

	phmPlayerBase_t findPlayerData(const TCHAR* playerName);

	void actorsListToFile(ULevel* pLevel, char* title = "hostMigration_t", char* fileName = "actorsList.txt");
	TCHAR lastValidActorName[256];
	bool findLastValidActor(ULevel* pLevel);

	bool removeNonBuiltinActors(ULevel* pLevel);
	void reinitBuiltinActors(ULevel* pLevel);
	
	bool gotoState(hmState_t state, DWORD param1 = 0x0, DWORD param2 = 0x0);

	// {{ 20070222
	TCHAR hostName[256];
	int hostTeamIdx; // 20070523
	void setHostName(TCHAR* pName, int teamIdx); // [+] 20070523	 
	// }} 20070222

	phmData_t pDataInProcessing; // 20070302
	void tick(ULevel* pLevel);

	bool checkDelayedLogin(phmPlayerBase_t pPlayerBase); 
	
	hostMigration_t(void);
	~hostMigration_t(void);
};
typedef struct hostMigration_t hostMigration_t;
typedef struct hostMigration_t* phostMigration_t;
//---------------------------------------------------------------------------
extern hostMigration_t g_hostMigration;
#define HmRegisterType(type) \
	g_hostMigration.addType(#type, type##::canCreate, type##::create, type##::testSuite);
//---------------------------------------------------------------------------
void _logToFile(char* fileName, TCHAR* fmt, ...);
//---------------------------------------------------------------------------
#endif
#endif