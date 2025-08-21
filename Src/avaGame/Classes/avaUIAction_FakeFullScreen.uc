/*
	속임수로 창모드를 전체화면 크기로 설정 할 수 있는 액션.

	2007/10/11	고광록
*/
class avaUIAction_FakeFullScreen extends UIAction
	native;

event Activated()
{
	local bool bEnable;

	if (InputLinks[0].bHasImpulse)
		bEnable = true;
	else if (InputLinks[1].bHasImpulse)
		bEnable = false;

	`log("SetFakeFullScreen" @bEnable);

	SetFakeFullScreen(bEnable);
}

//! GEngine->GameViewport를 얻어온다.
native function SetFakeFullScreen(bool bEnable);

/**
 * Determines whether this class should be displayed in the list of available ops in the level kismet editor.
 *
 * @return	TRUE if this sequence object should be available for use in the level kismet editor
 */
event bool IsValidLevelSequenceObject()
{
	return true;
}

DefaultProperties
{
	ObjName="FakeFullScreen"
	ObjCategory="Misc"

	VariableLinks.Empty

	InputLinks(0)=(LinkDesc="FakeFullScreen")
	InputLinks(1)=(LinkDesc="FullScreen")
}