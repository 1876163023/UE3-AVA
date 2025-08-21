/**
 * Set User Key for UI Kismet
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_SetUserKey extends UIAction;

var() int		BindingIndex;
var() string	BindName;
var() bool		bSaveConfig;
var() bool		bSwapKey;
var() int		SwappedKeySlot;

event Activated()
{
	Local PlayerInput PlayerInput;
	Local PlayerController LocalPC;
	Local WorldInfo WorldInfo;

	WorldInfo = GetWorldInfo();
	if( WorldInfo == none )
	{
		return;
	}

	foreach WorldInfo.LocalPlayerControllers( LocalPC )
		PlayerInput = LocalPC.PlayerInput;

	if( PlayerInput == none )
	{
		`log("Couldn't execute SetUserKey '"$ BindingIndex $", Name = "$BindName);
		return;
	}


	if( ! bSwapKey )
		PlayerInput.SetUserKey( BindingIndex, name(BindName), bSaveConfig ); 
	else
		PlayerInput.SwapUserKey( BindingIndex, name(BindName), SwappedKeySlot , bSaveConfig ); 
}

DefaultProperties
{
	ObjName="Set UserKey"

	ObjCategory="Misc"

	bAutoTargetOwner=true

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Bind Index",PropertyName=BindingIndex))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Bind Name",PropertyName=BindName))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Save Config",PropertyName=bSaveConfig))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Swap Key",PropertyName=bSwapKey, bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Swapped KeySlot",PropertyName=SwappedKeySlot, bHidden=true))

	ObjClassVersion = 2
}
