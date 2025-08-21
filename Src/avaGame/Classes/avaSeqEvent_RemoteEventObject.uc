/*
	UI Kismet���� �߻��� ActivateLevelEventObject�� ���ؼ� �̺�Ʈ�� �߻��Ǵ� Ŭ����.

	2007/01/30	����
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

	// ���ڿ��� ��ũ�� �� �ְ� ���ش�.
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Object",MaxVars=1,bWriteable=true))
}
