/**
 * Changes the value of a label's text
 *
 * Copyright ?2005 Epic Games, Inc. All Rights Reserved.
 */
class avaUIAction_SetCharacter extends UIAction_SetValue;

var() class<avaCharacter>			Template;

DefaultProperties
{
	ObjName="ava SetCharacter"
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="New Value",PropertyName=NewText))
}
