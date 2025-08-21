//=============================================================================
//  avaSeqEvent_KOTH
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//  
//	2006/04/18 by OZ
//		King Of The Hill 을 위한 Sequence Event 를 정의한다.
//		Event 에는 'Reset', 'Start', 'Stop', 'Success' 가 있다.
//=============================================================================
class avaSeqEvent_KOTH extends SequenceEvent;


function ActivateEvent( name EventType, int nTeamIdx )	//Trigger(name EventType, Controller EventInstigator)
{
	local array<int> ActivateIndices;
	local int i;
	local SeqVar_Int ObjVar;

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
	else if (CheckActivate(Originator, None, false, ActivateIndices))
	{
		foreach LinkedVariables(class'SeqVar_Int', ObjVar, "Team")
		{
			ObjVar.IntValue = nTeamIdx;
		}
	}
}

defaultproperties
{
	ObjName="KOTH Event"
	ObjCategory="Objective"
	bPlayerOnly=false
	MaxTriggerCount=0

	OutputLinks[0]=(LinkDesc="Start")
	OutputLinks[1]=(LinkDesc="Stop")
	OutputLinks[2]=(LinkDesc="Reset")
	OutputLinks[3]=(LinkDesc="Success")

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Team",bWriteable=true)
}
