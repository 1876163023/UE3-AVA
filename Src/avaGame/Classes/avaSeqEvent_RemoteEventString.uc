/*
	UI Kismet에서 발생한 ActivateLevelEventStinrg에 대해서 이벤트가 발생되는 클래스.

	2007/01/13	고광록
*/
class avaSeqEvent_RemoteEventString extends SeqEvent_RemoteEvent
	native;

event Activated()
{
	local SeqVar_String str;

	// UI Action에서 받은 문자열을 출력한다.

/*
	// Style1
	foreach LinkedVariables( class'SeqVar_String', str, "StringValue" )
	{
		`log( "Recv strValue: ["@str.StrValue@"]" );
	}
*/

	// Style2
	if ( VariableLinks.Length >= 1 && VariableLinks[0].LinkedVariables.Length > 0 )
	{
		str = SeqVar_String( VariableLinks[0].LinkedVariables[0] );
		`log( "2nd Recv strValue: ["@str.StrValue@"]" );
	}
}

defaultproperties
{
	ObjName="ava Remote Event String"
	ObjCategory="UI"

	EventName=DefaultEvent
	bPlayerOnly=FALSE

	// 문자열을 링크할 수 있게 해준다.
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="StringValue",MinVars=1,MaxVars=1,bWriteable=true))
}
