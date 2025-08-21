/**
 * This action retrieves the index and value of the item at the index for the list that activated this action.
 *
 * Copyright © 1998-2007 Epic Games, Inc. All Rights Reserved
 */
class UIAction_GetListIndex extends UIAction_GetValue
	native(inherit);

cpptext
{
	/**
	 * Copies the value of the current element for the UILists in the Targets array to the Selected Item variable link.
	 */
	virtual void Activated();
}

DefaultProperties
{
	ObjClassVersion=6
	ObjName="Selected List Item"

	bAutoActivateOutputLinks=false
	bAutoTargetOwner=true

	// add a variable link to receive the value of the list's selected item
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Current Item",bWriteable=true))

	// add a variable link to receive the index of the list
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Current Index",bWriteable=true,bHidden=true))

	// add a variable link to top index of the list
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Top Index",bWriteable=true,bHidden=true))

	OutputLinks(0)=(LinkDesc="Valid")
	OutputLinks(1)=(LinkDesc="Invalid")
}
