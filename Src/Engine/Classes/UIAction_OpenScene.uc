/**
 * Opens a new scene.
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_OpenScene extends UIAction_Scene
	native(inherit);

/** Output variable for the scene that was opened. */
var	UIScene		OpenedScene;
var() string	SceneName;
var() EScenePriorityType		Priority;

cpptext
{
	virtual void Activated();

	/* === UObject interface === */
	/**
	 * Called after this object has been de-serialized from disk.
	 *
	 * This version converts the deprecated PRIVATE_DisallowReparenting flag to PRIVATE_EditorNoReparent, if set.
	 */
	virtual void PostLoad();

}

DefaultProperties
{
	ObjName="Open Scene"

	Priority=EScenePrior_UIScene_Normal
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Opened Scene",PropertyName=OpenedScene,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Scene Name",PropertyName=SceneName))
}
