/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class CameraMode extends Object
	native;

simulated function ProcessViewRotation( float DeltaTime, Actor ViewTarget, out Rotator out_ViewRotation, out Rotator out_DeltaRot );

defaultproperties
{
}
