/*
	UI Kismet���� �߻��� ActivateLevelEventInt�� ���ؼ� �̺�Ʈ�� �߻��Ǵ� Ŭ����.

	2007/01/29	����
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

	// ���ڿ��� ��ũ�� �� �ְ� ���ش�.
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="IntValue",MinVars=1,MaxVars=1,bWriteable=true))
}
