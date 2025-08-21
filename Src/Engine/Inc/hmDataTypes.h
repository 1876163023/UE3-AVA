//
// hmDataTypes.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifdef EnableHostMigration
#ifndef hmDataTypesH
#define hmDataTypesH
//---------------------------------------------------------------------------
#include "hostMigration.h"
//---------------------------------------------------------------------------
struct hmDroppingPickup_t : hmMissionObject_t {	
	BOOL bJustAddAmmo;
	BOOL bDrawInRadar;
	float Lifetime;
	int IndicatorType;
	int TeamIdx;

	virtual bool backup(AActor* pActor, FVector Location, FRotator Rotation);
	virtual bool restore(ULevel* pLevel);

	static bool create(AActor* pActor, FVector Location, FRotator Rotation);
};
typedef struct hmDroppingPickup_t hmDroppingPickup_t;
typedef struct hmDroppingPickup_t* phmDroppingPickup_t;
//---------------------------------------------------------------------------
struct hmPlayer_t : hmPlayerBase_t {
	virtual AActor* findOnLevel(ULevel* pLevel);
	// {{ [!] 20070309 static [+] 20070212
	static void backupWeaponModifier(FArchive& ar, AavaWeapon* pWeapon);
	static void restoreWeaponModifier(FArchive& ar, AavaWeapon* pWeapon, APawn* pPawn = 0x0); // [!] 20070905 static void restoreWeaponModifier(FArchive& ar, AavaWeapon* pWeapon, AavaPawn* pPawn = 0x0);
	// }} [!] 20070309 static [+] 20070212

	// {{ 20071011 base relative
	int hmIdxBase;
	FVector relativeLocation;
	// }} 20071011 base relative

	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);	
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);
	
	static void testSuite(phostMigration_t pSender, UWorld* pWorld);

	hmPlayer_t(UObject* pObject, ULevel* pLevel);	
};
typedef struct hmPlayer_t hmPlayer_t;
typedef struct hmPlayer_t* phmPlayer_t;
//---------------------------------------------------------------------------
struct hmLocalPlayer_t : hmPlayer_t {
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);

	static bool canCreate(UObject* pObject);		
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);
	
	virtual void onDestroy(phostMigration_t pSender);

	hmLocalPlayer_t(UObject* pObject, ULevel* pLevel);	
};
typedef struct hmLocalPlayer_t hmLocalPlayer_t;
typedef struct hmLocalPlayer_t* phmLocalPlayer_t;
//---------------------------------------------------------------------------
struct hmRemotePlayer_t : hmPlayer_t {
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);

	static bool canCreate(UObject* pObject);		
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);
	virtual void onDestroy(phostMigration_t pSender);

	hmRemotePlayer_t(UObject* pObject, ULevel* pLevel);
};
typedef struct hmRemotePlayer_t hmRemotePlayer_t;
typedef struct hmRemotePlayer_t* phmRemotePlayer_t;
//---------------------------------------------------------------------------
struct hmActor_t : hmData_t {
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);

	static bool canCreate(UObject* pObject);		
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);

	hmActor_t(UObject* pObject, ULevel* pLevel);
};
typedef struct hmActor_t hmActor_t;
typedef struct hmActor_t* phmActor_t;
//---------------------------------------------------------------------------
struct hmPickup_t : hmData_t {
	// AavaPickup/AavaKActor/AKActor/ADynamicSMActor/AActor
	virtual AActor* findOnLevel(ULevel* pLevel);
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(ULevel* pLevel);

	static bool canCreate(UObject* pObject);
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);

	// {{ SpawnActor params
	FString className;
	FString inventoryClassName;	
	// }} SpawnActor params

	hmPickup_t(UObject* pObject, ULevel* pLevel);
};
typedef struct hmPickup_t hmPickup_t;
typedef struct hmPickup_t* phmPickup_t;
//---------------------------------------------------------------------------
// {{ [+] 20070503
struct hmPickupProvider_t : hmData_t { 
	// 20070503
	// avaPickupProvider/TriggerVolume/Volume/Brush/Actor
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);	

	static bool canCreate(UObject* pObject);
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);

	FString className; // [+] 20070228
	FString inventoryClassName; // [+] 20070507

	hmPickupProvider_t(UObject* pObject, ULevel* pLevel);
};
typedef struct hmPickupProvider_t hmPickupProvider_t;
typedef struct hmPickupProvider_t* phmPickupProvider_t;
// }} [+] 20070503
//---------------------------------------------------------------------------
/* 20070214 작업보류
struct hmPickupC4_t : hmData_t {
	virtual AActor* findOnLevel(ULevel* pLevel);
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(ULevel* pLevel);

	static bool canCreate(UObject* pObject);
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);

	// {{ SpawnActor params
	FString className;
	FString inventoryClassName;	
	// }} SpawnActor params

	hmPickupC4_t(UObject* pObject, ULevel* pLevel);
};
typedef struct hmPickupC4_t hmPickupC4_t;
typedef struct hmPickupC4_t* phmPickupC4_t;
*/
//---------------------------------------------------------------------------
struct hmKActor_t : hmData_t { 
	// 20070214
	// AavaKActor/AKActor/ADynamicSMActor/AActor
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);	

	static bool canCreate(UObject* pObject);
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);

	FString className; // [+] 20070228

	hmKActor_t(UObject* pObject, ULevel* pLevel);
};
typedef struct hmKActor_t hmKActor_t;
typedef struct hmKActor_t* phmKActor_t;
//---------------------------------------------------------------------------
struct hmKActorSpawnable_t : hmData_t { 
	// 20070228
	// AKActorSpawnable/AKActor/ADynamicSMActor/AActor
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(ULevel* pLevel);

	static bool canCreate(UObject* pObject);
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);

	FString className;

	hmKActorSpawnable_t(UObject* pObject, ULevel* pLevel);
};
typedef struct hmKActorSpawnable_t hmKActorSpawnable_t;
typedef struct hmKActorSpawnable_t* phmKActorSpawnable_t;
//---------------------------------------------------------------------------
struct hmKBreakable_t : hmData_t { 
	//AavaKBreakable/AavaKActor/AKActor/ADynamicSMActor/AActor
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);

	static bool canCreate(UObject* pObject);
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);

	hmKBreakable_t(UObject* pObject, ULevel* pLevel);
};
typedef struct hmKBreakable_t hmKBreakable_t;
typedef struct hmKBreakable_t* phmKBreakable_t;
//---------------------------------------------------------------------------
struct hmProjectile_t : hmData_t {
	// AavaProjectile/AProjectile/AKActor
	virtual AActor* findOnLevel(ULevel* pLevel);
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(ULevel* pLevel);

	static bool canCreate(UObject* pObject);
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);

	// {{ SpawnActor params
	FString className;
	FString inventoryClassName;	
	// }} SpawnActor params

	hmProjectile_t(UObject* pObject, ULevel* pLevel);
};
typedef struct hmProjectile_t hmProjectile_t;
typedef struct hmProjectile_t* phmProjectile_t;
//---------------------------------------------------------------------------
struct hmKProjectile_t : hmData_t {
	virtual AActor* findOnLevel(ULevel* pLevel);
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(ULevel* pLevel);

	static bool canCreate(UObject* pObject);
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);

	// {{ SpawnActor params
	FString className;
	FString inventoryClassName;	
	// }} SpawnActor params

	hmKProjectile_t(UObject* pObject, ULevel* pLevel, bool bBackup = true);
};
typedef struct hmKProjectile_t hmKProjectile_t;
typedef struct hmKProjectile_t* phmKProjectile_t;
//---------------------------------------------------------------------------
// hmProj_C4_t
//---------------------------------------------------------------------------
struct hmProj_C4_t : hmKProjectile_t {
	// AavaProj_C4/AavaKProjectile/AavaKActor/AKActor/ADynamicSMActor/AActor
	virtual AActor* findOnLevel(ULevel* pLevel);
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);	
	virtual bool restore(ULevel* pLevel);

	// {{ 20070302
	AActor* pProj_C4;
	FString playerNameSetted;
	int idTeamSetted;
	bool bSetupSettedPlayerPended;
	void setSettedPlayer(AActor* pSender, FString playerName, int idTeam) {
		pProj_C4 = pSender;
		playerNameSetted = playerName;
		idTeamSetted = idTeam;
		bSetupPended = true;
	}
	// }} 20070302
	virtual bool tick(void); // 20070302

	static bool canCreate(UObject* pObject);
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);

	hmProj_C4_t(UObject* pObject, ULevel* pLevel);
};
typedef struct hmProj_C4_t hmProj_C4_t;
typedef struct hmProj_C4_t* phmProj_C4_t;
//---------------------------------------------------------------------------
struct hmGameInfo_t : hmData_t {	
	virtual AActor* findOnLevel(ULevel* pLevel);
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);

	AavaGame* pGameInfo;
	DWORD timeEndRound;
	virtual bool tick(void); // 20070303

	static bool canCreate(UObject* pObject);		
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);
	virtual bool canRestore(phostMigration_t pSender, ULevel* pLevel);
	virtual void onDestroy(phostMigration_t pSender);

	hmGameInfo_t(UObject* pObject, ULevel* pLevel);	
};
typedef struct hmGameInfo_t hmGameInfo_t;
typedef struct hmGameInfo_t* phmGameInfo_t;
//---------------------------------------------------------------------------
struct hmKismetState_t : hmData_t {	
	// AavaKismetState/AActor
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);

	static bool canCreate(UObject* pObject);		
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);	

	hmKismetState_t(UObject* pObject, ULevel* pLevel);	
};
typedef struct hmKismetState_t hmKismetState_t;
typedef struct hmKismetState_t* phmKismetState_t;
//---------------------------------------------------------------------------
// {{ 20070516
struct hmShatterGlassActor_t : hmData_t {	
	// AavaShatterGlassActor/AActor
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);

	static bool canCreate(UObject* pObject);		
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);	

	hmShatterGlassActor_t(UObject* pObject, ULevel* pLevel);	
};
typedef struct hmShatterGlassActor_t hmShatterGlassActor_t;
typedef struct hmShatterGlassActor_t* phmShatterGlassActor_t;
// }} 20070516
//---------------------------------------------------------------------------
// {{ 20070723
struct hmMatineeActor_t : hmData_t {	
	// AMatineeActor/AActor
	bool bMatineeHmTriggered;
	int hmId;
	AActor* _once_findOnLevel(ULevel* pLevel);
	virtual AActor* findOnLevel(ULevel* pLevel);
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);

	static bool canCreate(UObject* pObject);		
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);	

	hmMatineeActor_t(UObject* pObject, ULevel* pLevel);	
};
typedef struct hmMatineeActor_t hmMatineeActor_t;
typedef struct hmMatineeActor_t* phmMatineeActor_t;
// }} 20070723
//---------------------------------------------------------------------------
// {{ 20070801
struct hmVehicle_t : hmData_t {	
	// AavaVehicle/AavaVehicleBase/ASVehicle/AVehicle/APawn/AActor

	TArray<BYTE> WeaponBytes;
	bool bVehicleFactoryEventTriggered;
	int hmId;
	virtual AActor* findOnLevel(ULevel* pLevel);
	virtual void backup(UObject* pObject, ULevel* pLevel = 0x0);
	virtual bool restore(UObject* pObject, ULevel* pLevel = 0x0);

	static bool canCreate(UObject* pObject);		
	static phmData_t create(phostMigration_t pSender, UObject* pObject, ULevel* pLevel);	

	hmVehicle_t(UObject* pObject, ULevel* pLevel);	
};
typedef struct hmVehicle_t hmVehicle_t;
typedef struct hmVehicle_t* phmVehicle_t;
// }} 20070801
//---------------------------------------------------------------------------
// 20061122 ImplementHostMigration은 Game implementation module에 포함되어야한다.
// 계통을 잘보3, 순서엄수

// hmProj_C4_t          AavaProj_C4/AavaKProjectile/AavaKActor/AKActor/ADynamicSMActor/AActor
// hmKProjectile_t                  AavaKProjectile/AavaKActor/AKActor/ADynamicSMActor/AActor
// hmKBreakable_t                    AavaKBreakable/AavaKActor/AKActor/ADynamicSMActor/AActor
// hmPickup_t                            AavaPickup/AavaKActor/AKActor/ADynamicSMActor/AActor
// hmPickupProvider_t                         AavaPickupProvider/ATriggerVolume/ABrush/AActor // [+] 20070503
// hmKActor_t                                       AavaKActor/AKActor/ADynamicSMActor/AActor
// hmKActorSpawnable_t                        AKActorSpawnable/AKActor/ADynamicSMActor/AActor // [-][+] 20070228
// hmProjectile_t                                           AavaProjectile/AProjectile/AActor
// hmGameInfo_t    AavaGameReplicationInfo/AGameReplicationInfo/AReplicationInfo/AInfo/AActor

// hmKismetState_t                                                     AavaKismetState/AActor // [+] 20070419
// hmShatterGlassActor_t                                         AavaShatterGlassActor/AActor // [+] 20070419
// hmMatineeActor_t														 AMatineeActor/AActor // [+] 20070723
// hmVehicle_t                    AavaVehicle/AavaVehicleBase/ASVehicle/AVehicle/APawn/AActor // [+] 20070801

#define ImplementHostMigration \
	void hmRegisterTypes(void) { \
		HmRegisterType(hmLocalPlayer_t); \
		HmRegisterType(hmRemotePlayer_t); \
		HmRegisterType(hmProj_C4_t); \
		HmRegisterType(hmKProjectile_t); \
		HmRegisterType(hmKBreakable_t); \
		HmRegisterType(hmPickup_t); \
		HmRegisterType(hmPickupProvider_t); \
		HmRegisterType(hmKActor_t); \
		HmRegisterType(hmProjectile_t); \
		HmRegisterType(hmGameInfo_t); \
		HmRegisterType(hmKismetState_t); \
		HmRegisterType(hmShatterGlassActor_t); \
		HmRegisterType(hmMatineeActor_t); \
		HmRegisterType(hmVehicle_t); \
		HmRegisterType(hmActor_t); \
	}
//---------------------------------------------------------------------------
#endif
#endif