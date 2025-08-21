/**
 * Changes the value of a label's text
 *
 * Copyright ?2005 Epic Games, Inc. All Rights Reserved.
 */
class avaUIAction_SetWeapon extends UIAction_SetValue;

var() class<avaWeaponAttachment>			Template;

DefaultProperties
{
	ObjName="ava SetWeapon"
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="New Value",PropertyName=NewText))
}
