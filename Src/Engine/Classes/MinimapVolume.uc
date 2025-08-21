/**
 * Copyright ?2006 Redduck, Inc. All Rights Reserved.
 *
 * Used to create minimap texture for a level
 */
class MinimapVolume extends Volume
	native
	placeable;

defaultproperties
{
	Begin Object Name=BrushComponent0
		CollideActors=False
		BlockActors=False
		BlockZeroExtent=False
		BlockNonZeroExtent=False
		BlockRigidBody=False
	End Object

	bCollideActors=False
	bBlockActors=False
	bProjTarget=False
	SupportedEvents.Empty
}
