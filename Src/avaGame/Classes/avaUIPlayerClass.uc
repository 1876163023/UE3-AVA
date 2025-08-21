/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaUIPlayerClass extends Interaction
	within avaGameViewportClient
	config(Game);

/** True if the menus are open. */
//var bool bOpened;	
//var bool bInitialized;
//
//var UIScene			ClassSelectScene;
//
//
//simulated function OnSceneDeactivated( UIScene DeactivatedScene )
//{
//	`log( "avaUIPlayerClass.OnSceneDeativated" );
//	bOpened = false;
//}
//
//simulated function OnSceneActivated( UIScene ActivatedScene, bool bInitialActivation )
//{
//	`log( "avaUIPlayerClass.OnSceneActivated" );
//	bOpened = true;
//}
//
//simulated function OpenPlayerClassUI()
//{
//	`log( "avaUIPlayerClass.OpenPlayerClassUI" );
//	ClassSelectScene.OnSceneDeactivated = OnSceneDeactivated;
//	ClassSelectScene.OnSceneActivated   = OnSceneActivated;
//	UIController.OpenScene( ClassSelectScene, GamePlayers[0] );
//}
//
//simulated function ClosePlayerClassUI()
//{
//	local avaPlayerReplicationInfo avaPRI;
//	avaPRI = avaPlayerReplicationInfo(GamePlayers[0].Actor.PlayerReplicationInfo);
//	if ( !avaPRI.IsPlayerClassValid() )
//		return;
//	UIController.CloseScene(ClassSelectScene);
//}
//
//event bool InputKey( int ControllerId, name Key, EInputEvent Event, float AmountDepressed = 1.f )
//{
//	if(Key == 'F4' && Event == IE_Pressed)
//	{
//		if ( bOpened )	ClosePlayerClassUI();
//		else			OpenPlayerClassUI();
//		return true;
//	}
//	else
//		return super.InputKey(ControllerId,Key,Event,AmountDepressed);
//}
//
//
//defaultproperties
//{
//	ClassSelectScene = UIScene'TestUI_ClassSelect.ClassSelect'
//}
