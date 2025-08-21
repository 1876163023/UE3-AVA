class avaSeqAct_SetAudioVolume extends SequenceAction;

var() array<name> GroupNames;
var() float FloatVolume;

event activated()
{
	Local name GroupName;

	foreach GroupNames(GroupName)
	{
		class'avaOptionSettings'.static.SetAudioVolume( GroupName, FloatVolume );
	}
}

event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

defaultproperties
{
	ObjName="Set AudioVolume"
	ObjCategory="Adjust"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Name',LinkDesc="Group Name",PropertyName=GroupNames))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Volume",PropertyName=FloatVolume, bWriteable=true))
}