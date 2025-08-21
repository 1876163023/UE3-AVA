/**
 * Unset User Key for UI Kismet
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_UnsetUserKey extends UIAction;

var() bool		bFactoryDefault;

DefaultProperties
{
	ObjName="Unset UserKey"

	ObjCategory="Misc"

	bAutoTargetOwner=true

	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="FactoryDefault",PropertyName=bFactoryDefault))
}
