/*
	UI Kismet에서 발생한 ActivateLevelEventObject에 대해서 이벤트가 발생되는 클래스.

	2007/01/30	고광록
*/
class avaSeqEvent_RemoteEventObject extends SeqEvent_RemoteEvent
	native;

event Activated()
{
	local SeqVar_Object VarObj;

	if ( VariableLinks.Length >= 1 && VariableLinks[0].LinkedVariables.Length > 0 )
	{
		VarObj = SeqVar_Object( VariableLinks[0].LinkedVariables[0] );
		`log( "2nd Recv strValue: ["@VarObj.GetObjectValue()@"]" );
	}
}

defaultproperties
{
	ObjName="ava Remote Event Object"
	ObjCategory="UI"

	EventName=DefaultEvent
	bPlayerOnly=FALSE

	// 문자열을 링크할 수 있게 해준다.
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Object",MaxVars=1,bWriteable=true))
}
