/*
	Level(World)�� Kismet���� Event�� �߻���Ű�� �׼�.

	2007/01/30	����
*/
class avaUIAction_ActivateLevelEventObject extends UIAction_ActivateLevelEvent
	native;

cpptext
{
	void Activated();
}

DefaultProperties
{
	ObjName="ava Activate Level Event Object"
	ObjCategory="Level"

	EventName=DefaultEvent

	// ���ڿ��� ��ũ�� �� �ְ� ���ش�.
	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Object",MaxVars=1,bWriteable=true))

	OutputLinks.Empty
	OutputLinks(0)=(LinkDesc="Failed")
	OutputLinks(1)=(LinkDesc="Success")
}
