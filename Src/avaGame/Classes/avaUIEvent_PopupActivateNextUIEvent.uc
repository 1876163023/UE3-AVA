/**
 * 팝업에서 '확인'버튼을 누를시에 발생시킬 Named Remote Event를 지정
 */
class avaUIEvent_PopupActivateNextUIEvent extends avaUIEvent_PopupBase;


defaultproperties
{
	ObjName = "NextUIEvent(Popup)"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="NextUIEvent Name", bWriteable=true))
}