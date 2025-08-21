class avaSeqEvent_TimeElapse extends SequenceEvent;

var() int	Timer;

function Trigger( actor orig )
{
//    local int i;
    if ( CheckActivate( orig, none, false ))
    {
    }
}

defaultproperties
{
	ObjName="TimeElapse Event"
	ObjCategory="Game"
	bPlayerOnly=false
	MaxTriggerCount=0
	VariableLinks.Empty
}