/*=============================================================================
	LaunchEngineLoop.cpp: Main engine loop.
	Copyright © 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "LaunchPrivate.h"

// Static linking support forward declaration.
void InitializeRegistrantsAndRegisterNames();

//	AutoInitializeRegistrants* declarations.
	extern void AutoInitializeRegistrantsCore( INT& Lookup );
	extern void AutoInitializeRegistrantsEngine( INT& Lookup );
	extern void AutoInitializeRegistrantsGameFramework( INT& Lookup );
	extern void AutoInitializeRegistrantsUnrealScriptTest( INT& Lookup );
	extern void AutoInitializeRegistrantsIpDrv( INT& Lookup );
#if defined(XBOX)
	extern void AutoInitializeRegistrantsXeAudio( INT& Lookup );
	extern void AutoInitializeRegistrantsXeDrv( INT& Lookup );
	extern void AutoInitializeRegistrantsOnlineSubsystemLive( INT& Lookup );
#elif PS3
	extern void AutoInitializeRegistrantsPS3Drv( INT& Lookup );
#elif PLATFORM_UNIX
	extern void AutoInitializeRegistrantsLinuxDrv( INT& Lookup );
#else
	extern void AutoInitializeRegistrantsEditor( INT& Lookup );
	extern void AutoInitializeRegistrantsUnrealEd( INT& Lookup );
	extern void AutoInitializeRegistrantsALAudio( INT& Lookup );
	extern void AutoInitializeRegistrantsWinDrv( INT& Lookup );
#endif
#if   GAMENAME == WARGAME
	extern void AutoInitializeRegistrantsWarfareGame( INT& Lookup );
#if !CONSOLE
	extern void AutoInitializeRegistrantsWarfareEditor( INT& Lookup );
#endif
#if defined(XBOX)
	extern void AutoInitializeRegistrantsVinceOnlineSubsystemLive( INT& Lookup );
#endif
#elif GAMENAME == GEARGAME
	extern void AutoInitializeRegistrantsGearGame( INT& Lookup );
#if !CONSOLE
	extern void AutoInitializeRegistrantsGearEditor( INT& Lookup );
#endif
#elif GAMENAME == UTGAME
	extern void AutoInitializeRegistrantsUTGame( INT& Lookup );
#if !CONSOLE
	extern void AutoInitializeRegistrantsUTEditor( INT& Lookup );
#endif

#elif GAMENAME == EXAMPLEGAME
	extern void AutoInitializeRegistrantsExampleGame( INT& Lookup );
#if !CONSOLE
	extern void AutoInitializeRegistrantsExampleEditor( INT& Lookup );
#endif
#else
	#error Hook up your game name here
#endif

//	AutoGenerateNames* declarations.
	extern void AutoGenerateNamesCore();
	extern void AutoGenerateNamesEngine();
	extern void AutoGenerateNamesGameFramework();
	extern void AutoGenerateNamesUnrealScriptTest();
#if !defined(CONSOLE) && !PLATFORM_UNIX
	extern void AutoGenerateNamesEditor();
	extern void AutoGenerateNamesUnrealEd();
#endif
	extern void AutoGenerateNamesIpDrv();
#if XBOX
	extern void AutoGenerateNamesOnlineSubsystemLive();
#endif
#if   GAMENAME == WARGAME
	extern void AutoGenerateNamesWarfareGame();
#if !CONSOLE
	extern void AutoGenerateNamesWarfareEditor();
#endif
#if XBOX
	extern void AutoGenerateNamesVinceOnlineSubsystemLive();
#endif
#elif GAMENAME == GEARGAME
	extern void AutoGenerateNamesGearGame();
#if !CONSOLE
	extern void AutoGenerateNamesGearEditor();
#endif
#elif GAMENAME == UTGAME
	extern void AutoGenerateNamesUTGame();
#if !CONSOLE
	extern void AutoGenerateNamesUTEditor();
#endif

#elif GAMENAME == EXAMPLEGAME
	extern void AutoGenerateNamesExampleGame();
#if !CONSOLE
	extern void AutoGenerateNamesExampleEditor();
#endif
#else
	#error Hook up your game name here
#endif

/*-----------------------------------------------------------------------------
	Global variables.
-----------------------------------------------------------------------------*/

#define SPAWN_CHILD_PROCESS_TO_COMPILE 0

// General.
#ifndef CONSOLE
extern "C" {HINSTANCE hInstance;}
#endif
extern "C" {TCHAR GPackage[64]=TEXT("Launch");}

/** Critical section used by MallocThreadSafeProxy for synchronization										*/

#ifdef XBOX
/** Remote debug command																					*/
static CHAR							RemoteDebugCommand[1024];		
/** Critical section for accessing remote debug command														*/
static FCriticalSection				RemoteDebugCriticalSection;	
#endif

/** Helper function called on first allocation to create and initialize GMalloc */
void GCreateMalloc()
{
#if PS3
	GMalloc = new FMallocPS3();
#elif defined(XBOX)
	GMalloc = new FMallocXenon();
#elif _DEBUG
	GMalloc = new FMallocDebug();
#elif __GNUC__
	GMalloc = new FMallocAnsi();
#else
	GMalloc = new FMallocWindows();
#endif

#if TRACK_MEM_USING_STAT_SECTIONS
	GMalloc = new FMallocProxySimpleTrack( GMalloc );
#elif KEEP_ALLOCATION_BACKTRACE
#ifndef XBOX
	GMalloc = new FMallocDebugProxyWindows( GMalloc );
#else
	GMalloc = new FMallocDebugProxyXenon( GMalloc );
#endif
#endif

	// if the allocator is already thread safe, there is no need for the thread safe proxy
	if (!GMalloc->IsInternallyThreadSafe())
	{
		GMalloc = new FMallocThreadSafeProxy( GMalloc );
	}

	GMalloc->Init();
}

#if defined(XBOX)
static FOutputDeviceFile					Log;
static FOutputDeviceAnsiError				Error;
static FFeedbackContextAnsi					GameWarn;
static FFileManagerXenon					FileManager;
#elif PS3
static FSynchronizeFactoryPU				SynchronizeFactory;
static FThreadFactoryPU						ThreadFactory;
static FOutputDeviceNull					Log;
static FOutputDeviceAnsiError				Error;
static FFeedbackContextAnsi					GameWarn;
static FFileManagerPS3						FileManager;
static FQueuedThreadPoolPS3					ThreadPool;
#elif PLATFORM_UNIX
static FSynchronizeFactoryLinux				SynchronizeFactory;
static FThreadFactoryLinux					ThreadFactory;
static FOutputDeviceFile					Log;
static FOutputDeviceAnsiError				Error;
static FFeedbackContextAnsi					GameWarn;
static FFileManagerLinux					FileManager;
#else
#include "FFeedbackContextEditor.h"
static FOutputDeviceFile					Log;
static FOutputDeviceWindowsError			Error;
static FFeedbackContextWindows				GameWarn;
static FFeedbackContextEditor				UnrealEdWarn;
static FFileManagerWindows					FileManager;
static FCallbackEventDeviceEditor			UnrealEdEventCallback;
static FCallbackQueryDeviceEditor			UnrealEdQueryCallback;
static FOutputDeviceConsoleWindows			LogConsole;
static FOutputDeviceConsoleWindowsInherited InheritedLogConsole(LogConsole);
static FSynchronizeFactoryWin				SynchronizeFactory;
static FThreadFactoryWin					ThreadFactory;
static FQueuedThreadPoolWin					ThreadPool;
#endif

static FCallbackEventObserver				GameEventCallback;
static FCallbackQueryDevice					GameQueryCallback;

#if !FINAL_RELEASE
// this will allow the game to receive object propagation commands from the network
FListenPropagator							ListenPropagator;
#endif

extern	TCHAR								GCmdLine[4096];
/** Whether we are using wxWindows or not */
extern	UBOOL								GUsewxWindows;

/** Thread used for async IO manager */
FRunnableThread*							AsyncIOThread;

/** 
* if TRUE then FDeferredUpdateResource::UpdateResources needs to be called 
* (should only be set on the rendering thread)
*/
UBOOL FDeferredUpdateResource::bNeedsUpdate = TRUE;

#if !CONSOLE && _MSC_VER
#include "..\..\Launch\Resources\resource.h"
/** Resource ID of icon to use for Window */
#if   GAMENAME == WARGAME || GAMENAME == GEARGAME
INT			GGameIcon	= IDICON_GoW;
INT			GEditorIcon	= IDICON_Editor;
#elif GAMENAME == UTGAME
INT			GGameIcon	= IDICON_UTGame;
INT			GEditorIcon	= IDICON_Editor;
#elif GAMENAME == EXAMPLEGAME
INT			GGameIcon	= IDICON_DemoGame;
INT			GEditorIcon	= IDICON_DemoEditor;
#else
	#error Hook up your game name here
#endif
#endif

#if !defined(CONSOLE) && defined(_MSC_VER)
#include "..\Debugger\UnDebuggerCore.h"
#endif

// From UMakeCommandlet.cpp - See if scripts need rebuilding at runtime
extern UBOOL AreScriptPackagesOutOfDate();

/**
 * The single function that sets the gamename based on the #define GAMENAME
 * Licensees need to add their own game(s) here!
 */
void appSetGameName()
{
	// Initialize game name
#if   GAMENAME == EXAMPLEGAME
	appStrcpy(GGameName, TEXT("Example"));
#elif GAMENAME == WARGAME
	appStrcpy(GGameName, TEXT("War"));
#elif GAMENAME == GEARGAME
	appStrcpy(GGameName, TEXT("Gear"));
#elif GAMENAME == UTGAME
	appStrcpy(GGameName, TEXT("UT"));
#else
	#error Hook up your game name here
#endif
}

/**
 * A single function to get the list of the script packages that are used by 
 * the current game (as specified by the GAMENAME #define)
 *
 * @param PackageNames					The output array that will contain the package names for this game (with no extension)
 * @param bCanIncludeEditorOnlyPackages	If possible, should editor only packages be included in the list?
 */
void appGetGameScriptPackageNames(TArray<FString>& PackageNames, UBOOL bCanIncludeEditorOnlyPackages)
{
#if defined(CONSOLE) || PLATFORM_UNIX
	// consoles and unix NEVER want to load editor only packages
	bCanIncludeEditorOnlyPackages = FALSE;
#endif

#if   GAMENAME == EXAMPLEGAME
	PackageNames.AddItem(TEXT("ExampleGame"));
//	@todo: ExampleEditor is not in .u form yet
//	if (bCanIncludeEditorOnlyPackages)
//	{
//		PackageNames.AddItem(TEXT("ExampleEditor"));
//	}
#elif GAMENAME == WARGAME
	PackageNames.AddItem(TEXT("WarfareGame"));
	PackageNames.AddItem(TEXT("WarfareGameContent"));
	// we need to cook this as it has weapons which players can carry across maps
	//PackageNames.AddItem(TEXT("WarfareGameContentWeapons"));
	if (bCanIncludeEditorOnlyPackages)
	{
		PackageNames.AddItem(TEXT("WarfareEditor"));
	}
#elif GAMENAME == GEARGAME
	PackageNames.AddItem(TEXT("GearGame"));
	PackageNames.AddItem(TEXT("GearGameContent"));
	// we need to cook this as it has weapons which players can carry across maps
	//PackageNames.AddItem(TEXT("GearGameContentWeapons"));
	if (bCanIncludeEditorOnlyPackages)
	{
		PackageNames.AddItem(TEXT("GearEditor"));
	}

#elif GAMENAME == UTGAME
	PackageNames.AddItem(TEXT("UTGame"));
	if (bCanIncludeEditorOnlyPackages)
	{
		PackageNames.AddItem(TEXT("UTEditor"));
	}

#else
	#error Hook up your game name here
#endif
}

/**
 * A single function to get the list of the script packages containing native
 * classes that are used by the current game (as specified by the GAMENAME #define)
 * Licensees need to add their own game(s) to the definition of this function!
 *
 * @param PackageNames					The output array that will contain the package names for this game (with no extension)
 * @param bCanIncludeEditorOnlyPackages	If possible, should editor only packages be included in the list?
 */
void appGetGameNativeScriptPackageNames(TArray<FString>& PackageNames, UBOOL bCanIncludeEditorOnlyPackages)
{
#if defined(CONSOLE) || PLATFORM_UNIX
	// consoles and unix NEVER want to load editor only packages
	bCanIncludeEditorOnlyPackages = FALSE;
#endif

#if   GAMENAME == EXAMPLEGAME
	PackageNames.AddItem(TEXT("ExampleGame"));
//	@todo: ExampleEditor is not in .u form yet
//	if (bCanIncludeEditorOnlyPackages)
//	{
//		PackageNames.AddItem(TEXT("ExampleEditor"));
//	}
#elif GAMENAME == WARGAME
	PackageNames.AddItem(TEXT("WarfareGame"));
	if (bCanIncludeEditorOnlyPackages)
	{
		PackageNames.AddItem(TEXT("WarfareEditor"));
	}

#ifdef XBOX
	// only include this if we are not checking native class sizes.  Currently, the native classes which reside on specific console
	// platforms will cause native class size checks to fail even tho class sizes are hopefully correct on the target platform as the PC 
	// doesn't have access to that native class.
	if( ParseParam(appCmdLine(),TEXT("CHECK_NATIVE_CLASS_SIZES")) == FALSE )
	{
		PackageNames.AddItem(TEXT("OnlineSubsystemLive"));
		PackageNames.AddItem(TEXT("VinceOnlineSubsystemLive"));
	}
#endif
#elif GAMENAME == GEARGAME
	PackageNames.AddItem(TEXT("GearGame"));
	if (bCanIncludeEditorOnlyPackages)
	{
		PackageNames.AddItem(TEXT("GearEditor"));
	}

#elif GAMENAME == UTGAME
	PackageNames.AddItem(TEXT("UTGame"));
	if (bCanIncludeEditorOnlyPackages)
	{
		PackageNames.AddItem(TEXT("UTEditor"));
	}
#else
	#error Hook up your game name here
#endif
}

/**
 * A single function to get the list of the script packages that are used by the base engine.
 *
 * @param PackageNames					The output array that will contain the package names for this game (with no extension)
 * @param bCanIncludeEditorOnlyPackages	If possible, should editor only packages be included in the list?
 */
void appGetEngineScriptPackageNames(TArray<FString>& PackageNames, UBOOL bCanIncludeEditorOnlyPackages)
{
#if defined(CONSOLE) || PLATFORM_UNIX
	// consoles and unix NEVER want to load editor only packages
	bCanIncludeEditorOnlyPackages = FALSE;
#endif
	PackageNames.AddItem(TEXT("Core"));
	PackageNames.AddItem(TEXT("Engine"));
	PackageNames.AddItem(TEXT("GameFramework"));
	if( bCanIncludeEditorOnlyPackages )
	{
		PackageNames.AddItem(TEXT("Editor"));
		PackageNames.AddItem(TEXT("UnrealEd"));
	}	
	PackageNames.AddItem(TEXT("UnrealScriptTest"));
}

/**
 * Gets the list of all native script packages that the game knows about.
 * 
 * @param PackageNames The output list of package names
 * @param bExcludeGamePackages TRUE if the list should only contain base engine packages
 * @param bIncludeLocalizedSeekFreePackages TRUE if the list should include the _LOC_int loc files
 */
void GetNativeScriptPackageNames(TArray<FString>& PackageNames, UBOOL bExcludeGamePackages, UBOOL bIncludeLocalizedSeekFreePackages)
{
	// Assemble array of native script packages.
	appGetEngineScriptPackageNames(PackageNames, TRUE);
	if( !bExcludeGamePackages )
	{
		appGetGameNativeScriptPackageNames(PackageNames, TRUE);
	}

#if USE_SEEKFREE_LOADING
	// required for seek free loading
	bIncludeLocalizedSeekFreePackages = TRUE;
#endif	

	// insert any localization for Seek free packages if requested
	if (bIncludeLocalizedSeekFreePackages)
	{
		for( INT PackageIndex=0; PackageIndex<PackageNames.Num(); PackageIndex++ )
		{
			// only insert localized package if it exists
			FString LocalizedPackageName = PackageNames(PackageIndex) + LOCALIZED_SEEKFREE_SUFFIX;
#if USE_SEEKFREE_LOADING
			FString LocalizedFileName;
			if( GPackageFileCache->FindPackageFile( *LocalizedPackageName, NULL, LocalizedFileName ) )
#endif
			{
				PackageNames.InsertItem(*LocalizedPackageName, PackageIndex);
				// skip over the package that we just localized
				PackageIndex++;
			}
		}
	}
}

/**
 * Gets the list of packages that are precached at startup for seek free loading
 *
 * @param PackageNames The output list of package names
 * @param EngineConfigFilename Optional engine config filename to use to lookup the package settings
 */
void GetNonNativeStartupPackageNames(TArray<FString>& PackageNames, const TCHAR* EngineConfigFilename=NULL)
{
	// look for any packages that we want to force preload at startup
	TMultiMap<FString,FString>* PackagesToPreload = GConfig->GetSectionPrivate( TEXT("Engine.PackagesToForceLoadAtStartupForSeekFree"), 0, 1, 
		EngineConfigFilename ? EngineConfigFilename : GEngineIni );
	if (PackagesToPreload)
	{
		// go through list and add to the array
		for( TMultiMap<FString,FString>::TIterator It(*PackagesToPreload); It; ++It )
		{
			if (It.Key() == TEXT("Package"))
			{
				// add this package to the list to be fully loaded later
				PackageNames.AddItem(*(It.Value()));
			}
		}
	}
}

/**
 * Kicks off a list of packages to be read in asynchronously in the background by the
 * async file manager. The package will be serialized from RAM later.
 * 
 * @param PackageNames The list of package names to async preload
 */
void AsyncPreloadPackageList(const TArray<FString>& PackageNames)
{
	// Iterate over all native script packages and preload them.
	for (INT PackageIndex=0; PackageIndex<PackageNames.Num(); PackageIndex++)
	{
		// let ULinkerLoad class manage preloading
		ULinkerLoad::AsyncPreloadPackage(*PackageNames(PackageIndex));
	}
}

/**
 * Fully loads a list of packages.
 * 
 * @param PackageNames The list of package names to load
 */
void LoadPackageList(const TArray<FString>& PackageNames)
{
	// Iterate over all native script packages and fully load them.
	for( INT PackageIndex=0; PackageIndex<PackageNames.Num(); PackageIndex++ )
	{
		UObject* Package = UObject::LoadPackage(NULL, *PackageNames(PackageIndex), LOAD_None);
	}
}

/**
 * This will load up all of the various "core" .u packages.
 * 
 * We do this as an optimization for load times.  We also do this such that we can be assured that 
 * all of the .u classes are loaded so we can then verify them.
 *
 * @param bExcludeGamePackages	Whether to exclude game packages
 */
void LoadAllNativeScriptPackages( UBOOL bExcludeGamePackages )
{
	TArray<FString> PackageNames;

	// call the shared function to get all native script package names
	GetNativeScriptPackageNames(PackageNames, bExcludeGamePackages, FALSE);

	// load them
	LoadPackageList(PackageNames);
}

/**
 * @return Name of the startup map from the engine ini file
 */
FString GetStartupMap()
{
	// default map
	FString DefaultLocalMap;
	GConfig->GetString(TEXT("URL"), TEXT("LocalMap"), DefaultLocalMap, GEngineIni);
		
	// parse the map name out and add it to the list
	FURL TempURL(NULL, *DefaultLocalMap, TRAVEL_Absolute);
	return FFilename(TempURL.Map).GetBaseFilename();
}

/**
 * If desired by ini setting, this will kick off async precaching of all packages
 * needed for startup (native script packages, those specified in the 
 * Engine.PackagesToForceLoadAtStartupForSeekFree, and the startup map). It will also
 * fully load in the native script packages, even if it doesn't precache any
 * packages.
 *
 * @param NonScriptStartupPackageNames Out array that contains all of the startup packages that were precached that should be loaded later
 */
void PreloadStartupPackagesForSeekFree(TArray<FString>& NonScriptStartupPackageNames)
{
	// should script packages load from memory?
	UBOOL bPreloadPackagesFromMemory = FALSE;
	GConfig->GetBool(TEXT("Engine.SeekFree"), TEXT("bPreloadPackagesFromMemory"), bPreloadPackagesFromMemory, GEngineIni);

	// if we want to preload them, then kick off script and some extra packages
	if (bPreloadPackagesFromMemory)
	{
		TArray<FString> NativeScriptPackages;
		// call the shared function to get all native script package names
		GetNativeScriptPackageNames(NativeScriptPackages, FALSE, FALSE);

		// start preloading them
		AsyncPreloadPackageList(NativeScriptPackages);
	    
		// get list of non-native startup packages
		GetNonNativeStartupPackageNames(NonScriptStartupPackageNames);
		
		// kick them off to be preloaded
		AsyncPreloadPackageList(NonScriptStartupPackageNames);

		// kick off startup map (but don't add it to the list to load later, because UGameEngine::Init will load it)
		ULinkerLoad::AsyncPreloadPackage(*GetStartupMap());

		// now we want to actually load the native script package
		LoadPackageList(NativeScriptPackages);
	}
	// if we don't want to preload packages from RAM, then just load the native script packages like normal
	else
	{
		LoadAllNativeScriptPackages(FALSE);

		// should we preload non-nativce startup packages from disk even if we aren't doing async preloading?
		UBOOL bPreloadEvenWithoutAsyncPreload = FALSE;
		GConfig->GetBool(TEXT("Engine.PackagesToForceLoadAtStartupForSeekFree"), TEXT("bPreloadEvenWithoutAsyncPreload"), bPreloadEvenWithoutAsyncPreload, GEngineIni);
		if (bPreloadEvenWithoutAsyncPreload)
		{
			TArray<FString> NonNativeStartupPackages;

			// get list of non-native startup packages
			GetNonNativeStartupPackageNames(NonNativeStartupPackages);

			// now we want to actually load the native script package
			LoadPackageList(NonNativeStartupPackages);
		}
	}
}

/**
 * Get a list of all packages that may be needed at startup, and could be
 * loaded async in the background when doing seek free loading
 *
 * @param PackageNames The output list of package names
 * @param EngineConfigFilename Optional engine config filename to use to lookup the package settings
 */
void appGetAllPotentialStartupPackageNames(TArray<FString>& PackageNames, const TCHAR* EngineConfigFilename)
{
	// native script packages
	GetNativeScriptPackageNames(PackageNames, FALSE, TRUE);
	// startup packages from .ini
	GetNonNativeStartupPackageNames(PackageNames, EngineConfigFilename);
	// add the startup map
	PackageNames.AddItem(*GetStartupMap());

	// go through and add localized versions of each package for all known languages
	// since they could be used at runtime depending on the language at runtime
	INT NumPackages = PackageNames.Num();
	for (INT PackageIndex = 0; PackageIndex < NumPackages; PackageIndex++)
	{
		// add the packagename with _XXX language extension
	    for (INT LangIndex = 0; GKnownLanguageExtensions[LangIndex]; LangIndex++)
		{
			PackageNames.AddItem(*(PackageNames(PackageIndex) + TEXT("_") + GKnownLanguageExtensions[LangIndex]));
		}
	}
}

/**
 * Checks for native class script/ C++ mismatch of class size and member variable
 * offset. Note that only the first and last member variable of each class in addition
 * to all member variables of noexport classes are verified to work around a compiler
 * limitation. The code is disabled by default as it has quite an effect on compile
 * time though is an invaluable tool for sanity checking and bringing up new
 * platforms.
 */
void CheckNativeClassSizes()
{
#if CHECK_NATIVE_CLASS_SIZES  // pass in via /DCHECK_NATIVE_CLASS_SIZES or set CL=/DCHECK_NATIVE_CLASS_SIZES and then this will be activated.  Good for setting up on your continuous integration machine
	debugf(TEXT("CheckNativeClassSizes..."));
	UBOOL Mismatch = FALSE;
	VERIFY_CLASS_SIZE_NODIE(ACoverNode);
	VERIFY_CLASS_SIZE_NODIE(AWeapon);
	VERIFY_CLASS_SIZE_NODIE(AKActor);
	#define NAMES_ONLY
	#define AUTOGENERATE_NAME(name)
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
	#define VERIFY_CLASS_SIZES
	#include "CoreClasses.h"
	#include "EngineGameEngineClasses.h"
	#include "EngineClasses.h"
	#include "EngineAIClasses.h"
	#include "EngineMaterialClasses.h"
	#include "EngineTerrainClasses.h"
	#include "EnginePhysicsClasses.h"
	#include "EngineSequenceClasses.h"
	#include "EngineSoundClasses.h"
	#include "EngineInterpolationClasses.h"
	#include "EngineParticleClasses.h"
	#include "EngineAnimClasses.h"
	#include "EngineDecalClasses.h"
	#include "EnginePrefabClasses.h"
	#include "EngineUserInterfaceClasses.h"
	#include "EngineUIPrivateClasses.h"
	#include "EngineUISequenceClasses.h"
	#include "GameFrameworkClasses.h"
	#include "UnrealScriptTestClasses.h"
#ifndef CONSOLE	
	#include "EditorClasses.h"
	#include "UnrealEdClasses.h"
	#include "UnrealEdCascadeClasses.h"
	#include "UnrealEdPrivateClasses.h"
#endif
	#include "IpDrvClasses.h"
#if GAMENAME == WARGAME
#include "WarfareGameClasses.h"
#include "WarfareGameCameraClasses.h"
#include "WarfareGameSequenceClasses.h"
#include "WarfareGameSpecialMovesClasses.h"
#include "WarfareGameVehicleClasses.h"
#include "WarfareGameSoundClasses.h"
#include "WarfareGameAIClasses.h"
#ifndef CONSOLE
	#include "WarfareEditorClasses.h"
#endif
#elif GAMENAME == GEARGAME
#include "GearGameClasses.h"
#include "GearGameCameraClasses.h"
#include "GearGameSequenceClasses.h"
#include "GearGameSpecialMovesClasses.h"
#include "GearGameVehicleClasses.h"
#include "GearGameSoundClasses.h"
#include "GearGameAIClasses.h"
#ifndef CONSOLE
	#include "GearEditorClasses.h"
#endif
#elif GAMENAME == UTGAME
	#include "UTGameClasses.h"
	#include "UTGameSequenceClasses.h"
#ifndef CONSOLE
	#include "UTEditorClasses.h"
#endif

#elif GAMENAME == EXAMPLEGAME
	#include "ExampleGameClasses.h"
#ifndef CONSOLE
    #include "ExampleEditorClasses.h"
#endif
#else
	#error Hook up your game name here
#endif
	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
	#undef VERIFY_CLASS_SIZES

	if( ( Mismatch == TRUE ) && ( GIsUnattended == TRUE ) )
	{
		appErrorf( NAME_Error, *LocalizeUnrealEd("Error_ScriptClassSizeMismatch") );
	}
	else if( Mismatch == TRUE )
	{
		appErrorf( NAME_FriendlyError, *LocalizeUnrealEd("Error_ScriptClassSizeMismatch") );
	}
	else
	{
		debugf(TEXT("CheckNativeClassSizes completed with no errors"));
	}
#endif  // #fi CHECK_NATIVE_CLASS_SIZES 
}


//#define EPICINTERNAL_CHECK_FOR_WARFARE_MILESTONE_BUILD 0 // this is set by the build machine
#define EPICINTERNAL_THIS_IS_WARFARE_MILESTONE_BUILD 0 // in the warfare branch set this to 1.  In the main branch set this to 0
/**
 * This is meant for internal epic usage to check and see if the game is allowed to run.
 * This is specifically meant to be used to check and see if warfare is using a special
 * milestone build or not
 **/
void CheckForWarfareMiletoneBuild()
{
	if( ( FString(GGameName) == TEXT("War") )
		&& ( EPICINTERNAL_THIS_IS_WARFARE_MILESTONE_BUILD == 0 )
		)
	{
		appMsgf( AMT_OK, TEXT("Your build is not the special warfare build!  Please use the warfare-artist-sync.bat to get the correct build." ) );
		GIsRequestingExit = TRUE;
	}
}

/**
 * This is meant for internal epic usage to check and see if the editor is allowed to run.  
 * It looks to see if a number of files exist and based on which ones exist and their contents
 * it will either allow the editor to run or it will tell the user to upgrade.
 **/
void CheckForQABuildCandidateStatus()
{
	// check to see here if we are allowed to run the editor!
	// Query whether this is an epic internal build or not.

	// QA build process code
	UBOOL bOnlyUseQABuildSemaphore = FALSE;

	if( ( GFileManager->FileSize( TEXT("\\\\Build-Server-01") PATH_SEPARATOR TEXT("BuildFlags") PATH_SEPARATOR TEXT("OnlyUseQABuildSemaphore.txt") ) >= 0 )
		&& ( FString(GGameName) != TEXT("War") )
		)
	{
		bOnlyUseQABuildSemaphore = TRUE;
	}

	const TCHAR* QABuildInfoFileName = TEXT("\\\\Build-Server-01") PATH_SEPARATOR TEXT("BuildFlags") PATH_SEPARATOR TEXT("QABuildInfo.ini");

	// if the QABuildInfo.ini file exists
	if( GFileManager->FileSize( QABuildInfoFileName ) >= 0 )
	{
		FConfigFile TmpConfigFile;
		TmpConfigFile.Read( QABuildInfoFileName );
		DOUBLE QABuildVersion = 0;

		TmpConfigFile.GetDouble( TEXT("QAVersion"), TEXT("EngineVersion"), QABuildVersion );

		if( ( bOnlyUseQABuildSemaphore == TRUE )
			&& ( GEngineVersion != static_cast<INT>(QABuildVersion) )
			)
		{
			appMsgf( AMT_OK, TEXT("Your build is not the QA Candidate that is being tested.  Please sync to the QA build which is labeled as:  currentQABuildInTesting" ) );
			GIsRequestingExit = TRUE;
		}
		else if( GEngineVersion < static_cast<INT>(QABuildVersion) )
		{
			appMsgf( AMT_OK, TEXT("Your build is out of date.  Please sync to the latest build which labeled as:  latestUnrealEngine3" ) );
			GIsRequestingExit = TRUE;
		}
	}
}

/*-----------------------------------------------------------------------------
	FEngineLoop implementation.
-----------------------------------------------------------------------------*/

INT FEngineLoop::PreInit( const TCHAR* CmdLine )
{
	// setup the streaming resource flush function pointer
	GFlushStreamingFunc = &FlushResourceStreaming;

	// Set the game name.
	appSetGameName();

	// Figure out whether we want to override the package map with the seekfree version. Needs to happen before the first call
	// to UClass::Link!
	GUseSeekFreePackageMap = GUseSeekFreePackageMap || ParseParam( CmdLine, TEXT("SEEKFREEPACKAGEMAP") );

	// Override compression settings wrt size.
	GAlwaysBiasCompressionForSize = ParseParam( CmdLine, TEXT("BIASCOMPRESSIONFORSIZE") );

#ifndef XBOX
	// This is done in appXenonInit on Xenon.
	GSynchronizeFactory = &SynchronizeFactory;
	GThreadFactory		= &ThreadFactory;
#ifdef __GNUC__
#if PS3
	//@todo joeg -- Get this working for linux
	GThreadPool = &ThreadPool;
	verify(GThreadPool->Create(1));
#endif
	appInit( CmdLine, &Log, NULL, &Error, &GameWarn, &FileManager, &GameEventCallback, &GameQueryCallback, FConfigCacheIni::Factory );
#else	// __GNUC__
	GThreadPool = &ThreadPool;
	verify(GThreadPool->Create(1));
	// see if we were launched from our .com command line launcher
	InheritedLogConsole.Connect();
	appInit( CmdLine, &Log, &InheritedLogConsole, &Error, &GameWarn, &FileManager, &GameEventCallback, &GameQueryCallback, FConfigCacheIni::Factory );
#endif	// __GNUC__

#else	// XBOX
	appInit( CmdLine, &Log, NULL       , &Error, &GameWarn, &FileManager, &GameEventCallback, &GameQueryCallback, FConfigCacheIni::Factory );
	// WarGame uses a movie so we don't need to load and display the splash screen.
#if GAMENAME != WARGAME
	appXenonShowSplash(TEXT("Splash\\Splash.bmp"));
#endif
#endif	// XBOX

	// Initialize texture LOD settings before loading any textures from disk.
	FTextureLODSettings::GlobalSettings.Initialize( GEngineIni, TEXT("TextureLODSettings") );
	// Initialize global shadow volume setting
    GConfig->GetBool( TEXT("Engine.Engine"), TEXT("AllowShadowVolumes"), GAllowShadowVolumes, GEngineIni );

#ifndef CONSOLE
	// Figure out whether we're the editor, ucc or the game.
	TCHAR* CommandLine			= new TCHAR[ appStrlen(appCmdLine())+1 ];
	appStrcpy( CommandLine, appCmdLine() );
	const TCHAR* ParsedCmdLine	= CommandLine;
	FString Token				= ParseToken( ParsedCmdLine, 0);

	// trim any whitespace at edges of string - this can happen if the token was quoted with leaing or trailing whitespace
	// VC++ tends to do this in its "external tools" config
	Token = Token.Trim();

#if SPAWN_CHILD_PROCESS_TO_COMPILE
	// If the scripts need rebuilding, ask the user if they'd like to rebuild them
	if(Token != TEXT("MAKE"))
	{	
		UBOOL bIsDone = FALSE;
		while (!bIsDone && AreScriptPackagesOutOfDate())
		{
			// figure out if we should compile scripts or not
			UBOOL bShouldCompileScripts = GIsUnattended;
			if (!GIsUnattended)
			{
				INT Result = appMsgf(AMT_YesNoCancel,TEXT("Scripts are outdated. Would you like to rebuild now?"));

				// check for Cancel
				if (Result == 2)
				{
					exit(1);
				}
				else
				{
					// 0 is Yes, 1 is No (unlike AMT_YesNo, where 1 is Yes)
					bShouldCompileScripts = Result == 0;
				}
			}
			if (bShouldCompileScripts)
			{
#ifdef _MSC_VER
				// get executable name
				TCHAR ExeName[MAX_PATH];
				GetModuleFileName(NULL, ExeName, ARRAY_COUNT(ExeName));

				// create the new process, with params as follows:
				//  - if we are running unattended, pass on the unattended flag
				//  - if not unattended, then tell the subprocess to pause when there is an error compiling
				//  - if we are running silently, pass on the flag
				void* ProcHandle = appCreateProc(ExeName, *FString::Printf(TEXT("make %s %s"), 
					GIsUnattended ? TEXT("-unattended") : TEXT("-nopauseonsuccess"), 
					GIsSilent ? TEXT("-silent") : TEXT("")));
				
				INT ReturnCode;
				// wait for it to finish and get return code
				while (!appGetProcReturnCode(ProcHandle, &ReturnCode))
				{
					appSleep(0);
				}

				// if we are running unattended, we can't run forever, so only allow one pass of compiling script code
				if (GIsUnattended)
				{
					bIsDone = TRUE;
				}
#else
				// if we can't spawn childprocess, just make within this process
				Token = TEXT("MAKE");
				bIsDone = TRUE;
#endif
			}
			else
			{
				bIsDone = TRUE;
			}
		}

		// @todo: Reload the package cache here, as the child process can create packages!
		// this is the reason SPAWN_CHILD_PROCESS_TO_COMPILE defaults to 0
	}
#else // SPAWN_CHILD_PROCESS_TO_COMPILE

	// If the scripts need rebuilding, ask the user if they'd like to rebuild them
	if(Token != TEXT("MAKE") && AreScriptPackagesOutOfDate())
	{
		if(appMsgf(AMT_YesNo,TEXT("Scripts are outdated. Would you like to rebuild now?")))
		{
			Token = TEXT("MAKE");
		}
	}

#endif

	if( Token == TEXT("MAKE") )
	{
		// allow ConditionalLink() to call Link() for non-intrinsic classes during make
		GUglyHackFlags |= HACK_ClassLoadingDisabled;

		// Rebuilding script requires some hacks in the engine so we flag that.
		GIsUCCMake = TRUE;
	}
#endif	// CONSOLE

	// Deal with static linking.
	InitializeRegistrantsAndRegisterNames();

	// Create the streaming manager and add the default streamers.
	FStreamingManagerTexture* TextureStreamingManager = new FStreamingManagerTexture();
	TextureStreamingManager->AddTextureStreamingHandler( new FStreamingHandlerTextureStatic() );
	TextureStreamingManager->AddTextureStreamingHandler( new FStreamingHandlerTextureLevelForced() );
	GStreamingManager = new FStreamingManagerCollection();
	GStreamingManager->AddStreamingManager( TextureStreamingManager );
	
	GIOManager = new FIOManager();

	// Create the async IO manager.
#ifdef XBOX
	FAsyncIOSystemXenon*	AsyncIOSystem = new FAsyncIOSystemXenon();
#elif PS3
	FAsyncIOSystemPS3*		AsyncIOSystem = new FAsyncIOSystemPS3();
#elif _MSC_VER
	FAsyncIOSystemWindows*	AsyncIOSystem = new FAsyncIOSystemWindows();
#else
	#error implement async io manager for platform
#endif
	// This will auto- register itself with GIOMananger and be cleaned up when the manager is destroyed.
	AsyncIOThread = GThreadFactory->CreateThread( AsyncIOSystem, TEXT("AsyncIOSystem"), 0, 0, 0, TPri_AboveNormal );
	check(AsyncIOThread);
#if XBOX
	// See UnXenon.h
	AsyncIOThread->SetProcessorAffinity(ASYNCIO_HWTHREAD);
#endif

	// Init physics engine before loading anything, in case we want to do things like cook during post-load.
	InitGameRBPhys();

#if WITH_FACEFX
	// Make sure FaceFX is initialized.
	UnInitFaceFX();
#endif // WITH_FACEFX

#ifndef CONSOLE
#ifndef __GNUC__
	if( Token == TEXT("EDITOR") )
	{		
		// release our .com launcher -- this is to mimic previous behavior of detaching the console we launched from
		InheritedLogConsole.DisconnectInherited();

#if _MSC_VER
		// here we start COM (used by some console support DLLs)
		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
#endif

		// We're the editor.
		GIsClient	= 1;
		GIsServer	= 1;
		GIsEditor	= 1;
		GIsUCC		= 0;
		GGameIcon	= GEditorIcon;

		// UnrealEd requires a special callback handler and feedback context.
		GCallbackEvent	= &UnrealEdEventCallback;
		GCallbackQuery	= &UnrealEdQueryCallback;
		GWarn		= &UnrealEdWarn;

		// Remove "EDITOR" from command line.
		appStrcpy( GCmdLine, ParsedCmdLine );

		// Set UnrealEd as the current package (used for e.g. log and localization files).
		appStrcpy( GPackage, TEXT("UnrealEd") );
 
#if EPICINTERNAL_CHECK_FOR_QA_SEMAPHORE // this should be set by the build machine
		// so if we are checking for qa semaphore BUT we also are doing a warfare milestone build and we are warfare then don't check all other games do check
#if !( EPICINTERNAL_CHECK_FOR_WARFARE_MILESTONE_BUILD && ( GAMENAME == WARGAME ) )
		if( GIsEpicInternal == TRUE )
		{
			//CheckForQABuildCandidateStatus();
			if( GIsRequestingExit == TRUE )
			{
				//appPreExit()  // this crashes as we have not initialized anything yet and appPreExit assumes we have
				return 1;
			}
		}
#endif // warfare milestone build checking
#endif
	}
	else
#endif
	{
		// See whether the first token on the command line is a commandlet.

		//@hack: We need to set these before calling StaticLoadClass so all required data gets loaded for the commandlets.
		GIsClient	= 1;
		GIsServer	= 1;
		GIsEditor	= 1;
		GIsUCC		= 1;

		UClass* Class = NULL;

		// We need to disregard the empty token as we try finding Token + "Commandlet" which would result in finding the
		// UCommandlet class if Token is empty.
		if( Token.Len() )
		{
			DWORD CommandletLoadFlags = LOAD_NoWarn|LOAD_Quiet;
			UBOOL bNoFail = FALSE;
			if ( Token == TEXT("RUN") )
			{
				Token = ParseToken( ParsedCmdLine, 0);
				if ( Token.Len() == 0 )
				{
					warnf(TEXT("You must specify a commandlet to run, e.g. game.exe run test.samplecommandlet"));
					return 1;
				}
				bNoFail = TRUE;
				if ( Token == TEXT("MAKE") || Token == TEXT("MAKECOMMANDLET") )
				{
					// We can't bind to .u files if we want to build them via the make commandlet, hence the LOAD_DisallowFiles.
					CommandletLoadFlags |= LOAD_DisallowFiles;

					// allow the make commandlet to be invoked without requiring the package name
					Token = TEXT("Editor.MakeCommandlet");
				}

				if ( Token == TEXT("SERVER") || Token == TEXT("SERVERCOMMANDLET") )
				{
					// allow the server commandlet to be invoked without requiring the package name
					Token = TEXT("Engine.ServerCommandlet");
				}

				Class = LoadClass<UCommandlet>(NULL, *Token, NULL, CommandletLoadFlags, NULL );
				if ( Class == NULL && Token.InStr(TEXT("Commandlet")) == INDEX_NONE )
				{
					Class = LoadClass<UCommandlet>(NULL, *(Token+TEXT("Commandlet")), NULL, CommandletLoadFlags, NULL );
				}
			}
			else
			{
				// allow these commandlets to be invoked without requring the package name
				if ( Token == TEXT("MAKE") || Token == TEXT("MAKECOMMANDLET") )
				{
					// We can't bind to .u files if we want to build them via the make commandlet, hence the LOAD_DisallowFiles.
					CommandletLoadFlags |= LOAD_DisallowFiles;

					// allow the make commandlet to be invoked without requiring the package name
					Token = TEXT("Editor.MakeCommandlet");
				}

				else if ( Token == TEXT("SERVER") || Token == TEXT("SERVERCOMMANDLET") )
				{
					// allow the server commandlet to be invoked without requiring the package name
					Token = TEXT("Engine.ServerCommandlet");
				}
			}

			// See whether we're trying to run a commandlet. @warning: only works for native commandlets
			 
			//  Try various common name mangling approaches to find the commandlet class...
			if( !Class )
			{
				Class = FindObject<UClass>(ANY_PACKAGE,*Token,FALSE);
			}
			if( !Class )
			{
				Class = FindObject<UClass>(ANY_PACKAGE,*(Token+TEXT("Commandlet")),FALSE);
			}

			if( !Class && Token.InStr(TEXT(".")) != -1)
			{
				Class = LoadObject<UClass>(NULL,*Token,NULL,LOAD_NoWarn,NULL);
			}
			if( !Class && Token.InStr(TEXT(".")) != -1 )
			{
				Class = LoadObject<UClass>(NULL,*(Token+TEXT("Commandlet")),NULL,LOAD_NoWarn,NULL);
			}

			// ... and if successful actually load it.
			if( Class )
			{
				if ( Class->HasAnyClassFlags(CLASS_Intrinsic) )
				{
					// if this commandlet is native-only, we'll need to manually load its parent classes to ensure that it has
					// correct values in its PropertyLink array after it has been linked
					TArray<UClass*> ClassHierarchy;
					for ( UClass* ClassToLoad = Class->GetSuperClass(); ClassToLoad != NULL; ClassToLoad = ClassToLoad->GetSuperClass() )
					{
						ClassHierarchy.AddItem(ClassToLoad);
					}
					for ( INT i = ClassHierarchy.Num() - 1; i >= 0; i-- )
					{
						UClass* LoadedClass = UObject::StaticLoadClass( UObject::StaticClass(), NULL, *(ClassHierarchy(i)->GetPathName()), NULL, CommandletLoadFlags, NULL );
						check(LoadedClass);
					}
				}
				Class = UObject::StaticLoadClass( UCommandlet::StaticClass(), NULL, *Class->GetPathName(), NULL, CommandletLoadFlags, NULL );
			}
			
			if ( Class == NULL && bNoFail == TRUE )
			{
				appMsgf(AMT_OK, TEXT("Failed to find a commandlet named '%s'"), *Token);
				return 1;
			}
		}

		if( Class != NULL )
		{
			//@todo - make this block a separate function

			// The first token was a commandlet so execute it.
	
			// Remove commandlet name from command line.
			appStrcpy( GCmdLine, ParsedCmdLine );

			// Set UCC as the current package (used for e.g. log and localization files).
			appStrcpy( GPackage, TEXT("UCC") );

			// Bring up console unless we're a silent build.
			if( GLogConsole && !GIsSilent )
			{
				GLogConsole->Show( TRUE );
			}

			debugf( TEXT("Executing %s"), *Class->GetFullName() );

			// Allow commandlets to individually override those settings.
			UCommandlet* Default = CastChecked<UCommandlet>(Class->GetDefaultObject(TRUE));
			Class->ConditionalLink();

			if ( GIsRequestingExit )
			{
				// commandlet set GIsRequestingExit in StaticInitialize
				return 1;
			}

			GIsClient	= Default->IsClient;
			GIsServer	= Default->IsServer;
			GIsEditor	= Default->IsEditor;
			GIsGame		= !GIsEditor;
			GIsUCC		= TRUE;

			// Reset aux log if we don't want to log to the console window.
			if( !Default->LogToConsole )
			{
				GLog->RemoveOutputDevice( GLogConsole );
			}

#if !CONSOLE && _MSC_VER
			if( ParseParam(appCmdLine(),TEXT("AUTODEBUG")) )
			{
				debugf(TEXT("Attaching to Debugger"));
				UDebuggerCore* Debugger = new UDebuggerCore();
				GDebugger 			= Debugger;

				// we want the UDebugger to break on the first bytecode it processes
				Debugger->SetBreakASAP(TRUE);

				// we need to allow the debugger to process debug opcodes prior to the first tick, so enable the debugger.
				Debugger->NotifyBeginTick();
			}
#endif

			GIsInitialLoad		= FALSE;
			GIsRequestingExit	= TRUE;	// so CTRL-C will exit immediately

			// allow the commandlet the opportunity to create a custom engine
			Class->GetDefaultObject<UCommandlet>()->CreateCustomEngine();
			if ( GEngine == NULL )
			{
				if ( GIsEditor )
				{
					UClass* EditorEngineClass	= UObject::StaticLoadClass( UEditorEngine::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.EditorEngine"), NULL, LOAD_None, NULL );

					// must do this here so that the engine object that we create on the next line receives the correct property values
					EditorEngineClass->GetDefaultObject(TRUE);
					EditorEngineClass->ConditionalLink();
					GEngine = GEditor			= ConstructObject<UEditorEngine>( EditorEngineClass );
					debugf(TEXT("Initializing Editor Engine..."));
					GEditor->InitEditor();
					debugf(TEXT("Initializing Editor Engine Completed"));
				}
				else
				{
					UClass* EngineClass = UObject::StaticLoadClass( UEngine::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.GameEngine"), NULL, LOAD_None, NULL );
					// must do this here so that the engine object that we create on the next line receives the correct property values
					EngineClass->GetDefaultObject(TRUE);
					EngineClass->ConditionalLink();
					GEngine = ConstructObject<UEngine>( EngineClass );
					debugf(TEXT("Initializing Game Engine..."));
					GEngine->Init();
					debugf(TEXT("Initializing Game Engine Completed"));
				}
			}

			UCommandlet* Commandlet = ConstructObject<UCommandlet>( Class );
			check(Commandlet);

			// Execute the commandlet.
			DOUBLE CommandletExecutionStartTime = appSeconds();

			Commandlet->InitExecution();
			Commandlet->ParseParms( appCmdLine() );
			INT ErrorLevel = Commandlet->Main( appCmdLine() );

			// Log warning/ error summary.
			if( Commandlet->ShowErrorCount )
			{
				if( GWarn->Errors.Num() || GWarn->Warnings.Num() )
				{
					SET_WARN_COLOR(COLOR_WHITE);
					warnf(TEXT(""));
					warnf(TEXT("Warning/Error Summary"));
					warnf(TEXT("---------------------"));

					SET_WARN_COLOR(COLOR_RED);
					for(INT I = 0; I < Min(50, GWarn->Errors.Num()); I++)
					{
						warnf((TCHAR*)GWarn->Errors(I).GetCharArray().GetData());
					}
					if (GWarn->Errors.Num() > 50)
					{
						SET_WARN_COLOR(COLOR_WHITE);
						warnf(TEXT("NOTE: Only first 50 warnings displayed."));
					}

					SET_WARN_COLOR(COLOR_YELLOW);
					for(INT I = 0; I < Min(50, GWarn->Warnings.Num()); I++)
					{
						warnf((TCHAR*)GWarn->Warnings(I).GetCharArray().GetData());
					}
					if (GWarn->Warnings.Num() > 50)
					{
						SET_WARN_COLOR(COLOR_WHITE);
						warnf(TEXT("NOTE: Only first 50 warnings displayed."));
					}
				}

				warnf(TEXT(""));

				if( ErrorLevel != 0 )
				{
					SET_WARN_COLOR(COLOR_RED);
					warnf( TEXT("Commandlet->Main return this error code: %d"), ErrorLevel );
					warnf( TEXT("With %d error(s), %d warning(s)"), GWarn->Errors.Num(), GWarn->Warnings.Num() );
				}
				else if( ( GWarn->Errors.Num() == 0 ) )
				{
					SET_WARN_COLOR(GWarn->Warnings.Num() ? COLOR_YELLOW : COLOR_GREEN);
					warnf( TEXT("Success - %d error(s), %d warning(s)"), GWarn->Errors.Num(), GWarn->Warnings.Num() );
				}
				else
				{
					SET_WARN_COLOR(COLOR_RED);
					warnf( TEXT("Failure - %d error(s), %d warning(s)"), GWarn->Errors.Num(), GWarn->Warnings.Num() );
					ErrorLevel = 1;
				}
				CLEAR_WARN_COLOR();
			}
			else
			{
				warnf( TEXT("Finished.") );
			}
		
			DOUBLE CommandletExecutionEndTime = appSeconds();
			warnf( TEXT("\nExecution of commandlet took:  %.2f seconds"), CommandletExecutionEndTime - CommandletExecutionStartTime );

			// We're ready to exit!
			return ErrorLevel;
		}
		else
#else
	{
#endif
		{
			// We're a regular client.
			GIsClient		= 1;
			GIsServer		= 0;
#ifndef CONSOLE
			GIsEditor		= 0;
			GIsUCC			= 0;
#endif

// handle launching dedicated server on Xenon
#ifdef XBOX
			// copy command line
			TCHAR* CommandLine	= new TCHAR[ appStrlen(appCmdLine())+1 ];
			appStrcpy( CommandLine, appCmdLine() );	
			// parse first token
			const TCHAR* ParsedCmdLine	= CommandLine;
			FString Token = ParseToken( ParsedCmdLine, 0);
			Token = Token.Trim();
			// dedicated server command line option
			if( Token == TEXT("SERVER") )
			{
				GIsClient = 0;
				GIsServer = 1;

				// Remove commandlet name from command line
				appStrcpy( GCmdLine, ParsedCmdLine );
				// show server splash screen
				appXenonShowSplash(TEXT("Splash\\SplashServer.bmp"));
			}		
#endif //XBOX

#if !FINAL_RELEASE
			// regular clients can listen for propagation requests
			FObjectPropagator::SetPropagator(&ListenPropagator);
#endif
		}
	}

	// at this point, GIsGame is always not GIsEditor, because the only other way to set GIsGame is to be in a PIE world
	GIsGame = !GIsEditor;

#if EPICINTERNAL_CHECK_FOR_WARFARE_MILESTONE_BUILD
	CheckForWarfareMiletoneBuild(); // this will set GIsRequestingExit
#endif

	// Exit if wanted.
	if( GIsRequestingExit )
	{
		if ( GEngine != NULL )
		{
			GEngine->PreExit();
		}
		appPreExit();
		// appExit is called outside guarded block.
		return 1;
	}

	// Movie recording.
	GIsDumpingMovie	= ParseParam(appCmdLine(),TEXT("DUMPMOVIE"));

	// Benchmarking.
	GIsBenchmarking	= ParseParam(appCmdLine(),TEXT("BENCHMARK"));
	
#if !defined(CONSOLE) && defined(_MSC_VER)
	if(!GIsBenchmarking)
	{
		// release our .com launcher -- this is to mimic previous behavior of detaching the console we launched from
		InheritedLogConsole.DisconnectInherited();
	}
#endif

	// Don't update ini files if benchmarking or -noini
	if( GIsBenchmarking || ParseParam(appCmdLine(),TEXT("NOINI")))
	{
		GConfig->Detach( GEngineIni );
		GConfig->Detach( GInputIni );
		GConfig->Detach( GGameIni );
		GConfig->Detach( GEditorIni );
	}

#if !CONSOLE
	// -forceshadermodel2 will force the shader model 2 path even on sm3 hardware
	if (ParseParam(appCmdLine(),TEXT("FORCESHADERMODEL2")))
	{
		GRHIShaderPlatform = SP_PCD3D_SM2;
		GSupportsFPBlending = FALSE;
	}
#endif

	// -onethread will disable renderer thread
	if (!ParseParam(appCmdLine(),TEXT("ONETHREAD")))
	{
		// Create the rendering thread.  Note that commandlets don't make it to this point.
		if(GNumHardwareThreads > 1)
		{
			GUseThreadedRendering = TRUE;
			StartRenderingThread();
		}
	}
	return 0;
}

/** Whether to use latent occlusion queries or not */
extern UBOOL GUseLatentOcclusionQueries;

INT FEngineLoop::Init()
{
#if !USE_SEEKFREE_LOADING
	// Load the shader caches.
	LoadShaderCaches();
#endif

#if CHECK_NATIVE_CLASS_SIZES
	// Load all native script packages for verification afterwards.
	LoadAllNativeScriptPackages( FALSE );
#endif

#if USE_SEEKFREE_LOADING
	// Kick off async preloads of all startup packages, and fully load all native packages.
	// This is a superset of LoadAllNativeScriptPackages, which is done by the cooker, etc
	TArray<FString> NonScriptStartupPackageNames;
	PreloadStartupPackagesForSeekFree(NonScriptStartupPackageNames);

	// Iterate over all class objects and force the default objects to be created. Additionally also
	// assembles the token reference stream at this point. This is required for class objects that are
	// not taken into account for garbage collection but have instances that are.
	for( TObjectIterator<UClass> It; It; ++It )
	{
		UClass*	Class = *It;
		// Force the default object to be created.
		Class->GetDefaultObject( TRUE );
		// Assemble reference token stream for garbage collection/ RTGC.
		Class->AssembleReferenceTokenStream();
	}

	// Iterate over all objects and mark them to be part of root set.
	for( FObjectIterator It; It; ++It )
	{
		UObject*		Object		= *It;
		ULinkerLoad*	LinkerLoad	= Cast<ULinkerLoad>(Object);
		// Exclude linkers from root set.
		if( LinkerLoad == NULL || LinkerLoad->HasAnyFlags(RF_ClassDefaultObject) )
		{
			Object->AddToRoot();
		}
	}
	debugf(TEXT("%i objects alive at end of initial load."), UObject::GetObjectArrayNum());
#endif
	GIsInitialLoad = FALSE;

	// Figure out which UEngine variant to use.
	UClass* EngineClass = NULL;
	if( !GIsEditor )
	{
		// We're the game.
		EngineClass = UObject::StaticLoadClass( UGameEngine::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.GameEngine"), NULL, LOAD_None, NULL );
		GEngine = ConstructObject<UEngine>( EngineClass );
	}
	else
	{
#if !defined(CONSOLE) && !PLATFORM_UNIX
		// We're UnrealEd.
		EngineClass = UObject::StaticLoadClass( UUnrealEdEngine::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.UnrealEdEngine"), NULL, LOAD_None, NULL );
		GEngine = GEditor = GUnrealEd = ConstructObject<UUnrealEdEngine>( EngineClass );

		//@todo: put this in a better place? i think we want it for all editor, unrealed, ucc, etc
		// load any Console support modules that exist
		GConsoleSupportContainer->LoadAllConsoleSupportModules();

		debugf(TEXT("Supported Consoles:"));
		for (FConsoleSupportIterator It; It; ++It)
		{
			debugf(TEXT("  %s"), It->GetConsoleName());
		}
#else
		check(0);
#endif
	}

	// If the -nosound or -benchmark parameters are used, disable sound.
	if(ParseParam(appCmdLine(),TEXT("nosound")) || GIsBenchmarking)
	{
		GEngine->bUseSound = FALSE;
	}

#if USE_SEEKFREE_LOADING
	// fully load the packages that were previously kicked off earlier (except the map)
	LoadPackageList(NonScriptStartupPackageNames);
#endif

	check( GEngine );
	debugf(TEXT("Initializing Engine..."));
	GEngine->Init();
	debugf(TEXT("Initializing Engine Completed"));

	// Verify native class sizes and member offsets.
	CheckNativeClassSizes();

	// Init variables used for benchmarking and ticking.
	OldRealTime					= appSeconds();
	GCurrentTime				= appSeconds();
	FrameCounter				= 0;
	MaxFrameCounter				= 0;
	LastFrameCycles				= appCycles();

	Parse(appCmdLine(),TEXT("SECONDS="),MaxFrameCounter);
	MaxFrameCounter				*= 30;

	// Optionally Exec an exec file
	FString Temp;
	if( Parse(appCmdLine(), TEXT("EXEC="), Temp) )
	{
		Temp = FString(TEXT("exec ")) + Temp;
		UGameEngine* Engine = Cast<UGameEngine>(GEngine);
		if ( Engine != NULL && Engine->GamePlayers.Num() && Engine->GamePlayers(0) )
		{
			Engine->GamePlayers(0)->Exec( *Temp, *GLog );
		}
	}

	GIsRunning		= TRUE;

	// let the propagator do it's thing now that we are done initializing
	GObjectPropagator->Unpause();

	// Only enable latent occlusion queries outside of the editor.
	GUseLatentOcclusionQueries = !GIsEditor;

	warnf(TEXT(">>>>>>>>>>>>>> Initial startup: %.2fs <<<<<<<<<<<<<<<"), appSeconds() - GStartTime);
	return 0;
}

void FEngineLoop::Exit()
{
	GIsRunning	= 0;
	GLogConsole	= NULL;

#if !USE_SEEKFREE_LOADING
	// Save the local shader cache if it has changed.
	// This avoids loss of shader compilation work due to crashing.
    // GEMINI_TODO: Revisit whether this is worth thet slowdown in material editing once the engine has stabilized.
	SaveLocalShaderCache();
#endif

	// Output benchmarking data.
	if( GIsBenchmarking )
	{
		FLOAT	MinFrameTime = 1000.f,
				MaxFrameTime = 0.f,
				AvgFrameTime = 0;

		// Determine min/ max framerate (discarding first 10 frames).
		for( INT i=10; i<FrameTimes.Num(); i++ )
		{		
			MinFrameTime = Min( MinFrameTime, FrameTimes(i) );
			MaxFrameTime = Max( MaxFrameTime, FrameTimes(i) );
			AvgFrameTime += FrameTimes(i);
		}
		AvgFrameTime /= FrameTimes.Num() - 10;

		// Output results to Benchmark/benchmark.log
		FString OutputString = TEXT("");
		appLoadFileToString( OutputString, *(appGameDir() + TEXT("Logs\\benchmark.log")) );
		OutputString += FString::Printf(TEXT("min= %6.2f   avg= %6.2f   max= %6.2f%s"), 1000.f / MaxFrameTime, 1000.f / AvgFrameTime, 1000.f / MinFrameTime, LINE_TERMINATOR );
		appSaveStringToFile( OutputString, *(appGameDir() + TEXT("Logs\\benchmark.log")) );

		FrameTimes.Empty();
	}

	// Make sure we're not in the middle of loading something.
	UObject::FlushAsyncLoading();
	// Block till all outstanding resource streaming requests are fulfilled.
	GStreamingManager->BlockTillAllRequestsFinished();
	
	if ( GEngine != NULL )
	{
		GEngine->PreExit();
	}
	appPreExit();
	DestroyGameRBPhys();

#if WITH_FACEFX
	// Make sure FaceFX is shutdown.
	UnShutdownFaceFX();
#endif // WITH_FACEFX

	// Stop the rendering thread.
	StopRenderingThread();

	delete GStreamingManager;
	GStreamingManager	= NULL;

	delete GIOManager;
	GIOManager			= NULL;

	GThreadFactory->Destroy( AsyncIOThread );

#if STATS
	// Shutdown stats
	GStatManager.Destroy();
#endif
	// Shutdown sockets layer
	appSocketExit();
}

void FEngineLoop::Tick()
{
	if (GHandleDirtyDiscError)
	{
		appSleepInfinite();
	}

	// Flush debug output which has been buffered by other threads.
	GLog->FlushThreadedLogs();

#if !USE_SEEKFREE_LOADING
	// If the local shader cache has changed, save it.
	static UBOOL bIsFirstTick = TRUE;
	if (!GIsEditor || bIsFirstTick)
	{
		SaveLocalShaderCache();
		bIsFirstTick = FALSE;
	}
#endif

#if TRACK_DECOMPRESS_TIME
    extern DOUBLE GTotalUncompressTime;
    static DOUBLE LastUncompressTime = 0.0;
    if (LastUncompressTime != GTotalUncompressTime)
    {
	    debugf(TEXT("Total decompression time for load was %.3f seconds"), GTotalUncompressTime);
	    LastUncompressTime = GTotalUncompressTime;
    }
#endif

	if( GDebugger )
	{
		GDebugger->NotifyBeginTick();
	}

	// Calculate real delta time.

	if( GIsBenchmarking && MaxFrameCounter && (FrameCounter > MaxFrameCounter) )
	{
		appRequestExit(0);
	}

	FLOAT DeltaTime;
	if( GIsBenchmarking )
	{
		DeltaTime		= 1.0f / 30.0f;
		OldRealTime		= GCurrentTime;
		GCurrentTime	+= DeltaTime;
	}
	else
	{
		GCurrentTime	= appSeconds();
		DeltaTime		= GCurrentTime - OldRealTime;
		OldRealTime		= GCurrentTime;
	}

	ENQUEUE_UNIQUE_RENDER_COMMAND(
		ResetDeferredUpdates,
	{
		FDeferredUpdateResource::ResetNeedsUpdate();
	});

	// Update.
	GEngine->Tick( DeltaTime );

	// Update RHI.
	RHITick( DeltaTime );

	FrameCounter++;

	// Enforce optional maximum tick rate.
	if( !GIsBenchmarking )
	{		
		FLOAT MaxTickRate = GEngine->GetMaxTickRate();
		if( MaxTickRate>0.0 )
		{
			FLOAT Delta = (1.0/MaxTickRate) - (appSeconds()-OldRealTime);
			appSleep( Max(0.f,Delta) );
		}
	}

	// Find the objects which need to be cleaned up the next frame.
	FPendingCleanupObjects* PreviousPendingCleanupObjects = PendingCleanupObjects;
	PendingCleanupObjects = GetPendingCleanupObjects();

	// Keep track of this frame's location in the rendering command queue.
	PendingFrameFence.BeginFence();

	// Give the rendering thread time to catch up if it's more than one frame behind.
	PendingFrameFence.Wait( 1 );

	// Delete the objects which were enqueued for deferred cleanup before the previous frame.
	delete PreviousPendingCleanupObjects;

#if !defined(CONSOLE) && !PLATFORM_UNIX
	// Handle all incoming messages if we're not using wxWindows in which case this is done by their
	// message loop.
	if( !GUsewxWindows )
	{
		MSG Msg;
		while( PeekMessageX(&Msg,NULL,0,0,PM_REMOVE) )
		{
			TranslateMessage( &Msg );
			DispatchMessageX( &Msg );
		}
	}

	// If editor thread doesn't have the focus, don't suck up too much CPU time.
	if( GIsEditor )
	{
		static UBOOL HadFocus=1;
		UBOOL HasFocus = (GetWindowThreadProcessId(GetForegroundWindow(),NULL) == GetCurrentThreadId() );
		if( HadFocus && !HasFocus )
		{
			// Drop our priority to speed up whatever is in the foreground.
			SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL );
		}
		else if( HasFocus && !HadFocus )
		{
			// Boost our priority back to normal.
			SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );
		}
		if( !HasFocus )
		{
			// Sleep for a bit to not eat up all CPU time.
			appSleep(0.005f);
		}
		HadFocus = HasFocus;
	}
#endif

	if( GIsBenchmarking )
	{
#if STATS
		FrameTimes.AddItem( GFPSCounter.GetFrameTime() * 1000.f );
#endif
	}

#ifdef XBOX
	// Handle remote debugging commands.
	{
		FScopeLock ScopeLock(&RemoteDebugCriticalSection);
		new(GEngine->DeferredCommands) FString(ANSI_TO_TCHAR(RemoteDebugCommand));
		RemoteDebugCommand[0] = '\0';
	}
#endif

#if PS3
	extern void appPlatformTick( FLOAT DeltaTime );
	appPlatformTick( DeltaTime );
#endif

	if( GEngine->GamePlayers.Num() && GEngine->GamePlayers(0) )
	{
		ULocalPlayer* Player = GEngine->GamePlayers(0);
		for( INT DeferredCommandsIndex=0; DeferredCommandsIndex<GEngine->DeferredCommands.Num(); DeferredCommandsIndex++ )
		{
			Player->Exec( *GEngine->DeferredCommands(DeferredCommandsIndex), *GLog );
		}
		GEngine->DeferredCommands.Empty();
	}
}


/*-----------------------------------------------------------------------------
	Static linking support.
-----------------------------------------------------------------------------*/

/**
 * Initializes all registrants and names for static linking support.
 */
void InitializeRegistrantsAndRegisterNames()
{
	// Static linking.
	for( INT k=0; k<ARRAY_COUNT(GNativeLookupFuncs); k++ )
	{
		GNativeLookupFuncs[k] = NULL;
	}
	INT Lookup = 0;
	// Auto-generated lookups and statics
	AutoInitializeRegistrantsCore( Lookup );
	AutoInitializeRegistrantsEngine( Lookup );
	AutoInitializeRegistrantsGameFramework( Lookup );
	AutoInitializeRegistrantsUnrealScriptTest( Lookup );
	AutoInitializeRegistrantsIpDrv( Lookup );
#if defined(XBOX)
	AutoInitializeRegistrantsXeAudio( Lookup );
	AutoInitializeRegistrantsXeDrv( Lookup );
	AutoInitializeRegistrantsOnlineSubsystemLive( Lookup );
#elif PS3
	AutoInitializeRegistrantsPS3Drv( Lookup );
#elif PLATFORM_UNIX
	AutoInitializeRegistrantsLinuxDrv( Lookup );
#else
	AutoInitializeRegistrantsEditor( Lookup );
	AutoInitializeRegistrantsUnrealEd( Lookup );
	AutoInitializeRegistrantsALAudio( Lookup );
	AutoInitializeRegistrantsWinDrv( Lookup );
#endif
#if   GAMENAME == WARGAME
	AutoInitializeRegistrantsWarfareGame( Lookup );
#if !CONSOLE
	AutoInitializeRegistrantsWarfareEditor( Lookup );
#endif
#if defined(XBOX)
	AutoInitializeRegistrantsVinceOnlineSubsystemLive( Lookup );
#endif
#elif GAMENAME == GEARGAME
	AutoInitializeRegistrantsGearGame( Lookup );
#if !CONSOLE
	AutoInitializeRegistrantsGearEditor( Lookup );
#endif
#elif GAMENAME == UTGAME
	AutoInitializeRegistrantsUTGame( Lookup );
#if !CONSOLE
	AutoInitializeRegistrantsUTEditor( Lookup );
#endif

#elif GAMENAME == EXAMPLEGAME
	AutoInitializeRegistrantsExampleGame( Lookup );
#if !CONSOLE
	AutoInitializeRegistrantsExampleEditor( Lookup );
#endif
#else
	#error Hook up your game name here
#endif
	// It is safe to increase the array size of GNativeLookupFuncs if this assert gets triggered.
	check( Lookup < ARRAY_COUNT(GNativeLookupFuncs) );

//	AutoGenerateNames* declarations.
	AutoGenerateNamesCore();
	AutoGenerateNamesEngine();
	AutoGenerateNamesGameFramework();
	AutoGenerateNamesUnrealScriptTest();
#if !defined(CONSOLE) && !PLATFORM_UNIX
	AutoGenerateNamesEditor();
	AutoGenerateNamesUnrealEd();
#endif
	AutoGenerateNamesIpDrv();
#if XBOX
	AutoGenerateNamesOnlineSubsystemLive();
#endif
#if   GAMENAME == WARGAME
	AutoGenerateNamesWarfareGame();
#if !CONSOLE
	AutoGenerateNamesWarfareEditor();
#endif
#if XBOX
	AutoGenerateNamesVinceOnlineSubsystemLive();
#endif
#elif GAMENAME == GEARGAME
	AutoGenerateNamesGearGame();
#if !CONSOLE
	AutoGenerateNamesGearEditor();
#endif
#elif GAMENAME == UTGAME
	AutoGenerateNamesUTGame();
#if !CONSOLE
	AutoGenerateNamesUTEditor();
#endif

#elif GAMENAME == EXAMPLEGAME
	AutoGenerateNamesExampleGame();
#if !CONSOLE
	AutoGenerateNamesExampleEditor();
#endif
#else
	#error Hook up your game name here
#endif
}

/*-----------------------------------------------------------------------------
	Remote debug channel support.
-----------------------------------------------------------------------------*/

#if (defined XBOX) && ALLOW_NON_APPROVED_FOR_SHIPPING_LIB

static int dbgstrlen( const CHAR* str )
{
    const CHAR* strEnd = str;
    while( *strEnd )
        strEnd++;
    return strEnd - str;
}
static inline CHAR dbgtolower( CHAR ch )
{
    if( ch >= 'A' && ch <= 'Z' )
        return ch - ( 'A' - 'a' );
    else
        return ch;
}
static INT dbgstrnicmp( const CHAR* str1, const CHAR* str2, int n )
{
    while( n > 0 )
    {
        if( dbgtolower( *str1 ) != dbgtolower( *str2 ) )
            return *str1 - *str2;
        --n;
        ++str1;
        ++str2;
    }
    return 0;
}
static void dbgstrcpy( CHAR* strDest, const CHAR* strSrc )
{
    while( ( *strDest++ = *strSrc++ ) != 0 );
}

HRESULT __stdcall DebugConsoleCmdProcessor( const CHAR* Command, CHAR* Response, DWORD ResponseLen, PDM_CMDCONT pdmcc )
{
	// Skip over the command prefix and the exclamation mark.
	Command += dbgstrlen("UNREAL") + 1;

	// Check if this is the initial connect signal
	if( dbgstrnicmp( Command, "__connect__", 11 ) == 0 )
	{
		// If so, respond that we're connected
		lstrcpynA( Response, "Connected.", ResponseLen );
		return XBDM_NOERR;
	}

	{
		FScopeLock ScopeLock(&RemoteDebugCriticalSection);
		if( RemoteDebugCommand[0] )
		{
			// This means the application has probably stopped polling for debug commands
			dbgstrcpy( Response, "Cannot execute - previous command still pending." );
		}
		else
		{
			dbgstrcpy( RemoteDebugCommand, Command );
		}
	}

	return XBDM_NOERR;
}

#endif

/** 
* Returns whether the line can be broken between these two characters
*/
UBOOL appCanBreakLineAt( TCHAR Previous, TCHAR Current )
{
#if (GAMENAME == WARGAME) && !PS3
	// This function is part of code from the Microsoft's July XDK and so cannot be used on the PS3
	extern bool WordWrap_CanBreakLineAt( WCHAR prev, WCHAR curr );

	return( WordWrap_CanBreakLineAt( Previous, Current ) );
#else
	return( appIsPunct( Current ) || appIsWhitespace( Current ) );
#endif
}

