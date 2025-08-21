/**
 *
 * Copyright © 1998-2007 Epic Games, Inc. All Rights Reserved.
 */


/*===========================================================================
    Class and struct declarations which are coupled to
	EngineUserInterfaceClasses.h but shouldn't be declared inside a class
===========================================================================*/

#ifndef NAMES_ONLY

#ifndef __ENGINEUSERINTERFACEGLOBALINCLUDES_H__
#define __ENGINEUSERINTERFACEGLOBALINCLUDES_H__

struct HUIHitProxy : HObject
{
	DECLARE_HIT_PROXY(HUIHitProxy,HObject);

	class UUIScreenObject* UIObject;

	HUIHitProxy(class UUIScreenObject* InObject);
	virtual EMouseCursor GetMouseCursor()
	{
		return MC_Cross;
	}
};

/**
 * FRawInputKeyEventData::ModifierKeyFlags bit values.
 * @note:	if more values are needed, the FRawInputKeyEventData::ModifierKeyFlags
 *			variable must be changed from a byte to an int
 */
enum EInputKeyModifierFlag
{
	/** Alt required */
	KEYMODIFIER_AltRequired		=	0x01,
	/** Ctrl required */
	KEYMODIFIER_CtrlRequired	=	0x02,
	/** Shift required */
	KEYMODIFIER_ShiftRequired	=	0x04,
	/** Alt excluded */
	KEYMODIFIER_AltExcluded		=	0x08,
	/** Ctrl excluded */
	KEYMODIFIER_CtrlExcluded	=	0x10,
	/** Shift excluded */
	KEYMODIFIER_ShiftExcluded	=	0x20,
	/** Pressed */
	KEYMODIFIER_Pressed			=	0x40,
	/** Released */
	KEYMODIFIER_Released		=	0x80,

	/** all */
	KEYMODIFIER_All				=	0xFF,
};

#define CANVAS_SUPPORTS_CLIP_MASK 0

#endif	// __ENGINEUSERINTERFACEGLOBALINCLUDES_H__
#endif	// NAMES_ONLY

