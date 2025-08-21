class avaSeqEvent_BombEvent extends SequenceEvent;

event Activated()
{
	if (InputLinks[0].bHasImpulse)		// Enable This Event
		bEnabled = true;
	else if (InputLinks[1].bHasImpulse)	// Disable This Event
		bEnabled = false;
}

/** attempts to activate the event with the appropriate output for the given event type and instigator */
function Trigger(name EventType, Controller EventInstigator)
{
	local array<int> ActivateIndices;
	local int i;
	local SeqVar_Object ObjVar;

	`log( "avaSeqEvent_BombEvent : " $EventInstigator );

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
	else if (CheckActivate(Originator, EventInstigator, false, ActivateIndices))
	{

		foreach LinkedVariables(class'SeqVar_Object', ObjVar, "Instigator")
		{
			ObjVar.SetObjectValue( EventInstigator );
			`log( "bomb event " $ EventType$ " " $ObjVar.GetObjectValue() );
		}
	}
}

defaultproperties
{
	ObjName="Bomb Event"
	ObjCategory="Objective"
	bPlayerOnly=false
	MaxTriggerCount=0

	InputLinks(0)=(LinkDesc="Enable")
	InputLinks(1)=(LinkDesc="Disable")

	OutputLinks[0]=(LinkDesc="Installed")
	OutputLinks[1]=(LinkDesc="Defused")
	OutputLinks[2]=(LinkDesc="Exploded")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Instigator",bWriteable=true)
}
