/**
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */

class avaSkelControl_CantileverBeam extends SkelControlLookAt
	native;

/** The TargetLocation's goal (e.g. where it wants to be) */
var vector WorldSpaceGoal;

/** from the initial bone, where to go to get the starting location for WorldSpaceGoal (in localbonespace) */
var(LookAt) vector InitialWorldSpaceGoalOffset;

/** Current Velocity that TargetLocation is travelling at*/
var vector Velocity;

var(Spring) float SpringStiffness;
var(Spring) float SpringDamping;

/** how much we want the tip of the beam to get of the base velocity */
var() float PercentBeamVelocityTransfer;

//! TargetLocation의 최대 거리 설정.
var(Limit) float MaxTargetDistance;
//! Target의 최대속력.
var(Limit) float MaxTargetSpeed;

/** the speed the entire beam is travelling at. (a delegate for cases like a tank, where we want the whole tank to effect less than the turret moving) */
delegate vector EntireBeamVelocity()
{
	return Vect(0,0,0);
}


cpptext
{
	virtual void TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp);
}

defaultproperties
{
	SpringStiffness=0
	SpringDamping=0
	WorldSpaceGoal=(X=0,Y=0,Z=0)
	Velocity=(X=0,Y=0,Z=0)
	PercentBeamVelocityTransfer = 0.9;
	MaxTargetDistance=30
	MaxTargetSpeed=120
}