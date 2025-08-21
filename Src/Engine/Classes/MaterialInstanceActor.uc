/**
 *	Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 *
 *	Utility class designed to allow you to connect a MaterialInstance to a Matinee action.
 */

class MaterialInstanceActor extends Actor
	native
	placeable
	hidecategories(Movement)
	hidecategories(Advanced)
	hidecategories(Collision)
	hidecategories(Display)
	hidecategories(Actor)
	hidecategories(Attachment);

/** Pointer to MaterialInstance that we want to control paramters of using Matinee. */
var()	MaterialInstanceConstant	MatInst;

defaultproperties
{
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EngineResources.MatInstActSprite'
		HiddenGame=true
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	bNoDelete=true
}
