/*=============================================================================
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __REMOTECONTROL_H__
#define __REMOTECONTROL_H__

#if !CONSOLE && !defined(FINAL_RELEASE)
#define	USING_REMOTECONTROL 1
#include "RemoteControlExec.h"
/**
 * Registers factories for the standard RemoteControl pages.
 */
void RegisterCoreRemoteControlPages();
#else
#define USING_REMOTECONTROL 0
#endif

#endif // __REMOTECONTROL_H__
