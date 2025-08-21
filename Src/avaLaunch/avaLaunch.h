/*=============================================================================
LaunchPrivate.h: Unreal launcher.
Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma warning(disable:4819)
/** Whether to use the call stack capturing malloc proxy */
#define KEEP_ALLOCATION_BACKTRACE	0
/** Whether to start with capture enabled */
#define START_BACKTRACE_ON_LAUNCH	1

#if !FINAL_RELEASE
//@warning: this needs to be the very first include
#include "UnrealEd.h"
#endif

#include "Engine.h"
#include "UnIpDrv.h"
#include "WinDrv.h"
#include "avaNet.h"

// Includes for CIS.
#if CHECK_NATIVE_CLASS_SIZES
#include "EngineMaterialClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineSequenceClasses.h"
//#include "EngineUserInterfaceClasses.h" // included by Editor.h which is Included by UnrealEd.h above
#include "EngineUISequenceClasses.h"
#include "EngineSoundClasses.h"
#include "EngineInterpolationClasses.h"
//#include "EngineParticleClasses.h" // included by UnrealEd.h above
#include "EngineAIClasses.h"
#include "EngineAnimClasses.h"
#include "EnginePrefabClasses.h"
#include "EngineDSPClasses.h"
//#include "EngineTerrainClasses.h" // included by UnTerrain.h below
#include "UnTerrain.h"
#include "UnCodecs.h"
#include "GameFrameworkClasses.h"
#include "avaGameClasses.h"
#if !FINAL_RELEASE
#include "avaGameEditorClasses.h"
#endif
#include "EditorPrivate.h"
#include "ALAudio.h"
#include "OpenGLPC.h"
#include "D3DDrv.h"
#include "NullRHI.h"
#endif

#include "FMallocAnsi.h"
#include "FMallocDebug.h"
#include "FMallocWindows.h"
#include "FMallocDebugProxyWindows.h"
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
