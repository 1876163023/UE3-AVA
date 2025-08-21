/**
 * This action allows designers to change the value of a widget that contains boolean data.
 *
 * Copyright 2005 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_GetBoolValue extends UIAction_GetValue;

var()				bool					bNewValue;

DefaultProperties
{
	ObjName="Get Bool Value"
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Value",PropertyName=bNewValue))
}
