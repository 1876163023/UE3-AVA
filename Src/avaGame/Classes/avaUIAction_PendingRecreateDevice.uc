/*
	Pending RecrateDevice(RenderDevice)

	2007/12/10	고광록
		해상도(or FakeFullScreen) 변경이 완료 되었는지 알기 위한 액션.
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

	// 이걸 안하면.... OutputLinks(0..*).bHasImpulse = TRUE가 들어가게 된다.
	// DeActivated()함수에서 그렇게 처리함.
	bAutoActivateOutputLinks = false
}