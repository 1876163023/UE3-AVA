class avaUIAction_GetUIState extends UIAction;

event Activated()
{
	Local int i, VarLinksindex, LinkedVarIndex;
	Local UIScreenObject ScreenObject;
	Local SeqVarLink VarLink;

	for( i = 0 ; i < Targets.Length ; i++ )
	{
		ScreenObject = UIScreenObject(Targets[i]);
		if( ScreenObject != None )
			break;
	}

	for( VarLinksIndex = 0 ; VarLinksIndex < VariableLinks.Length ; VarLinksIndex++ )
	{
		VarLink = VariableLinks[VarLinksIndex];
		for( LinkedVarIndex = 0 ; LinkedVarIndex < VarLink.LinkedVariables.Length ; LinkedVarIndex++ )
		{
			if( VarLink.LinkDesc == "UIState")
			{
				SeqVar_Object(VarLink.LinkedVariables[LinkedVarIndex]).SetObjectValue(ScreenObject.GetCurrentState());
			}
		}
	}
}

defaultproperties
{
	ObjName="Get UIState"
	ObjCategory="UI"
	
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="UIState",bWriteable=true))
}