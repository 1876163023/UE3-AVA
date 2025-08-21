/**
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class avaVehicleSimTank extends SVehicleSimTank
	native;

/** Since Tanks osscilate a bit once we hit on target we don't steer as much in the same after we close. */
var bool bForceOnTarget;

/** When driving into something, reduce friction on the wheels. */
var()	float	FrontalCollisionGripFactor;

//! 회전시 기본 EngineTorque에 곱해준다.
var()	float	TurnEngineTorqueFactor;

cpptext
{
	virtual void UpdateVehicle(ASVehicle* Vehicle, FLOAT DeltaTime);
}

DefaultProperties
{
	FrontalCollisionGripFactor=1.0
	TurnEngineTorqueFactor=1.0
}
