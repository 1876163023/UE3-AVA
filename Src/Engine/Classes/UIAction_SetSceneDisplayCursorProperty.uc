/**
 * Set Scene's DisplayCursor property.
 *  If no scene is specified and bAutoTargetOwner is true for this action, closes the owner scene.
 *
 * Copyright 2005-2006 Red duck, Inc. All Rights Reserved.
 */
class UIAction_SetSceneDisplayCursorProperty extends UIAction_Scene
	native(inherit);
	
var() bool bDisplayCursor;

cpptext
{
	virtual void Activated();
}

DefaultProperties
{
	ObjName="Set Scene Display Cursor Property"

	bAutoTargetOwner=true
	
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="bDisplayCursor",PropertyName=bDisplayCursor,bWriteable=true))
}