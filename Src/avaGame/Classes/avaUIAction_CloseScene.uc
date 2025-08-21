/**
 * Closes a scene.  If no scene is specified and bAutoTargetOwner is true for this action, closes the owner scene.
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class avaUIAction_CloseScene extends UIAction;

var() array<UIScene>		ScenesToClose;

event Activated()
{
	Local UIScene OwnerScene;
	Local UIScene SceneToClose;

	OwnerScene = GetOwnerScene();
	if( OwnerScene == none )
	{
		OutputLinks[1].bHasImpulse = true;
		return;
	}

	if( OwnerScene.SceneClient == none )
	{
		OutputLinks[1].bHasImpulse = true;
		return;
	}

	foreach  ScenesToClose( SceneToClose )
	{
		OwnerScene.SceneClient.CloseScene(SceneToClose);
	}

	OutputLinks[0].bHasImpulse=true;
}

DefaultProperties
{
	ObjName="(AVA) Close Scene"
	ObjCategory="UI"

	bCallHandler=false
	bAutoActivateOutputLinks=false

	OutputLinks(0)=(LinkDesc="Success")
	OutputLinks(1)=(LinkDesc="Failed")

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Scenes",PropertyName=ScenesToClose))
	bAutoTargetOwner=true
}
