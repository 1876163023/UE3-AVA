class avaSeqAct_GetAudioVolume extends SequenceAction;

var() name GroupName;
var() float FloatVolume;

event activated()
{
	FloatVolume = class'avaOptionSettings'.static.GetAudioVolume( GroupName );
}

event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

defaultproperties
{
	ObjName="Get AudioVolume"
	ObjCategory="Adjust"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Name',LinkDesc="Group Name",PropertyName=GroupName, MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Volume",PropertyName=FloatVolume, bWriteable=true))
}