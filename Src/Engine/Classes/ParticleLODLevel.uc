/**
 *	ParticleLODLevel
 *
 *	Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class ParticleLODLevel extends Object
	native(Particle)
	collapsecategories
	hidecategories(Object)
	editinlinenew;

/** The index value of the LOD level												*/
var const				int						Level;

/** The percentage value of the slider when it was created							*/
var	const				int						LevelSetting;

/** True if the LOD level is enabled, meaning it should be updated and rendered.	*/
var						bool					bEnabled;

/** The required module for this LOD level											*/
var editinline			ParticleModuleRequired	RequiredModule;

/** An array of particle modules that contain the adjusted data for the LOD level	*/
var editinline export	array<ParticleModule>	Modules;

// Module<SINGULAR> used for emitter type "extension".
var				export	ParticleModule			TypeDataModule;

/** */
var native				array<ParticleModule>	SpawnModules;
var native				array<ParticleModule>	UpdateModules;

var						bool					ConvertedModules;
var						int						PeakActiveParticles;

cpptext
{
	virtual void						PostEditChange(UProperty* PropertyThatChanged);

	virtual void						PostLoad();
	virtual void						UpdateModuleLists();

	virtual UBOOL						GenerateFromLODLevel(UParticleLODLevel* SourceLODLevel, FLOAT Percentage, UBOOL bGenerateModuleData = TRUE);
	virtual UBOOL						GenerateFromBoundingLODLevels(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, UBOOL bGenerateModuleData = TRUE);

	/**
	 *	CalculateMaxActiveParticleCount
	 *	Determine the maximum active particles that could occur with this emitter.
	 *	This is to avoid reallocation during the life of the emitter.
	 *
	 *	@return		The maximum active particle count for the LOD level.
	 */
	virtual INT	CalculateMaxActiveParticleCount();
	
	// For Cascade
	void	AddCurvesToEditor(UInterpCurveEdSetup* EdSetup);
	void	RemoveCurvesFromEditor(UInterpCurveEdSetup* EdSetup);
	void	ChangeEditorColor(FColor& Color, UInterpCurveEdSetup* EdSetup);
}

defaultproperties
{
	bEnabled=true
	ConvertedModules=true
	PeakActiveParticles=0
}
