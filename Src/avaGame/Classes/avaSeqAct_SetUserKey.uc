/**
 * Set User Key for UI Kismet
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class avaSeqAct_SetUserKey extends UIAction;

var() int		BindingIndex;
var() string	BindName;
var() bool		bSaveConfig;
var() bool		bSwapKey;
var() int		SwappedKeySlot;
var() string	ResultComment;

event Activated()
{
	Local PlayerInput PlayerInput;
	Local PlayerController LocalPC;
	Local WorldInfo WorldInfo;
	Local bool bHasBindName, bResult;
	Local string UserKeyName;
	Local KeyBind KeyBind, SelKeyBind, InnerKeyBind;	// KeyBind To Swap, Selected KeyBind, inner KeyBind unencapsulated

	WorldInfo = GetWorldInfo();
	if( WorldInfo == none )
	{
		OutputLinks[1].bHasImpulse =true;
		ResultComment = "invalid worldinfo";
		return;
	}

	foreach WorldInfo.LocalPlayerControllers( LocalPC )
		PlayerInput = LocalPC.PlayerInput;

	if( PlayerInput == none )
	{
		OutputLinks[1].bHasImpulse = true;
		ResultComment = "invalid playerinput";
		return;
	}
	bHasBindName = PlayerInput.HasBindingName( BindName, KeyBind);
	PlayerInput.GetKeyBindByINdex(BindingIndex, SelKeyBind);
	PlayerInput.GetKeyBindByIndex(BindingIndex, InnerKeyBind, true);

	if( bHasBindName )
	{
		// 바인드 슬롯이 존재하고 키스왑을 요청했을때만
		if( KeyBind.Slot > 0 && bSwapKey )
		{
			if( (BindName == "MouseScrollUp" || BindName == "MouseScrollDown") &&
				PlayerInput.IsSpecificCommand( InnerKeyBind, TRUE, TRUE, TRUE, FALSE, FALSE) )
			{
				UserKeyName = class'avaStringHelper'.static.Replace(SelKeyBind.Slot,"%d","UserKeyName[%d]");
				UserKeyName = Localize("PlayerInput",UserKeyName,"avaGame");

				ResultComment = Localize("UIOptionScene","Text_CantBindKey_MouseScroll","avaNet");
				Resultcomment = class'avaStringHelper'.static.Replace(UserKeyName, "%s", ResultComment);
				bResult = false;
			}
			else
			{
				PlayerInput.SwapUserKey( BindingIndex, name(BindName), SwappedKeySlot , bSaveConfig );
				UserKeyName = class'avaStringHelper'.static.Replace(SwappedKeySlot,"%d","UserKeyName[%d]");
				UserKeyName = Localize("PlayerInput",UserKeyName,"avaGame");

				ResultComment = Localize("UIOptionScene","Text_SwapBindingKey","avaNet");
				Resultcomment = class'avaStringHelper'.static.Replace(UserKeyName, "%s", ResultComment);
				bSwapKey = true;
				bResult = true;
			}
		}
		else
		{
			if( KeyBind.Slot <= 0 )
				ResultComment = Localize("UIOptionScene","Text_CantBindKey","avaNet");
			else
			{
				UserKeyName = class'avaStringHelper'.static.Replace(SwappedKeySlot,"%d","UserKeyName[%d]");
				ResultComment = Localize("UIOptionScene","Text_AlreadyBoundKey","avaNet");
				ResultComment = class'avaStringHelper'.static.Replace(UserKeyName, "%s", ResultComment);
			}
			bResult = false;	
		}
	}
	else
	{
		PlayerInput.SetUserKey( BindingIndex, name(BindName), bSaveConfig );			
		bResult = true;
	}

	OutputLinks[bResult ? 0 : 1].bHasImpulse = true;
}

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

DefaultProperties
{
	ObjName="Set UserKey"

	ObjCategory="Adjust"

	bAutoActivateOutputLinks=false

	OutputLinks[0]=(LinkDesc="success")
	OutputLinks[1]=(LinkDesc="failed")

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Bind Index",PropertyName=BindingIndex))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Bind Name",PropertyName=BindName))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Save Config",PropertyName=bSaveConfig,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Swap Key",PropertyName=bSwapKey, bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Swapped KeySlot",PropertyName=SwappedKeySlot, bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Result Comment",PropertyName=ResultComment))

	ObjClassVersion = 3
}
