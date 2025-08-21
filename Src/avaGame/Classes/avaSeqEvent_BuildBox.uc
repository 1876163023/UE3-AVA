//=============================================================================
//  avaSeqEvent_BuildBox
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//  
//	2006/05/11 by OZ
//		BuildBox �� ���� Sequence Event �̴�.
//		avaVolume_BuildBox �� ���ؼ� Event �� �߻��ȴ�.
//=============================================================================
class avaSeqEvent_BuildBox extends SequenceEvent;

function ActivateEvent( name EventType )
{
	local array<int> ActivateIndices;
	local int i;
	//local SeqVar_Int ObjVar;

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
		// �ʿ��� Variable�� �ִٸ� ���⼭ Setting
		//foreach LinkedVariables(class'SeqVar_Int', ObjVar, "Team")
		//{
		//	ObjVar.IntValue = nTeamIdx;
		//}
	}
}

defaultproperties
{
	ObjName="BuildBox Event"
	ObjCategory="Objective"
	bPlayerOnly=false
	MaxTriggerCount=0

	OutputLinks[0]=(LinkDesc="Start")
	OutputLinks[1]=(LinkDesc="ReStart")
	OutputLinks[2]=(LinkDesc="Stop")
	OutputLinks[3]=(LinkDesc="Cancel")
	OutputLinks[4]=(LinkDesc="Complete")

	VariableLinks.Empty
}