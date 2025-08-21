/**
 * This event is activated the user "clicks" or otherwise chooses a list item.
 *
 * The Activator for this event should be a UIList object.
 *
 * Copyright © 1998-2007 Epic Games, Inc. All Rights Reserved
 */
class UIEvent_SubmitListData extends UIEvent_SubmitData
	native(inherit);

cpptext
{
	/** USequenceOp interface */
	/**
	 * Allows the operation to initialize the values for any VariableLinks that need to be filled prior to executing this
	 * op's logic.  This is a convenient hook for filling VariableLinks that aren't necessarily associated with an actual
	 * member variable of this op, or for VariableLinks that are used in the execution of this ops logic.
	 *
	 * Initializes the value of the "SelectedItem" linked variable by copying the value of the activator list's currently
	 * selected element into the linked variable.
	 */
	virtual void InitializeLinkedVariableValues();
}

/**
 * Stores the value of the selected item associated with this event's activator list.
 */
var() editconst	transient	int		SelectedItem;

DefaultProperties
{
	ObjName="Submit List Selection"

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Selected Item",PropertyName=SelectedItem,bWriteable=true))
}
