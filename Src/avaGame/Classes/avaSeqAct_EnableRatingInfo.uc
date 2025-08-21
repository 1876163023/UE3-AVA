class avaSeqAct_EnableRatingInfo extends SequenceAction native;

var() bool bEnable;

native event Activated();


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
	ObjCategory="Misc"
	ObjName="EnableRatingInfo"

	bEnable=true
	bCallHandler	=	false
	
    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Bool',LinkDesc="bEnable",PropertyName=bEnable,MaxVars=1)
}