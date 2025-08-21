class avaSeqEvent_Hm extends SequenceEvent;

function Trigger( actor orig)
{
    if ( CheckActivate( orig, none, false ))
    {    
        `log( "[dEAthcURe::avaSeqEvent_Hm::Trigger] " @ orig);
    }
}

defaultproperties
{
    //ObjCategory="Game"
    ObjName="Hm"
    MaxTriggerCount=0
    bPlayerOnly=false	
}

