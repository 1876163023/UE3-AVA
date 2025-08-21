class avaSeqEvent_SwapTeam extends SequenceEvent;

function Trigger( actor orig)
{
    if ( CheckActivate( orig, none, false ))
    {    
    }
}

defaultproperties
{
    ObjCategory		=	"Game"
	ObjName			=	"Event Swap Team"
    MaxTriggerCount	=	0
	bPlayerOnly		=	false
    VariableLinks.Empty
}