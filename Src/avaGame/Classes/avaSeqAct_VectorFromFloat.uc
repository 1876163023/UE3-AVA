class avaSeqAct_VectorFromFloat extends SequenceAction;


var() float X;
var() float Y;
var() float Z;

event Activated()
{
	local SeqVar_Vector v;

	foreach LinkedVariables( class'SeqVar_Vector', v, "Target" )
	{
		v.VectValue.X = X;
		v.VectValue.Y = Y;
		v.VectValue.Z = Z;
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
	ObjCategory="Set Variable"
	ObjName="Vector from Float"
	bCallHandler=false

	X=0.0
	Y=0.0
	Z=0.0
    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Vector',LinkDesc="Target",MinVars=1,bWriteable=true )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Float',LinkDesc="X",PropertyName=X,MinVars=1,MaxVars=1 )
	VariableLinks(2)=(ExpectedType=class'SeqVar_Float',LinkDesc="Y",PropertyName=Y,MinVars=1,MaxVars=1 )
	VariableLinks(3)=(ExpectedType=class'SeqVar_Float',LinkDesc="Z",PropertyName=Z,MinVars=1,MaxVars=1 )
}

