/*
	죽었을 경우에 발생되는 Sequence Event 이다...
*/

class avaSeqEvent_Killed extends SequenceEvent;

function Trigger( Controller killer, Controller victim )
{
	local SeqVar_Object		ObjVar;
	if ( CheckActivate(victim, None, false ) )
	{

		foreach LinkedVariables(class'SeqVar_Object', ObjVar, "killer")
		{
			ObjVar.SetObjectValue( killer );
		}

		foreach LinkedVariables(class'SeqVar_Object', ObjVar, "victim")
		{
			ObjVar.SetObjectValue( victim );
		}
	}
}

defaultproperties
{
	ObjName="Killed"
	ObjCategory="Pawn"
	bPlayerOnly=false

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="killer",bWriteable=true)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="victim",bWriteable=true)

}