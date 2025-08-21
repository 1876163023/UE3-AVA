/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaParticleModuleAirResistance extends ParticleModuleAccelerationBase
	native(Particle)
	editinlinenew
	collapsecategories
	hidecategories(Object);

var(Acceleration) rawdistributionvector	AirResistanceBySize;
var(Acceleration) rawdistributionvector	AirResistanceByVelocity;

cpptext
{
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual void	UpdateEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime, UParticleModule* LowerLODModule, FLOAT Multiplier);
}

defaultproperties
{
	bSpawnModule=false
	bUpdateModule=true

	Begin Object Class=DistributionVectorConstantCurve Name=DistributionAirResistanceBySize
	End Object
	AirResistanceBySize=(Distribution=DistributionAirResistanceBySize)

	Begin Object Class=DistributionVectorConstantCurve Name=DistributionAirResistanceByVelocity
	End Object
	AirResistanceByVelocity=(Distribution=DistributionAirResistanceByVelocity)
}
