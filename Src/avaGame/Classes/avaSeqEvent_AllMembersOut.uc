// 한쪽 Team Member 가 모두 나간 경우에 대한 처리이다....
class avaSeqEvent_AllMembersOut extends SequenceEvent;

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
	ObjName="All Out"

    MaxTriggerCount=0           // from avaSeqEvent_FlagEvent
	bPlayerOnly=false

	OutputLinks[1]=(LinkDesc="Team0")
	OutputLinks[2]=(LinkDesc="Team1")

    VariableLinks.Empty
    VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Team",bWriteable=true)
}