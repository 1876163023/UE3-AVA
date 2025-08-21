/**
 * Copyright 2006 Epic Games, Inc. All Rights Reserved.
 *
 * Used to associate lights with volumes.
 */
class AmbientCubeVolume extends Volume
	native
	placeable;

var() int CubeSizeX, CubeSizeY, CubeSizeZ;
var() int Priority;

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

	CubeSizeX = 32
	CubeSizeY = 32
	CubeSizeZ = 128
}
