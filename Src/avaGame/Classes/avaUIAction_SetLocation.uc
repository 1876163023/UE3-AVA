class avaUIAction_SetLocation extends UIAction
	native;

var() bool bFitToScreen;

var() vector2d LocationOffset;

cpptext
{
	virtual void Activated();	
}

defaultproperties
{
	bFitToScreen = true

	ObjName="Set Location(UI Widget)"
	ObjCategory="UI"

	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Left",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Top",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Right",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Bottom",MaxVars=1))
}