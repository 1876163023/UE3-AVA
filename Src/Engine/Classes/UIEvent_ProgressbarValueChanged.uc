/**
 * This event is activated when the value of a UIProgressBar changes.
 *
 * Copyright © 1998-2007 Epic Games, Inc. All Rights Reserved.
 *
 * @note: native because C++ code activates this event
 */
class UIEvent_ProgressBarValueChanged extends UIEvent_ValueChanged
	native(inherit);

cpptext
{
	/** USequenceOp interface */
	/**
	 * Allows the operation to initialize the values for any VariableLinks that need to be filled prior to executing this
	 * op's logic.  This is a convenient hook for filling VariableLinks that aren't necessarily associated with an actual
	 * member variable of this op, or for VariableLinks that are used in the execution of this ops logic.
	 *
	 * Initializes the value of the "New Value" linked variable
	 */
	virtual void InitializeLinkedVariableValues();
}

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return TargetObject == None || UIProgressBar(TargetObject) != None;
}

DefaultProperties
{
	ObjName="Progress bar"

	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Value",bWriteable=true))
}
