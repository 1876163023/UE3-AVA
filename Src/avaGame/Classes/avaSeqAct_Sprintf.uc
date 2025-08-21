class avaSeqAct_Sprintf extends SequenceAction
	native;


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
	ObjName="Sprintf"
	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="SourceStr"), MaxVars=1)
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Str"))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Float"))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Int"))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="TargetStr"))
}