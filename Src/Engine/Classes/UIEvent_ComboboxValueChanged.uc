/**
 * This event is activated when the value of a UIComboBox is changed.
 *
 * Copyright 2007 Epic Games, Inc. All Rights Reserved
 */
class UIEvent_ComboboxValueChanged extends UIEvent_ValueChanged
	native(inherit);

cpptext
{
	/** USequenceOp interface */
	/**
	 * Allows the operation to initialize the values for any VariableLinks that need to be filled prior to executing this
	 * op's logic.  This is a convenient hook for filling VariableLinks that aren't necessarily associated with an actual
	 * member variable of this op, or for VariableLinks that are used in the execution of this ops logic.
	 *
	 * Initializes the value of the linked variables.
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
	return TargetObject == None || UIComboBox(TargetObject) != None;
}

DefaultProperties
{
	ObjName="Combo box"

	OutputLinks(0)=(LinkDesc="Text Changed")
	OutputLinks(1)=(LinkDesc="Item Selected")

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Text Value",bWriteable=false))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Current Item",bWriteable=false))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Current List Index",bWriteable=false,bHidden=false))
}
