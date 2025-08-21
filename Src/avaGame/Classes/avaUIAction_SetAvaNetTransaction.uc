class avaUIAction_SetAvaNetTransaction extends UIAction
	native;

var() string	SessionName;

cpptext
{
	virtual void Activated();
}

defaultproperties
{
	ObjName="Set AvaNet Transaction"
	ObjCategory="Transaction"

	InputLinks(0)=(LinkDesc="Begin")
	InputLinks(1)=(LinkDesc="Undo")
	InputLinks(2)=(LinkDesc="End")

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Session Name",PropertyName=SessionName, MaxVars=1))
}