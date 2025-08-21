class avaUIAction_GotoScene extends UIAction;

var() string	NextSceneName;
var() bool		bDirect;
var() name		NextUIEventName;

event Activated()
{
	local WorldInfo	WorldInfo;
	local bool		bPreload;

	if (InputLinks[0].bHasImpulse)
		bPreload = false;
	else if (InputLinks[1].bHasImpulse)
		bPreload = true;

	WorldInfo = GetOwnerScene().GetWorldInfo();
	avaNetEntryGame( WorldInfo.Game ).GotoScene( NextSceneName, bDirect, NextUIEventName, bPreload );

	if ( bPreload == false )
		OutputLinks[0].bHasImpulse = true;
	else
		OutputLinks[1].bHasImpulse = true;
}

defaultproperties
{
	ObjName="Goto Scene"
	ObjCategory="avaNet"

	bDirect = true

	VariableLinks.Empty

	//{ UIScene�� Preload�� ���� 2������ ����.(2007/04/11 ����)
	InputLinks(0)=(LinkDesc="Open")
	InputLinks(1)=(LinkDesc="Preload")

	OutputLinks(0)=(LinkDesc="Opened")
	OutputLinks(1)=(LinkDesc="Loaded")
	//}
}