/*=============================================================================
	FaceFXStudioLibs.cpp: Code for linking in the FaceFX Studio libraries.
	Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"

#if WITH_FACEFX

#include "../../../External/FaceFX/FxStudio/Inc/FxStudioMainWin.h"

// This is only here to force the compiler to link in the libraries!
OC3Ent::Face::FxStudioMainWin* GFaceFXStudio = NULL;

// Define this to link in the debug libraries.
//#define USE_FACEFX_DEBUG_LIBS

#if defined(_DEBUG) && defined(USE_FACEFX_DEBUG_LIBS)
	#pragma message("Linking Win32 DEBUG FaceFX Studio Libs")
	#pragma comment(lib, "../../External/FaceFX/FxCG/lib/win32/DEBUG-FxCG.lib")
	#pragma comment(lib, "../../External/FaceFX/FxStudio/Analysis/FxAnalysisFonix/lib/win32/FxAnalysisFonix.lib")
//	#pragma comment(lib, "../../External/FaceFX/FxStudio/lib/win32/DEBUG-FxStudio.lib")
#else
	#pragma message("Linking Win32 RELEASE FaceFX Studio Libs")
	#pragma comment(lib, "../../External/FaceFX/FxCG/lib/win32/FxCG.lib")
	#pragma comment(lib, "../../External/FaceFX/FxStudio/Analysis/FxAnalysisFonix/lib/win32/FxAnalysisFonix.lib")
//	#pragma comment(lib, "../../External/FaceFX/FxStudio/lib/win32/FxStudio.lib")
#endif

#endif // WITH_FACEFX

