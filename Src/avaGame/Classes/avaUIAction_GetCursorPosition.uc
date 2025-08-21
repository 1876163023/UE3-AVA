class avaUIAction_GetCursorPosition extends UIAction;

event activated()
{
	Local float CurX, CurY;
	Local IntPoint CursorPos;
	Local UIScene OwnerScene;
	Local int VarLinksIndex, LinkedVarIndex;
	Local SeqVarLink VarLink;

	OwnerScene = GetOwnerScene();
	if( OwnerScene != None && OwnerScene.SceneClient != None )
	{
		CursorPos = OwnerScene.SceneClient.MousePosition;
		CurX = CursorPos.X;
		CurY = CursorPos.Y;
	}

	for( VarLinksIndex = 0 ; VarLinksIndex < VariableLinks.Length ; VarLinksIndex++ )
	{
		VarLink = VariableLinks[VarLinksIndex];
		for( LinkedVarIndex = 0 ; LinkedVarIndex < VarLink.LinkedVariables.Length ; LinkedVarIndex++ )
		{
			if( VarLink.LinkDesc == "CursorX" )
			{
				SeqVar_Float(VarLink.LinkedVariables[LinkedVarIndex]).FloatValue = CurX;
			}
			else if ( VarLink.LinkDesc == "CursorY" )
			{
				SeqVar_Float(VarLink.LinkedVariables[LinkedVarIndex]).FloatValue = CurY;
			}
		}
	}
}

defaultproperties
{
	ObjName="Get CursorPos"
	ObjCategory="UI"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="CursorX",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="CursorY",bWriteable=true))
}