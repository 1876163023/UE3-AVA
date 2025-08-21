/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleRotationRate extends ParticleModuleRotationRateBase
	native(Particle)
	editinlinenew
	collapsecategories
	hidecategories(Object);

/** Initial rotation rate distribution, in degrees per second. */
var(Rotation) rawdistributionfloat	StartRotationRate;

cpptext
{
	virtual void	Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	SpawnEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime, UParticleModule* LowerLODModule, FLOAT Multiplier);
}

defaultproperties
{
	bSpawnModule=true

	Begin Object Class=DistributionFloatConstant Name=DistributionStartRotationRate
	End Object
	StartRotationRate=(Distribution=DistributionStartRotationRate)
}

