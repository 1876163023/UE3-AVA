/*
	���� �� ȣ��Ǵ� �̺�Ʈ.

	2007/12/10	����
		FakeFullScreen���� FullScreen���� ��ȯ�ϱ� ���ؼ� �̺�Ʈ �ʿ�.
*/
class avaUIEvent_ProcInvenCharge extends UIEvent;

defaultproperties
{
	ObjName="Proc Inven Charge Cash"
	ObjCategory="ProcInven"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Response",bWriteable=true))

	ObjClassVersion=1
}