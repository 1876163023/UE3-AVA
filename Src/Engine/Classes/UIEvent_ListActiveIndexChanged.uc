/**
 * This event is activated when the index of a UIList is changed.
 *
 * Copyright 2006 Epic Games, Inc. All Rights Reserved
 *
 * @note: native because C++ code activates this event
 */
class UIEvent_ListActiveIndexChanged extends UIEvent_ValueChanged
	native(inherit);

cpptext
{
	/** USequenceOp interface */
	/**
	 * Allows the operation to initialize the values for any VariableLinks that need to be filled prior to executing this
	 * op's logic.  This is a convenient hook for filling VariableLinks that aren't necessarily associated with an actual
	 * member variable of this op, or for VariableLinks that are used in the execution of this ops logic.
	 *
	 * Initializes the value of the PreviousIndex and CurrentIndex linked variables
	 */
	virtual void InitializeLinkedVariableValues();
}

/** the index of the UIList before this event was called */
var	transient	int		PreviousIndex;

/** the index that the UIList changed to */
var	transient	int		CurrentIndex;

/** the item that the UIList changed to */
var transient	int		CurrentItem;

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return TargetObject == None || UIList(TargetObject) != None;
}

DefaultProperties
{
	ObjName="Active List Index"

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Old Index",bWriteable=true,PropertyName="PreviousIndex"))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="New Index",bWriteable=true,PropertyName="CurrentIndex"))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="New Item",bWriteable=true,PropertyName="CurrentItem"))

	ObjClassVersion=1
}
