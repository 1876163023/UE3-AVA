/*
	UI Kismet���� �߻��� ActivateLevelEventStinrg�� ���ؼ� �̺�Ʈ�� �߻��Ǵ� Ŭ����.

	2007/01/13	����
*/
class avaSeqEvent_RemoteEventString extends SeqEvent_RemoteEvent
	native;

event Activated()
{
	local SeqVar_String str;

	// UI Action���� ���� ���ڿ��� ����Ѵ�.

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

	// ���ڿ��� ��ũ�� �� �ְ� ���ش�.
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="StringValue",MinVars=1,MaxVars=1,bWriteable=true))
}
