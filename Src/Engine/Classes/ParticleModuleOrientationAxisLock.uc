/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleOrientationAxisLock extends ParticleModuleOrientationBase
	native(Particle)
	editinlinenew
	collapsecategories
	hidecategories(Object);

// Flags indicating lock
enum EParticleAxisLock
{
	EPAL_NONE,
	EPAL_X,
	EPAL_Y,
	EPAL_Z,
	EPAL_NEGATIVE_X,
	EPAL_NEGATIVE_Y,
	EPAL_NEGATIVE_Z
};

var(Orientation) EParticleAxisLock	LockAxisFlags;

cpptext
{
	virtual void	Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);

	virtual void	PostEditChange(UProperty* PropertyThatChanged);

	virtual void	SpawnEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime, UParticleModule* LowerLODModule, FLOAT Multiplier);
	virtual void	UpdateEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime, UParticleModule* LowerLODModule, FLOAT Multiplier);
	virtual void	SetLockAxis(EParticleAxisLock eLockFlags);
}

defaultproperties
{
	bSpawnModule=true
	bUpdateModule=true
}
