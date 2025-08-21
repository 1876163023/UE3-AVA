class avAUICond_CompareUIState extends SequenceCondition;

var() class<UIState>	StateClassToCompare;
var Object				Target;

event Activated()
{
	Local bool bMatch;
	Local UIScreenObject ScreenObject;

	ScreenObject = UIScreenObject(Target);
	if( ScreenObject != none )
		if( ScreenObject.HasActiveStateOfClass( StateClassToCompare  , ScreenObject.GetBestPlayerIndex() ) )
			bMatch = true;
	
	OutputLinks[ bMatch ? 0 : 1 ].bHasImpulse = true;
}

event bool IsValidLevelSequenceObject()
{
	return false;
}

defaultproperties
{
	ObjName="Compare UIState"
	ObjCategory="avaNet"

	InputLinks(0)=(LinkDesc="In")
	OutputLinks(0)=(LinkDesc="Match")
	OutputLinks(1)=(LinkDesc="Mismatch")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="UIObject",PropertyName="Target",MaxVars=1)
}