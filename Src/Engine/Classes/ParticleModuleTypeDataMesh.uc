/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleTypeDataMesh extends ParticleModuleTypeDataBase
	native(Particle)
	editinlinenew
	collapsecategories
	hidecategories(Object);
	
var(Mesh)	StaticMesh				Mesh;			// The Base Mesh
var(Mesh)	bool					CastShadows;
var(Mesh)	bool					DoCollisions;

enum EMeshScreenAlignment
{
    PSMA_MeshFaceCameraWithRoll,
    PSMA_MeshFaceCameraWithSpin,
    PSMA_MeshFaceCameraWithLockedAxis
};

var(Mesh)	EMeshScreenAlignment	MeshAlignment;

cpptext
{
	virtual void						PostEditChange(UProperty* PropertyThatChanged);
	virtual FParticleEmitterInstance*	CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent);
	virtual void						SetToSensibleDefaults();

	virtual UBOOL	SupportsSpecificScreenAlignmentFlags() const	{	return TRUE;	}
}

defaultproperties
{
	CastShadows=false
	DoCollisions=false
	MeshAlignment=PSMA_MeshFaceCameraWithRoll
}
