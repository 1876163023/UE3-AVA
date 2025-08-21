class avaSeqAct_SetResolution extends SequenceAction;

var() string ResStr;
var() bool bConfirmResolution;

event Activated()
{
	class'avaOptionSettings'.static.SetResolutionStr(ResStr, true);

	if( bConfirmResolution )
		class'avaOptionSettings'.static.SetConfirmedResolution( class'avaOptionSettings'.static.GetResolution() );
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
	ObjName="Set Resolution"
	ObjCategory="Adjust"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Res Str",PropertyName=ResStr))
}