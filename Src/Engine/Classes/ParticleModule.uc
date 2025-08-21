/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModule extends Object
	native(Particle)
	editinlinenew
	collapsecategories
	hidecategories(Object)
	abstract;

var				bool			bSpawnModule;
var				bool			bUpdateModule;
var				bool			bCurvesAsColor;
var				bool			b3DDrawMode;
var				bool			bSupported3DDrawMode;
var				bool			bEnabled;
var				bool			bEditable;

var(Cascade)	color			ModuleEditorColor;

/** ModuleType
 *	Indicates the kind of emitter the module can be applied to.
 *	ie, EPMT_Beam - only applies to beam emitters.
 *
 *	The TypeData field is present to speed up finding the TypeData module.
 */
enum EModuleType
{
	/** General - all emitter types can use it			*/
	EPMT_General,
	/** TypeData - TypeData modules						*/
	EPMT_TypeData,
	/** Beam - only applied to beam emitters			*/
	EPMT_Beam,
	/** Trail - only applied to trail emitters			*/
	EPMT_Trail
};

/**
 *	Particle Selection Method, for any emitters that utilize particles
 *	as the source points.
 */
enum EParticleSourceSelectionMethod
{
	/** Random		- select a particle at random		*/
	EPSSM_Random,
	/** Sequential	- select a particle in order		*/
	EPSSM_Sequential
};

cpptext
{
	virtual void	Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime);
	virtual void	Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime);

	virtual UINT	RequiredBytes(FParticleEmitterInstance* Owner = NULL);
	virtual UINT	RequiredBytesPerInstance(FParticleEmitterInstance* Owner = NULL);

	// For Cascade
	virtual void	SpawnEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime, UParticleModule* LowerLODModule, FLOAT Multiplier);
	virtual void	UpdateEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime, UParticleModule* LowerLODModule, FLOAT Multiplier);

			void	GetCurveObjects(TArray<UObject*>& OutCurves);
	virtual	void	AddModuleCurvesToEditor(UInterpCurveEdSetup* EdSetup);
			void	RemoveModuleCurvesFromEditor(UInterpCurveEdSetup* EdSetup);
			UBOOL	ModuleHasCurves();
			UBOOL	IsDisplayedInCurveEd(UInterpCurveEdSetup* EdSetup);
			void	ChangeEditorColor(FColor& Color, UInterpCurveEdSetup* EdSetup);

	virtual void Render3DPreview(FParticleEmitterInstance* Owner, const FSceneView* View,FPrimitiveDrawInterface* PDI)	{};

	virtual EModuleType	GetModuleType() const	{	return EPMT_General;	}

	virtual void	AutoPopulateInstanceProperties(UParticleSystemComponent* PSysComp);

	virtual UBOOL	GenerateLODModuleValues(UParticleModule* SourceModule, FLOAT Percentage, UParticleLODLevel* LODLevel);

	virtual UBOOL	ConvertFloatDistribution(UDistributionFloat* FloatDist, UDistributionFloat* SourceFloatDist, FLOAT Percentage);
	virtual UBOOL	ConvertVectorDistribution(UDistributionVector* VectorDist, UDistributionVector* SourceVectorDist, FLOAT Percentage);
	virtual UBOOL	ConvertColorFloatDistribution(UDistributionFloat* FloatDist);
	virtual UBOOL	ConvertColorVectorDistribution(UDistributionVector* VectorDist);
}

defaultproperties
{
	bSupported3DDrawMode=false
	b3DDrawMode=false
	bEnabled=true
	bEditable=true
}
