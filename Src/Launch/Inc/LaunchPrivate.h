/*=============================================================================
	LaunchPrivate.h: Unreal launcher.
	Copyright © 1997-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#define WARGAME		1
#define UTGAME		2
#define EXAMPLEGAME 3
#define GEARGAME	4

/** Whether to use the call stack capturing malloc proxy */
#define KEEP_ALLOCATION_BACKTRACE	0
/** Whether to start with capture enabled */
#define START_BACKTRACE_ON_LAUNCH	0

//@warning: this needs to be the very first include
#include "UnrealEd.h"

#include "Engine.h"
#include "UnIpDrv.h"
#include "WinDrv.h"

// Includes for CIS.
#if CHECK_NATIVE_CLASS_SIZES
#include "EngineMaterialClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineSequenceClasses.h"
//#include "EngineUserInterfaceClasses.h" // included by Editor.h which is Included by UnrealEd.h above
#include "EngineUIPrivateClasses.h"
#include "EngineUISequenceClasses.h"
#include "EngineSoundClasses.h"
#include "EngineInterpolationClasses.h"
//#include "EngineParticleClasses.h" // included by UnrealEd.h above
#include "EngineAIClasses.h"
#include "EngineAnimClasses.h"
#include "EngineDecalClasses.h"
#include "EnginePrefabClasses.h"
//#include "EngineTerrainClasses.h" // included by UnTerrain.h below
#include "UnTerrain.h"
#include "UnCodecs.h"
#include "GameFrameworkClasses.h"
#include "UnrealScriptTestClasses.h"
#include "UnrealEdPrivateClasses.h"
#if GAMENAME == WARGAME
#include "WarfareGameClasses.h"
#include "WarfareGameCameraClasses.h"
#include "WarfareGameSequenceClasses.h"
#include "WarfareGameSpecialMovesClasses.h"
#include "WarfareGameVehicleClasses.h"
#include "WarfareGameSoundClasses.h"
#include "WarfareGameAIClasses.h"
#include "WarfareEditorClasses.h"
#elif GAMENAME == GEARGAME
#include "GearGameClasses.h"
#include "GearGameCameraClasses.h"
#include "GearGameSequenceClasses.h"
#include "GearGameSpecialMovesClasses.h"
#include "GearGameVehicleClasses.h"
#include "GearGameSoundClasses.h"
#include "GearGameAIClasses.h"
#include "GearEditorClasses.h"
#elif GAMENAME == UTGAME
#include "UTGame.h"
#include "UTEditorClasses.h"
#elif GAMENAME == EXAMPLEGAME
#include "ExampleGameClasses.h"
#include "ExampleEditorClasses.h"
#else
#error Hook up your game name here
#endif
#include "EditorPrivate.h"
#include "ALAudio.h"
#include "D3DDrv.h"
#include "NullRHI.h"
#endif

#include "FMallocAnsi.h"
#include "FMallocDebug.h"
#include "FMallocWindows.h"
#include "FMallocDebugProxyWindows.h"
#include "FMallocProxySimpleTrack.h"
#include "FMallocThreadSafeProxy.h"
#include "FFeedbackContextAnsi.h"
#include "FFeedbackContextWindows.h"
#include "FFileManagerWindows.h"
#include "FCallbackDevice.h"
#include "FConfigCacheIni.h"
#include "LaunchEngineLoop.h"
#include "UnThreadingWindows.h"

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
