/*
	���Ӽ��� â��带 ��üȭ�� ũ��� ���� �� �� �ִ� �׼�.

	2007/10/11	����
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

//! GEngine->GameViewport�� ���´�.
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