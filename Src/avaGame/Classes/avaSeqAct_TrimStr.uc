class avaSeqAct_TrimStr extends SequenceAction
	native;


var() string SourceStr;
var() string TargetStr;

var() bool bTrim;
var() bool bTrimTrailing;

cpptext
{
	virtual void Activated();
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
	ObjCategory="String Manipulation"
	ObjName="Trim Str"
	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Src",PropertyName=SourceStr,bWriteable=false,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Tgt",PropertyName=TargetStr,bWriteable=false,MaxVars=1))

	bTrim=true
	bTrimTrailing=false
}