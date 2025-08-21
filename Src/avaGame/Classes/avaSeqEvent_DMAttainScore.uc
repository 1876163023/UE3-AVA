// DeatMatch Rule 에서 Player 의  Score 가 WinCondition 에 도달한 경우에 호출해준다....
class avaSeqEvent_DMAttainScore extends SequenceEvent;

function Trigger( actor orig )
{
	local SeqVar_Object ObjVar;
    if ( CheckActivate( orig, none, false ))
    {
		foreach LinkedVariables(class'SeqVar_Object', ObjVar, "Instigator")
		{
			ObjVar.SetObjectValue( orig );
		}
    }
}

defaultproperties
{
    ObjCategory		=	"Game"
	ObjName			=	"Attain Score[DM]"
    MaxTriggerCount	=	0 
	bPlayerOnly		=	false
}