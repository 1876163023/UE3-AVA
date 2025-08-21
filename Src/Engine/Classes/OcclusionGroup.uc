/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class OcclusionGroup extends Volume
	placeable
	native;

var() const editconst OcclusionGroupComponent	Component;

cpptext
{
	virtual void PostBeginPlay();
}

defaultproperties
{
	Begin Object Class=OcclusionGroupComponent Name=OcclusionGroupComponent0
	End Object
	Component=OcclusionGroupComponent0
	Components.Add(OcclusionGroupComponent0)

	bStatic=true
	bNoDelete=true
}
