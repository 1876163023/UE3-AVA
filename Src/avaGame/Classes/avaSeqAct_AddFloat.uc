// 연결되어 있는 variable을 amount 만큼 증가한다.
// 어떤 값을 원하는 만큼 증가시킬 때 사용한다.

class avaSeqAct_AddFloat extends SequenceAction;


var() float AddAmount;

event Activated()
{
	local SeqVar_Float v;

	if ( AddAmount == 0 )
	{
//		`log( self$" AddAmount == 0", 'Error' );
	}

	foreach LinkedVariables( class'SeqVar_Float', v, "Target" )
	{
		v.FloatValue += AddAmount;
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
	ObjName="Add Float"
	bCallHandler=false

	AddAmount=1.0
    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Float',LinkDesc="Target",MinVars=1,bWriteable=true )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Float',LinkDesc="Amount",PropertyName=AddAmount, MinVars=0,MaxVars=1 )
}

