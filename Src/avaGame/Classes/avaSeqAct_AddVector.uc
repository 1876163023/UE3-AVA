class avaSeqAct_AddVector extends SequenceAction;


var() vector AddAmount;

event Activated()
{
	local SeqVar_Vector v;

	foreach LinkedVariables( class'SeqVar_Vector', v, "Target" )
	{
		v.VectValue += AddAmount;
	}
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
	ObjCategory="Add Value"
	ObjName="Add Vector"
	bCallHandler=false

	AddAmount=(X=1.0,Y=1.0,Z=1.0)
    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Vector',LinkDesc="Target",MinVars=1,bWriteable=true )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Vector',LinkDesc="Amount",PropertyName=AddAmount,MinVars=0,MaxVars=1 )
}
