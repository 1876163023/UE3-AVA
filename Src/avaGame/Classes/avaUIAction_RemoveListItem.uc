/**
 * Action for moving elements within a UIList.
 *
 * Copyright 2006 Epic Games, Inc. All Rights Reserved
 */
class avaUIAction_RemoveListItem extends UIAction
	native(inherit);

cpptext
{
	/**
	 * Executes the logic for this action.
	 */
	virtual void Activated();
}

/**
 * The index of the element to move.
 */
var()		int				ElementIndex;

DefaultProperties
{
	ObjName="Remove Element"
	ObjCategory="List"

	bAutoActivateOutputLinks=false
	bAutoTargetOwner=true

	ElementIndex=INDEX_NONE

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Element Index",PropertyName=ElementIndex))

	OutputLinks(0)=(LinkDesc="Failure")
	OutputLinks(1)=(LinkDesc="Success")
}
