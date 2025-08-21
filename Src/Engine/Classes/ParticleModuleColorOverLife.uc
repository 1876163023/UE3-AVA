/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleColorOverLife extends ParticleModuleColorBase
	native(Particle)
	editinlinenew
	collapsecategories
	hidecategories(Object);

var(Color)					rawdistributionvector	ColorOverLife;
var(Color)					rawdistributionfloat	AlphaOverLife;
var(Color)					bool					bClampAlpha;

cpptext
{
	virtual void	PostLoad();
	virtual	void	PostEditChange(UProperty* PropertyThatChanged);
	virtual	void	AddModuleCurvesToEditor(UInterpCurveEdSetup* EdSetup);
	virtual void	Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual void	SpawnEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime, UParticleModule* LowerLODModule, FLOAT Multiplier);
	virtual void	UpdateEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime, UParticleModule* LowerLODModule, FLOAT Multiplier);
}

defaultproperties
{
	bSpawnModule=true
	bUpdateModule=true
	bCurvesAsColor=true
	bClampAlpha=true

	Begin Object Class=DistributionVectorConstantCurve Name=DistributionColorOverLife
	End Object
	ColorOverLife=(Distribution=DistributionColorOverLife)

	Begin Object Class=DistributionFloatConstant Name=DistributionAlphaOverLife
		Constant=1.0f;
	End Object
	AlphaOverLife=(Distribution=DistributionAlphaOverLife)
}
