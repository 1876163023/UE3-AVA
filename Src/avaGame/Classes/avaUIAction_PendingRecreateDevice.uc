/*
	Pending RecrateDevice(RenderDevice)

	2007/12/10	����
		�ػ�(or FakeFullScreen) ������ �Ϸ� �Ǿ����� �˱� ���� �׼�.
*/
class avaUIAction_PendingRecreateDevice extends UIAction
	native;

event Activated()
{
	if ( PendingRecreateDevice() )
	{
		OutputLinks[0].bHasImpulse = true;
		OutputLinks[1].bHasImpulse = false;
	}
	else
	{
		OutputLinks[0].bHasImpulse = false;
		OutputLinks[1].bHasImpulse = true;
	}
}

native function bool PendingRecreateDevice();

defaultproperties
{
	ObjName="Pending RecreateDevice"
	ObjCategory="Misc"

	VariableLinks.Empty

	OutputLinks(0)=(LinkDesc="True")
	OutputLinks(1)=(LinkDesc="False")

	// �̰� ���ϸ�.... OutputLinks(0..*).bHasImpulse = TRUE�� ���� �ȴ�.
	// DeActivated()�Լ����� �׷��� ó����.
	bAutoActivateOutputLinks = false
}