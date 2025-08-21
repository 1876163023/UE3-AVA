/**
 * This action allows designers to change the index of a UIList.
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved
 */
class UIAction_SetListIndex extends UIAction_SetValue;

/** the index to change the list to */
var()	int		NewIndex;

/** whether to clamp invalid index values */
var()	bool	bClampInvalidValues;

/** whether the list should activate the ListIndexChanged event */
var()	bool	bActivateListChangeEvent;

var()	int		TopIndex;

/** NotifySubmitSelection or not */
var()	bool	bSubmitSelection;

DefaultProperties
{
	ObjClassVersion=2
	ObjName="Set List Index"
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="New Index",PropertyName=NewIndex))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Top Index",PropertyName=TopIndex))

	OutputLinks(0)=(LinkDesc="Success")
	OutputLinks(1)=(LinkDesc="Failed")

	NewIndex=-1
	TopIndex=-1
	bClampInvalidValues=true
	bActivateListChangeEvent=true
	bSubmitSelection=false
}
 