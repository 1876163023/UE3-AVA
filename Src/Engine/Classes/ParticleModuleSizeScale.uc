/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleSizeScale extends ParticleModuleSizeBase
	native(Particle)
	editinlinenew
	collapsecategories
	hidecategories(Object);

var()					rawdistributionvector	SizeScale;
var()					bool					EnableX;
var()					bool					EnableY;
var()					bool					EnableZ;

cpptext
{
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);
	virtual void	UpdateEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime, UParticleModule* LowerLODModule, FLOAT Multiplier);
}

defaultproperties
{
	bSpawnModule=false
	bUpdateModule=true

	EnableX=true
	EnableY=true
	EnableZ=true

	Begin Object Class=DistributionVectorConstant Name=DistributionSizeScale
	End Object
	SizeScale=(Distribution=DistributionSizeScale)
}
