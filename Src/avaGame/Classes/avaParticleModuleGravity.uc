/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaParticleModuleGravity extends ParticleModuleAccelerationBase
	native(Particle)
	editinlinenew
	collapsecategories
	hidecategories(Object);

var(Acceleration) rawdistributionfloat GravityAmount;

cpptext
{
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual void	UpdateEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime, UParticleModule* LowerLODModule, FLOAT Multiplier);
}

defaultproperties
{
	bSpawnModule=false
	bUpdateModule=true

	Begin Object Class=DistributionFloatConstantCurve Name=DistributionGravityAmount
	End Object
	GravityAmount=(Distribution=DistributionGravityAmount)
}
