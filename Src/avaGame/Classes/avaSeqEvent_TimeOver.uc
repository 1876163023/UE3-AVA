class avaSeqEvent_TimeOver extends SequenceEvent;

event Activated()
{
	if (InputLinks[0].bHasImpulse)		// Enable This Event
		bEnabled = true;
	else if (InputLinks[1].bHasImpulse)	// Disable This Event
		bEnabled = false;
}

function Trigger( actor orig, bool practice, int roundnum )
{
    local int i;

    if ( CheckActivate( orig, none, false ))
    {
        `log( self$" triggered practice="$ practice $ " round=" $ roundnum );

        if ( VariableLinks.Length > 0 )
            for( i=0; i<VariableLinks[0].LinkedVariables.Length; ++ i )
            {
                SeqVar_Bool( VariableLinks[0].LinkedVariables[i] ).bValue = int(practice);
            }

        if ( VariableLinks.Length > 1 )
            for( i=0; i<VariableLinks[1].LinkedVariables.Length; ++ i )
            {
                SeqVar_Int( VariableLinks[1].LinkedVariables[i] ).IntValue = roundnum;
            }
    }
}

defaultproperties
{
    ObjCategory="Game"
    ObjName="Time Over"
    MaxTriggerCount=0
    bPlayerOnly=false

	InputLinks(0)=(LinkDesc="Enable")
	InputLinks(1)=(LinkDesc="Disable")

    VariableLinks.Empty
    VariableLinks(0)=(ExpectedType=class'SeqVar_Bool',LinkDesc="Practice",bWriteable=true)
    VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="RoundNum",bWriteable=true)
}

