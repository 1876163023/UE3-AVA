/*
	UI Kismet에서 발생한 ActivateLevelEventInt에 대해서 이벤트가 발생되는 클래스.

	2007/01/29	고광록
*/
class avaSeqEvent_RemoteEventInt extends SeqEvent_RemoteEvent
	native;

event Activated()
{
	local SeqVar_Int i;

/*
	// Style1
	foreach LinkedVariables( class'SeqVar_String', i, "IntValue" )
	{
		`log( "Recv intValue: ["@i.IntValue@"]" );
	}
*/

	// Style2
	if ( VariableLinks.Length >= 1 && VariableLinks[0].LinkedVariables.Length > 0 )
	{
		i = SeqVar_Int( VariableLinks[0].LinkedVariables[0] );
		`log( "2nd Recv intValue: ["@i.IntValue@"]" );
	}
}

defaultproperties
{
	ObjName="ava Remote Event Int"
	ObjCategory="UI"

	EventName=DefaultEvent
	bPlayerOnly=FALSE

	// 문자열을 링크할 수 있게 해준다.
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="IntValue",MinVars=1,MaxVars=1,bWriteable=true))
}
