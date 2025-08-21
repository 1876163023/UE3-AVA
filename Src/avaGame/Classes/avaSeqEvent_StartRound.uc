class avaSeqEvent_StartRound extends SequenceEvent;

function Trigger( Actor orig, int Round)
{
    local int i;

    if ( CheckActivate( orig, none, false ))
    {
        if ( VariableLinks.Length > 0 )
		{
            for( i=0 ; i< VariableLinks[0].LinkedVariables.Length ; ++ i )
            {
                SeqVar_Int( VariableLinks[0].LinkedVariables[i] ).IntValue = Round;
            }
		}
    }
}

defaultproperties
{
	ObjName="Start Round"
	ObjCategory="Level"
	bPlayerOnly=false
	MaxTriggerCount=0

	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Round",bWriteable=true)
}