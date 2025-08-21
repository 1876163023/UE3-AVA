/**
 * Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This is a movable trigger volume. It can be moved by matinee, being based on
 * dynamic objects, etc.
 */
class DynamicTriggerVolume extends TriggerVolume
	native
	placeable;

cpptext
{
	/** Returns the size of the extent to use when moving the object through the world */
	virtual FVector GetCylinderExtent() const
	{
		FVector Extent(0.f);
		if (BrushComponent != NULL)
		{
			Extent = BrushComponent->Bounds.BoxExtent;
		}
		return Extent;
	}
}

defaultproperties
{
	bStatic=false

	bAlwaysRelevant=true
	bReplicateMovement=true
	bOnlyDirtyReplication=true
	RemoteRole=ROLE_None

	bColored=true
	BrushColor=(R=100,G=255,B=255,A=255)
}