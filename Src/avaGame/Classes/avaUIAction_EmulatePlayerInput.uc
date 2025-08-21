class avaUIAction_EmulatePlayerInput extends UIAction
	native;

var() name InputKeyToEmulate;

cpptext
{
	virtual void Activated()
	{
		FInputEventParameters InputEventParms;
		UUIScreenObject* ScreenObject;
		UBOOL bResult = FALSE;

		InputEventParms.PlayerIndex = PlayerIndex;
		InputEventParms.InputKeyName = InputKeyToEmulate;

		for( INT ObjIdx = 0 ; ObjIdx < Targets.Num() ; ObjIdx++ )
		{
			if( (ScreenObject = Cast<UUIScreenObject>(Targets(ObjIdx))) != NULL 
				&& ScreenObject->EmulatePlayerInput( InputEventParms ) )
				break;
		}
	}
}

//event activated()
//{
//	Local InputEventParameters InputEventParms;
//	Local UIScreenObject ScreenObj;
//
//	InputEventParms.PlayerIndex = PlayerIndex;
//	InputEventParms.InputKeyName = InputKeyToEmulate;
//
//	foreach Targets(ScreenObj)
//	{
//		if( ScreenObj.EmulatePlayerInput( InputEventParms ) )
//			break;
//	}
//}

defaultproperties
{
	ObjName="Emulate PlayerInput"
	ObjCategory="Input"

	VariableLinks.Add((ExpectedType=class'SeqVar_Name',LinkDesc="Input Key Name",PropertyName=InputKeyToEmulate))
}