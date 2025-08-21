class avaUIAction_CreatePopup extends UIAction;

var() string	Msg;
var() string	NextSceneName;

event Activated()
{
	//local WorldInfo					WorldInfo;
	//WorldInfo = GetOwnerScene().GetWorldInfo();
	//avaNetEntryGame( WorldInfo.Game ).PopUpMessage(Msg,NextSceneName);
	class'avaNetHandler'.static.GetAvaNetHandler().PopUpMessage(EPT_Notice, Msg, NextSceneName);
}

defaultproperties
{
	ObjName="Create Popup"
	ObjCategory="avaNet"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Message",		PropertyName="Msg",			 MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="NextScene",	PropertyName="NextSceneName",MaxVars=1))
}