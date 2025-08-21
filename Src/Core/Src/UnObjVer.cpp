/*=============================================================================
	UnObjVer.cpp: Unreal version definitions.
	Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

// Defined separately so the build script can get to it easily (DO NOT CHANGE THIS MANUALLY)
#define ENGINE_VERSION	2719
#define AVA_VERSION	1409
#define AVA_BUILT_FROM_CHANGELIST 25438

#define	BUILT_FROM_CHANGELIST	166108

INT GAVAVersion					= AVA_VERSION;
INT	GAVABuiltFromChangeList		= AVA_BUILT_FROM_CHANGELIST;

INT	GEngineVersion				= ENGINE_VERSION;
INT	GBuiltFromChangeList		= BUILT_FROM_CHANGELIST;

INT	GEngineMinNetVersion		= 2327;
INT	GEngineNegotiationVersion	= 2327;

// @see UnObjVer.h for the list of changes/defines
INT	GPackageFileVersion			= VER_LATEST_ENGINE;
INT	GPackageFileMinVersion		= 225; // Code still handles 224 if saved with changelist #93355 or later

//!{ 2006-05-08	Çã Ã¢ ¹Î
INT	GPackageFileLicenseeVersion = VER_LATEST_AVA;
//!} 2006-05-08	Çã Ã¢ ¹Î
INT GPackageFileCookedContentVersion = VER_LATEST_COOKED_PACKAGE;

