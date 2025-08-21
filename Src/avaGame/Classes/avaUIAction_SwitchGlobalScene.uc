class avaUIAction_SwitchGlobalScene extends UIAction;

enum EAVAGlobalSceneType
{
	AVAGST_None,
	AVAGST_ChannelList,
	AVAGST_Lobby,
	AVAGST_ClanLobby,
	AVAGST_CreateCharacter,
	AVAGST_Inventory,
	AVAGST_ReadyRoom,
	AVAGST_FrClanRoom,
	AVAGST_Stat
};

var() EAVAGlobalSceneType SceneType;

final function UIInteraction GetUIController()
{
	Local WorldInfo WorldInfo;
	Local PlayerController PC;
	Local UIInteraction UIController;

	WorldInfo = GetWorldInfo();

	if( WorldInfo == None )
		return none;

	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		if ( LocalPlayer(PC.Player) == None )
			continue;

		if ( LocalPlayer(PC.Player).ViewportClient == None )
			continue;

		UIController = LocalPlayer(PC.Player).ViewportClient.UIController;
		return UIController;
	}
	return none;
}

final function UISceneClient GetSceneClient()
{
	Local WorldInfo WorldInfo;
	Local PlayerController PC;
	Local UIInteraction UIController;

	WorldInfo = GetWorldInfo();

	if( WorldInfo == None )
		return none;

	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		UIController = LocalPlayer(PC.Player).ViewportClient.UIController;
		if( UIController != None && UIController.SceneClient != None )
			return UIController.SceneClient;
	}
	return none;
}

final function CloseSceneByTag( name SceneTag )
{
	Local UIInteraction UIController;
	local UIScene SceneToClose;
	local GameUISceneClient SceneClient;

	UIController = GetUIController();

	SceneToClose = UIController.FindSceneByTag( SceneTag );

	if (SceneToClose != None)
	{
		SceneClient = GameUISceneClient(GetSceneClient());
		if( SceneClient != None )
			SceneClient.CloseScene( SceneToClose );
	}
}

event Activated()
{
	Local UIInteraction UIController;
	local UIScene SceneToOpen;
	local string SceneNameToOpen;


	switch (SceneType)
	{
	case AVAGST_None :
		break;
	case AVAGST_ChannelList :
		SceneNameToOpen = "Channel.ChannelList";
		break;
	case AVAGST_Lobby :
		SceneNameToOpen = "Lobby.Lobby";
		break;
	case AVAGST_ClanLobby :
		SceneNameToOpen = "ClanLobby.ClanLobby";
		break;
	case AVAGST_CreateCharacter :
		SceneNameToOpen = "CreateCharacter.CreateCharacter";
		break;
	case AVAGST_Inventory :
		SceneNameToOpen = "CustomParent.CustomParent";
		break;
	case AVAGST_ReadyRoom :
		SceneNameToOpen = "ReadyRoom.ReadyRoom";
		break;
	case AVAGST_FrClanRoom :
		SceneNameToOpen = "ReadyRoom.FrClanRoom";
		break;
	case AVAGST_Stat :
		SceneNameToOpen = "Stat.Stat";
		break;
	}	

	`log( "Switch AVA Global Scene " @ SceneType @ SceneNameToOpen );

	CloseSceneByTag( 'CreateCharacter' );
	CloseSceneByTag( 'Title' );
	CloseSceneByTag( 'ChannelList' );
	CloseSceneByTag( 'Lobby' );
	CloseSceneByTag( 'ClanLobby' );
	CloseSceneByTag( 'ReadyRoom' );
	CloseSceneByTag( 'FrClanRoom' );
	CloseSceneByTag( 'CustomParent' );
	CloseSceneByTag( 'Skill' );
	CloseSceneByTag( 'Award' );
	CloseSceneByTag( 'Stat' );
	CloseSceneByTag( 'HostMigration' );
	CloseSceneByTag( 'Result' );
	CloseSceneByTag( 'Result_MilitaryDrill' );

	if (SceneNameToOpen != "")
	{
		UIController = GetUIController();

		SceneToOpen = UIScene( DynamicLoadObject( class'GameInfo'.static.GetFullUIPath(SceneNameToOpen), class'Engine.UIScene' ) );

		`log( "Trying to switch AVA global scene " @ SceneToOpen @ SceneNameToOpen );

		UIController.OpenScene( SceneToOpen );
	}
}

defaultproperties
{
	ObjName = "(AVA) Switch Global Scene"
	ObjCategory = "UI"	

	ObjClassVersion = 1
}