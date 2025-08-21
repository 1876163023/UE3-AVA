/*
	��üȭ�鿡���� �������� ��Ŀ�� ���� ���¸� �����ϴ� �׼�.

	2007/04/30	����
*/
class avaUIAction_SetFocusMode extends UIAction
	native;

event Activated()
{
	local bool bLock;

	if (InputLinks[0].bHasImpulse)
		bLock = true;
	else if (InputLinks[1].bHasImpulse)
		bLock = false;

	SetFocusMode(bLock);
}

//! GEngine->GameViewport�� ���´�.
native function SetFocusMode(bool bLock);

DefaultProperties
{
	ObjName="SetFocusMode"
	ObjCategory="Misc"

	VariableLinks.Empty

	InputLinks(0)=(LinkDesc="Lock")
	InputLinks(1)=(LinkDesc="Unlock")
}