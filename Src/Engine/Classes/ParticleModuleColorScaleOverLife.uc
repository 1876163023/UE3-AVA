/**
 *	ParticleModuleColorScaleOverLife
 *
 *	The base class for all Beam modules.
 *
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */

class ParticleModuleColorScaleOverLife extends ParticleModuleColorBase
	native(Particle)
	editinlinenew
	collapsecategories
	hidecategories(Object);

/** The scale factor for the color.													*/
var(Color)				rawdistributionvector	ColorScaleOverLife;

/** The scale factor for the alpha.													*/
var(Color)				rawdistributionfloat	AlphaScaleOverLife;

/** Whether it is EmitterTime or ParticleTime related.								*/
var(Color)				bool					bEmitterTime;

cpptext
{
	virtual void	Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual void	SpawnEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime, UParticleModule* LowerLODModule, FLOAT Multiplier);
	virtual void	UpdateEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime, UParticleModule* LowerLODModule, FLOAT Multiplier);
}

defaultproperties
{
	bSpawnModule=true
	bUpdateModule=true

	Begin Object Class=DistributionVectorConstantCurve Name=DistributionColorScaleOverLife
	End Object
	ColorScaleOverLife=(Distribution=DistributionColorScaleOverLife)

	Begin Object Class=DistributionFloatConstant Name=DistributionAlphaScaleOverLife
		Constant=1.0f;
	End Object
	AlphaScaleOverLife=(Distribution=DistributionAlphaScaleOverLife)
}
