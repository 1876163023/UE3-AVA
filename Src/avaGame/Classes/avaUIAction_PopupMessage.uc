class avaUIAction_PopupMessage extends UIAction;

var() EPopUpMsgType		MsgType;
var() string			PopupMsg;
var() string			NextScene;
var() name				NextUIEventName;

event Activated()
{
	class'avaNetHandler'.static.GetAvaNetHandler().PopUpMessage(MsgType, PopupMsg, NextScene, NextUIEventName);
}

defaultproperties
{
	ObjName="ava UI Popup Message"

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="PopupMsg",PropertyName=PopupMsg)
	VariableLinks(1)=(ExpectedType=class'SeqVar_String',LinkDesc="NextScene",PropertyName=NextScene,bWriteable=true)

//	NextScene = None;
}