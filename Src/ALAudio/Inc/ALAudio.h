/**
 * Copyright ?1997-2005 Epic Games, Inc. All Rights Reserved.
 */

#ifndef _INC_ALAUDIO
#define _INC_ALAUDIO

/*-----------------------------------------------------------------------------
	Dependencies.
-----------------------------------------------------------------------------*/

#include "Core.h"
#include "Engine.h"

/*-----------------------------------------------------------------------------
	Audio public includes.
-----------------------------------------------------------------------------*/

#include "ALAudioDevice.h"

// OpenAL function prototypes.
#define AL_EXT( name, strname ) extern UBOOL SUPPORTS##name;
#define AL_PROC( name, strname, ret, func, parms ) extern ret ( CDECL * func ) parms;
//#include "ALFuncs.h" 
#undef AL_EXT
#undef AL_PROC

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

#endif

