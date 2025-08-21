class avaUIAction_IsStartable extends UIAction;

var bool	bStartable;
var string	ErrorMessage;

event Activated()
{
	bStartable = class'avaNetHandler'.static.GetAvaNetHandler().IsGameStartableEx(ErrorMessage);
}

defaultproperties
{
	ObjName="(Room) Is Startable"
	ObjCategory="avaNet"
	
	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="bStartable",PropertyName=bStartable,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="ErrorMessage",PropertyName=ErrorMessage,bWriteable=true))
}
