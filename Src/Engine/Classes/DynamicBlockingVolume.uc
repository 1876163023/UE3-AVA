/**
 * Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This is a movable blocking volume. It can be moved by matinee, being based on
 * dynamic objects, etc.
 */
class DynamicBlockingVolume extends BlockingVolume
	native
	placeable;

cpptext
{
	virtual void CheckForErrors();

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
	//@todo - Change back to PHYS_None
	Physics=PHYS_Interpolating

	bStatic=false

	bAlwaysRelevant=true
	bReplicateMovement=true
	bOnlyDirtyReplication=true
	RemoteRole=ROLE_None

	BrushColor=(R=255,G=255,B=100,A=255)
}