/*
	Scroll등을 해서 TopIndex가 바뀌는 경우에 발생하는 이벤트.
*/
class UIEvent_ListTopIndexChanged extends UIEvent_ValueChanged
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
var	transient	int		PreviousTopIndex;

/** the index that the UIList changed to */
var	transient	int		CurrentTopIndex;

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
	ObjName="List TopIndex"

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Old TopIndex",bWriteable=true,PropertyName="PreviousTopIndex"))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="New TopIndex",bWriteable=true,PropertyName="CurrentTopIndex"))
}