/*=============================================================================
	UnFaceFXSupport.cpp: FaceFX support.
	Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#if WITH_FACEFX

#include "UnFaceFXSupport.h"
#include "UnFaceFXRegMap.h"
#include "UnFaceFXMaterialParameterProxy.h"
#include "UnFaceFXMorphTargetProxy.h"

#include "../../../External/FaceFX/FxSDK/Inc/UnFaceFXNode.h"
#include "../../../External/FaceFX/FxSDK/Inc/UnFaceFXMaterialNode.h"
#include "../../../External/FaceFX/FxSDK/Inc/UnFaceFXMorphNode.h"

// Define this to link in the debug libraries.
//#define USE_FACEFX_DEBUG_LIBS

#if defined(XBOX)

#if defined(_DEBUG) && defined(USE_FACEFX_DEBUG_LIBS)
	#pragma message("Linking Xenon DEBUG FaceFX Libs")
	#pragma comment(lib, "External/FaceFX/FxSDK/lib/xbox360/DEBUG-FxSDK_XBox360.lib")
#else
	#pragma message("Linking Xenon RELEASE FaceFX Libs")
	#pragma comment(lib, "External/FaceFX/FxSDK/lib/xbox360/FxSDK_XBox360.lib")
#endif

#elif defined(PS3)

#else // WIN32

#if defined(_DEBUG) && defined(USE_FACEFX_DEBUG_LIBS)
	#pragma message("Linking Win32 DEBUG FaceFX Libs")
	#pragma comment(lib, "../../External/FaceFX/FxSDK/lib/win32/DEBUG-FxSDK.lib")
#else
	#pragma message("Linking Win32 RELEASE FaceFX Libs")
	#pragma comment(lib, "../../External/FaceFX/FxSDK/lib/win32/FxSDK.lib")
#endif

#endif

using namespace OC3Ent;
using namespace Face;

void* FxAllocateUE3( FxSize NumBytes )
{
	return appMalloc(NumBytes);
}

void* FxAllocateDebugUE3( FxSize NumBytes, const FxAChar* /*system*/ )
{
	return appMalloc(NumBytes);
}

void FxFreeUE3( void* Ptr, FxSize /*n*/ )
{
	appFree(Ptr);
}

void UnInitFaceFX( void )
{
	debugf(TEXT("Initializing FaceFX..."));
	FxMemoryAllocationPolicy allocPolicyUE3(MAT_Custom, FxFalse, FxAllocateUE3, FxAllocateDebugUE3, FxFreeUE3);
	FxSDKStartup(allocPolicyUE3);
	FFaceFXMaterialParameterProxy::StaticClass();
	FFaceFXMorphTargetProxy::StaticClass();
	FFaceFXRegMap::Startup();
	debugf(TEXT("FaceFX initialized:"));
	debugf(TEXT("    version  %f"), static_cast<FxReal>(FxSDKGetVersion()/1000.0f));
	debugf(TEXT("    licensee %s"), *FString(FxSDKGetLicenseeName().GetCstr()));
	debugf(TEXT("    project  %s"), *FString(FxSDKGetLicenseeProjectName().GetCstr()));
}

void UnShutdownFaceFX( void )
{
	debugf(TEXT("Shutting down FaceFX..."));
	FFaceFXRegMap::Shutdown();
	FxSDKShutdown();
	debugf(TEXT("FaceFX shutdown."));
}

TArray<FFaceFXRegMapEntry> FFaceFXRegMap::RegMap;

void FFaceFXRegMap::Startup( void )
{
}

void FFaceFXRegMap::Shutdown( void )
{
	RegMap.Empty();
}

FFaceFXRegMapEntry* FFaceFXRegMap::GetRegisterMapping( const FName& RegName )
{
	INT NumRegMapEntries = RegMap.Num();
	for( INT i = 0; i < NumRegMapEntries; ++i )
	{
		if( RegMap(i).UnrealRegName == RegName )
		{
			return &RegMap(i);
		}
	}
	return NULL;
}

void FFaceFXRegMap::AddRegisterMapping( const FName& RegName )
{
	if( !GetRegisterMapping(RegName) )
	{
		RegMap.AddItem(FFaceFXRegMapEntry(RegName, FxName(TCHAR_TO_ANSI(*RegName.ToString()))));
	}
}

#endif // WITH_FACEFX

