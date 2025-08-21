class avaUIAction_CreateAdminUI extends UIAction;


event Activated()
{
	Local WorldInfo WorldInfo;
	Local PlayerController PC;
	Local avaGameViewportClient ViewportClient;
	Local int ViewportConsoleIndex, FindIndex;
	Local bool bResult;

	
	bResult = false;
	WorldInfo = GetOwnerScene().GetWorldInfo();
	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		ViewportClient = avaGameViewportClient(LocalPlayer(PC.Player).ViewportClient);
		if( ViewportClient != None )
		{
			FindIndex = ViewportClient.GlobalInteractions.find(ViewportClient.AdminUI);
			if( FindIndex < 0 )
			{
				ViewportConsoleIndex = ViewportClient.GlobalInteractions.find(ViewportClient.ViewportConsole);
				if( ViewportClient.InsertInteraction( ViewportClient.AdminUI, ViewportConsoleIndex ) != -1)
					bResult = true;
			}
			else
			{
				bResult = true;
			}
		}
	}

	if( bResult )
		OutputLinks[0].bHasImpulse=true;
	else
		OutputLinks[1].bHasImpulse=false;
}


defaultproperties
{
	ObjName="(Channel) Create AdminUI"
	ObjCategory="avaNet"

	bAutoActivateOutputLinks=false
	OutputLinks(0)=(LinkDesc="Success")
	OutputLinks(1)=(LinkDesc="Fail")

	VariableLinks.Empty
}