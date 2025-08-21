/*
	전체화면에서의 윈도우의 포커스 고정 상태를 수정하는 액션.

	2007/04/30	고광록
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

//! GEngine->GameViewport를 얻어온다.
native function SetFocusMode(bool bLock);

DefaultProperties
{
	ObjName="SetFocusMode"
	ObjCategory="Misc"

	VariableLinks.Empty

	InputLinks(0)=(LinkDesc="Lock")
	InputLinks(1)=(LinkDesc="Unlock")
}