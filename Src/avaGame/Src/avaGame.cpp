/*=============================================================================
	avaGame.cpp
	Copyright  1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "PrecompiledHeaders.h"

// Includes.
#include "avaGame.h"

#include "../Src/ScenePrivate.h"

/*-----------------------------------------------------------------------------
	The following must be done once per package.
-----------------------------------------------------------------------------*/

#define STATIC_LINKING_MOJO 1

#include "avaActorComponents.h"
#include "avaBulletTrailComponent.h"
#include "avaFactories.h"
#include "avaShatterGlass.h"

#if WITH_NOVODEX
#include "../../engine/src/UnNovodexSupport.h"
#endif // WITH_NOVODEX

// Register things.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName AVAGAME_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "avaGameClasses.h"
#if !FINAL_RELEASE
#include "avaGameEditorClasses.h"
#endif
#include "avaGameParticleClasses.h"
#include "avaGameSequenceClasses.h"
#include "avaGameNetClasses.h"
#include "avaGameCameraClasses.h"
#include "avaGameVideoClasses.h"
#include "avaGameUIPrivateClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NAMES_ONLY

// Register natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "avaGameClasses.h"
#if !FINAL_RELEASE
#include "avaGameEditorClasses.h"
#endif
#include "avaGameParticleClasses.h"
#include "avaGameSequenceClasses.h"
#include "avaGameNetClasses.h"
#include "avaGameCameraClasses.h"
#include "avaGameVideoClasses.h"
#include "avaGameUIPrivateClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NATIVES_ONLY
#undef NAMES_ONLY

// avaEngineLoop.cpp에 있음
BEGIN_COMMANDLET(EncryptIni,avaGame)
END_COMMANDLET

#include "avaCommandlets.h"
#include "avaTransactions.h"

/**
 * Initialize registrants, basically calling StaticClass() to create the class and also 
 * populating the lookup table.
 *
 * @param	Lookup	current index into lookup table
 */
void AutoInitializeRegistrantsavaGame( INT& Lookup )
{
	AUTO_INITIALIZE_REGISTRANTS_AVAGAME;
	AUTO_INITIALIZE_REGISTRANTS_AVAGAME_PARTICLE;
	AUTO_INITIALIZE_REGISTRANTS_AVAGAME_NET;
	AUTO_INITIALIZE_REGISTRANTS_AVAGAME_SEQUENCE; // 20070420 dEAthcURe|HM
	AUTO_INITIALIZE_REGISTRANTS_AVAGAME_CAMERA;
	AUTO_INITIALIZE_REGISTRANTS_AVAGAME_UIPRIVATE;

#if !FINAL_RELEASE
	AUTO_INITIALIZE_REGISTRANTS_AVAGAME_EDITOR;
#endif

	AUTO_INITIALIZE_REGISTRANTS_AVAGAME_VIDEO;
	UavaDSPPresetFactoryNew::StaticClass();
	UavaSoundScapePropertyFactoryNew::StaticClass();
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesavaGame()
{
	#define NAMES_ONLY
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
    #define AUTOGENERATE_NAME(name) AVAGAME_##name = FName(TEXT(#name));
	#include "avaGameClasses.h"
#if !FINAL_RELEASE
#include "avaGameEditorClasses.h"
#endif
	#include "avaGameSequenceClasses.h"
	#include "avaGameParticleClasses.h"
	#include "avaGameNetClasses.h"	
	#include "avaGameCameraClasses.h"
	#include "avaGameVideoClasses.h"
	#include "avaGameUIPrivateClasses.h"

	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
}

#if _XENON_UTILITY_
    #pragma comment(lib, "XenonUtility.lib")
#endif

#if _TNT_

#endif

#if _XCR_
    #pragma comment(lib, "XeCR.lib")
#endif

// {{ dEAthcURe|HM
#ifdef EnableHostMigration
#include "hmMacro.h"
#include "hmDataTypes.h"
ImplementHostMigration
#endif
// }} dEAthcURe|HM

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(AavaGame);
IMPLEMENT_CLASS(AavaEmitter);
IMPLEMENT_CLASS(AavaReplicatedEmitter);
IMPLEMENT_CLASS(UavaCheatManager);
IMPLEMENT_CLASS(AavaPickupFactory);
IMPLEMENT_CLASS(AavaGameObjective);
IMPLEMENT_CLASS(UavaSkeletalMeshComponent);
IMPLEMENT_CLASS(AavaGameStats);
IMPLEMENT_CLASS(AavaPlayerReplicationInfo);
IMPLEMENT_CLASS(AavaLinkedReplicationInfo);
IMPLEMENT_CLASS(UavaPhysicalMaterialProperty);
IMPLEMENT_CLASS(UavaActorFactoryMover);
IMPLEMENT_CLASS(UavaStringHelper);
IMPLEMENT_CLASS(UclassIterator);
IMPLEMENT_CLASS(AavaCameraActor);
IMPLEMENT_CLASS(AavaPDACamNavPoint);
IMPLEMENT_CLASS(AavaMissionNavPoint);
IMPLEMENT_CLASS(UavaGameViewportClient);
IMPLEMENT_CLASS(UavaStateUI);
IMPLEMENT_CLASS(AavaEmit_Camera);
IMPLEMENT_CLASS(UavaTexture2DComposite);
IMPLEMENT_CLASS(AavaGameInfoMessage);

//IMPLEMENT_CLASS(AavaMissionObject); // 20061101 dEAthcURe|HM
//IMPLEMENT_CLASS(AavaPickup); // 20061128 dEAthcURe|HM
IMPLEMENT_CLASS(AavaProj_C4); // 20061207 dEAthcURe|HM
IMPLEMENT_CLASS(UavaModifier); // 20070212 dEAthcURe|HM

IMPLEMENT_CLASS(UavaUIAction_SetColorCorrectionParam);
IMPLEMENT_CLASS(AavaNavPoint_ColorCorrection);
IMPLEMENT_CLASS(AavaVolume_ColorCorrection);
IMPLEMENT_CLASS(AavaColorCorrectionManager);
IMPLEMENT_CLASS(AavaClassReplicationInfo);

IMPLEMENT_CLASS(UavaOptionSettings);
IMPLEMENT_CLASS(UavaEventTrigger);
IMPLEMENT_CLASS(AavaPickupProvider); // [+] 20070503 dEAthcURe|HM

// {{ 20070419 dEAthcURe|HM kismet state
IMPLEMENT_CLASS(AavaKismetState);
IMPLEMENT_CLASS(AavaKsSample);
// }} 20070419 dEAthcURe|HM kismet state

// {{ 20070420 dEAthcURe|HM kismet state
IMPLEMENT_CLASS(UavaSeqAct_SetHm);
IMPLEMENT_CLASS(UavaSeqVar_Hm);
// }} 20070420 dEAthcURe|HM kismet state

// {{ 20070423 dEAthcURe|HM kismet state
IMPLEMENT_CLASS(UavaSeqAct_SetHmInt);
IMPLEMENT_CLASS(UavaSeqVar_HmInt);
// }} 20070423 dEAthcURe|HM kismet state

// {{ 20070420 dEAthcURe|HM kismet state
void UavaSeqVar_Hm::retrieveValue(void)
{
	if(GWorld) {
		AWorldInfo* pWorldInfo = GWorld->GetWorldInfo();
		if(pWorldInfo) {
			AavaGameReplicationInfo* pGri = Cast<AavaGameReplicationInfo>(pWorldInfo->GRI);
			if(pGri) {
				if(pGri->eventfindHmVariable(VarName.ToString())!=-1) {
					bValue = 1;
					if(GIsGame && !GIsEditor)
						debugf(TEXT("[UavaSeqVar_Hm::retrieveValue] %s %s"), *VarName.ToString(), TEXT("true"));
				}
				else {
					bValue = 0;
					if(GIsGame && !GIsEditor)
						debugf(TEXT("[UavaSeqVar_Hm::retrieveValue] %s %s"), *VarName.ToString(), TEXT("false"));
				}
			}
		}
	}
}

UBOOL* UavaSeqVar_Hm::GetBoolRef()
{
	retrieveValue();
	return (UBOOL*)&bValue;
}

FString UavaSeqVar_Hm::GetValueStr()
{
	retrieveValue();
	return bValue == TRUE ? GTrue : GFalse;
}

void UavaSeqVar_Hm::PublishValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink)
{
	if(GIsGame && !GIsEditor)
		debugf(TEXT("[UavaSeqVar_Hm::PublishValue]"));
	Super::PublishValue(Op, Property, VarLink);
}

void UavaSeqVar_Hm::PopulateValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink)
{
	if(GIsGame && !GIsEditor)
		debugf(TEXT("[UavaSeqVar_Hm::PopulateValue]"));
	Super::PopulateValue(Op, Property, VarLink);
}

void UavaSeqAct_SetHm::Activated()
{
	// read new value
	UBOOL bValue = 1;
	TArray<UBOOL*> boolVars;
	GetBoolVars(boolVars,TEXT("Value"));
	if (boolVars.Num() > 0)
	{
		for (INT Idx = 0; Idx < boolVars.Num(); Idx++)
		{
			bValue = bValue && *(boolVars(Idx));
		}
	}
	else
	{
		// no attached variables, use default value
		bValue = DefaultValue;
	}
	// and apply the new value
	if(GWorld) {
		AWorldInfo* pWorldInfo = GWorld->GetWorldInfo();
		if(pWorldInfo) {
			AavaGameReplicationInfo* pGri = Cast<AavaGameReplicationInfo>(pWorldInfo->GRI);
			if(pGri && VariableLinks.Num()) {
				for (INT LinkIdx = 0; LinkIdx < VariableLinks(0).LinkedVariables.Num(); LinkIdx++) { // variablelinks(0) <- target
					if (VariableLinks(0).LinkedVariables(LinkIdx) != NULL) {
						FString varName = VariableLinks(0).LinkedVariables(LinkIdx)->VarName.ToString();
						debugf(TEXT("varName=%s"), *varName);
						if(bValue) pGri->eventsetHmVariable(varName);
						else pGri->eventresetHmVariable(varName);					
					}
				}
			}
		}
	}
}
// }} 20070420 dEAthcURe|HM kismet state

// {{ 20070423 dEAthcURe|HM kismet state
void UavaSeqVar_HmInt::retrieveValue(void)
{
	if(GWorld) {
		AWorldInfo* pWorldInfo = GWorld->GetWorldInfo();
		if(pWorldInfo) {
			AavaGameReplicationInfo* pGri = Cast<AavaGameReplicationInfo>(pWorldInfo->GRI);
			if(pGri) {
				int idx = pGri->findHmIntVar(VarName.ToString());
				if(idx==-1) {
					IntValue = -1;
					debugf(TEXT("[UavaSeqVar_HmInt::retrieveValue] not found %s"), *VarName.ToString());
					return;
				}

				TCHAR varName[512];				
				_stscanf(*pGri->HmIntVars[idx], TEXT("%s %d"), varName, &IntValue);

				debugf(TEXT("[UavaSeqVar_HmInt::retrieveValue] found %s %d"), *VarName.ToString(), IntValue);
			}
		}
	}
}

INT* UavaSeqVar_HmInt::GetIntRef()
{
	retrieveValue();
	return &IntValue;
}

FString UavaSeqVar_HmInt::GetValueStr()
{
	retrieveValue();
	return FString::Printf(TEXT("%d"),IntValue);
}

void UavaSeqVar_HmInt::PublishValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink)
{
	debugf(TEXT("[UavaSeqVar_HmInt::PublishValue]"));

	// USeqVar_Int에서 가져옴
	if (Op != NULL && Property != NULL)
	{
		TArray<INT*> IntVars;
		Op->GetIntVars(IntVars,*VarLink.LinkDesc);
		if (Property->IsA(UIntProperty::StaticClass()))
		{
			// first calculate the value
			INT Value = 0;
			for (INT Idx = 0; Idx < IntVars.Num(); Idx++)
			{
				Value += *(IntVars(Idx));
				
			}
			// apply the value to the property
			*(INT*)((BYTE*)Op + Property->Offset) = Value;
			debugf(TEXT("[UavaSeqVar_HmInt::PublishValue] Value=%d"), Value); //  test dd
		}
		else
		// if dealing with an array of ints
		if (Property->IsA(UArrayProperty::StaticClass()) &&
			((UArrayProperty*)Property)->Inner->IsA(UIntProperty::StaticClass()))
		{
			// grab the array
			UArrayProperty *ArrayProp = (UArrayProperty*)Property;
			INT ElementSize = ArrayProp->Inner->ElementSize;
			FArray *DestArray = (FArray*)((BYTE*)Op + ArrayProp->Offset);
			// resize it to fit the variable count
			DestArray->Empty(ElementSize, DEFAULT_ALIGNMENT, IntVars.Num());
			DestArray->AddZeroed(IntVars.Num(), ElementSize, DEFAULT_ALIGNMENT);
			for (INT Idx = 0; Idx < IntVars.Num(); Idx++)
			{
				// assign to the array entry
				*(INT*)((BYTE*)DestArray->GetData() + Idx * ElementSize) = *IntVars(Idx);
			}
		}
	}
}

void UavaSeqVar_HmInt::PopulateValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink)
{
	debugf(TEXT("[UavaSeqVar_HmInt::PopulateValue]"));
	if (Op != NULL && Property != NULL)
	{
		TArray<INT*> IntVars;
		Op->GetIntVars(IntVars,*VarLink.LinkDesc);
		if (Property->IsA(UIntProperty::StaticClass()))
		{
			INT Value = *(INT*)((BYTE*)Op + Property->Offset);
			for (INT Idx = 0; Idx < IntVars.Num(); Idx++)
			{
				*(IntVars(Idx)) = Value;
				debugf(TEXT("[UavaSeqVar_HmInt::PopulateValue] Value=%d"), Value); //  test dd
			}
		}
		else
		if (Property->IsA(UArrayProperty::StaticClass()) &&
			((UArrayProperty*)Property)->Inner->IsA(UIntProperty::StaticClass()))
		{
			// grab the array
			UArrayProperty *ArrayProp = (UArrayProperty*)Property;
			INT ElementSize = ArrayProp->Inner->ElementSize;
			FArray *SrcArray = (FArray*)((BYTE*)Op + ArrayProp->Offset);
			// write out as many entries as are attached
			for (INT Idx = 0; Idx < IntVars.Num() && Idx < SrcArray->Num(); Idx++)
			{
				*(IntVars(Idx)) = *(INT*)((BYTE*)SrcArray->GetData() + Idx * ElementSize);
			}
		}
	}
}

void UavaSeqAct_SetHmInt::Activated()
{
	// read new value
	INT intValue = -1;	
	TArray<INT*> intVars;
	GetIntVars(intVars,TEXT("Value"));
	if (intVars.Num() > 0)
	{
		intValue = *(intVars(0));		
		debugf(TEXT("value=%d"), intValue);
	}

	// and apply the new value
	if(GWorld) {
		AWorldInfo* pWorldInfo = GWorld->GetWorldInfo();
		if(pWorldInfo) {
			AavaGameReplicationInfo* pGri = Cast<AavaGameReplicationInfo>(pWorldInfo->GRI);
			if(pGri && VariableLinks.Num()) {
				for (INT LinkIdx = 0; LinkIdx < VariableLinks(0).LinkedVariables.Num(); LinkIdx++) { // variablelinks(0) <- target
					if (VariableLinks(0).LinkedVariables(LinkIdx) != NULL) {
						FString varName = VariableLinks(0).LinkedVariables(LinkIdx)->VarName.ToString();
						if(GIsGame && !GIsEditor)
							debugf(TEXT("varName=%s"), *varName);
						pGri->setHmIntVar(varName, intValue);						
					}
				}
			}
		}
	}
}
// }} 20070423 dEAthcURe|HM kismet state

void UavaGameViewportClient::Draw(FViewport* Viewport,FCanvas* Canvas)
{
	EShowFlags SavedShowFlags = ShowFlags;

	Super::Draw(Viewport, Canvas);

	ShowFlags = SavedShowFlags;
}

void UavaPhysicalMaterialProperty::PostLoad()
{
	__super::PostLoad();

	if (GIsUCC) return;

	for (INT i=0; i<ImpactEffects.Num(); ++i)
	{
		FMaterialImpactEffect& MIE = ImpactEffects(i);

		if (MIE.DecalMaterial != NULL && MIE.ImpactDecals.Num() == 0)
		{
			FImpactDecalData* IDD = new (MIE.ImpactDecals) FImpactDecalData;

			IDD->DecalMaterial = MIE.DecalMaterial;			
			IDD->DecalWidth = MIE.DecalWidth;
			IDD->DecalHeight = MIE.DecalHeight;

			MIE.DecalMaterial = NULL;
			
			MarkPackageDirty();
		}	
	}		
}

// {{ 20061128 dEAthcURe|HM
#ifdef EnableHostMigration
void AavaPickup::hmSerialize(FArchive& Ar)
{	
	// check 20070207
	//Autogenerated code by HmSerializeGenerator/generateSerializer.rb

	Super::hmSerialize(Ar);

	/* test disable 20070323 검증후삭제할것
	if (Ar.IsLoading() ) {
		_hms_defLoading;
		_hms_loadValue(bJustAddAmmo); // BITFIELD bJustAddAmmo:1;
		_hms_loadValue(bDoNotSwitch); // BITFIELD bDoNotSwitch:1 GCC_BITFIELD_MAGIC;
		_hms_loadValue(bDrawInRadar); // BITFIELD bDrawInRadar:1;
		_hms_loadValue(bDynamicSpawned); // BITFIELD bDynamicSpawned:1;
		_hms_loadValue(bDoNotRemove); // BITFIELD bDoNotRemove:1;
	}
	else {
		_hms_defSaving;
		_hms_saveValue(bJustAddAmmo); // BITFIELD bJustAddAmmo:1;
		_hms_saveValue(bDoNotSwitch); // BITFIELD bDoNotSwitch:1 GCC_BITFIELD_MAGIC;
		_hms_saveValue(bDrawInRadar); // BITFIELD bDrawInRadar:1;
		_hms_saveValue(bDynamicSpawned); // BITFIELD bDynamicSpawned:1;
		_hms_saveValue(bDoNotRemove); // BITFIELD bDoNotRemove:1;
	}

	Ar << MaxAmmo; // INT MaxAmmo;
	Ar << TeamIdx; // BYTE TeamIdx;
	//Ar << StaticMeshName; // FStringNoInit StaticMeshName; //!
	Ar << nAddToLvl; // INT nAddToLvl;
	//Ar << SocMeshName; // FStringNoInit SocMeshName; //!
	Ar << nRemoveFromLvl; // INT nRemoveFromLvl;
	Ar << IconCode; // INT IconCode;
	Ar << RespawnTime; // FLOAT RespawnTime;

	//{{other values
	// class AInventory* Inventory;
	// class UClass* InventoryClass;
	// class UCylinderComponent* CylinderComponent;
	// class USkeletalMeshComponent* SocMesh;
	// TArrayNoInit<class UStaticMeshComponent*> Items;
	// class AavaPickUp_Indicator* Indicator;
	//}}other values
	*/
}
#endif
// }} 20061128 dEAthcURe|HM

void AavaPickupFactory::TickSpecial( FLOAT DeltaSeconds )
{
	Super::TickSpecial(DeltaSeconds);

	if ( bRotatingPickup && PickupMesh && (GWorld->GetNetMode() != NM_DedicatedServer) && !PickupMesh->HiddenGame && (GWorld->GetWorldInfo()->TimeSeconds - LastRenderTime < 0.2f) )
	{
		PickupMesh->Rotation.Yaw += appRound(DeltaSeconds * YawRotationRate);
		// this is just visual, so we don't need to update it immediately
		PickupMesh->BeginDeferredUpdateTransform();
	}
}

//======================================================================
// Commandlet for level error checking
IMPLEMENT_CLASS(UavaLevelCheckCommandlet)

INT UavaLevelCheckCommandlet::Main( const FString& Params )
{
	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if( !PackageList.Num() )
		return 0;

	// Iterate over all level packages.
	FString MapExtension = GConfig->GetStr(TEXT("URL"), TEXT("MapExt"), GEngineIni);
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		const FFilename& Filename = PackageList(PackageIndex);

		if (Filename.GetExtension() == MapExtension)
		{
			warnf(NAME_Log, TEXT("Loading %s"), *Filename);

			// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
			UPackage*	Package = CastChecked<UPackage>(UObject::LoadPackage( NULL, *Filename, 0 ));

			// We have to use GetPackageLinker as GetLinker() returns NULL for top level packages.
			BeginLoad();
			ULinkerLoad* Linker	= UObject::GetPackageLinker( Package, NULL, 0, NULL, NULL );
			EndLoad();

			UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
			if ( World )
			{
				AWorldInfo* WorldInfo = World->GetWorldInfo();
				if ( WorldInfo )
					warnf(NAME_Log,TEXT("Gravity %f"),WorldInfo->GlobalGravityZ);
			}

			// Garbage collect to restore clean plate.
			//UObject::CollectGarbage(RF_Native);
		}
	}
	return 0;
}

//======================================================================
// Commandlet for replacing one kind of actor with another kind of actor, copying changed properties from the most-derived common superclass
IMPLEMENT_CLASS(UavaReplaceActorCommandlet)

INT UavaReplaceActorCommandlet::Main(const FString& Params)
{
	const TCHAR* Parms = *Params;

	// get the specified filename/wildcard
	FString PackageWildcard;
	if (!ParseToken(Parms, PackageWildcard, 0))
	{
		appErrorf(TEXT("Syntax: ucc utreplaceactor <file/wildcard> <Package.Class to remove> <Package.Class to replace with>"));
	}

	// find all the files matching the specified filename/wildcard
	TArray<FString> FilesInPath;
	GFileManager->FindFiles(FilesInPath, *PackageWildcard, 1, 0);
	if (FilesInPath.Num() == 0)
	{
		appErrorf(TEXT("No packages found matching %s!"), *PackageWildcard);
	}
	// get the directory part of the filename
	INT ChopPoint = Max(PackageWildcard.InStr(TEXT("/"), 1) + 1, PackageWildcard.InStr(TEXT("\\"), 1) + 1);
	if (ChopPoint < 0)
	{
		ChopPoint = PackageWildcard.InStr( TEXT("*"), 1 );
	}
	FString PathPrefix = (ChopPoint < 0) ? TEXT("") : PackageWildcard.Left(ChopPoint);

	// get the class to remove and the class to replace it with
	FString ClassName;
	if (!ParseToken(Parms, ClassName, 0))
	{
		appErrorf(TEXT("Syntax: ucc utreplaceactor <file/wildcard> <Package.Class to remove> <Package.Class to replace with>"));
	}
	UClass* ClassToReplace = (UClass*)StaticLoadObject(UClass::StaticClass(), ANY_PACKAGE, *ClassName, NULL, LOAD_NoWarn | LOAD_Quiet, NULL);
	if (ClassToReplace == NULL)
	{
		appErrorf(TEXT("Invalid class to remove: %s"), *ClassName);
	}
	else
	{
		ClassToReplace->AddToRoot();
	}
	if (!ParseToken(Parms, ClassName, 0))
	{
		appErrorf(TEXT("Syntax: ucc utreplaceactor <file/wildcard> <Package.Class to remove> <Package.Class to replace with>"));
	}
	UClass* ReplaceWithClass = (UClass*)StaticLoadObject(UClass::StaticClass(), ANY_PACKAGE, *ClassName, NULL, LOAD_NoWarn | LOAD_Quiet, NULL);
	if (ReplaceWithClass == NULL)
	{
		appErrorf(TEXT("Invalid class to replace with: %s"), *ClassName);
	}
	else
	{
		ReplaceWithClass->AddToRoot();
	}

	// find the most derived superclass common to both classes
	UClass* CommonSuperclass = NULL;
	for (UClass* BaseClass1 = ClassToReplace; BaseClass1 != NULL && CommonSuperclass == NULL; BaseClass1 = BaseClass1->GetSuperClass())
	{
		for (UClass* BaseClass2 = ReplaceWithClass; BaseClass2 != NULL && CommonSuperclass == NULL; BaseClass2 = BaseClass2->GetSuperClass())
		{
			if (BaseClass1 == BaseClass2)
			{
				CommonSuperclass = BaseClass1;
			}
		}
	}
	checkSlow(CommonSuperclass != NULL);

	for (INT i = 0; i < FilesInPath.Num(); i++)
	{
		FString PackageName = FilesInPath(i);
		// get the full path name to the file
		FString FileName = PathPrefix + PackageName;
		// skip if read-only
		if (GFileManager->IsReadOnly(*FileName))
		{
			warnf(TEXT("Skipping %s (read-only)"), *FileName);
		}
		else
		{
			// load the package
			warnf(TEXT("Loading %s..."), *PackageName); 
			UPackage* Package = LoadPackage(NULL, *PackageName, LOAD_None);

			// load the world we're interested in
			GWorld = FindObject<UWorld>(Package, TEXT("TheWorld"));
			if (GWorld == NULL)
			{
				warnf(TEXT("Skipping %s (not a map)"));
			}
			else
			{
				// add the world to the root set so that the garbage collection to delete replaced actors doesn't garbage collect the whole world
				GWorld->AddToRoot();
				// initialize the levels in the world
				GWorld->Init();
				GWorld->GetWorldInfo()->PostEditChange(NULL);
				// iterate through all the actors in the world, looking for matches with the class to replace (must have exact match, not subclass)
				for (FActorIterator It; It; ++It)
				{
					if (It->GetClass() == ClassToReplace)
					{
						// replace an instance of the old actor
						warnf(TEXT("Replacing actor %s"), *It->GetName());
						// make sure we spawn the new actor in the same level as the old
						//@warning: this relies on the outer of an actor being the level
						GWorld->CurrentLevel = It->GetLevel();
						checkSlow(GWorld->CurrentLevel != NULL);
						// spawn the new actor
						AActor* NewActor = GWorld->SpawnActor(ReplaceWithClass, NAME_None, It->Location, It->Rotation, NULL, true);
						// copy non-native non-transient properties common to both that were modified in the old actor to the new actor
						for (UProperty* Property = CommonSuperclass->PropertyLink; Property != NULL; Property = Property->PropertyLinkNext)
						{
							//@note: skipping properties containing components - don't have a reasonable way to deal with them and the instancing mess they create
							if ( !(Property->PropertyFlags & CPF_Native) && !(Property->PropertyFlags & CPF_Transient) &&
								!(Property->PropertyFlags & CPF_Component) &&
								!Property->Identical((BYTE*)*It + Property->Offset, (BYTE*)It->GetClass()->GetDefaultObject() + Property->Offset) )
							{
								Property->CopyCompleteValue((BYTE*)NewActor + Property->Offset, (BYTE*)*It + Property->Offset);
							}
						}
						// check for any references to the old Actor and replace them with the new one
						TMap<AActor*, AActor*> ReplaceMap;
						ReplaceMap.Set(*It, NewActor);
						FArchiveReplaceObjectRef<AActor> ReplaceAr(GWorld, ReplaceMap, false, false, false);
						// destroy the old actor
						GWorld->DestroyActor(*It);
					}
					else
					{
						// check for any references to the old class and replace them with the new one
						TMap<UClass*, UClass*> ReplaceMap;
						ReplaceMap.Set(ClassToReplace, ReplaceWithClass);
						FArchiveReplaceObjectRef<UClass> ReplaceAr(*It, ReplaceMap, false, false, false);
						if (ReplaceAr.GetCount() > 0)
						{
							warnf(TEXT("Replaced %i class references in actor %s"), ReplaceAr.GetCount(), *It->GetName());
						}
					}
				}

				// collect garbage to delete replaced actors and any objects only referenced by them (components, etc)
				GWorld->PerformGarbageCollection();

				// save the world
				warnf(TEXT("Saving %s..."), *PackageName);
				SavePackage(Package, GWorld, 0, *FileName, GWarn);
				// clear GWorld by removing it from the root set and replacing it with a new one
				GWorld->RemoveFromRoot();
				UWorld::CreateNew();
			}
		}

		// get rid of the loaded world
		warnf(TEXT("Cleaning up..."));
		CollectGarbage(RF_Native);
	}

	return 0;
}

void AavaProjectile::TickSpecial( FLOAT DeltaSeconds )
{
	Super::TickSpecial(DeltaSeconds);

//! Projectile::TickSpecial에서 이미 처리하고 있다.
/*
	if ( bRotationFollowsVelocity )
	{
		Rotation = Velocity.Rotation();
	}
*/
}

FLOAT AavaProjectile::GetGravityZ()
{
	return Super::GetGravityZ() * CustomGravityScale;
}

/** Util for getting the PhysicalMaterial applied to this KActor's StaticMesh. */
UPhysicalMaterial* AavaProjectile::GetActorPhysMaterial()
{
	UPhysicalMaterial* PhysMat = GEngine->DefaultPhysMaterial;
	if( StaticMeshComponent->PhysMaterialOverride )
	{
		PhysMat = StaticMeshComponent->PhysMaterialOverride;
	}
	else if(	StaticMeshComponent->StaticMesh && 
				StaticMeshComponent->StaticMesh->BodySetup && 
				StaticMeshComponent->StaticMesh->BodySetup->PhysMaterial )
	{
		PhysMat = StaticMeshComponent->StaticMesh->BodySetup->PhysMaterial;
	}

	return PhysMat;
}


// {{ dEAthcURe|HM
#ifdef EnableHostMigration
void AavaProj_C4::hmSerialize(FArchive& Ar)
{
	// check 20070207
	//Autogenerated code by HmSerializeGenerator/generateSerializer.rb

	Super::hmSerialize(Ar);

	/* test disable 20070323 검증후삭제할것
	if (Ar.IsLoading() ) {
		_hms_defLoading;
		_hms_loadValue(LastAlert); // BITFIELD LastAlert:1;
	}
	else {
		_hms_defSaving;
		_hms_saveValue(LastAlert); // BITFIELD LastAlert:1;
	}

	Ar << DebugDefusing; // FLOAT DebugDefusing;
	Ar << RemainForDefuse; // FLOAT RemainForDefuse;
	Ar << LastAlertChangeTime; // FLOAT LastAlertChangeTime;
	Ar << RemainForExplode; // FLOAT RemainForExplode;
	Ar << DefuseTime; // FLOAT DefuseTime;
	Ar << DefusingTime; // FLOAT DefusingTime;
	Ar << DebugExplode; // FLOAT DebugExplode;
	Ar << ExplodeTime; // FLOAT ExplodeTime;

	//{{other values
	// class APlayerReplicationInfo* SettedPRI;
	// class AavaPlayerReplicationInfo* DefusingPRI;
	// class APlayerController* LocalPC;
	// class USoundCue* BombAlertSound;
	// class ATriggerVolume* BombVolume;
	//}}other values
	*/

	if (Ar.IsLoading() ) {
		int bBombVolume;		
		Ar << bBombVolume;
		if(bBombVolume) {
			FString bvClassName;			
			FVector location;

			Ar << bvClassName;
			Ar << location;

			int idxStart = 0;
			ATriggerVolume* pTriggerVolume = 0x0;		
			do {
				pTriggerVolume = Cast<ATriggerVolume>(GWorld->CurrentLevel->findActorByClassName(bvClassName, idxStart));
				if(pTriggerVolume) {					
					FVector diff = location - pTriggerVolume->Location;
					if(diff.Size() < 10.0f) {				
						BombVolume = pTriggerVolume;
						break;
					}
				}			
			} while(pTriggerVolume);
		}

		int bSettedPRI;
		Ar << bSettedPRI;
		if(bSettedPRI) {
			FString settedPlayerName;
			int idTeam;

			Ar << settedPlayerName;
			Ar << idTeam;

			// AavaProj_C4의 SettedPRI를 세팅하기 위해 pending
			if(g_hostMigration.pDataInProcessing) {
				phmProj_C4_t pHmC4 = (phmProj_C4_t)g_hostMigration.pDataInProcessing;
				if(pHmC4) {
					pHmC4->setSettedPlayer(this, settedPlayerName, idTeam);					
				}
			}
			// AavaProj_C4의 SettedPRI를 세팅하기 위해 pending
		}
	}
	else {
		int bBombVolume = false;
		if(BombVolume) {			
			FString bvClassName = BombVolume->GetClass()->GetName();			
			FVector location = BombVolume->Location;			

			bBombVolume = true;

			Ar << bBombVolume;
			Ar << bvClassName;
			Ar << location;
		}
		else {
			Ar << bBombVolume; // false
		}

		int bSettedPRI = false;
		if(SettedPRI) {
			FString settedPlayerName = SettedPRI->PlayerName;
			int idTeam = SettedPRI->TeamID;

			bSettedPRI = true;
			Ar << bSettedPRI;
			Ar << settedPlayerName;
			Ar << idTeam;
		}
		else {
			Ar << bSettedPRI; // false
		}
	}
}
#endif
// }} dEAthcURe|HM

// {{ [+] 20070212 dEAthcURe|HM

#ifdef EnableHostMigration
void UavaModifier::hmSerialize(FArchive& Ar)
{
	//Autogenerated code by HmSerializeGenerator/generateSerializer.rb

	//Super::hmSerialize(Ar); // leave it disabled	

	Ar << Id; // INT Id;

	//{{other values
	// TArrayNoInit<struct FExtraMesh> CommonExtraMeshes;
	// TArrayNoInit<struct FAttachedItem> CommonAttachedItems;
	//}}other values
}
#endif
// }} [+] 20070212 dEAthcURe|HM

FString AavaGameStats::GetMapFilename()
{
	return GWorld->URL.Map;
}

AActor* UavaActorFactoryMover::CreateActor(const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData)
{
	AActor* Actor = Super::CreateActor(Location, Rotation, ActorFactoryData);

	if (bCreateKismetEvent && Actor != NULL && Actor->SupportedEvents.FindItemIndex(EventClass) != INDEX_NONE)
	{
		USequence* GameSequence = GWorld->GetGameSequence();
		if (GameSequence != NULL)
		{
			GameSequence->Modify();

			// create a new sequence to put the new event in
			USequence* NewSequence = ConstructObject<USequence>(USequence::StaticClass(), GameSequence, Actor->GetFName(), RF_Transactional);
			NewSequence->ParentSequence = GameSequence;
			GameSequence->SequenceObjects.AddItem(NewSequence);
			NewSequence->ObjName = NewSequence->GetName();
			NewSequence->OnCreated();
			NewSequence->Modify();

			// now create the event
			USequenceEvent* NewEvent = ConstructObject<USequenceEvent>(EventClass, NewSequence, NAME_None, RF_Transactional);
			NewEvent->ObjName = FString::Printf(TEXT("%s %s"), *Actor->GetName(), *NewEvent->ObjName);
			NewEvent->Originator = Actor;
			NewEvent->ParentSequence = NewSequence;
			NewSequence->SequenceObjects.AddItem(NewEvent);
			NewEvent->OnCreated();
			NewEvent->Modify();
		}
	}

	return Actor;
}

//FString AavaVoteSystem::ParsePlayerName(const FString& PlayerName, const FString& Message)
//{
//	FString Str,Temp;
//	INT Offset;
//
//	Temp = Message;
//
//	Offset = Temp.InStr(TEXT("%s"));
//	if (Offset != -1)
//	{
//		Str = Temp.Left(Offset);
//		Str += PlayerName;
//		Str += Temp.Right(Temp.Len() - Offset - 2);
//	}
//	return Str;
//}
//
//FString AavaVoteSystem::ParseTimeLeft(const FString& fTimeLeft, const FString& Message)
//{
//	FString Str,Temp;
//	INT Offset;
//
//	Temp = Message;
//
//	Offset = Temp.InStr(TEXT("%t"));
//	if (Offset != -1)
//	{
//		Str = Temp.Left(Offset);
//		Str += fTimeLeft;
//		Str += Temp.Right(Temp.Len() - Offset - 2);
//	}
//	return Str;
//}
//
//FString AavaVoteSystem::ParseVoteCount(const FString& nAccepts, const FString& nDenies, const FString& Message)
//{
//	FString Str,Temp;
//	INT Offset;
//
//	Temp = Message;
//
//	Offset = Temp.InStr(TEXT("%n"));
//	if (Offset != -1)
//	{
//		Str = Temp.Left(Offset);
//		Str += nAccepts;
//		Str += Temp.Right(Temp.Len() - Offset - 2);
//		Temp = Str;
//	}
//
//	Offset = Temp.InStr(TEXT("%n"));
//	if (Offset != -1)
//	{
//		Str = Temp.Left(Offset);
//		Str += nDenies;
//		Str += Temp.Right(Temp.Len() - Offset - 2);
//	}
//
//	return Str;
//}

FString UavaStringHelper::Replace(const FString & Replace,const FString& Match, const FString& Target)
{
	FString Str,Temp;
	INT Offset;
	Temp = Target;

	Offset = Temp.InStr(Match);
	if (Offset != INDEX_NONE)
	{
		Str = Temp.Left(Offset);
		Str += Replace;
		Str += Temp.Right(Temp.Len() - Offset - Match.Len());
	}
	else
		Str = Target;

	return Str;
}

FString UavaStringHelper::GetString(const FString& SourceName)
{
	return SourceName;
}

FString UavaStringHelper::Trim( const FString& SourceStr )
{
	FString DestStr = SourceStr;

	DestStr = DestStr.Trim();
	DestStr = DestStr.TrimTrailing();

	return DestStr;
}

FString UavaStringHelper::TrimQuotes( const FString& SourceStr, INT& QuotesRemoved )
{
	FString DestStr = SourceStr;
	
	UBOOL bQuotesRemoved = FALSE;
	DestStr = DestStr.TrimQuotes(&bQuotesRemoved);

	QuotesRemoved = (INT)bQuotesRemoved;
	return DestStr;
}

FString UavaStringHelper::PackString(const TArray<FString>& StringsToPack,const FString& Delim/*=TEXT("|")*/)
{
	FString Result;
	for( INT i = 0 ; i < StringsToPack.Num() - 1 ; i++ )
		Result += (StringsToPack(i) + Delim);

	if( StringsToPack.Num() > 0 )
		Result += StringsToPack(StringsToPack.Num() - 1);

	return Result;
}

UBOOL UclassIterator::FindSubClass(class UClass* BaseClass,TArray<class UClass*>& SubClassList)
{
	for( TObjectIterator<UClass> It ; It ; ++It )
	{
		if( It->IsChildOf(BaseClass) && !(It->ClassFlags&CLASS_Abstract) )
		{
			SubClassList.AddItem( *It );
		}
	}
	return !( SubClassList.Num() == 0 );
}

//  [8/31/2006 otterrrr]
// Picking광선의 시작점(RayOrg)과 방향(RayDir)을 반환해준다.
void AavaCameraActor::CalcPickRay(int ScreenX, int ScreenY, FVector& RayOrg, FVector& RayDir)
{
	FLOAT px, py;
	FVector ViewLoc;
	FRotator ViewRot;
	FSceneView *SceneView;

	ULocalPlayer* Player = GEngine->GamePlayers(0);				// Assume that the index of LocalPlayer is '0'
	FViewport* Viewport = Player->ViewportClient->Viewport;
	QWORD ShowFlags = Player->ViewportClient->ShowFlags;

	FSceneViewFamilyContext ViewFamily(Viewport ,GWorld->Scene, ShowFlags ,GWorld->GetTimeSeconds(),GWorld->GetRealTimeSeconds(), GWorld->bGatherDynamicShadowStatsSnapshot ? &GWorld->DynamicShadowStats : NULL, TRUE );
	SceneView = Player->CalcSceneView( &ViewFamily, ViewLoc, ViewRot, Viewport);

	px =  (((2.0f * ScreenX) / SceneView->SizeX) - 1.0f) / (SceneView->ProjectionMatrix.M[0][0]);
	py =  (((-2.0f * ScreenY) / SceneView->SizeY) + 1.0f) / (SceneView->ProjectionMatrix.M[1][1]);

	RayOrg = FVector(0,0,0);
	RayDir = FVector(px, py, 1.0f);

	RayOrg = SceneView->ViewMatrix.Inverse().TransformFVector(RayOrg);
	RayDir = SceneView->ViewMatrix.Inverse().TransformNormal(RayDir);
	RayDir.Normalize();
}
//  [8/31/2006 otterrrr]

//  [9/11/2006 otterrrr]
// avaStateUI를 그대로 유지하기 위해 MessagePointer를 저장하고 RedirectMessages를 구현
void UavaStateUI::RedirectMessages(TArray<struct FStateUIMessage>& NewMessageRef)
{
	pMessage = &NewMessageRef;
}

// it doesn't works. when FString::operator=
//void UavaStateUI::NativeAppendMessage(const FString& Text,FLOAT Lifetime,FColor TextColor)
//{
//	FStateUIMessage UIMessage;
//
//	UIMessage.Text = Text;
//	UIMessage.Life = Lifetime;
//	UIMessage.Color = TextColor;
//
//	pMessage->AddItem(UIMessage);
//}

void UavaStateUI::NativeAppendMessage(struct FStateUIMessage NewMessage)
{
	if(pMessage)
		pMessage->Push(NewMessage);
	else
		warnf(TEXT("Failed to AppendMessage"));
}

void UavaStateUI::ClearMessage()
{
	if(pMessage)
		pMessage->Reset();
	else
		warnf(TEXT("Failed to ClearMessage"));
}
//  [9/11/2006 otterrrr]

// Camera 붙이는 Emitter 이다... GOW 에서 가지고 왔음
void AavaEmit_Camera::UpdateLocation( const FVector& CamLoc, const FRotator& CamRot, FLOAT CamFOVDeg )
{
	FRotationMatrix M(CamRot);

	FVector	const X = M.GetAxis(0);
	M.Mirror(AXIS_None,AXIS_X);

	// not 100% clear on why this is necessary, but...
	FRotator	NewRot = M.Rotator();
	NewRot.Roll = -NewRot.Roll;

	// base dist assumes fov of 80 deg.  arbitrary, really
	FLOAT DistAdjustedForFOV = DistFromCamera * appTan( float(80.f*0.5f*PI/180.f))/appTan(float(CamFOVDeg*0.5f*PI/180.f));

	SetLocation( CamLoc + X * DistAdjustedForFOV );
	SetRotation( NewRot );

	// have to do this, since in UWorld::Tick the actors do the component update for all
	// actors before the camera ticks.  without this, the blood appears to be a frame behind
	// the camera.
	ConditionalUpdateComponents();
}

//  [2006/11/24 윤태식, 추가] 
static ULocalPlayer* GetLocalPlayer()
{
	ULocalPlayer* LocalPlayer = NULL;
	if( GEngine && GEngine->GamePlayers.Num() > 0 )
		LocalPlayer = GEngine->GamePlayers(0);
	return LocalPlayer;
}

void UavaUIAction_SetColorCorrectionParam::Activated()
{
	FSceneViewState* ViewState = (FSceneViewState*)(GetLocalPlayer()->ViewState);
	
}
//  [2006/11/24 윤태식, 추가끝]

//  [2006/12/1 윤태식, 추가]

void AavaColorCorrectionManager::ActivateColorArea(int id, FLOAT Weight)
{	
}

void AavaColorCorrectionManager::DeactivateColorArea(int id)
{
}

void AavaColorCorrectionManager::CreateTexture(INT& id, FLOAT Weight, BYTE PixelFormat, FLOAT Hue, FLOAT Sat, FLOAT Light, FLOAT Constrast, FVector Shadows, FVector Highlights, FVector MidTones, FLOAT Desaturation, UBOOL bSetStrictWeight)
{
}

void AavaNavPoint_ColorCorrection::PostEditChange( UProperty* PropertyThatChanged )
{
	if(! PropertyThatChanged )
		return;

	if( PropertyThatChanged->GetFName() == FName(TEXT("Hue")) )
	{
		Clamp(Hue, -180.0f, 180.0f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("Saturation")))
	{
		Clamp(Saturation,-100.0f, 100.0f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("Lightness")))
	{
		Clamp(Lightness, -100.0f, 100.0f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("Contrast")))
	{
		Clamp( Contrast, -128.0f, 127.0f );
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("Shadows")))
	{
		Clamp(Shadows.X, -1.0f,1.0f);
		Clamp(Shadows.Y, -1.0f,1.0f);
		Clamp(Shadows.Z, -1.0f,1.0f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("Highlights")))
	{
		Clamp(HighLights.X, 0.2f, 5.0f);
		Clamp(HighLights.Y, 0.2f, 5.0f);
		Clamp(HighLights.Z, 0.2f, 5.0f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("MidTones")))
	{
		Clamp(MidTones.X, 0.5f, 1.5f);
		Clamp(MidTones.Y, 0.5f, 1.5f);
		Clamp(MidTones.Z, 0.5f, 1.5f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("Desaturation")))
	{
		Clamp(Desaturation, 0.0f, 1.0f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("FalloffStartDistance")))
	{
		FalloffStartDistance = FalloffStartDistance < 0.0f ? 0.0f : FalloffStartDistance;
		FalloffEndDistance = FalloffEndDistance < FalloffStartDistance ? FalloffStartDistance : FalloffEndDistance;
		GoodSprite->Scale = FalloffStartDistance / GoodSprite->Sprite->GetSurfaceWidth();
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("FalloffEndDistance")))
	{
		FalloffEndDistance = FalloffEndDistance < FalloffStartDistance ? FalloffStartDistance : FalloffEndDistance;
		GoodSprite->Scale = GoodSprite->Scale = FalloffStartDistance / GoodSprite->Sprite->GetSurfaceWidth();
	}
}

void AavaVolume_ColorCorrection::PostEditChange( UProperty* PropertyThatChanged )
{
	if(! PropertyThatChanged )
		return;

	if( PropertyThatChanged->GetFName() == FName(TEXT("Hue")) )
	{
		Clamp(Hue, -180.0f, 180.f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("Saturation")))
	{
		Clamp(Saturation, -100.0f, 100.0f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("Lightness")))
	{
		Clamp(Lightness, -100.0f, 100.0f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("Contrast")))
	{
		Clamp(Contrast, -128.0f, 127.0f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("Shadows")))
	{
		Clamp(Shadows.X, -1.0f,1.0f);
		Clamp(Shadows.Y, -1.0f,1.0f);
		Clamp(Shadows.Z, -1.0f,1.0f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("Highlights")))
	{
		Clamp(HighLights.X, 0.2f, 5.0f);
		Clamp(HighLights.Y, 0.2f, 5.0f);
		Clamp(HighLights.Z, 0.2f, 5.0f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("MidTones")))
	{
		Clamp(MidTones.X, 0.5f, 1.5f);
		Clamp(MidTones.Y, 0.5f, 1.5f);
		Clamp(MidTones.Z, 0.5f, 1.5f);
	}
	else if (PropertyThatChanged->GetFName() == FName(TEXT("Desaturation")))
	{
		Clamp(Desaturation, 0.0f, 1.0f);
	}
}
//  [2006/12/1 윤태식, 추가끝]

//  [2006/12/6 윤태식, 추가]
void AavaColorCorrectionManager::SetSampleMode(UBOOL bSetSampleMode)
{
}

void AavaColorCorrectionManager::ClearColorArea()
{	
}

void appPreCreateViewportFrame()
{
	UavaOptionSettings* OptionSettings = Cast<UavaOptionSettings>(UavaOptionSettings::StaticClass()->ClassDefaultObject);
	check( OptionSettings && GEngine );

	GEngine->Client->StartupResolutionX = OptionSettings->StartUpResX;
	GEngine->Client->StartupResolutionY = OptionSettings->StartUpResY;
}

//  [2006/12/6 윤태식, 추가끝]

AWorldInfo* UavaEventTrigger::GetWorldInfo()
{
	if( GWorld != NULL)
		return GWorld->GetWorldInfo();
	return NULL;
}

void UavaTexture2DComposite::UpdateCompositeTextueEx(int nSizeX, int nSizeY, int NumMipsToGenerate)
{
	// initialize the list of valid regions
	InitValidSourceRegions();

	if( ValidRegions.Num() == 0 )
	{
		debugf( NAME_Warning, TEXT("UTexture2DComposite: no regions to process") );
	}
	else
	{
		// calc index of first available mip common to the set of source textures
		INT FirstSrcMipIdx = GetFirstAvailableMipIndex();

		// calc the texture size for the comp texture based on the MaxLODBias
		INT SrcSizeX = nSizeX >> FirstSrcMipIdx;
		INT SrcSizeY = nSizeY >> FirstSrcMipIdx;
		// use the same format as the source textures
		EPixelFormat SrcFormat = (EPixelFormat)ValidRegions(0).Texture2D->Format;

		// re-init the texture and add first mip
		Init(SrcSizeX,SrcSizeY,SrcFormat);
		// add the rest of the mips as needed
		InitMips( NumMipsToGenerate );
		// make sure miptail index is 0 so texture is created without a miptail
		MipTailBaseIdx = 0;

		// fill in all of the Mip data 
		CopyRectRegions();

		// re-init the texture resource
		UpdateResource();
	}
}

void UavaTexture2DComposite::InitValidSourceRegions()
{
	ValidRegions.Empty();

	UTexture2D* CompareTex = NULL;

	// update source regions list with only valid entries
	for( INT SrcIdx=0; SrcIdx < SourceRegions.Num(); SrcIdx++ )
	{
		const FSourceTexture2DRegion& SourceRegion = SourceRegions(SrcIdx);
		// the source texture for this region must exist
		if( SourceRegion.Texture2D == NULL )
		{
			debugf( NAME_Warning, TEXT("FCompositeTexture2DUtil: Source texture missing - skipping...") );
		}
		// texture formats should match
		else if( CompareTex && SourceRegion.Texture2D->Format != CompareTex->Format )
		{
			debugf( NAME_Warning, TEXT("FCompositeTexture2DUtil: Source texture format mismatch [%s] - skipping..."), 
				*SourceRegion.Texture2D->GetFullName() );				
		}
		// source textures must have the same number of mips
		else if( CompareTex && SourceRegion.Texture2D->Mips.Num() != CompareTex->Mips.Num() )
		{
			debugf( NAME_Warning, TEXT("FCompositeTexture2DUtil: Source texture number Mips mismatch [%s] - skipping..."), 
				*SourceRegion.Texture2D->GetFullName() );
		}
		// make sure source textures are not streamable
		else if( !SourceRegion.Texture2D->IsFullyStreamedIn() )
		{
			debugf( NAME_Warning, TEXT("FCompositeTexture2DUtil: Source texture is not fully streamed in [%s] - skipping..."), 
				*SourceRegion.Texture2D->GetFullName() );
		}
		// valid so add it to the list
		else
		{
			ValidRegions.AddItem( SourceRegion );
			// First valid texture - remember to compare to others that follow.
			if(!CompareTex)
			{
				CompareTex = SourceRegion.Texture2D;
			}
		}
	}
}

void UavaTexture2DComposite::RenderThread_CopyRectRegions()
{
	check(ValidRegions.Num() > 0);

	// calc index of first available mip common to the set of source textures
	INT FirstSrcMipIdx = GetFirstAvailableMipIndex();

	// create a temp RHI texture 2d used to hold intermediate mip data
	FTexture2DRHIRef TempTexture2D = RHICreateTexture2D(SizeX,SizeY,Format,Mips.Num(),0);

	// process each mip level
	for( INT MipIdx=0; MipIdx < Mips.Num(); MipIdx++ )
	{
		// destination mip data
		FTexture2DMipMap& DstMip = Mips(MipIdx);
		// list of source regions that need to be copied for this mip level
		TArray<FCopyTextureRegion2D> CopyRegions;

		for( INT RegionIdx=0; RegionIdx < ValidRegions.Num(); RegionIdx++ )		
		{
			FSourceTexture2DRegion& Region = ValidRegions(RegionIdx);
			FTexture2DResource* SrcTex2DResource = (FTexture2DResource*)Region.Texture2D->Resource;		
			if( SrcTex2DResource && 
				SrcTex2DResource->IsInitialized() )
			{
				check( Region.Texture2D->Mips.IsValidIndex(MipIdx + FirstSrcMipIdx) );

				// scale regions for the current mip level. The regions are assumed to be sized w/ respect to the base (maybe not loaded) mip level
				INT RegionOffsetX = Region.OffsetX >> (MipIdx + FirstSrcMipIdx);
				INT RegionOffsetY = Region.OffsetY >> (MipIdx + FirstSrcMipIdx);
				INT RegionSizeX = Max(Region.SizeX >> (MipIdx + FirstSrcMipIdx),1);
				INT RegionSizeY = Max(Region.SizeY >> (MipIdx + FirstSrcMipIdx),1);
				// scale create the new region for the RHI texture copy
				FCopyTextureRegion2D& CopyRegion = *new(CopyRegions) FCopyTextureRegion2D(
					SrcTex2DResource->GetTexture2DRHI(),		
					RegionOffsetX,
					RegionOffsetY,
					RegionSizeX,
					RegionSizeY
					);
			}
		}

		// copy from all of the source regions for the current mip level to the temp texture
		RHICopyTexture2D(TempTexture2D,MipIdx,SizeX,SizeY,Format,CopyRegions);
	}

	// find the base level for the miptail
	INT MipTailIdx = RHIGetMipTailIdx(TempTexture2D);
	// set the miptail level for this utexture (-1 means no miptail so just use the last mip)
	MipTailBaseIdx = (MipTailIdx == -1) ? Mips.Num()-1 : MipTailIdx;

	// copy the results from the temp texture to the mips array
	for( INT MipIdx=0; MipIdx <= MipTailBaseIdx; MipIdx++ )
	{
		// destination mip data
		FTexture2DMipMap& DstMip = Mips(MipIdx);

		// lock the temp texture
		UINT TempStride;
		BYTE* TempData = (BYTE*)RHILockTexture2D( TempTexture2D, MipIdx, FALSE, TempStride );

		// copy to the mips array
		BYTE* DstData = (BYTE*)DstMip.Data.Lock( LOCK_READ_WRITE );
		appMemcpy(DstData,TempData,DstMip.Data.GetBulkDataSize());
		DstMip.Data.Unlock();

		// unlock the temp RHI texture
		RHIUnlockTexture2D( TempTexture2D, MipIdx );
	}

}

/**
* Locks each region of the source RHI texture 2d resources and copies the block of data
* for that region to the destination mip buffer. This is done for all mip levels.
*
* (Only called by the rendering thread)
*/
void UavaTexture2DComposite::RHICopyTexture2D(FTexture2DRHIParamRef DstTexture, UINT MipIdx, INT BaseSizeX, INT BaseSizeY, INT Format, const TArray<FCopyTextureRegion2D>& Regions)
{
	check( IsValidRef(DstTexture) );

	// scale the base SizeX,SizeY for the current mip level
	INT MipSizeX = Max((INT)GPixelFormats[Format].BlockSizeX,BaseSizeX >> MipIdx);
	INT MipSizeY = Max((INT)GPixelFormats[Format].BlockSizeY,BaseSizeY >> MipIdx);

	// lock the destination texture
	UINT DstStride;
	BYTE* DstData = (BYTE*)RHILockTexture2D( DstTexture, MipIdx, TRUE, DstStride );

	for( INT RegionIdx=0; RegionIdx < Regions.Num(); RegionIdx++ )		
	{
		const FCopyTextureRegion2D& Region = Regions(RegionIdx);
		check( IsValidRef(Region.SrcTexture) );

		INT SrcMipSizeX = Max((INT)GPixelFormats[Format].BlockSizeX,Region.SizeX);
			
		// lock source RHI texture
		UINT SrcStride=0;
		BYTE* SrcData = (BYTE*)RHILockTexture2D( 
			Region.SrcTexture,
			MipIdx,
			FALSE,
			SrcStride
			);	

		// align/truncate the region offset to block size
		INT DstRegionOffsetX = (Clamp( Region.OffsetX, 0, MipSizeX - GPixelFormats[Format].BlockSizeX ) / GPixelFormats[Format].BlockSizeX) * GPixelFormats[Format].BlockSizeX;
		INT DstRegionOffsetY = (Clamp( Region.OffsetY, 0, MipSizeY - GPixelFormats[Format].BlockSizeY ) / GPixelFormats[Format].BlockSizeY) * GPixelFormats[Format].BlockSizeY;
		INT SrcRegionOffsetX = 0;
		INT SrcRegionOffsetY = 0;

		// scale region size to the current mip level. Size is aligned to the block size
		check(Region.SizeX != 0 && Region.SizeY != 0);
		INT RegionSizeX = Clamp( Align( Region.SizeX, GPixelFormats[Format].BlockSizeX), 0, MipSizeX );
		INT RegionSizeY = Clamp( Align( Region.SizeY, GPixelFormats[Format].BlockSizeY), 0, MipSizeY );
		// handle special case for full copy
		if( Region.SizeX == -1 || Region.SizeY == -1 )
		{
			RegionSizeX = MipSizeX;
			RegionSizeY = MipSizeY;
		}

		// size in bytes of an entire row for this mip
		DWORD SrcPitchBytes = (SrcMipSizeX / GPixelFormats[Format].BlockSizeX) * GPixelFormats[Format].BlockBytes;
		DWORD DstPitchBytes = (MipSizeX / GPixelFormats[Format].BlockSizeX) * GPixelFormats[Format].BlockBytes;
		
		// size in bytes of the offset to the starting part of the row to copy for this mip
		DWORD DstRowOffsetBytes = (DstRegionOffsetX / GPixelFormats[Format].BlockSizeX) * GPixelFormats[Format].BlockBytes;

		// size in bytes of the amount to copy within each row
		DWORD RowSizeBytes = (RegionSizeX / GPixelFormats[Format].BlockSizeX) * GPixelFormats[Format].BlockBytes;

		INT SrcCurBlockOffsetY = 0;
		// copy each region row in increments of the block size
		for( INT CurOffsetY=DstRegionOffsetY; CurOffsetY < (DstRegionOffsetY+RegionSizeY); CurOffsetY += GPixelFormats[Format].BlockSizeY )
		{
			INT DstCurBlockOffsetY = CurOffsetY / GPixelFormats[Format].BlockSizeY;
			BYTE* SrcOffset = SrcData + (SrcCurBlockOffsetY * SrcPitchBytes);
			BYTE* DstOffset = DstData + (DstCurBlockOffsetY * DstPitchBytes) + DstRowOffsetBytes;
			appMemcpy( DstOffset, SrcOffset, RowSizeBytes );
			++SrcCurBlockOffsetY;
		}

		// done reading from source mip so unlock it
		RHIUnlockTexture2D( Region.SrcTexture, MipIdx );
	}

	// unlock the destination texture
	RHIUnlockTexture2D( DstTexture, MipIdx );
}
