/*
	Level(World)�� Kismet���� Event�� �߻���Ű�� �׼�.

	2007/01/13	����
*/
class avaUIAction_ActivateLevelEventString extends UIAction_ActivateLevelEvent
	native;

cpptext
{
	void Activated();
}

DefaultProperties
{
	ObjName="ava Activate Level Event String"
	ObjCategory="Level"

	EventName=DefaultEvent

	// ���ڿ��� ��ũ�� �� �ְ� ���ش�.
	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="StringValue",MinVars=0,MaxVars=1,bWriteable=true))

	OutputLinks.Empty
	OutputLinks(0)=(LinkDesc="Failed")
	OutputLinks(1)=(LinkDesc="Success")
}
