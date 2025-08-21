class avaSeqEvent_PickDogTag extends SequenceEvent;

function Trigger(name EventType, Controller EventInstigator)
{
	local array<int>		ActivateIndices;
	local int				i;
	local SeqVar_Object		ObjVar;

	for (i = 0; i < OutputLinks.length; i++)
	{
		if (EventType == name(OutputLinks[i].LinkDesc))
		{
			ActivateIndices[ActivateIndices.length] = i;
		}
	}

	if (ActivateIndices.length == 0)
	{
		ScriptLog("Not activating" @ self @ "for event" @ EventType @ "because there are no matching outputs");
	}
	else if (CheckActivate(EventInstigator, EventInstigator, false, ActivateIndices))
	{
		foreach LinkedVariables(class'SeqVar_Object', ObjVar, "Instigator")
		{
			ObjVar.SetObjectValue( EventInstigator );
		}
	}
}

defaultproperties
{
	ObjName="Pick DogTag"
	ObjCategory="Objective"
	bPlayerOnly=false
	MaxTriggerCount=0

	OutputLinks[0]=(LinkDesc="PickDogTag")
	OutputLinks[1]=(LinkDesc="PickDogTagPack")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Instigator",bWriteable=true)
}