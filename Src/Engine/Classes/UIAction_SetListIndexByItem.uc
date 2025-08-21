/**
 * This action allows designers to change the index of a UIList by Item
 *
 * Copyright 2006 Epic Games, Inc. All Rights Reserved
 */
class UIAction_SetListIndexByItem extends UIAction_SetValue;

/** the index to change the list to */
var()	int		NewItem;

/** whether to clamp invalid index values */
var()	bool	bClampInvalidValues;

/** whether the list should activate the ListIndexChanged event */
var()	bool	bActivateListChangeEvent;

/** NotifySubmitSelection or not */
var()	bool	bSubmitSelection;

DefaultProperties
{
	ObjName="Set ListIndex By Item"
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="New Item",PropertyName=NewItem))

	OutputLinks(0)=(LinkDesc="Success")
	OutputLinks(1)=(LinkDesc="Failed")

	NewItem=-1
	bClampInvalidValues=true
	bActivateListChangeEvent=true
	bSubmitSelection=false
}
