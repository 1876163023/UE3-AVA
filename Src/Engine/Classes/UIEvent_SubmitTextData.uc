/**
 * This event is activated when a control that contains text data submits the string value.
 *
 * Copyright © 1998-2007 Epic Games, Inc. All Rights Reserved.
 *
 * @note: native because C++ code activates this event
 */
class UIEvent_SubmitTextData extends UIEvent_SubmitData
	native(inherit);

/** the string value that is being published */
var()	string		Value;

/** Indicates whether the string value in the owning widget should be cleared */
var()	bool		bClearValue;

cpptext
{
	/** USequenceOp interface */
	/**
	 * Allows the operation to initialize the values for any VariableLinks that need to be filled prior to executing this
	 * op's logic.  This is a convenient hook for filling VariableLinks that aren't necessarily associated with an actual
	 * member variable of this op, or for VariableLinks that are used in the execution of this ops logic.
	 *
	 * Initializes the value of the "Value" linked variable by copying the initial value of "Value" into the linked variable.
	 */
	virtual void InitializeLinkedVariableValues();
}

DefaultProperties
{
	ObjName="Submit Text"

	bClearValue=true
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Value",PropertyName=Value,bWriteable=true))
}
