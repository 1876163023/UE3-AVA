class avaSeqEvent_SucceedMission extends SequenceEvent;

function Trigger( actor orig)
{
    if ( CheckActivate( orig, none, false ))
    {    
    }
}

defaultproperties
{
    ObjCategory		=	"Game"
	ObjName			=	"Succeed Mission"
    MaxTriggerCount	=	0
	bPlayerOnly		=	false
    VariableLinks.Empty
}