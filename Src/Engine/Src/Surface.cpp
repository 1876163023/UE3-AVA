/*=============================================================================
	Surface.cpp: USurface implementation.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

IMPLEMENT_CLASS(USurface);

void USurface::execGetSurfaceWidth _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(FLOAT*)Result = GetSurfaceWidth();
}

void USurface::execGetSurfaceHeight _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(FLOAT*)Result = GetSurfaceHeight();
}
