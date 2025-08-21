//=============================================================================
//  avaSeqEvent_KBreakable
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//  
//	2006/09/18 by OZ
//
//=============================================================================
class avaSeqEvent_KBreakable extends SequenceEvent;

function ActivateEvent( name EventType )
{
	local array<int>	ActivateIndices;
	local int			i;
	for (i = 0; i < OutputLinks.length; i++)
	{
		if (EventType == name(OutputLinks[i].LinkDesc) )
		{
			ActivateIndices[ActivateIndices.length] = i;
		}
	}

	if (ActivateIndices.length == 0)
	{
		return;
	}

	CheckActivate(Originator, None, false, ActivateIndices);
}

defaultproperties
{
	ObjName="KBreakable Event"
	ObjCategory="Actor"
	bPlayerOnly=false
	MaxTriggerCount=0

	OutputLinks[0]=(LinkDesc="BreakAll")
	VariableLinks.Empty
}