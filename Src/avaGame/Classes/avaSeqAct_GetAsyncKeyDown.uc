class avaSeqAct_GetAsyncKeyDown extends SequenceAction;

var() bool bAltPressed;
var() bool bCtrlPressed;
var() bool bShiftPressed;

event Activated()
{
	Local WorldInfo WorldInfo;
	Local PlayerController PC, LocalPC;
	Local UIInteraction UIController;
	Local UIScene ActiveScene;
	//Local SeqVar_Bool BoolVar;

	WorldInfo = GetWorldInfo();
	assert(WorldInfo != none);

	foreach WorldInfo.LocalPlayerControllers(LocalPC)
	{
		PC = LocalPC;
	}
	UIController = LocalPlayer(PC.Player).ViewportClient.UIController;
	ActiveScene = (UIController != none && UIController.SceneClient != None) ? UIController.SceneClient.GetActiveScene() : none;
	if( ActiveScene == none )
		return;

	// IsHoldingAlt 함수는 ControllerID를 인자로받지만 쓰지 않는다.
	// 게다가 ControllerID는 PlayerIndex로 대체될 예정이다.
	// 우선은 BestPlayerIndex를 쓰지만 별다른 이유는 없다.
	bAltPressed = ActiveScene.IsHoldingAlt(ActiveScene.GetBestPlayerIndex());
	bCtrlPressed = ActiveScene.IsHoldingCtrl(ActiveScene.GetBestPlayerIndex());
	bShiftPressed = ActiveScene.IsHoldingShift(ActiveScene.GetBestPlayerIndex());
}

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

defaultproperties
{
	ObjName="Get Async KeyDown"
	ObjCategory="Input"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Alt",PropertyName=bAltPressed,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Ctrl",PropertyName=bCtrlPressed,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Shift",PropertyName=bShiftPressed,bWriteable=true))
}