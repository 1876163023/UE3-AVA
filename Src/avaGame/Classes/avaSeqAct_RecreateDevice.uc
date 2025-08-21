class avaSeqAct_RecreateDevice extends SequenceAction;

var() bool bRecreateWhenChanged;

event Activated()
{
	class'avaOptionSettings'.static.RecreateDevice(bRecreateWhenChanged);
}

event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

defaultproperties
{
	ObjName = "Recreate Device"
	ObjCategory = "Adjust"

	VariableLinks.Empty
}