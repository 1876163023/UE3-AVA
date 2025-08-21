class avaSeqAct_AddString extends SequenceAction;

var() string Target;
var() bool bBefore;


event Activated()
{
	local string strValue;
	local SequenceVariable v;

	foreach LinkedVariables( class'SequenceVariable', v, "Value" )
	{
		if (SeqVar_Bool(v) != None)
			strValue = strValue $ (SeqVar_Bool(v).bValue > 0 ? "True" : "False");
		else if (SeqVar_Int(v) != None)
			strValue = strValue $ string(SeqVar_Int(v).IntValue);
		else if (SeqVar_Float(v) != None)
			strValue = strValue $ string(SeqVar_Float(v).FloatValue);
		else if (SeqVar_String(v) != None)
			strValue = strValue $ SeqVar_String(v).StrValue;
		else if (SeqVar_Object(v) != None)
			strValue = strValue $ string(SeqVar_Object(v).GetObjectValue());
		else if (SeqVar_Vector(v) != None)
			strValue = strValue $ string(SeqVar_Vector(v).VectValue);
	}

	if (bBefore)
		Target = strValue $ Target;
	else
		Target = Target $ strValue;
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
	ObjName="Add String"
	bCallHandler=false
	
    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="Target", PropertyName=Target, MinVars=1,bWriteable=true )
	VariableLinks(1)=(ExpectedType=class'SequenceVariable',LinkDesc="Value", PropertyName=Value, MinVars=1 )
}

