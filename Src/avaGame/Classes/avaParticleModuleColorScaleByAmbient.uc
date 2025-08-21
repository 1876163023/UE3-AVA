/**
 *	ParticleModuleColorScaleByAmbient
 *
 *	Lit Particle With Ambient Cube Value.
 *
 * Copyright 2007 Redduck, Inc. All Rights Reserved.
 */

class avaParticleModuleColorScaleByAmbient extends ParticleModuleColorBase
	native(Particle)
	editinlinenew
	collapsecategories
	hidecategories(Object);
	
var(Ambient) vector ParticleColorOffset;
var(Ambient) float	AmbientBase;

cpptext
{
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual void	UpdateEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime, UParticleModule* LowerLODModule, FLOAT Multiplier);
}

defaultproperties
{
	bSpawnModule=false
	bUpdateModule=true
	ParticleColorOffset=(X=0,Y=0,Z=0)
	AmbientBase=0
}
