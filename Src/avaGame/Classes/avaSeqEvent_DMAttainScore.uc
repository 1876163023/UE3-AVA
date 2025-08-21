// DeatMatch Rule ���� Player ��  Score �� WinCondition �� ������ ��쿡 ȣ�����ش�....
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