/// avaTeamGame 에서 한쪽 팀이 전멸한 경우에 발생한다.
class avaSeqEvent_Massacre extends SequenceEvent;

event Activated()
{
	if (InputLinks[0].bHasImpulse)		// Enable This Event
		bEnabled = true;
	else if (InputLinks[1].bHasImpulse)	// Disable This Event
		bEnabled = false;
}

function Trigger( actor orig, int teamNum )
{
	local array<int> ActivateIndices;
	local int i;

	ActivateIndices[ActivateIndices.length] = 0;

	if (teamNum == 0)
		ActivateIndices[ActivateIndices.length] = 1;

	if (teamNum == 1)
		ActivateIndices[ActivateIndices.length] = 2;

	if (CheckActivate(orig, none, false, ActivateIndices))
	{
		if ( VariableLinks.length > 0 )
		{
			for( i=0; i<VariableLinks[0].LinkedVariables.Length; ++i )
			{
				SeqVar_Int( VariableLinks[0].LinkedVariables[i] ).IntValue = teamNum;
			}
		}
	}   
}

defaultproperties
{
    ObjCategory="Game"
	ObjName="Team Massacre"
    MaxTriggerCount=0           // from avaSeqEvent_FlagEvent
	bPlayerOnly=false

	InputLinks(0)=(LinkDesc="Enable")
	InputLinks(1)=(LinkDesc="Disable")

	OutputLinks[1]=(LinkDesc="Team0")
	OutputLinks[2]=(LinkDesc="Team1")

    VariableLinks.Empty
    VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Team",bWriteable=true)
}
