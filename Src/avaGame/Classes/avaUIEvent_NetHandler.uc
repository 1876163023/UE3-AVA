/*
	avaNetHandler���� �߻��ϴ� �̺�Ʈ�� kismet���� �ű�� ���� Ŭ����.
*/
class avaUIEvent_NetHandler extends UIEvent;

defaultproperties
{
	ObjName="NetHandler Event"

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Param1",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Param2",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Param3",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Param4",bWriteable=true))
}