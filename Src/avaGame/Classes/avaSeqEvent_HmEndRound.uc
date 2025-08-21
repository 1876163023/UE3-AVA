class avaSeqEvent_HmEndRound extends SequenceEvent;

function Trigger( actor orig)
{
    if ( CheckActivate( orig, none, false ))
    {    
        `log( "[dEAthcURe::avaSeqEvent_HmEndRound::Trigger] " @ orig);
    }
}

defaultproperties
{
    //ObjCategory="Game"
    ObjName="HmEndRound"
    MaxTriggerCount=0
    bPlayerOnly=false	
}

