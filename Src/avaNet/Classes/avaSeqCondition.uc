class avaSeqCondition extends SequenceCondition
	abstract;

var() bool BoolResult;
/**
 * Determines whether this class should be displayed in the list of available ops in the level kismet editor.
 *
 * @return	TRUE if this sequence object should be available for use in the level kismet editor
 */
event bool IsValidLevelSequenceObject()
{
	return false;
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
	return true;
}


defaultproperties
{
	ObjCategory="avaNet"
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Bool",PropertyName=BoolResult,bWriteable=true,bHidden=true))
}
