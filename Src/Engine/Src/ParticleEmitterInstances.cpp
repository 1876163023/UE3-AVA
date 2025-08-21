/*=============================================================================
	ParticleEmitterInstances.cpp: Particle emitter instance implementations.
	Copyright 2003-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineParticleClasses.h"
#include "EngineMaterialClasses.h"
#include "LevelUtils.h"
#include "UnNovodexSupport.h"

IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(FParticleSpriteEmitterInstance);
IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(FParticleSpriteSubUVEmitterInstance);
IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(FParticleMeshEmitterInstance);
IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(FParticleBeam2EmitterInstance);
IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(FParticleTrail2EmitterInstance);
//IMPLEMENT_PARTICLEEMITTERINSTANCE_TYPE(FParticleSpriteNxFluidEmitterInstance);

/*-----------------------------------------------------------------------------
	FParticleEmitterInstance
-----------------------------------------------------------------------------*/
/**
 *	ParticleEmitterInstance
 *	The base structure for all emitter instance classes
 */
FParticleEmitterInstanceType FParticleEmitterInstance::StaticType(TEXT("FParticleEmitterInstance"),NULL);
 
/** Constructor	*/
FParticleEmitterInstance::FParticleEmitterInstance() :
	  SpriteTemplate(NULL)
    , Component(NULL)
    , CurrentLODLevelIndex(0)
    , CurrentLODLevel(NULL)
    , TypeDataOffset(0)
    , SubUVDataOffset(0)
    , KillOnDeactivate(0)
    , bKillOnCompleted(0)
    , ParticleData(NULL)
    , ParticleIndices(NULL)
    , InstanceData(NULL)
    , InstancePayloadSize(0)
    , PayloadOffset(0)
    , ParticleSize(0)
    , ParticleStride(0)
    , ActiveParticles(0)
    , MaxActiveParticles(0)
    , SpawnFraction(0.0f)
    , SecondsSinceCreation(0.0f)
    , EmitterTime(0.0f)
    , LoopCount(0)
	, IsRenderDataDirty(0)
    , Module_AxisLock(NULL)
    , EmitterDuration(0.0f)
	, TrianglesToRender(0)
{
}

/** Destructor	*/
FParticleEmitterInstance::~FParticleEmitterInstance()
{
	appFree(ParticleData);
    appFree(ParticleIndices);
    appFree(InstanceData);
}

/**
 *	Set the KillOnDeactivate flag to the given value
 *
 *	@param	bKill	Value to set KillOnDeactivate to.
 */
void FParticleEmitterInstance::SetKillOnDeactivate(UBOOL bKill)
{
	KillOnDeactivate = bKill;
}

/**
 *	Set the KillOnCompleted flag to the given value
 *
 *	@param	bKill	Value to set KillOnCompleted to.
 */
void FParticleEmitterInstance::SetKillOnCompleted(UBOOL bKill)
{
	bKillOnCompleted = bKill;
}

/**
 *	Initialize the parameters for the structure
 *
 *	@param	InTemplate		The ParticleEmitter to base the instance on
 *	@param	InComponent		The owning ParticleComponent
 *	@param	bClearResources	If TRUE, clear all resource data
 */
void FParticleEmitterInstance::InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources)
{
	SpriteTemplate = CastChecked<UParticleSpriteEmitter>(InTemplate);
    Component = InComponent;
	SetupEmitterDuration();
}

/**
 *	Initialize the instance
 */
void FParticleEmitterInstance::Init()
{
	// This assert makes sure that packing is as expected.
	// Added FBaseColor...
	// Linear color change
	// Added Flags field
	check(sizeof(FBaseParticle) == 128);

	// Calculate particle struct size, size and average lifetime.
	ParticleSize = sizeof(FBaseParticle);
	INT	ReqBytes;
	INT ReqInstanceBytes = 0;
	INT TempInstanceBytes;

	UParticleLODLevel* LODLevel = SpriteTemplate->GetLODLevel(0);
	check(LODLevel);
	UParticleModule* TypeDataModule = LODLevel->TypeDataModule;
	if (TypeDataModule)
	{
		ReqBytes = TypeDataModule->RequiredBytes(this);
		if (ReqBytes)
		{
			TypeDataOffset	 = ParticleSize;
			ParticleSize	+= ReqBytes;
		}

		TempInstanceBytes = TypeDataModule->RequiredBytesPerInstance(this);
		if (TempInstanceBytes)
		{
			ReqInstanceBytes += TempInstanceBytes;
		}
	}

	// NOTE: This code assumes that the same module order occurs in all LOD levels
	for (INT i = 0; i < LODLevel->Modules.Num(); i++)
	{
		UParticleModule* ParticleModule = LODLevel->Modules(i);
		check(ParticleModule);
		if (ParticleModule->IsA(UParticleModuleTypeDataBase::StaticClass()) == FALSE)
		{
			ReqBytes	= ParticleModule->RequiredBytes(this);
			if (ReqBytes)
			{
				ModuleOffsetMap.Set(ParticleModule, ParticleSize);
				ParticleSize	+= ReqBytes;
			}

			TempInstanceBytes = ParticleModule->RequiredBytesPerInstance(this);
			if (TempInstanceBytes)
			{
			    ModuleInstanceOffsetMap.Set(ParticleModule, ReqInstanceBytes);
				ReqInstanceBytes += TempInstanceBytes;
			}
		}

		if (ParticleModule->IsA(UParticleModuleOrientationAxisLock::StaticClass()))
		{
			Module_AxisLock	= Cast<UParticleModuleOrientationAxisLock>(ParticleModule);
		}
	}

	if ((InstanceData == NULL) || (ReqInstanceBytes > InstancePayloadSize))
	{
		InstanceData = (BYTE*)(appRealloc(InstanceData, ReqInstanceBytes));
		InstancePayloadSize = ReqInstanceBytes;
	}

	appMemzero(InstanceData, InstancePayloadSize);

	// Offset into emitter specific payload (e.g. TrailComponent requires extra bytes).
	PayloadOffset = ParticleSize;
	
	// Update size with emitter specific size requirements.
	ParticleSize += RequiredBytes();

	// Make sure everything is at least 16 byte aligned so we can use SSE for FVector.
	ParticleSize = Align(ParticleSize, 16);

	// E.g. trail emitters store trailing particles directly after leading one.
	ParticleStride			= CalculateParticleStride(ParticleSize);

	// Set initial values.
	SpawnFraction			= 0;
	SecondsSinceCreation	= 0;
	
	Location				= Component->LocalToWorld.GetOrigin();
	OldLocation				= Location;
	
	TrianglesToRender		= 0;
	MaxVertexIndex			= 0;

	if (ParticleData == NULL)
	{
		MaxActiveParticles	= 0;
		ActiveParticles		= 0;
	}

	ParticleBoundingBox.Init();
	check(LODLevel->RequiredModule);
	if (LODLevel->RequiredModule->RandomImageChanges == 0)
	{
		LODLevel->RequiredModule->RandomImageTime	= 1.0f;
	}
	else
	{
		LODLevel->RequiredModule->RandomImageTime	= 0.99f / (LODLevel->RequiredModule->RandomImageChanges + 1);
	}

	// Resize to sensible default.
	if (GIsGame == TRUE)
	{
		if ((LODLevel->PeakActiveParticles > 0) || (SpriteTemplate->InitialAllocationCount > 0))
		{
			// In-game... we assume the editor has set this properly, but still clamp at 100 to avoid wasting
			// memory.
			if (SpriteTemplate->InitialAllocationCount > 0)
			{
				Resize(Min( SpriteTemplate->InitialAllocationCount, 100 ));
			}
			else
			{
				Resize(Min( LODLevel->PeakActiveParticles, 100 ));
			}
		}
		else
		{
			// This is to force the editor to 'select' a value
			Resize(10);
		}
	}

	LoopCount = 0;

	// Propagate killon flags
	SetKillOnDeactivate(LODLevel->RequiredModule->bKillOnDeactivate);
	SetKillOnCompleted(LODLevel->RequiredModule->bKillOnCompleted);

	// Reset the burst lists
	if (BurstFired.Num() < SpriteTemplate->LODLevels.Num())
	{
		BurstFired.AddZeroed(SpriteTemplate->LODLevels.Num() - BurstFired.Num());
	}
	for (INT LODIndex = 0; LODIndex < SpriteTemplate->LODLevels.Num(); LODIndex++)
	{
		LODLevel = SpriteTemplate->LODLevels(LODIndex);
		check(LODLevel);
		if (BurstFired(LODIndex).BurstFired.Num() < LODLevel->RequiredModule->BurstList.Num())
		{
			BurstFired(LODIndex).BurstFired.AddZeroed(LODLevel->RequiredModule->BurstList.Num() - BurstFired(LODIndex).BurstFired.Num());
		}
	}
	ResetBurstList();

	// Tag it as dirty w.r.t. the renderer
	IsRenderDataDirty	= 1;
}

/**
 *	Resize the particle data array
 *
 *	@param	NewMaxActiveParticles	The new size to use
 */
void FParticleEmitterInstance::Resize(INT NewMaxActiveParticles, UBOOL bSetMaxActiveCount)
{
	if (NewMaxActiveParticles > MaxActiveParticles)
	{
		// Alloc (or realloc) the data array
		// Allocations > 16 byte are always 16 byte aligned so ParticleData can be used with SSE.
		ParticleData = (BYTE*) appRealloc(ParticleData, ParticleStride * NewMaxActiveParticles);
		check(ParticleData);

		// Clear out any new memory
		if (MaxActiveParticles == 0)
		{
			appMemzero(ParticleData, ParticleStride * NewMaxActiveParticles);
		}
		else
		{
			appMemzero(ParticleData + (MaxActiveParticles * ParticleStride), 
				(NewMaxActiveParticles - MaxActiveParticles) * ParticleStride);
		}

		// Allocate memory for indices.
		if (ParticleIndices == NULL)
		{
			// Make sure that we clear all when it is the first alloc
			MaxActiveParticles = 0;
		}
		ParticleIndices	= (WORD*) appRealloc(ParticleIndices, sizeof(WORD) * NewMaxActiveParticles);

		// Fill in default 1:1 mapping.
		for (INT i=MaxActiveParticles; i<NewMaxActiveParticles; i++)
		{
			ParticleIndices[i] = i;
		}

		// Set the max count
		MaxActiveParticles = NewMaxActiveParticles;
	}

	// Set the PeakActiveParticles
	if (bSetMaxActiveCount)
	{
		UParticleLODLevel* LODLevel	= SpriteTemplate->GetLODLevel(0);
		check(LODLevel);
		if (MaxActiveParticles > LODLevel->PeakActiveParticles)
		{
			LODLevel->PeakActiveParticles = MaxActiveParticles;
		}
	}
}

/**
 *	Tick the instance.
 *
 *	@param	DeltaTime			The time slice to use
 *	@param	bSuppressSpawning	If TRUE, do not spawn during Tick
 */
void FParticleEmitterInstance::Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
	SCOPE_CYCLE_COUNTER(STAT_SpriteTickTime);

	// Grab the current LOD level
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);

	// Make sure we don't try and do any interpolation on the first frame we are attached (OldLocation is not valid in this circumstance)
	if(Component->bJustAttached)
	{
		Location	= Component->LocalToWorld.GetOrigin();
		OldLocation	= Location;
	}
	else
	{
		// Keep track of location for world- space interpolation and other effects.
		OldLocation	= Location;
		Location	= Component->LocalToWorld.GetOrigin();
	}

	// If this the FirstTime we are being ticked?
	UBOOL bFirstTime = (SecondsSinceCreation > 0.0f) ? FALSE : TRUE;
	SecondsSinceCreation += DeltaTime;

	// Update time within emitter loop.
	EmitterTime = SecondsSinceCreation;
	if (EmitterDuration > KINDA_SMALL_NUMBER)
	{
		EmitterTime = appFmod(SecondsSinceCreation, EmitterDuration);
	}

	// Get the emitter delay time
	FLOAT EmitterDelay = LODLevel->RequiredModule->EmitterDelay;

	// Determine if the emitter has looped
	if ((SecondsSinceCreation - (EmitterDuration * LoopCount)) >= EmitterDuration)
	{
		LoopCount++;
		ResetBurstList();

		if (LODLevel->RequiredModule->bDurationRecalcEachLoop == TRUE)
		{
			SetupEmitterDuration();
		}
	}

	// Don't delay unless required
	if ((LODLevel->RequiredModule->bDelayFirstLoopOnly == TRUE) && (LoopCount > 0))
	{
		EmitterDelay = 0;
	}

	// 'Reset' the emitter time so that the modules function correctly
	EmitterTime -= EmitterDelay;

	// Kill off any dead particles
	KillParticles();

	// If not suppressing spawning...
	if (!bSuppressSpawning && (EmitterTime >= 0.0f))
	{
		SCOPE_CYCLE_COUNTER(STAT_SpriteSpawnTime);
		// If emitter is not done - spawn at current rate.
		// If EmitterLoops is 0, then we loop forever, so always spawn.
		if ((LODLevel->RequiredModule->EmitterLoops == 0) || 
			(LoopCount < LODLevel->RequiredModule->EmitterLoops) ||
			(SecondsSinceCreation < (EmitterDuration * LODLevel->RequiredModule->EmitterLoops)) ||
			bFirstTime)
		{
            bFirstTime = FALSE;

			// Figure out spawn rate for this tick.
			FLOAT SpawnRate = LODLevel->RequiredModule->SpawnRate.GetValue(EmitterTime, Component);

			// Take bursts into account
			INT Burst = 0;
			FLOAT BurstTime = GetCurrentBurstRateOffset(DeltaTime, Burst);
			SpawnRate += BurstTime;

			// Spawn new particles...
			if (SpawnRate > 0.f)
			{
				SpawnFraction = Spawn(SpawnFraction, SpawnRate, DeltaTime, Burst, BurstTime);
			}
		}
	}

	// Reset particle parameters.
	for (INT ParticleIndex = 0; ParticleIndex < ActiveParticles; ParticleIndex++)
	{
		DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[ParticleIndex]);
		Particle.Velocity		= Particle.BaseVelocity;
		Particle.Size			= Particle.BaseSize;
		Particle.RotationRate	= Particle.BaseRotationRate;
		Particle.Color			= Particle.BaseColor;
        Particle.RelativeTime	+= Particle.OneOverMaxLifetime * DeltaTime;

		INC_DWORD_STAT(STAT_SpriteParticlesUpdated);
	}

	// Update the particles
	SCOPE_CYCLE_COUNTER(STAT_SpriteUpdateTime);

	UParticleModuleTypeDataBase* pkBase = 0;
	if (LODLevel->TypeDataModule)
	{
		pkBase = Cast<UParticleModuleTypeDataBase>(LODLevel->TypeDataModule);
		//@todo. Need to track TypeData offset into payload!
		pkBase->PreUpdate(this, TypeDataOffset, DeltaTime);
	}

	// Update existing particles (might respawn dying ones).
	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels(0);
	for (INT ModuleIndex = 0; ModuleIndex < LODLevel->UpdateModules.Num(); ModuleIndex++)
	{
		UParticleModule* HighModule	= LODLevel->UpdateModules(ModuleIndex);
		if (HighModule && HighModule->bEnabled)
		{
			UParticleModule* OffsetModule = HighestLODLevel->UpdateModules(ModuleIndex);
			UINT* Offset = ModuleOffsetMap.Find(OffsetModule);
			HighModule->Update(this, Offset ? *Offset : 0, DeltaTime);
		}
	}

	// Handle the TypeData module
	if (pkBase)
	{
		//@todo. Need to track TypeData offset into payload!
		pkBase->Update(this, TypeDataOffset, DeltaTime);
		pkBase->PostUpdate(this, TypeDataOffset, DeltaTime);
	}

	// Calculate bounding box and simulate velocity.
	UpdateBoundingBox(DeltaTime);

	// Invalidate the contents of the vertex/index buffer.
	IsRenderDataDirty = 1;

	// 'Reset' the emitter time so that the delay functions correctly
	EmitterTime += EmitterDelay;
}

/**
 *	Tick the instance in the editor.
 *	This function will interpolate between the current LODLevels to allow for
 *	the designer to visualize how the selected LOD setting would look.
 *
 *	@param	HighLODLevel		The higher LOD level selected
 *	@param	LowLODLevel			The lower LOD level selected
 *	@param	Multiplier			The interpolation value to use between the two
 *	@param	DeltaTime			The time slice to use
 *	@param	bSuppressSpawning	If TRUE, do not spawn during Tick
 */
void FParticleEmitterInstance::TickEditor(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, FLOAT Multiplier, FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
	// Verify all the data...
	check(HighLODLevel);
	check(LowLODLevel);
	check(HighLODLevel->UpdateModules.Num() == LowLODLevel->UpdateModules.Num());
	check(HighLODLevel->SpawnModules.Num() == LowLODLevel->SpawnModules.Num());
	
	// We don't allow different TypeDataModules
	if (HighLODLevel->TypeDataModule)
	{
		check(LowLODLevel->TypeDataModule);
		check(HighLODLevel->TypeDataModule->GetClass() == LowLODLevel->TypeDataModule->GetClass());
	}

	// Stats
	SCOPE_CYCLE_COUNTER(STAT_SpriteTickTime);

	// Make sure we don't try and do any interpolation on the first frame we are attached (OldLocation is not valid in this circumstance)
	if(Component->bJustAttached)
	{
		Location	= Component->LocalToWorld.GetOrigin();
		OldLocation	= Location;
	}
	else
	{
		// Keep track of location for world- space interpolation and other effects.
		OldLocation	= Location;
		Location	= Component->LocalToWorld.GetOrigin();
	}

	// FirstTime this instance has been ticked?
	UBOOL bFirstTime = (SecondsSinceCreation > 0.0f) ? FALSE : TRUE;
	SecondsSinceCreation += DeltaTime;

	// Update time within emitter loop.
	EmitterTime = SecondsSinceCreation;
	FLOAT	Duration	= EmitterDurations(HighLODLevel->Level);
	if (Duration > KINDA_SMALL_NUMBER)
	{
		EmitterTime = appFmod(SecondsSinceCreation, Duration);
	}

	// Take delay into account
	FLOAT EmitterDelay = HighLODLevel->RequiredModule->EmitterDelay;

	// Handle looping
	if ((SecondsSinceCreation - (Duration * LoopCount)) >= Duration)
	{
		LoopCount++;
		ResetBurstList();

		if (HighLODLevel->RequiredModule->bDurationRecalcEachLoop == TRUE)
		{
			SetupEmitterDuration();
		}
	}

	// Don't delay if it is not requested
	if ((LoopCount > 0) && (HighLODLevel->RequiredModule->bDelayFirstLoopOnly == TRUE))
	{
		EmitterDelay = 0;
	}

	// 'Reset' the emitter time so that the modules function correctly
	EmitterTime -= EmitterDelay;

	// Kill any dead particles off
	KillParticles();

	// If not suppressing spawning...
	if (!bSuppressSpawning && (EmitterTime >= 0.0f))
	{
		SCOPE_CYCLE_COUNTER(STAT_SpriteSpawnTime);
		// If emitter is not done - spawn at current rate.
		// If EmitterLoops is 0, then we loop forever, so always spawn.
		if ((HighLODLevel->RequiredModule->EmitterLoops == 0) || 
			(LoopCount < HighLODLevel->RequiredModule->EmitterLoops) ||
			(SecondsSinceCreation < (Duration * HighLODLevel->RequiredModule->EmitterLoops)) ||
			bFirstTime)
		{
            bFirstTime = FALSE;

			// Figure out spawn rate for this tick.
			FLOAT	HighSpawnRate	= HighLODLevel->RequiredModule->SpawnRate.GetValue(EmitterTime, Component);
			FLOAT	LowSpawnRate	= LowLODLevel->RequiredModule->SpawnRate.GetValue(EmitterTime, Component);
			FLOAT	SpawnRate		= (HighSpawnRate * Multiplier) + (LowSpawnRate * (1.0f - Multiplier));

			INT		Burst		= 0;
			FLOAT	BurstTime	= GetCurrentBurstRateOffsetEditor(HighLODLevel, LowLODLevel, Multiplier, DeltaTime, Burst);
			SpawnRate	+= BurstTime;

			// Spawn new particles...
			if (SpawnRate > 0.f)
			{
				SpawnFraction = SpawnEditor(HighLODLevel, LowLODLevel, Multiplier, SpawnFraction, SpawnRate, DeltaTime, Burst, BurstTime);
			}
		}
	}

	// Reset particle information.
	for (INT i=0; i<ActiveParticles; i++)
	{
		DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
		Particle.Velocity		= Particle.BaseVelocity;
		Particle.Size			= Particle.BaseSize;
		Particle.RotationRate	= Particle.BaseRotationRate;
		Particle.Color			= Particle.BaseColor;
        Particle.RelativeTime	+= Particle.OneOverMaxLifetime * DeltaTime;

		INC_DWORD_STAT(STAT_SpriteParticlesUpdated);
	}
	SCOPE_CYCLE_COUNTER(STAT_SpriteUpdateTime);

	// Grab the type data module if present
	UParticleModuleTypeDataBase* pkBase = 0;
	if (HighLODLevel->TypeDataModule)
	{
		pkBase = Cast<UParticleModuleTypeDataBase>(HighLODLevel->TypeDataModule);
		
		//@todo. Need to track TypeData offset into payload!
		pkBase->PreUpdate(this, TypeDataOffset, DeltaTime);
	}

	// Update existing particles (might respawn dying ones).
	for (INT ModuleIndex = 0; ModuleIndex < HighLODLevel->UpdateModules.Num(); ModuleIndex++)
	{
		if (!HighLODLevel->UpdateModules(ModuleIndex))
		{
			continue;
		}

		UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels(0);
		UParticleModule* OffsetModule	= 	HighestLODLevel->UpdateModules(ModuleIndex);
		UINT* Offset = ModuleOffsetMap.Find(OffsetModule);

		UParticleModule* HighModule	= HighLODLevel->UpdateModules(ModuleIndex);
		UParticleModule* LowModule	= LowLODLevel->UpdateModules(ModuleIndex);

		if (HighModule->bEnabled)
		{
			HighModule->UpdateEditor(this, Offset ? *Offset : 0, DeltaTime, LowModule, Multiplier);
		}
		else
		if (LowModule->bEnabled)
		{
			LowModule->UpdateEditor(this, Offset ? *Offset : 0, DeltaTime, HighModule, (1.0f - Multiplier));
		}
	}

	// Update the TypeData module
	if (pkBase)
	{
		//@todo. Need to track TypeData offset into payload!
		pkBase->Update(this, TypeDataOffset, DeltaTime);
		pkBase->PostUpdate(this, TypeDataOffset, DeltaTime);
	}

	// Calculate bounding box and simulate velocity.
	UpdateBoundingBox(DeltaTime);

	// Invalidate the contents of the vertex/index buffer.
	IsRenderDataDirty = 1;

	// 'Reset' the emitter time so that the delay functions correctly
	EmitterTime += EmitterDelay;
}

/**
 *	Rewind the instance.
 */
void FParticleEmitterInstance::Rewind()
{
	SecondsSinceCreation = 0;
	EmitterTime = 0;
	LoopCount = 0;
	ResetBurstList();
}

/**
 *	Retrieve the bounding box for the instance
 *
 *	@return	FBox	The bounding box
 */
FBox FParticleEmitterInstance::GetBoundingBox()
{ 
	return ParticleBoundingBox;
}

/**
 *	Update the bounding box for the emitter
 *
 *	@param	DeltaTime		The time slice to use
 */
void FParticleEmitterInstance::UpdateBoundingBox(FLOAT DeltaTime)
{
	if (Component)
	{
		// Take component scale into account
		FVector Scale = FVector(1.0f, 1.0f, 1.0f);
		Scale *= Component->Scale * Component->Scale3D;
		AActor* Actor = Component->GetOwner();
		if (Actor && !Component->AbsoluteScale)
		{
			Scale *= Actor->DrawScale * Actor->DrawScale3D;
		}

		FLOAT	MaxSizeScale	= 1.0f;
		FVector	NewLocation;
		FLOAT	NewRotation;
		ParticleBoundingBox.Init();
		// For each particle, offset the box appropriately 
		for (INT i=0; i<ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			
			// Do linear integrator and update bounding box
			// Do angular integrator, and wrap result to within +/- 2 PI
			Particle.OldLocation	= Particle.Location;
			if ((Particle.Flags & STATE_Particle_Freeze) == 0)
			{
				if ((Particle.Flags & STATE_Particle_FreezeTranslation) == 0)
				{
					NewLocation	= Particle.Location + (DeltaTime * Particle.Velocity);
				}
				else
				{
					NewLocation	= Particle.Location;
				}
				if ((Particle.Flags & STATE_Particle_FreezeRotation) == 0)
				{
					NewRotation = (DeltaTime * Particle.RotationRate) + Particle.Rotation;
				}
				else
				{
					NewRotation	= Particle.Rotation;
				}
			}
			else
			{
				NewLocation	= Particle.Location;
				NewRotation	= Particle.Rotation;
			}

			FVector Size = Particle.Size * Scale;
			MaxSizeScale			= Max(MaxSizeScale, Size.GetAbsMax()); //@todo particles: this does a whole lot of compares that can be avoided using SSE/ Altivec.

			Particle.Rotation	 = appFmod(NewRotation, 2.f*(FLOAT)PI);
			Particle.Location	 = NewLocation;
			ParticleBoundingBox += NewLocation;
		}
		ParticleBoundingBox = ParticleBoundingBox.ExpandBy(MaxSizeScale);

		// Transform bounding box into world space if the emitter uses a local space coordinate system.
		UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
		check(LODLevel);
		if (LODLevel->RequiredModule->bUseLocalSpace) 
		{
			ParticleBoundingBox = ParticleBoundingBox.TransformBy(Component->LocalToWorld);
		}
	}
}

/**
 *	Retrieved the per-particle bytes that this emitter type requires.
 *
 *	@return	UINT	The number of required bytes for particles in the instance
 */
UINT FParticleEmitterInstance::RequiredBytes()
{
	UINT uiBytes = 0;

	// This code assumes that the module stacks are identical across LOD levevls...
	UParticleLODLevel* LODLevel = SpriteTemplate->GetLODLevel(0);
	check(LODLevel);

	// Check for SubUV utilization, and update the required bytes accordingly
	EParticleSubUVInterpMethod	InterpolationMethod	= (EParticleSubUVInterpMethod)LODLevel->RequiredModule->InterpolationMethod;
	if (InterpolationMethod != PSUVIM_None)
	{
		SubUVDataOffset = PayloadOffset;
		if (LODLevel->TypeDataModule &&
			LODLevel->TypeDataModule->IsA(UParticleModuleTypeDataMesh::StaticClass()))
		{
			if (InterpolationMethod == PSUVIM_Random)
			{
				uiBytes	= sizeof(FSubUVMeshRandomPayload);
			}
			else
			{
				uiBytes	= sizeof(FSubUVMeshPayload);
			}
		}
		else
		{
			if (InterpolationMethod == PSUVIM_Random)
			{
				uiBytes	= sizeof(FSubUVSpriteRandomPayload);
			}
			else
			{
				uiBytes	= sizeof(FSubUVSpritePayload);
			}
		}
	}

	return uiBytes;
}

/**
 *	Get the pointer to the instance data allocated for a given module.
 *
 *	@param	Module		The module to retrieve the data block for.
 *	@return	BYTE*		The pointer to the data
 */
BYTE* FParticleEmitterInstance::GetModuleInstanceData(UParticleModule* Module)
{
	// If there is instance data present, look up the modules offset
	if (InstanceData)
	{
		UINT* Offset = ModuleInstanceOffsetMap.Find(Module);
		if (Offset)
		{
			if (*Offset < (UINT)InstancePayloadSize)
			{
				return &(InstanceData[*Offset]);
			}
		}
	}
	return NULL;
}

/**
 *	Calculate the stride of a single particle for this instance
 *
 *	@param	ParticleSize	The size of the particle
 *
 *	@return	UINT			The stride of the particle
 */
UINT FParticleEmitterInstance::CalculateParticleStride(UINT ParticleSize)
{
	return ParticleSize;
}

/**
 *	Reset the burst list information for the instance
 */
void FParticleEmitterInstance::ResetBurstList()
{
	if (SpriteTemplate != NULL)
	{
		// Reset the burst flags
		for (INT LODIndex = 0; LODIndex < SpriteTemplate->LODLevels.Num(); LODIndex++)
		{
			UParticleLODLevel* LODLevel = SpriteTemplate->LODLevels(LODIndex);
			if (LODLevel != NULL)
			{
				for (INT BurstIndex = 0; BurstIndex < LODLevel->RequiredModule->BurstList.Num(); BurstIndex++)
				{
					BurstFired(LODLevel->Level).BurstFired(BurstIndex) = FALSE;
				}
			}
		}
	}
}

/**
 *	Get the current burst rate offset (delta time is artifically increased to generate bursts)
 *
 *	@param	DeltaTime	The time slice (In/Out)
 *	@param	Burst		The number of particles to burst (Output)
 *
 *	@return	FLOAT		The time slice increase to use
 */
FLOAT FParticleEmitterInstance::GetCurrentBurstRateOffset(FLOAT& DeltaTime, INT& Burst)
{
	FLOAT SpawnRateInc = 0.0f;

	// Grab the current LOD level
	UParticleLODLevel* LODLevel	= SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	if (LODLevel->RequiredModule->BurstList.Num() > 0)
    {
		// For each burst in the list
        for (INT i = 0; i < LODLevel->RequiredModule->BurstList.Num(); i++)
        {
            FParticleBurst* BurstEntry = &(LODLevel->RequiredModule->BurstList(i));
			// If it hasn't been fired
			if (BurstFired(LODLevel->Level).BurstFired(i) == FALSE)
            {
				// If it is time to fire it
                if (EmitterTime >= BurstEntry->Time)
                {
					// Make sure there is a valid time slice
					if (DeltaTime < 0.00001f)
					{
						DeltaTime = 0.00001f;
					}
					// Calculate the increase time slice
					SpawnRateInc += BurstEntry->Count / DeltaTime;
					Burst += BurstEntry->Count;
					BurstFired(LODLevel->Level).BurstFired(i)	= TRUE;
				}
            }
        }
   }

	return SpawnRateInc;
}

/**
 *	Get the current burst rate offset (delta time is artifically increased to generate bursts)
 *	Editor version (for interpolation)
 *
 *	@param	HighLODLevel	The higher LOD level
 *	@param	LowLODLevel		The lower LOD level
 *	@param	Multiplier		The interpolation value to use
 *	@param	DeltaTime		The time slice (In/Out)
 *	@param	Burst			The number of particles to burst (Output)
 *
 *	@return	FLOAT			The time slice increase to use
 */
FLOAT FParticleEmitterInstance::GetCurrentBurstRateOffsetEditor(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, 
	FLOAT Multiplier, FLOAT& DeltaTime, INT& Burst)
{
	FLOAT SpawnRateInc = 0.0f;

	FLOAT HighSpawnRateInc	= 0.0f;
	FLOAT LowSpawnRateInc	= 0.0f;
	INT	HighBurstCount		= 0;
	INT	LowBurstCount		= 0;

	// Determine the High LOD burst information
	if (HighLODLevel->RequiredModule->BurstList.Num() > 0)
    {
        for (INT i = 0; i < HighLODLevel->RequiredModule->BurstList.Num(); i++)
        {
            FParticleBurst* pkBurst = &(HighLODLevel->RequiredModule->BurstList(i));
			if (BurstFired(HighLODLevel->Level).BurstFired(i) == FALSE)
            {
                if (EmitterTime >= pkBurst->Time)
                {
					if (DeltaTime < 0.00001f)
					{
						DeltaTime = 0.00001f;
					}
					HighSpawnRateInc += pkBurst->Count / DeltaTime;
					HighBurstCount	 += pkBurst->Count;
					BurstFired(HighLODLevel->Level).BurstFired(i) = TRUE;
				}
            }
        }
   }

	// Determine the Low LOD burst information
	if (LowLODLevel->RequiredModule->BurstList.Num() > 0)
    {
        for (INT i = 0; i < LowLODLevel->RequiredModule->BurstList.Num(); i++)
        {
            FParticleBurst* pkBurst = &(LowLODLevel->RequiredModule->BurstList(i));
			if (BurstFired(LowLODLevel->Level).BurstFired(i) == FALSE)
            {
                if (EmitterTime >= pkBurst->Time)
                {
					if (DeltaTime < 0.00001f)
					{
						DeltaTime = 0.00001f;
					}
					LowSpawnRateInc += pkBurst->Count / DeltaTime;
					LowBurstCount	 += pkBurst->Count;
					BurstFired(LowLODLevel->Level).BurstFired(i) = TRUE;
				}
            }
        }
   }

	// Interpolate between the two
	SpawnRateInc	+= (HighSpawnRateInc * Multiplier) + (LowSpawnRateInc	* (1.0f - Multiplier));
	Burst			+= appTrunc((HighBurstCount	 * Multiplier) + (LowBurstCount		* (1.0f - Multiplier)));

	return SpawnRateInc;
}

/**
 *	Spawn particles for this instance
 *
 *	@param	OldLeftover		The leftover time from the last spawn
 *	@param	Rate			The rate at which particles should be spawned
 *	@param	DeltaTime		The time slice to spawn over
 *	@param	Burst			The number of burst particle
 *	@param	BurstTime		The burst time addition (faked time slice)
 *
 *	@return	FLOAT			The leftover fraction of spawning
 */
FLOAT FParticleEmitterInstance::Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst, FLOAT BurstTime)
{
	// Ensure continous spawning... lots of fiddling.
	FLOAT	NewLeftover = OldLeftover + DeltaTime * Rate;
	INT		Number		= appFloor(NewLeftover);
	FLOAT	Increment	= 1.f / Rate;
	FLOAT	StartTime	= DeltaTime + OldLeftover * Increment - Increment;
	NewLeftover			= NewLeftover - Number;

	// If we have calculated less than the burst count, force the burst count
	if (Number < Burst)
	{
		Number = Burst;
	}

	// Take the burst time fakery into account
	if (BurstTime > 0.0f)
	{
		NewLeftover -= BurstTime / Burst;
		NewLeftover	= Clamp<FLOAT>(NewLeftover, 0, NewLeftover);
	}

	// Handle growing arrays.
	INT NewCount = ActiveParticles + Number;
	if (NewCount >= MaxActiveParticles)
	{
		if (DeltaTime < 0.25f)
		{
			Resize(NewCount + appTrunc(appSqrt((FLOAT)NewCount)) + 1);
		}
		else
		{
			Resize((NewCount + appTrunc(appSqrt((FLOAT)NewCount)) + 1), FALSE);
		}
	}

	FVector DeltaPosition = (OldLocation - Location);

	UParticleLODLevel* LODLevel	= SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	// Spawn particles.
	for (INT i=0; i<Number; i++)
	{
		DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[ActiveParticles]);

		FLOAT SpawnTime = StartTime - i * Increment;
	
		PreSpawn(Particle);

		if (LODLevel->RequiredModule->bUseLocalSpace == FALSE && LODLevel->RequiredModule->bInterpolatedSpawn == TRUE)
		{
			/// 현재가 Location, i == 0이면 OldLocation으로 가야하니...
			FLOAT s = (FLOAT)i / (Number-1);

			/// s == 0 --> -Location + OldLocation, 
			/// s == 1 --> 0

			Particle->Location -= s * DeltaPosition;			
		}

		if (LODLevel->TypeDataModule)
		{
			UParticleModuleTypeDataBase* pkBase = Cast<UParticleModuleTypeDataBase>(LODLevel->TypeDataModule);
			pkBase->Spawn(this, TypeDataOffset, SpawnTime);
		}

		for (INT ModuleIndex = 0; ModuleIndex < LODLevel->SpawnModules.Num(); ModuleIndex++)
		{
			UParticleModule* SpawnModule	= LODLevel->SpawnModules(ModuleIndex);

			UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels(0);
			UParticleModule* OffsetModule	= 	HighestLODLevel->SpawnModules(ModuleIndex);
			UINT* Offset = ModuleOffsetMap.Find(OffsetModule);
			
			if (SpawnModule->bEnabled)
			{
				SpawnModule->Spawn(this, Offset ? *Offset : 0, SpawnTime);
			}
		}
		PostSpawn(Particle, 1.f - FLOAT(i+1) / FLOAT(Number), SpawnTime);

		ActiveParticles++;

		INC_DWORD_STAT(STAT_SpriteParticlesSpawned);
	}
	return NewLeftover;
}

/**
 *	Spawn particles for this instance
 *	Editor version for interpolation
 *
 *	@param	HighLODLevel	The higher LOD level
 *	@param	LowLODLevel		The lower LOD level
 *	@param	Multiplier		The interpolation value to use
 *	@param	OldLeftover		The leftover time from the last spawn
 *	@param	Rate			The rate at which particles should be spawned
 *	@param	DeltaTime		The time slice to spawn over
 *	@param	Burst			The number of burst particle
 *	@param	BurstTime		The burst time addition (faked time slice)
 *
 *	@return	FLOAT			The leftover fraction of spawning
 */
FLOAT FParticleEmitterInstance::SpawnEditor(
	UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, FLOAT Multiplier, 
	FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst, FLOAT BurstTime)
{
	// Ensure continous spawning... lots of fiddling.
	FLOAT	NewLeftover = OldLeftover + DeltaTime * Rate;
	INT		Number		= appFloor(NewLeftover);
	FLOAT	Increment	= 1.f / Rate;
	FLOAT	StartTime	= DeltaTime + OldLeftover * Increment - Increment;
	NewLeftover			= NewLeftover - Number;

	if (Number < Burst)
	{
		Number = Burst;
	}

	if (BurstTime > 0.0f)
	{
		NewLeftover -= BurstTime / Burst;
		NewLeftover	= Clamp<FLOAT>(NewLeftover, 0, NewLeftover);
	}

	// Handle growing arrays.
	INT NewCount = ActiveParticles + Number;
	if (NewCount >= MaxActiveParticles)
	{
		if (DeltaTime < 0.25f)
		{
			Resize(NewCount + appTrunc(appSqrt((FLOAT)NewCount)) + 1);
		}
		else
		{
			Resize((NewCount + appTrunc(appSqrt((FLOAT)NewCount)) + 1), FALSE);
		}
	}

	// Spawn particles.
	for (INT i=0; i<Number; i++)
	{
		DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[ActiveParticles]);
		
		FLOAT SpawnTime = StartTime - i * Increment;
	
		PreSpawn(Particle);

		if (HighLODLevel->TypeDataModule)
		{
			check(LowLODLevel->TypeDataModule);

			UParticleModuleTypeDataBase* Base = Cast<UParticleModuleTypeDataBase>(HighLODLevel->TypeDataModule);
			UParticleModuleTypeDataBase* LowBase = Cast<UParticleModuleTypeDataBase>(LowLODLevel->TypeDataModule);
			
			Base->SpawnEditor(this, TypeDataOffset, SpawnTime, LowBase, Multiplier);
		}

		for (INT ModuleIndex = 0; ModuleIndex < HighLODLevel->SpawnModules.Num(); ModuleIndex++)
		{
			UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels(0);
			UParticleModule* OffsetModule	= 	HighestLODLevel->SpawnModules(ModuleIndex);
			UINT* Offset = ModuleOffsetMap.Find(OffsetModule);

			UParticleModule* HighModule	= HighLODLevel->SpawnModules(ModuleIndex);
			UParticleModule* LowModule	= LowLODLevel->SpawnModules(ModuleIndex);

			if (HighModule->bEnabled)
			{
				HighModule->SpawnEditor(this, Offset ? *Offset : 0, SpawnTime, LowModule, Multiplier);
			}
			else
			if (LowModule->bEnabled)
			{
				LowModule->SpawnEditor(this, Offset ? *Offset : 0, SpawnTime, HighModule, (1.0f - Multiplier));
			}
		}
		PostSpawn(Particle, 1.f - FLOAT(i+1) / FLOAT(Number), SpawnTime);

		ActiveParticles++;

		INC_DWORD_STAT(STAT_SpriteParticlesSpawned);
	}
	return NewLeftover;
}

/**
 *	Handle any pre-spawning actions required for particles
 *
 *	@param	Particle	The particle being spawned.
 */
void FParticleEmitterInstance::PreSpawn(FBaseParticle* Particle)
{
	// By default, just clear out the particle
	appMemzero(Particle, ParticleSize);
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	if (LODLevel->RequiredModule->bUseLocalSpace == FALSE)
	{
		// If not using local space, initialize the particle location
		Particle->Location = Location;
	}
}

/**
 *	Has the instance completed it's run?
 *
 *	@return	UBOOL	TRUE if the instance is completed, FALSE if not
 */
UBOOL FParticleEmitterInstance::HasCompleted()
{
	// Validity check
	if (SpriteTemplate == NULL)
	{
		return TRUE;
	}

	// If it hasn't finished looping or if it loops forever, not completed.
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	if ((LODLevel->RequiredModule->EmitterLoops == 0) || 
		(SecondsSinceCreation < (EmitterDuration * LODLevel->RequiredModule->EmitterLoops)))
	{
		return FALSE;
	}

	// If there are active particles, not completed
	if (ActiveParticles > 0)
	{
		return FALSE;
	}

	return TRUE;
}

/**
 *	Handle any post-spawning actions required by the instance
 *
 *	@param	Particle					The particle that was spawned
 *	@param	InterpolationPercentage		The percentage of the time slice it was spawned at
 *	@param	SpawnTIme					The time it was spawned at
 */
void FParticleEmitterInstance::PostSpawn(FBaseParticle* Particle, FLOAT InterpolationPercentage, FLOAT SpawnTime)
{
	// Interpolate position if using world space.
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	check(LODLevel);
	if (LODLevel->RequiredModule->bUseLocalSpace == FALSE)
	{
		if (FDistSquared(OldLocation, Location) > 1.f)
		{
			Particle->Location += InterpolationPercentage * (OldLocation - Location);	
		}
	}

	// Offset caused by any velocity
	Particle->OldLocation = Particle->Location;
	Particle->Location   += SpawnTime * Particle->Velocity;
}

/**
 *	Kill off any dead particles. (Remove them from the active array)
 */
void FParticleEmitterInstance::KillParticles()
{
	// Loop over the active particles... If their RelativeTime is > 1.0f (indicating they are dead),
	// move them to the 'end' of the active particle list.
	for (INT i=ActiveParticles-1; i>=0; i--)
	{
		const INT	CurrentIndex	= ParticleIndices[i];
		const BYTE* ParticleBase	= ParticleData + CurrentIndex * ParticleStride;
		FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);
		if (Particle.RelativeTime > 1.0f)
		{
			// Move it to the 'back' of the list
			ParticleIndices[i]	= ParticleIndices[ActiveParticles-1];
			ParticleIndices[ActiveParticles-1]	= CurrentIndex;
			ActiveParticles--;

			INC_DWORD_STAT(STAT_SpriteParticlesKilled);
		}
	}
}

/**
 *	This is used to force "kill" particles irrespective of their duration.
 *	Basically, this takes all particles and moves them to the 'end' of the 
 *	particle list so we can insta kill off trailed particles in the level.
 *
 *	NOTE: we should probably add a boolean to the particle trail type such
 *	that this is Data Driven.  For E3 we are using this.
 */
void FParticleEmitterInstance::KillParticlesForced()
{
	// Loop over the active particles... If their RelativeTime is > 1.0f (indicating they are dead),
	// move them to the 'end' of the active particle list.
	for (INT i=ActiveParticles-1; i>=0; i--)
	{
		const INT	CurrentIndex	= ParticleIndices[i];
		ParticleIndices[i]	= ParticleIndices[ActiveParticles-1];
		ParticleIndices[ActiveParticles-1]	= CurrentIndex;
		ActiveParticles--;

		INC_DWORD_STAT(STAT_SpriteParticlesKilled);
	}
}

/**
 *	Called when the instance if removed from the scene
 *	Perform any actions required, such as removing components, etc.
 */
void FParticleEmitterInstance::RemovedFromScene()
{
}

/**
 *	Retrieve the particle at the given index
 *
 *	@param	Index			The index of the particle of interest
 *
 *	@return	FBaseParticle*	The pointer to the particle. NULL if not present/active
 */
FBaseParticle* FParticleEmitterInstance::GetParticle(INT Index)
{
	// See if the index is valid. If not, return NULL
	if ((Index >= ActiveParticles) || (Index < 0))
	{
		return NULL;
	}

	// Grab and return the particle
	DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[Index]);
	return Particle;
}

/**
 *	Calculates the emitter duration for the instance.
 */
void FParticleEmitterInstance::SetupEmitterDuration()
{
	// Validity check
	if (SpriteTemplate == NULL)
	{
		return;
	}

	// Set up the array for each LOD level
	if (EmitterDurations.Num() != SpriteTemplate->LODLevels.Num())
	{
		EmitterDurations.Empty();
		EmitterDurations.Insert(0, SpriteTemplate->LODLevels.Num());
	}

	// Calculate the duration for each LOD level
	for (INT LODIndex = 0; LODIndex < SpriteTemplate->LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* TempLOD = SpriteTemplate->LODLevels(LODIndex);
		if (TempLOD->RequiredModule->bEmitterDurationUseRange)
		{
			FLOAT	Rand		= appFrand();
			FLOAT	Duration	= TempLOD->RequiredModule->EmitterDurationLow + 
				((TempLOD->RequiredModule->EmitterDuration - TempLOD->RequiredModule->EmitterDurationLow) * Rand);
			EmitterDurations(TempLOD->Level) = Duration + TempLOD->RequiredModule->EmitterDelay;
		}
		else
		{
			EmitterDurations(TempLOD->Level) = TempLOD->RequiredModule->EmitterDuration + TempLOD->RequiredModule->EmitterDelay;
		}

		if ((LoopCount == 1) && (TempLOD->RequiredModule->bDelayFirstLoopOnly == TRUE) && 
			((TempLOD->RequiredModule->EmitterLoops == 0) || (TempLOD->RequiredModule->EmitterLoops > 1)))
		{
			EmitterDurations(TempLOD->Level) -= TempLOD->RequiredModule->EmitterDelay;
		}
	}

	// Set the current duration
	EmitterDuration	= EmitterDurations(CurrentLODLevelIndex);
}

/*-----------------------------------------------------------------------------
	ParticleSpriteEmitterInstance
-----------------------------------------------------------------------------*/
/**
 *	ParticleSpriteEmitterInstance
 *	The structure for a standard sprite emitter instance.
 */
/** Constructor	*/
FParticleSpriteEmitterInstance::FParticleSpriteEmitterInstance() :
	FParticleEmitterInstance()
{
}

/** Destructor	*/
FParticleSpriteEmitterInstance::~FParticleSpriteEmitterInstance()
{
}

/**
 *	Retrieves the dynamic data for the emitter
 *	
 *	@param	bSelected					Whether the emitter is selected in the editor
 *
 *	@return	FDynamicEmitterDataBase*	The dynamic data, or NULL if it shouldn't be rendered
 */
FDynamicEmitterDataBase* FParticleSpriteEmitterInstance::GetDynamicData(UBOOL bSelected)
{
	// Make sure there is a template present
	if (!SpriteTemplate)
	{
		return NULL;
	}

	// If the template is disabled, don't return data.
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) || (LODLevel->RequiredModule->bEnabled == FALSE))
	{
		return NULL;
	}

	// Allocate it for now, but we will want to change this to do some form
	// of caching
	if (ActiveParticles > 0)
	{
		// Make sure we will not be allocating enough memory
		// Also clears TTP #33373
		check(MaxActiveParticles >= ActiveParticles);

		// Get the material instance. If there is none, or the material isn't flagged for use with particle systems, use the DefaultMaterial.
		UMaterialInstance* MaterialInst = SpriteTemplate->Material;
		if (MaterialInst == NULL || !MaterialInst->UseWithParticleSystem())
		{
			MaterialInst = GEngine->DefaultMaterial;
		}

		// Allocate the dynamic data
		FDynamicSpriteEmitterData* NewEmitterData = ::new FDynamicSpriteEmitterData(MaxActiveParticles, ParticleStride, 
			MaterialInst->GetInstanceInterface(bSelected),
			(LODLevel->RequiredModule->bUseMaxDrawCount == TRUE) ? LODLevel->RequiredModule->MaxDrawCount : -1
			);
		check(NewEmitterData);
		check(NewEmitterData->ParticleData);
		check(NewEmitterData->ParticleIndices);

		// Fill it in...
		INT		ParticleCount	= ActiveParticles;
		UBOOL	bSorted			= FALSE;
		INT		ScreenAlignment	= SpriteTemplate->ScreenAlignment;

		// Take scale into account
		NewEmitterData->Scale = FVector(1.0f, 1.0f, 1.0f);
		if (Component)
		{
			NewEmitterData->Scale *= Component->Scale * Component->Scale3D;
			AActor* Actor = Component->GetOwner();
			if (Actor && !Component->AbsoluteScale)
			{
				NewEmitterData->Scale *= Actor->DrawScale * Actor->DrawScale3D;
			}
		}

		NewEmitterData->ActiveParticleCount = ActiveParticles;
		NewEmitterData->ParticleCount = ActiveParticles;
		NewEmitterData->bSelected = bSelected;
		NewEmitterData->VertexCount = ActiveParticles * 4;
		NewEmitterData->IndexCount = ActiveParticles * 6;
		NewEmitterData->IndexStride = sizeof(WORD);
		NewEmitterData->ScreenAlignment = SpriteTemplate->ScreenAlignment;
		NewEmitterData->bUseLocalSpace = LODLevel->RequiredModule->bUseLocalSpace;
		NewEmitterData->EmitterRenderMode = LODLevel->RequiredModule->EmitterRenderMode;
		NewEmitterData->bLockAxis = FALSE;
		if (Module_AxisLock && (Module_AxisLock->bEnabled == TRUE))
		{
			if (Module_AxisLock->LockAxisFlags != EPAL_NONE)
			{
				NewEmitterData->LockAxisFlag = Module_AxisLock->LockAxisFlags;
				NewEmitterData->bLockAxis = TRUE;
			}
		}

		check(NewEmitterData->ParticleData);
		appMemcpy(NewEmitterData->ParticleData, ParticleData, MaxActiveParticles * ParticleStride);
		check(NewEmitterData->ParticleIndices);
		appMemcpy(NewEmitterData->ParticleIndices, ParticleIndices, MaxActiveParticles * sizeof(WORD));

		return NewEmitterData;
	}

	return NULL;
}

/*-----------------------------------------------------------------------------
	ParticleSpriteSubUVEmitterInstance
-----------------------------------------------------------------------------*/
/**
 *	ParticleSpriteSubUVEmitterInstance
 *	Structure for SubUV sprite instances
 */
/** Constructor	*/
FParticleSpriteSubUVEmitterInstance::FParticleSpriteSubUVEmitterInstance() :
	FParticleEmitterInstance()
{
}

/** Destructor	*/
FParticleSpriteSubUVEmitterInstance::~FParticleSpriteSubUVEmitterInstance()
{
}

/**
 *	Kill off any dead particles. (Remove them from the active array)
 */
void FParticleSpriteSubUVEmitterInstance::KillParticles()
{
	// Loop over the active particles... If their RelativeTime is > 1.0f (indicating they are dead),
	// move them to the 'end' of the active particle list.
	for (INT i=ActiveParticles-1; i>=0; i--)
	{
		const INT	CurrentIndex	= ParticleIndices[i];
		const BYTE* ParticleBase	= ParticleData + CurrentIndex * ParticleStride;
		FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);
		if (Particle.RelativeTime > 1.0f)
		{
            FLOAT* pkFloats = (FLOAT*)((BYTE*)&Particle + PayloadOffset);
			pkFloats[0] = 0.0f;
			pkFloats[1] = 0.0f;
			pkFloats[2] = 0.0f;
			pkFloats[3] = 0.0f;
			pkFloats[4] = 0.0f;

			ParticleIndices[i]	= ParticleIndices[ActiveParticles-1];
			ParticleIndices[ActiveParticles-1]	= CurrentIndex;
			ActiveParticles--;
		}
	}
}

/**
 *	Retrieves the dynamic data for the emitter
 *	
 *	@param	bSelected					Whether the emitter is selected in the editor
 *
 *	@return	FDynamicEmitterDataBase*	The dynamic data, or NULL if it shouldn't be rendered
 */
FDynamicEmitterDataBase* FParticleSpriteSubUVEmitterInstance::GetDynamicData(UBOOL bSelected)
{
	// Make sure the template is valid
	if (!SpriteTemplate)
	{
		return NULL;
	}

	// If the template is disabled, don't return data.
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) ||
		(LODLevel->RequiredModule->bEnabled == FALSE))
	{
		return NULL;
	}

	// Allocate it for now, but we will want to change this to do some form
	// of caching
	if (ActiveParticles > 0)
	{
		// Get the material instance. If there is none, or the material isn't flagged for use with particle systems, use the DefaultMaterial.
		UMaterialInstance* MaterialInst = SpriteTemplate->Material;
		if (MaterialInst == NULL || !MaterialInst->UseWithParticleSystem())
		{
			MaterialInst = GEngine->DefaultMaterial;
		}

		// Allocate the dynamic data for the instance
		FDynamicSubUVEmitterData* NewEmitterData = ::new FDynamicSubUVEmitterData(MaxActiveParticles, ParticleStride, 
			MaterialInst->GetInstanceInterface(bSelected),
			(LODLevel->RequiredModule->bUseMaxDrawCount == TRUE) ? LODLevel->RequiredModule->MaxDrawCount : -1
			);
		check(NewEmitterData);
		check(NewEmitterData->ParticleData);
		check(NewEmitterData->ParticleIndices);

		// Fill it in...
		NewEmitterData->ActiveParticleCount = ActiveParticles;
		NewEmitterData->bSelected = bSelected;
		NewEmitterData->ScreenAlignment = SpriteTemplate->ScreenAlignment;
		NewEmitterData->bUseLocalSpace = LODLevel->RequiredModule->bUseLocalSpace;
		NewEmitterData->SubUVDataOffset = SubUVDataOffset;
		NewEmitterData->SubImages_Horizontal = LODLevel->RequiredModule->SubImages_Horizontal;
		NewEmitterData->SubImages_Vertical = LODLevel->RequiredModule->SubImages_Vertical;
		NewEmitterData->bDirectUV = LODLevel->RequiredModule->bDirectUV;
		NewEmitterData->EmitterRenderMode = LODLevel->RequiredModule->EmitterRenderMode;

		INT		ParticleCount	= ActiveParticles;
		UBOOL	bSorted			= FALSE;
		INT		ScreenAlignment	= SpriteTemplate->ScreenAlignment;

		// Take scale into account
		NewEmitterData->Scale = FVector(1.0f, 1.0f, 1.0f);
		if (Component)
		{
			NewEmitterData->Scale *= Component->Scale * Component->Scale3D;
			AActor* Actor = Component->GetOwner();
			if (Actor && !Component->AbsoluteScale)
			{
				NewEmitterData->Scale *= Actor->DrawScale * Actor->DrawScale3D;
			}
		}

		NewEmitterData->VertexCount = ActiveParticles * 4;
		NewEmitterData->IndexCount = ActiveParticles * 6;
		NewEmitterData->IndexStride = sizeof(WORD);

		NewEmitterData->bLockAxis = FALSE;
		if (Module_AxisLock && (Module_AxisLock->bEnabled == TRUE))
		{
			if (Module_AxisLock->LockAxisFlags != EPAL_NONE)
			{
				NewEmitterData->LockAxisFlag = Module_AxisLock->LockAxisFlags;
				NewEmitterData->bLockAxis	= TRUE;
			}
		}

		appMemcpy(NewEmitterData->ParticleData, ParticleData, MaxActiveParticles * ParticleStride);
		appMemcpy(NewEmitterData->ParticleIndices, ParticleIndices, MaxActiveParticles * sizeof(WORD));

		return NewEmitterData;
	}

	return NULL;
}

/*-----------------------------------------------------------------------------
	ParticleMeshEmitterInstance
-----------------------------------------------------------------------------*/
/**
 *	Structure for mesh emitter instances
 */

/** Constructor	*/
FParticleMeshEmitterInstance::FParticleMeshEmitterInstance() :
	  FParticleEmitterInstance()
	, MeshTypeData(NULL)
	, MeshComponentIndex(-1)
	, MeshRotationActive(FALSE)
	, MeshRotationOffset(0)
{
}

/** Destructor	*/
FParticleMeshEmitterInstance::~FParticleMeshEmitterInstance()
{
	if (Component && (MeshComponentIndex >= 0) && (MeshComponentIndex < Component->SMComponents.Num()))
	{
		Component->SMComponents(MeshComponentIndex) = NULL;
		MeshComponentIndex = -1;
	}
}

/**
 *	Initialize the parameters for the structure
 *
 *	@param	InTemplate		The ParticleEmitter to base the instance on
 *	@param	InComponent		The owning ParticleComponent
 *	@param	bClearResources	If TRUE, clear all resource data
 */
void FParticleMeshEmitterInstance::InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources)
{
	FParticleEmitterInstance::InitParameters(InTemplate, InComponent, bClearResources);

	// Get the type data module
	UParticleLODLevel* LODLevel	= InTemplate->GetLODLevel(0);
	check(LODLevel);
	MeshTypeData = CastChecked<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
	check(MeshTypeData);

    // Grab the MeshRotationRate module offset, if there is one...
    MeshRotationActive = FALSE;
	if (SpriteTemplate->ScreenAlignment == PSA_Velocity)
	{
		MeshRotationActive = TRUE;
	}
	else
	{
	    for (INT i = 0; i < LODLevel->Modules.Num(); i++)
	    {
	        if (LODLevel->Modules(i)->IsA(UParticleModuleMeshRotationRate::StaticClass())	||
	            LODLevel->Modules(i)->IsA(UParticleModuleMeshRotation::StaticClass())		||
				LODLevel->Modules(i)->IsA(UParticleModuleUberRainImpacts::StaticClass())	||
				LODLevel->Modules(i)->IsA(UParticleModuleUberRainSplashA::StaticClass())
				)
	        {
	            MeshRotationActive = TRUE;
	            break;
	        }
		}
    }
}

/**
 *	Initialize the instance
 */
void FParticleMeshEmitterInstance::Init()
{
	FParticleEmitterInstance::Init();

	// If there is a mesh present (there should be!)
	if (MeshTypeData->Mesh)
	{
		UStaticMeshComponent* MeshComponent = NULL;

		// If the index is set, try to retrieve it from the component
		if (MeshComponentIndex != -1)
		{
			if (MeshComponentIndex < Component->SMComponents.Num())
			{
				MeshComponent = Component->SMComponents(MeshComponentIndex);
			}
			// If it wasn't retrieved, force it to get recreated
			if (MeshComponent == NULL)
			{
				MeshComponentIndex = -1;
			}
		}

		if (MeshComponentIndex == -1)
		{
			// create the component if necessary
			MeshComponent = ConstructObject<UStaticMeshComponent>(UStaticMeshComponent::StaticClass(),Component);
			MeshComponent->bAcceptsDecals		= FALSE;
			MeshComponent->CollideActors		= FALSE;
			MeshComponent->BlockActors			= FALSE;
			MeshComponent->BlockZeroExtent		= FALSE;
			MeshComponent->BlockNonZeroExtent	= FALSE;
			MeshComponent->BlockRigidBody		= FALSE;
			// allocate space for material instance constants
			MeshComponent->Materials.InsertZeroed(0, MeshTypeData->Mesh->LODModels(0).Elements.Num());
			MeshComponent->StaticMesh		= MeshTypeData->Mesh;
			MeshComponent->CastShadow		= MeshTypeData->CastShadows;
			MeshComponent->bAcceptsLights	= Component->bAcceptsLights;

			for (INT SlotIndex = 0; SlotIndex < Component->SMComponents.Num(); SlotIndex++)
			{
				if (Component->SMComponents(SlotIndex) == NULL)
				{
					MeshComponentIndex = SlotIndex;
					Component->SMComponents(SlotIndex) = MeshComponent;
				}
			}
			if (MeshComponentIndex == -1)
			{
				MeshComponentIndex = Component->SMComponents.AddItem(MeshComponent);
			}		
		}
		check(MeshComponent);

		// Constructing MaterialInstanceConstant for each mesh instance is done so that
		// particle 'vertex color' can be set on each individual mesh.
		// They are tagged as transient so they don't get saved in the package.
		for (INT MatIndex = 0; MatIndex < MeshTypeData->Mesh->LODModels(0).Elements.Num(); MatIndex++)
		{
			FStaticMeshElement* MeshElement = &(MeshTypeData->Mesh->LODModels(0).Elements(MatIndex));
			UMaterialInstance* Parent = MeshElement->Material;
			if (Parent)
			{
				UMaterial* CheckMat = Parent->GetMaterial();
				if (CheckMat)
				{
					// during post load, the item will be tagged as Lit if the material applied to the sprite temple is (LightingModel != MLM_Unlit). This means the Attach/Detach light calls will occur on it.
					// In the case of mesh emitters, the actual material utilized is the one applied to the mesh itself. This check just warns that the SpriteTemplate material does not match the mesh material. This can lead to improper lighting - ie the mesh may not need lighting according to it's material, but the sprite material thinks it does need it, and vice-versa.
					if (Component->bAcceptsLights != (CheckMat->LightingModel != MLM_Unlit))
					{
						UBOOL bLitMaterials = FALSE;
						// Make sure no other emitter requires listing
						for (INT CheckEmitterIndex = 0; CheckEmitterIndex < Component->Template->Emitters.Num(); CheckEmitterIndex++)
						{
							UParticleEmitter* CheckEmitter = Component->Template->Emitters(CheckEmitterIndex);
							if (CheckEmitter != SpriteTemplate)
							{
								UParticleSpriteEmitter* SpriteEmitter = Cast<UParticleSpriteEmitter>(CheckEmitter);
								if (SpriteEmitter)
								{
									UParticleLODLevel* LODLevel	= SpriteEmitter->GetLODLevel(0);
									if (LODLevel && LODLevel->RequiredModule && LODLevel->bEnabled)
									{
										if (SpriteEmitter->Material && SpriteEmitter->Material->GetMaterial()->LightingModel != MLM_Unlit)
										{
											bLitMaterials	= TRUE;
											break;
										}
									}
								}
							}
						}

						if (bLitMaterials != Component->bAcceptsLights)
						{
							INT ReportIndex = -1;
							for (INT EmitterIndex = 0; EmitterIndex < Component->Template->Emitters.Num(); EmitterIndex++)
							{
								UParticleEmitter* Emitter = Component->Template->Emitters(EmitterIndex);
								if (Emitter)
								{
									if (Emitter == SpriteTemplate)
									{
										ReportIndex = EmitterIndex;
										break;
									}
								}
							}
#if defined(_MESH_PARTICLE_EMITTER_LIT_MISMATCH_)
							warnf(TEXT("MeshEmitter - lighting mode mismatch!\n\t\tTo correct this problem, set the Emitter->Material to the same thing as the material applied to the static mesh being used in the mesh emitter.\n\tMaterial: %s\n\tEmitter:  %s\n\tPSys:     %s (#%d)\n\t\t\t(NOTE: It may be anoter emitter in the particle system...)"), 
							*(CheckMat->GetFullName()), *GetFullName(),
							Component ? (Component->Template? *Component->Template->GetName() : TEXT("No Template")) : TEXT("No Component!"), ReportIndex);
#endif	//#if defined(_MESH_PARTICLE_EMITTER_LIT_MISMATCH_)
						}
					}
				}

				UMaterialInstanceConstant* MatInst = NULL;
				if (MeshComponent->Materials.Num() > MatIndex)
				{
					MatInst = Cast<UMaterialInstanceConstant>(MeshComponent->Materials(MatIndex));
				}
				if (MatInst == NULL)
				{
					// create the instance constant if necessary
					MatInst = ConstructObject<UMaterialInstanceConstant>(UMaterialInstanceConstant::StaticClass(), MeshComponent);
					if (MeshComponent->Materials.Num() > MatIndex)
					{
						MeshComponent->Materials(MatIndex) = MatInst;
					}
					else
					{
						INT CheckIndex = MeshComponent->Materials.AddItem(MatInst);
						check(CheckIndex == MatIndex);
					}
				}
				MatInst->SetParent(Parent);
				MatInst->SetFlags(RF_Transient);
			}
		}
	}
}

/**
 *	Resize the particle data array
 *
 *	@param	NewMaxActiveParticles	The new size to use
 */
void FParticleMeshEmitterInstance::Resize(INT NewMaxActiveParticles, UBOOL bSetMaxActiveCount)
{
	INT OldMaxActiveParticles = MaxActiveParticles;
	FParticleEmitterInstance::Resize(NewMaxActiveParticles, bSetMaxActiveCount);

	if (MeshRotationActive)
    {
        for (INT i = OldMaxActiveParticles; i < NewMaxActiveParticles; i++)
	    {
		    DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
            FMeshRotationPayloadData* PayloadData	= (FMeshRotationPayloadData*)((BYTE*)&Particle + MeshRotationOffset);
			PayloadData->RotationRateBase			= FVector(0.0f);
	    }
    }
}

/**
 *	Tick the instance.
 *
 *	@param	DeltaTime			The time slice to use
 *	@param	bSuppressSpawning	If TRUE, do not spawn during Tick
 */
void FParticleMeshEmitterInstance::Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
	// See if we are handling mesh rotation
    if (MeshRotationActive)
    {
		// Update the rotation for each particle
        for (INT i = 0; i < ActiveParticles; i++)
	    {
		    DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
            FMeshRotationPayloadData* PayloadData	= (FMeshRotationPayloadData*)((BYTE*)&Particle + MeshRotationOffset);
			PayloadData->RotationRate				= PayloadData->RotationRateBase;
			if (SpriteTemplate->ScreenAlignment == PSA_Velocity)
			{
				// Determine the rotation to the velocity vector and apply it to the mesh
				FVector	NewDirection	= Particle.Velocity;
				NewDirection.Normalize();
				FVector	OldDirection(1.0f, 0.0f, 0.0f);

				FQuat Rotation	= FQuatFindBetween(OldDirection, NewDirection);
				FVector Euler	= Rotation.Euler();
				PayloadData->Rotation.X	= Euler.X;
				PayloadData->Rotation.Y	= Euler.Y;
				PayloadData->Rotation.Z	= Euler.Z;
			}
	    }
    }

	// Call the standard tick
	FParticleEmitterInstance::Tick(DeltaTime, bSuppressSpawning);

	// Apply rotation if it is active
    if (MeshRotationActive)
    {
        for (INT i = 0; i < ActiveParticles; i++)
	    {
		    DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			if ((Particle.Flags & STATE_Particle_FreezeRotation) == 0)
			{
	            FMeshRotationPayloadData* PayloadData	 = (FMeshRotationPayloadData*)((BYTE*)&Particle + MeshRotationOffset);
				PayloadData->Rotation					+= DeltaTime * PayloadData->RotationRate;
			}
        }
    }

	// Do we need to tick the mesh instances or will the engine do it?
	if ((ActiveParticles == 0) & bSuppressSpawning)
	{
		RemovedFromScene();
	}
}

/**
 *	Tick the instance in the editor.
 *	This function will interpolate between the current LODLevels to allow for
 *	the designer to visualize how the selected LOD setting would look.
 *
 *	@param	HighLODLevel		The higher LOD level selected
 *	@param	LowLODLevel			The lower LOD level selected
 *	@param	Multiplier			The interpolation value to use between the two
 *	@param	DeltaTime			The time slice to use
 *	@param	bSuppressSpawning	If TRUE, do not spawn during Tick
 */
void FParticleMeshEmitterInstance::TickEditor(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, FLOAT Multiplier, FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
    if (MeshRotationActive)
    {
        for (INT i = 0; i < ActiveParticles; i++)
	    {
		    DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
            FMeshRotationPayloadData* PayloadData	= (FMeshRotationPayloadData*)((BYTE*)&Particle + MeshRotationOffset);
			PayloadData->RotationRate				= PayloadData->RotationRateBase;
			if (SpriteTemplate->ScreenAlignment == PSA_Velocity)
			{
				// Determine the rotation to the velocity vector and apply it to the mesh
				FVector	NewDirection	= Particle.Velocity;
				NewDirection.Normalize();
				FVector	OldDirection(1.0f, 0.0f, 0.0f);

				FQuat Rotation	= FQuatFindBetween(OldDirection, NewDirection);
				FVector Euler	= Rotation.Euler();
				PayloadData->Rotation.X	= Euler.X;
				PayloadData->Rotation.Y	= Euler.Y;
				PayloadData->Rotation.Z	= Euler.Z;
			}
	    }
    }

	// 
	FParticleEmitterInstance::TickEditor(HighLODLevel, LowLODLevel, Multiplier, DeltaTime, bSuppressSpawning);

    if (MeshRotationActive)
    {
        for (INT i = 0; i < ActiveParticles; i++)
	    {
		    DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			if ((Particle.Flags & STATE_Particle_FreezeRotation) == 0)
			{
	            FMeshRotationPayloadData* PayloadData	 = (FMeshRotationPayloadData*)((BYTE*)&Particle + MeshRotationOffset);
				PayloadData->Rotation					+= DeltaTime * PayloadData->RotationRate;
			}
        }
    }

	// Do we need to tick the mesh instances or will the engine do it?
	if ((ActiveParticles == 0) & bSuppressSpawning)
	{
		RemovedFromScene();
	}
}

/**
 *	Update the bounding box for the emitter
 *
 *	@param	DeltaTime		The time slice to use
 */
void FParticleMeshEmitterInstance::UpdateBoundingBox(FLOAT DeltaTime)
{
	//@todo. Implement proper bound determination for mesh emitters.
	// Currently, just 'forcing' the mesh size to be taken into account.
	if (Component)
	{
		// Take scale into account
		FVector Scale = FVector(1.0f, 1.0f, 1.0f);
		Scale *= Component->Scale * Component->Scale3D;
		AActor* Actor = Component->GetOwner();
		if (Actor && !Component->AbsoluteScale)
		{
			Scale *= Actor->DrawScale * Actor->DrawScale3D;
		}

		// Get the static mesh bounds
		FBoxSphereBounds MeshBound = MeshTypeData->Mesh->Bounds;

		FLOAT MaxSizeScale	= 1.0f;
		FVector	NewLocation;
		FLOAT	NewRotation;
		ParticleBoundingBox.Init();
		for (INT i=0; i<ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			
			// Do linear integrator and update bounding box
			Particle.OldLocation = Particle.Location;
			if ((Particle.Flags & STATE_Particle_Freeze) == 0)
			{
				if ((Particle.Flags & STATE_Particle_FreezeTranslation) == 0)
				{
					NewLocation	= Particle.Location + DeltaTime * Particle.Velocity;
				}
				else
				{
					NewLocation = Particle.Location;
				}
				if ((Particle.Flags & STATE_Particle_FreezeRotation) == 0)
				{
					NewRotation	= Particle.Rotation + DeltaTime * Particle.RotationRate;
				}
				else
				{
					NewRotation = Particle.Rotation;
				}
			}
			else
			{
				// Don't move it...
				NewLocation = Particle.Location;
				NewRotation = Particle.Rotation;
			}

			// Do angular integrator, and wrap result to within +/- 2 PI
			FVector Size = Particle.Size * Scale;
			MaxSizeScale = Max(MaxSizeScale, Size.GetAbsMax()); //@todo particles: this does a whole lot of compares that can be avoided using SSE/ Altivec.

			Particle.Rotation = appFmod(NewRotation, 2.f*(FLOAT)PI);
			Particle.Location = NewLocation;
			ParticleBoundingBox += (Particle.Location + MeshBound.SphereRadius * Size);
			ParticleBoundingBox += (Particle.Location - MeshBound.SphereRadius * Size);
		}
		ParticleBoundingBox = ParticleBoundingBox.ExpandBy(MaxSizeScale);

		// Transform bounding box into world space if the emitter uses a local space coordinate system.
		UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
		check(LODLevel);
		if (LODLevel->RequiredModule->bUseLocalSpace) 
		{
			ParticleBoundingBox = ParticleBoundingBox.TransformBy(Component->LocalToWorld);
		}
	}
}

/**
 *	Retrieved the per-particle bytes that this emitter type requires.
 *
 *	@return	UINT	The number of required bytes for particles in the instance
 */
UINT FParticleMeshEmitterInstance::RequiredBytes()
{
	UINT uiBytes = FParticleEmitterInstance::RequiredBytes();
	MeshRotationOffset	= PayloadOffset + uiBytes;
	uiBytes += sizeof(FMeshRotationPayloadData);
	return uiBytes;
}

/**
 *	Handle any post-spawning actions required by the instance
 *
 *	@param	Particle					The particle that was spawned
 *	@param	InterpolationPercentage		The percentage of the time slice it was spawned at
 *	@param	SpawnTIme					The time it was spawned at
 */
void FParticleMeshEmitterInstance::PostSpawn(FBaseParticle* Particle, FLOAT InterpolationPercentage, FLOAT SpawnTime)
{
	FParticleEmitterInstance::PostSpawn(Particle, InterpolationPercentage, SpawnTime);
	if (SpriteTemplate->ScreenAlignment == PSA_Velocity)
	{
		// Determine the rotation to the velocity vector and apply it to the mesh
		FVector	NewDirection	= Particle->Velocity;
		NewDirection.Normalize();
		FVector	OldDirection(1.0f, 0.0f, 0.0f);

		FQuat Rotation	= FQuatFindBetween(OldDirection, NewDirection);
		FVector Euler	= Rotation.Euler();

		FMeshRotationPayloadData* PayloadData	= (FMeshRotationPayloadData*)((BYTE*)Particle + MeshRotationOffset);
		PayloadData->Rotation.X	+= Euler.X;
		PayloadData->Rotation.Y	+= Euler.Y;
		PayloadData->Rotation.Z	+= Euler.Z;
		//
	}
}

/**
 *	Retrieves the dynamic data for the emitter
 *	
 *	@param	bSelected					Whether the emitter is selected in the editor
 *
 *	@return	FDynamicEmitterDataBase*	The dynamic data, or NULL if it shouldn't be rendered
 */
FDynamicEmitterDataBase* FParticleMeshEmitterInstance::GetDynamicData(UBOOL bSelected)
{
	// Make sure the template is valid
	if (!SpriteTemplate)
	{
		return NULL;
	}

	// If the template is disabled, don't return data.
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) ||
		(LODLevel->RequiredModule->bEnabled == FALSE))
	{
		return NULL;
	}

	if ((MeshComponentIndex == -1) || (MeshComponentIndex >= Component->SMComponents.Num()))
	{
		// Not initialized?
		return NULL;
	}

	UStaticMeshComponent* MeshComponent = Component->SMComponents(MeshComponentIndex);
	if (MeshComponent == NULL)
	{
		// The mesh component has been GC'd?
		return NULL;
	}

	// Allocate it for now, but we will want to change this to do some form
	// of caching
	if (ActiveParticles > 0)
	{
		// Get the material isntance. If none is present, use the DefaultMaterial
		UMaterialInstance* MaterialInst = SpriteTemplate->Material;
		if (MaterialInst == NULL)
		{
			MaterialInst = GEngine->DefaultMaterial;
		}

		// Allocate the dynamic data
		FDynamicMeshEmitterData* NewEmitterData = ::new FDynamicMeshEmitterData(MaxActiveParticles, ParticleStride, 
			MaterialInst->GetInstanceInterface(bSelected), 
			LODLevel->RequiredModule->InterpolationMethod, 
			SubUVDataOffset, MeshTypeData->Mesh, MeshComponent,
			(LODLevel->RequiredModule->bUseMaxDrawCount == TRUE) ? LODLevel->RequiredModule->MaxDrawCount : -1
			);
		check(NewEmitterData);
		check(NewEmitterData->ParticleData);
		check(NewEmitterData->ParticleIndices);

		// Take scale into account
		NewEmitterData->Scale = FVector(1.0f, 1.0f, 1.0f);
		if (Component)
		{
			check(SpriteTemplate);
			UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
			check(LODLevel);
			check(LODLevel->RequiredModule);
			// Take scale into account
			if (LODLevel->RequiredModule->bUseLocalSpace == FALSE)
			{
				NewEmitterData->Scale *= Component->Scale * Component->Scale3D;
				AActor* Actor = Component->GetOwner();
				if (Actor && !Component->AbsoluteScale)
				{
					NewEmitterData->Scale *= Actor->DrawScale * Actor->DrawScale3D;
				}
			}
		}

		// Fill it in...
		INT		ParticleCount	= ActiveParticles;
		UBOOL	bSorted			= FALSE;

		NewEmitterData->ActiveParticleCount = ActiveParticles;
		NewEmitterData->bSelected = bSelected;
		NewEmitterData->ScreenAlignment = SpriteTemplate->ScreenAlignment;
		NewEmitterData->bUseLocalSpace = LODLevel->RequiredModule->bUseLocalSpace;
		NewEmitterData->MeshAlignment = MeshTypeData->MeshAlignment;
		NewEmitterData->bMeshRotationActive = MeshRotationActive;
		NewEmitterData->MeshRotationOffset = MeshRotationOffset;
		NewEmitterData->bScaleUV = LODLevel->RequiredModule->bScaleUV;
		NewEmitterData->SubImages_Horizontal = LODLevel->RequiredModule->SubImages_Horizontal;
		NewEmitterData->SubImages_Vertical = LODLevel->RequiredModule->SubImages_Vertical;
		NewEmitterData->SubUVOffset = FVector(0.0f);
		NewEmitterData->EmitterRenderMode = LODLevel->RequiredModule->EmitterRenderMode;

		if (Module_AxisLock && (Module_AxisLock->bEnabled == TRUE))
		{
			if (Module_AxisLock->LockAxisFlags != EPAL_NONE)
			{
				NewEmitterData->LockAxisFlag = Module_AxisLock->LockAxisFlags;
				NewEmitterData->bLockAxis = TRUE;
			}
		}

		appMemcpy(NewEmitterData->ParticleData, ParticleData, MaxActiveParticles * ParticleStride);
		appMemcpy(NewEmitterData->ParticleIndices, ParticleIndices, MaxActiveParticles * sizeof(WORD));

		return NewEmitterData;
	}
	return NULL;
}

/*-----------------------------------------------------------------------------
	ParticleBeam2EmitterInstance
-----------------------------------------------------------------------------*/
/**
 *	Structure for beam emitter instances
 */
/** Constructor	*/
FParticleBeam2EmitterInstance::FParticleBeam2EmitterInstance() :
	  FParticleEmitterInstance()
	, BeamTypeData(NULL)
	, BeamModule_Source(NULL)
	, BeamModule_Target(NULL)
	, BeamModule_Noise(NULL)
	, FirstEmission(TRUE)
	, LastEmittedParticleIndex(-1)
	, TickCount(0)
	, ForceSpawnCount(0)
	, BeamMethod(0)
	, BeamCount(0)
	, SourceActor(NULL)
	, SourceEmitter(NULL)
	, TargetActor(NULL)
	, TargetEmitter(NULL)
	, VertexCount(0)
	, TriangleCount(0)
{
	TextureTiles.Empty();
	UserSetSourceArray.Empty();
	UserSetSourceTangentArray.Empty();
	UserSetSourceStrengthArray.Empty();
	DistanceArray.Empty();
	TargetPointArray.Empty();
	TargetTangentArray.Empty();
	UserSetTargetStrengthArray.Empty();
	TargetPointSourceNames.Empty();
	UserSetTargetArray.Empty();
	UserSetTargetTangentArray.Empty();
	BeamTrianglesPerSheet.Empty();
}

/** Destructor	*/
FParticleBeam2EmitterInstance::~FParticleBeam2EmitterInstance()
{
	TextureTiles.Empty();
	UserSetSourceArray.Empty();
	UserSetSourceTangentArray.Empty();
	UserSetSourceStrengthArray.Empty();
	DistanceArray.Empty();
	TargetPointArray.Empty();
	TargetTangentArray.Empty();
	UserSetTargetStrengthArray.Empty();
	TargetPointSourceNames.Empty();
	UserSetTargetArray.Empty();
	UserSetTargetTangentArray.Empty();
	BeamTrianglesPerSheet.Empty();
}

// Accessors
/**
 *	Set the beam type
 *
 *	@param	NewMethod	
 */
void FParticleBeam2EmitterInstance::SetBeamType(INT NewMethod)
{
}

/**
 *	Set the tessellation factor
 *
 *	@param	NewFactor
 */
void FParticleBeam2EmitterInstance::SetTessellationFactor(FLOAT NewFactor)
{
}

/**
 *	Set the end point position
 *
 *	@param	NewEndPoint
 */
void FParticleBeam2EmitterInstance::SetEndPoint(FVector NewEndPoint)
{
	if (UserSetTargetArray.Num() < 1)
	{
		UserSetTargetArray.Add(1);

	}
	UserSetTargetArray(0) = NewEndPoint;
}

/**
 *	Set the distance
 *
 *	@param	Distance
 */
void FParticleBeam2EmitterInstance::SetDistance(FLOAT Distance)
{
}

/**
 *	Set the source point
 *
 *	@param	NewSourcePoint
 *	@param	SourceIndex			The index of the source being set
 */
void FParticleBeam2EmitterInstance::SetSourcePoint(FVector NewSourcePoint,INT SourceIndex)
{
	if (SourceIndex < 0)
		return;

	if (UserSetSourceArray.Num() < (SourceIndex + 1))
	{
		UserSetSourceArray.Add((SourceIndex + 1) - UserSetSourceArray.Num());
	}
	UserSetSourceArray(SourceIndex) = NewSourcePoint;
}

/**
 *	Set the source tangent
 *
 *	@param	NewTangentPoint		The tangent value to set it to
 *	@param	SourceIndex			The index of the source being set
 */
void FParticleBeam2EmitterInstance::SetSourceTangent(FVector NewTangentPoint,INT SourceIndex)
{
	if (SourceIndex < 0)
		return;

	if (UserSetSourceTangentArray.Num() < (SourceIndex + 1))		
	{
		UserSetSourceTangentArray.Add((SourceIndex + 1) - UserSetSourceTangentArray.Num());
	}
	UserSetSourceTangentArray(SourceIndex) = NewTangentPoint;
}

/**
 *	Set the source strength
 *
 *	@param	NewSourceStrength	The source strenght to set it to
 *	@param	SourceIndex			The index of the source being set
 */
void FParticleBeam2EmitterInstance::SetSourceStrength(FLOAT NewSourceStrength,INT SourceIndex)
{
	if (SourceIndex < 0)
		return;

	if (UserSetSourceStrengthArray.Num() < (SourceIndex + 1))
	{
		UserSetSourceStrengthArray.Add((SourceIndex + 1) - UserSetSourceStrengthArray.Num());
	}
	UserSetSourceStrengthArray(SourceIndex) = NewSourceStrength;
}

/**
 *	Set the target point
 *
 *	@param	NewTargetPoint		The target point to set it to
 *	@param	TargetIndex			The index of the target being set
 */
void FParticleBeam2EmitterInstance::SetTargetPoint(FVector NewTargetPoint,INT TargetIndex)
{
	if (TargetIndex < 0)
		return;

	if (UserSetTargetArray.Num() < (TargetIndex + 1))
	{
		UserSetTargetArray.Add((TargetIndex + 1) - UserSetTargetArray.Num());
	}
	UserSetTargetArray(TargetIndex) = NewTargetPoint;
}

/**
 *	Set the target tangent
 *
 *	@param	NewTangentPoint		The tangent to set it to
 *	@param	TargetIndex			The index of the target being set
 */
void FParticleBeam2EmitterInstance::SetTargetTangent(FVector NewTangentPoint,INT TargetIndex)
{
	if (TargetIndex < 0)
		return;

	if (UserSetTargetTangentArray.Num() < (TargetIndex + 1))
	{
		UserSetTargetTangentArray.Add((TargetIndex + 1) - UserSetTargetTangentArray.Num());
	}
	UserSetTargetTangentArray(TargetIndex) = NewTangentPoint;
}

/**
 *	Set the target strength
 *
 *	@param	NewTargetStrength	The strength to set it ot
 *	@param	TargetIndex			The index of the target being set
 */
void FParticleBeam2EmitterInstance::SetTargetStrength(FLOAT NewTargetStrength,INT TargetIndex)
{
	if (TargetIndex < 0)
		return;

	if (UserSetTargetStrengthArray.Num() < (TargetIndex + 1))
	{
		UserSetTargetStrengthArray.Add((TargetIndex + 1) - UserSetTargetStrengthArray.Num());
	}
	UserSetTargetStrengthArray(TargetIndex) = NewTargetStrength;
}

/**
 *	Initialize the parameters for the structure
 *
 *	@param	InTemplate		The ParticleEmitter to base the instance on
 *	@param	InComponent		The owning ParticleComponent
 *	@param	bClearResources	If TRUE, clear all resource data
 */
void FParticleBeam2EmitterInstance::InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources)
{
	FParticleEmitterInstance::InitParameters(InTemplate, InComponent, bClearResources);

	UParticleLODLevel* LODLevel	= InTemplate->GetLODLevel(0);
	check(LODLevel);
	BeamTypeData = CastChecked<UParticleModuleTypeDataBeam2>(LODLevel->TypeDataModule);
	check(BeamTypeData);

	//@todo. Determine if we need to support local space.
	if (LODLevel->RequiredModule->bUseLocalSpace)
	{
		LODLevel->RequiredModule->bUseLocalSpace	= FALSE;
	}

	BeamModule_Source			= NULL;
	BeamModule_Target			= NULL;
	BeamModule_Noise			= NULL;

	// Always have at least one beam
	if (BeamTypeData->MaxBeamCount == 0)
	{
		BeamTypeData->MaxBeamCount	= 1;
	}

	BeamCount					= BeamTypeData->MaxBeamCount;
	FirstEmission				= TRUE;
	LastEmittedParticleIndex	= -1;
	TickCount					= 0;
	ForceSpawnCount				= 0;

	BeamMethod					= BeamTypeData->BeamMethod;

	TextureTiles.Empty();
    TextureTiles.AddItem(BeamTypeData->TextureTile);

	UserSetSourceArray.Empty();
	UserSetSourceTangentArray.Empty();
	UserSetSourceStrengthArray.Empty();
	DistanceArray.Empty();
	TargetPointArray.Empty();
	TargetPointSourceNames.Empty();
	UserSetTargetArray.Empty();
	UserSetTargetTangentArray.Empty();
	UserSetTargetStrengthArray.Empty();

	// Resolve any actors...
	ResolveSource();
	ResolveTarget();
}

/**
 *	Initialize the instance
 */
void FParticleBeam2EmitterInstance::Init()
{
	// Setup the modules prior to initializing...
	SetupBeamModules();
	FParticleEmitterInstance::Init();
}

/**
 *	Tick the instance.
 *
 *	@param	DeltaTime			The time slice to use
 *	@param	bSuppressSpawning	If TRUE, do not spawn during Tick
 */
void FParticleBeam2EmitterInstance::Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
	SCOPE_CYCLE_COUNTER(STAT_BeamTickTime);
	if (Component)
	{
		// Make sure we don't try and do any interpolation on the first frame we are attached (OldLocation is not valid in this circumstance)
		if(Component->bJustAttached)
		{
			Location	= Component->LocalToWorld.GetOrigin();
			OldLocation	= Location;
		}
		else
		{
			// Keep track of location for world- space interpolation and other effects.
			OldLocation	= Location;
			Location	= Component->LocalToWorld.GetOrigin();
		}

		// Increment the time alive
		SecondsSinceCreation += DeltaTime;

		// Update time within emitter loop.
		EmitterTime = SecondsSinceCreation;

		// We don't support beam LOD, so always use level 0
		UParticleLODLevel* LODLevel	= SpriteTemplate->GetLODLevel(0);
		check(LODLevel);	// TTP #33141

		if (EmitterDuration > KINDA_SMALL_NUMBER)
		{
			EmitterTime = appFmod(SecondsSinceCreation, EmitterDuration);
		}

		// Take the delay into account
		FLOAT EmitterDelay = LODLevel->RequiredModule->EmitterDelay;

		// Handle looping
		if ((SecondsSinceCreation - (EmitterDuration * LoopCount)) >= EmitterDuration)
		{
			LoopCount++;
			ResetBurstList();

			if ((LoopCount == 1) && (LODLevel->RequiredModule->bDelayFirstLoopOnly == TRUE) && 
				((LODLevel->RequiredModule->EmitterLoops == 0) || (LODLevel->RequiredModule->EmitterLoops > 1)))
			{
				// Need to correct the emitter durations...
				for (INT LODIndex = 0; LODIndex < SpriteTemplate->LODLevels.Num(); LODIndex++)
				{
					UParticleLODLevel* TempLOD = SpriteTemplate->LODLevels(LODIndex);
					EmitterDurations(TempLOD->Level) -= TempLOD->RequiredModule->EmitterDelay;
				}
				EmitterDuration		= EmitterDurations(CurrentLODLevelIndex);
			}
		}

		// Only delay if requested
		if ((LODLevel->RequiredModule->bDelayFirstLoopOnly == TRUE) && (LoopCount > 0))
		{
			EmitterDelay = 0;
		}

		// 'Reset' the emitter time so that the modules function correctly
		EmitterTime -= EmitterDelay;

		// Kill before the spawn... Otherwise, we can get 'flashing'
		KillParticles();

		// If not suppressing spawning...
		if (!bSuppressSpawning && (EmitterTime >= 0.0f))
		{
			if ((LODLevel->RequiredModule->EmitterLoops == 0) || 
				(LoopCount < LODLevel->RequiredModule->EmitterLoops) ||
				(SecondsSinceCreation < (EmitterDuration * LODLevel->RequiredModule->EmitterLoops)))
			{
				// For beams, we probably want to ignore the SpawnRate distribution,
				// and focus strictly on the BurstList...
				FLOAT SpawnRate = 0.0f;
				// Figure out spawn rate for this tick.
				SpawnRate = LODLevel->RequiredModule->SpawnRate.GetValue(EmitterTime, Component);
				// Take Bursts into account as well...
				INT		Burst		= 0;
				FLOAT	BurstTime	= GetCurrentBurstRateOffset(DeltaTime, Burst);
				SpawnRate += BurstTime;

				// Spawn new particles...

				//@todo. Fix the issue of 'blanking' beams when the count drops...
				// This is a temporary hack!
				if ((ActiveParticles < BeamCount) && (SpawnRate <= 0.0f))
				{
					// Force the spawn of a single beam...
					SpawnRate = 1.0f / DeltaTime;
				}

				// Force beams if the emitter is marked "AlwaysOn"
				if ((ActiveParticles < BeamCount) && BeamTypeData->bAlwaysOn)
				{
					Burst		= BeamCount;
					if (DeltaTime > KINDA_SMALL_NUMBER)
					{
						BurstTime	 = Burst / DeltaTime;
						SpawnRate	+= BurstTime;
					}
				}

				if (SpawnRate > 0.f)
				{
					SpawnFraction = Spawn(SpawnFraction, SpawnRate, DeltaTime, Burst, BurstTime);
				}
			}
		}

		// Reset particle data
		for (INT i=0; i<ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			Particle.Velocity		= Particle.BaseVelocity;
			Particle.Size			= Particle.BaseSize;
			Particle.RotationRate	= Particle.BaseRotationRate;
			Particle.RelativeTime += Particle.OneOverMaxLifetime * DeltaTime;

			INC_DWORD_STAT(STAT_BeamParticlesUpdated);
		}

		// Type data module
		UParticleModuleTypeDataBase* pkBase = 0;
		if (LODLevel->TypeDataModule)
		{
			pkBase = Cast<UParticleModuleTypeDataBase>(LODLevel->TypeDataModule);
			//@todo. Need to track TypeData offset into payload!
			pkBase->PreUpdate(this, TypeDataOffset, DeltaTime);
		}

		// Update existing particles (might respawn dying ones).
		for (INT i = 0; i < LODLevel->UpdateModules.Num(); i++)
		{
			UParticleModule* ParticleModule	= LODLevel->UpdateModules(i);
			if (!ParticleModule || !ParticleModule->bEnabled)
			{
				continue;
			}
			UINT* Offset = ModuleOffsetMap.Find(ParticleModule);
			ParticleModule->Update(this, Offset ? *Offset : 0, DeltaTime);
		}

		//@todo. This should ALWAYS be true for Beams...
		if (pkBase)
		{
			// The order of the update here is VERY important
			UINT* Offset;
			if (BeamModule_Source && BeamModule_Source->bEnabled)
			{
				Offset = ModuleOffsetMap.Find(BeamModule_Source);
				BeamModule_Source->Update(this, Offset ? *Offset : 0, DeltaTime);
			}
			if (BeamModule_Target && BeamModule_Target->bEnabled)
			{
				Offset = ModuleOffsetMap.Find(BeamModule_Target);
				BeamModule_Target->Update(this, Offset ? *Offset : 0, DeltaTime);
			}
			if (BeamModule_Noise && BeamModule_Noise->bEnabled)
			{
				Offset = ModuleOffsetMap.Find(BeamModule_Noise);
				BeamModule_Noise->Update(this, Offset ? *Offset : 0, DeltaTime);
			}

			//@todo. Need to track TypeData offset into payload!
			pkBase->Update(this, TypeDataOffset, DeltaTime);
			pkBase->PostUpdate(this, TypeDataOffset, DeltaTime);
		}

		// Calculate bounding box and simulate velocity.
		UpdateBoundingBox(DeltaTime);

		if (!bSuppressSpawning)
		{
			// Ensure that we flip the 'FirstEmission' flag
			FirstEmission = FALSE;
		}

		// Invalidate the contents of the vertex/index buffer.
		IsRenderDataDirty = 1;

		// Bump the tick count
		TickCount++;

		// 'Reset' the emitter time so that the delay functions correctly
		EmitterTime += EmitterDelay;
	}
	INC_DWORD_STAT(STAT_BeamParticlesUpdateCalls);
}

/**
 *	Tick the instance in the editor.
 *	This function will interpolate between the current LODLevels to allow for
 *	the designer to visualize how the selected LOD setting would look.
 *
 *	@param	HighLODLevel		The higher LOD level selected
 *	@param	LowLODLevel			The lower LOD level selected
 *	@param	Multiplier			The interpolation value to use between the two
 *	@param	DeltaTime			The time slice to use
 *	@param	bSuppressSpawning	If TRUE, do not spawn during Tick
 */
void FParticleBeam2EmitterInstance::TickEditor(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, FLOAT Multiplier, FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
	Tick(DeltaTime, bSuppressSpawning);
}

/**
 *	Update the bounding box for the emitter
 *
 *	@param	DeltaTime		The time slice to use
 */
void FParticleBeam2EmitterInstance::UpdateBoundingBox(FLOAT DeltaTime)
{
	if (Component)
	{
		FLOAT MaxSizeScale	= 1.0f;
		ParticleBoundingBox.Init();

		//@todo. Currently, we don't support UseLocalSpace for beams
		//if (Template->UseLocalSpace == false) 
		{
			ParticleBoundingBox += Component->LocalToWorld.GetOrigin();
		}

		FVector	NoiseMin(0.0f);
		FVector NoiseMax(0.0f);
		// Noise points have to be taken into account...
		if (BeamModule_Noise)
		{
			BeamModule_Noise->GetNoiseRange(NoiseMin, NoiseMax);
		}

		// Take scale into account as well
		FVector Scale = FVector(1.0f, 1.0f, 1.0f);
		Scale *= Component->Scale * Component->Scale3D;
		AActor* Actor = Component->GetOwner();
		if (Actor && !Component->AbsoluteScale)
		{
			Scale *= Actor->DrawScale * Actor->DrawScale3D;
		}

		// Take each particle into account
		for (INT i=0; i<ActiveParticles; i++)
		{
			DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			
			INT						CurrentOffset		= TypeDataOffset;
			FBeam2TypeDataPayload*	BeamData			= NULL;
			FVector*				InterpolatedPoints	= NULL;
			FLOAT*					NoiseRate			= NULL;
			FLOAT*					NoiseDelta			= NULL;
			FVector*				TargetNoisePoints	= NULL;
			FVector*				NextNoisePoints		= NULL;
			FLOAT*					TaperValues			= NULL;

			BeamTypeData->GetDataPointers(this, (const BYTE*)Particle, CurrentOffset, 
				BeamData, InterpolatedPoints, NoiseRate, NoiseDelta, TargetNoisePoints, 
				NextNoisePoints, TaperValues);

			// Do linear integrator and update bounding box
			Particle->OldLocation = Particle->Location;
			Particle->Location	+= DeltaTime * Particle->Velocity;
			ParticleBoundingBox += Particle->Location;
			ParticleBoundingBox += Particle->Location + NoiseMin;
			ParticleBoundingBox += Particle->Location + NoiseMax;
			ParticleBoundingBox += BeamData->SourcePoint;
			ParticleBoundingBox += BeamData->SourcePoint + NoiseMin;
			ParticleBoundingBox += BeamData->SourcePoint + NoiseMax;
			ParticleBoundingBox += BeamData->TargetPoint;
			ParticleBoundingBox += BeamData->TargetPoint + NoiseMin;
			ParticleBoundingBox += BeamData->TargetPoint + NoiseMax;

			// Do angular integrator, and wrap result to within +/- 2 PI
			Particle->Rotation	+= DeltaTime * Particle->RotationRate;
			Particle->Rotation	 = appFmod(Particle->Rotation, 2.f*(FLOAT)PI);
			FVector Size = Particle->Size * Scale;
			MaxSizeScale		 = Max(MaxSizeScale, Size.GetAbsMax()); //@todo particles: this does a whole lot of compares that can be avoided using SSE/ Altivec.
		}
		ParticleBoundingBox = ParticleBoundingBox.ExpandBy(MaxSizeScale);

		//@todo. Transform bounding box into world space if the emitter uses a local space coordinate system.
/***
		if (Template->UseLocalSpace) 
		{
			ParticleBoundingBox = ParticleBoundingBox.TransformBy(Component->LocalToWorld);
		}
***/
	}
}

/**
 *	Retrieved the per-particle bytes that this emitter type requires.
 *
 *	@return	UINT	The number of required bytes for particles in the instance
 */
UINT FParticleBeam2EmitterInstance::RequiredBytes()
{
	UINT uiBytes = FParticleEmitterInstance::RequiredBytes();

	// Flag bits indicating particle 
	uiBytes += sizeof(INT);

	return uiBytes;
}

/**
 *	Spawn particles for this instance
 *
 *	@param	OldLeftover		The leftover time from the last spawn
 *	@param	Rate			The rate at which particles should be spawned
 *	@param	DeltaTime		The time slice to spawn over
 *	@param	Burst			The number of burst particle
 *	@param	BurstTime		The burst time addition (faked time slice)
 *
 *	@return	FLOAT			The leftover fraction of spawning
 */
FLOAT FParticleBeam2EmitterInstance::Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst, FLOAT BurstTime)
{
	FLOAT	NewLeftover = OldLeftover + DeltaTime * Rate;

	SCOPE_CYCLE_COUNTER(STAT_BeamSpawnTime);

	// Ensure continous spawning... lots of fiddling.
	INT		Number		= appFloor(NewLeftover);
	FLOAT	Increment	= 1.f / Rate;
	FLOAT	StartTime	= DeltaTime + OldLeftover * Increment - Increment;
	NewLeftover			= NewLeftover - Number;

	// Always match the burst at a minimum
	if (Number < Burst)
	{
		Number = Burst;
	}

	// Account for burst time simulation
	if (BurstTime > KINDA_SMALL_NUMBER)
	{
		NewLeftover -= BurstTime / Burst;
		NewLeftover	= Clamp<FLOAT>(NewLeftover, 0, NewLeftover);
	}

	// Force a beam
	UBOOL bNoLivingParticles = false;
	if (ActiveParticles == 0)
	{
		bNoLivingParticles = true;
		if (Number == 0)
			Number = 1;
	}

	// Don't allow more than BeamCount beams...
	if (Number + ActiveParticles > BeamCount)
	{
		Number	= BeamCount - ActiveParticles;
	}

	// Handle growing arrays.
	INT NewCount = ActiveParticles + Number;
	if (NewCount >= MaxActiveParticles)
	{
		if (DeltaTime < 0.25f)
		{
			Resize(NewCount + appTrunc(appSqrt((FLOAT)NewCount)) + 1);
		}
		else
		{
			Resize((NewCount + appTrunc(appSqrt((FLOAT)NewCount)) + 1), FALSE);
		}
	}

	UParticleLODLevel* LODLevel	= SpriteTemplate->GetLODLevel(0);
	check(LODLevel);

	// Spawn particles.
	for (INT i=0; i<Number; i++)
	{
		INT iParticleIndex = ParticleIndices[ActiveParticles];

		DECLARE_PARTICLE_PTR(pkParticle, ParticleData + ParticleStride * iParticleIndex);

		FLOAT SpawnTime = StartTime - i * Increment;
	
		PreSpawn(pkParticle);
		for (INT n = 0; n < LODLevel->SpawnModules.Num(); n++)
		{
			UParticleModule* SpawnModule	= LODLevel->SpawnModules(n);
			UINT* Offset = ModuleOffsetMap.Find(SpawnModule);
			if (SpawnModule->bEnabled)
			{
				SpawnModule->Spawn(this, Offset ? *Offset : 0, SpawnTime);
			}
		}

		// The order of the Spawn here is VERY important as the modules may(will) depend on it occuring as such.
		UINT* Offset;
		if (BeamModule_Source && BeamModule_Source->bEnabled)
		{
			Offset = ModuleOffsetMap.Find(BeamModule_Source);
			BeamModule_Source->Spawn(this, Offset ? *Offset : 0, DeltaTime);
		}
		if (BeamModule_Target && BeamModule_Target->bEnabled)
		{
			Offset = ModuleOffsetMap.Find(BeamModule_Target);
			BeamModule_Target->Spawn(this, Offset ? *Offset : 0, DeltaTime);
		}
		if (BeamModule_Noise && BeamModule_Noise->bEnabled)
		{
			Offset = ModuleOffsetMap.Find(BeamModule_Noise);
			BeamModule_Noise->Spawn(this, Offset ? *Offset : 0, DeltaTime);
		}
		if (LODLevel->TypeDataModule)
		{
			//@todo. Need to track TypeData offset into payload!
			LODLevel->TypeDataModule->Spawn(this, TypeDataOffset, SpawnTime);
		}

		PostSpawn(pkParticle, 1.f - FLOAT(i+1) / FLOAT(Number), SpawnTime);

		ActiveParticles++;

		INC_DWORD_STAT(STAT_BeamParticles);
		INC_DWORD_STAT(STAT_BeamParticlesSpawned);

		LastEmittedParticleIndex = iParticleIndex;
	}

	if (ForceSpawnCount > 0)
	{
		ForceSpawnCount = 0;
	}

	return NewLeftover;
}

/**
 *	Spawn particles for this instance
 *	Editor version for interpolation
 *
 *	@param	HighLODLevel	The higher LOD level
 *	@param	LowLODLevel		The lower LOD level
 *	@param	Multiplier		The interpolation value to use
 *	@param	OldLeftover		The leftover time from the last spawn
 *	@param	Rate			The rate at which particles should be spawned
 *	@param	DeltaTime		The time slice to spawn over
 *	@param	Burst			The number of burst particle
 *	@param	BurstTime		The burst time addition (faked time slice)
 *
 *	@return	FLOAT			The leftover fraction of spawning
 */
FLOAT FParticleBeam2EmitterInstance::SpawnEditor(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, FLOAT Multiplier, 
	FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst, FLOAT BurstTime)
{
	return Spawn(OldLeftover, Rate, DeltaTime, Burst, BurstTime);
}

/**
 *	Handle any pre-spawning actions required for particles
 *
 *	@param	Particle	The particle being spawned.
 */
void FParticleBeam2EmitterInstance::PreSpawn(FBaseParticle* Particle)
{
	FParticleEmitterInstance::PreSpawn(Particle);
	if (BeamTypeData)
	{
		BeamTypeData->PreSpawn(this, Particle);
	}
}

/**
 *	Kill off any dead particles. (Remove them from the active array)
 */
void FParticleBeam2EmitterInstance::KillParticles()
{
	// Loop over the active particles... If their RelativeTime is > 1.0f (indicating they are dead),
	// move them to the 'end' of the active particle list.
	for (INT i=ActiveParticles-1; i>=0; i--)
	{
		const INT	CurrentIndex	= ParticleIndices[i];
		const BYTE* ParticleBase	= ParticleData + CurrentIndex * ParticleStride;
		FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);
		if (Particle.RelativeTime > 1.0f)
		{
			ParticleIndices[i]	= ParticleIndices[ActiveParticles-1];
			ParticleIndices[ActiveParticles-1]	= CurrentIndex;
			ActiveParticles--;

			DEC_DWORD_STAT(STAT_BeamParticles);
			INC_DWORD_STAT(STAT_BeamParticlesKilled);
		}
	}
}

/**
 *	Setup the beam module pointers
 */
void FParticleBeam2EmitterInstance::SetupBeamModules()
{
	// Beams are a special case... 
	// We don't want standard Spawn/Update calls occuring on Beam-type modules.
	UParticleLODLevel* LODLevel	= SpriteTemplate->GetLODLevel(0);
	check(LODLevel);

	// Go over all the modules in the LOD level
	for (INT ii = 0; ii < LODLevel->Modules.Num(); ii++)
	{
		UParticleModule* CheckModule = LODLevel->Modules(ii);
		if (CheckModule->GetModuleType() == EPMT_Beam)
		{
			UBOOL bRemove = FALSE;

			if (CheckModule->IsA(UParticleModuleBeamSource::StaticClass()))
			{
				//if (BeamModule_Source)
				//{
				//	debugf(TEXT("Warning: Multiple beam source modules!"));
				//}
				BeamModule_Source	= Cast<UParticleModuleBeamSource>(CheckModule);
				bRemove = TRUE;
			}
			else
			if (CheckModule->IsA(UParticleModuleBeamTarget::StaticClass()))
			{
				//if (BeamModule_Target)
				//{
				//	debugf(TEXT("Warning: Multiple beam Target modules!"));
				//}
				BeamModule_Target	= Cast<UParticleModuleBeamTarget>(CheckModule);
				bRemove = TRUE;
			}
			else
			if (CheckModule->IsA(UParticleModuleBeamNoise::StaticClass()))
			{
				//if (BeamModule_Noise)
				//{
				//	debugf(TEXT("Warning: Multiple beam Noise modules!"));
				//}
				BeamModule_Noise	= Cast<UParticleModuleBeamNoise>(CheckModule);
				bRemove = TRUE;
			}

			//@todo. Remove from the Update/Spawn lists???
			if (bRemove)
			{
				for (INT jj = 0; jj < LODLevel->UpdateModules.Num(); jj++)
				{
					if (LODLevel->UpdateModules(jj) == CheckModule)
					{
						LODLevel->UpdateModules.Remove(jj);
						break;
					}
				}

				for (INT kk = 0; kk < LODLevel->SpawnModules.Num(); kk++)
				{
					if (LODLevel->SpawnModules(kk) == CheckModule)
					{
						LODLevel->SpawnModules.Remove(kk);
						break;
					}
				}
			}
		}
	}
}

/**
 *	Resolve the source for the beam
 */
void FParticleBeam2EmitterInstance::ResolveSource()
{
	if (BeamModule_Source)
	{
		if (BeamModule_Source->SourceName != NAME_None)
		{
			switch (BeamModule_Source->SourceMethod)
			{
			case PEB2STM_Actor:
				if (SourceActor == NULL)
				{
					FParticleSysParam Param;
					for (INT i = 0; i < Component->InstanceParameters.Num(); i++)
					{
						Param = Component->InstanceParameters(i);
						if (Param.Name == BeamModule_Source->SourceName)
						{
							SourceActor = Param.Actor;
							break;
						}
					}
				}
				break;
			case PEB2STM_Emitter:
			case PEB2STM_Particle:
				if (SourceEmitter == NULL)
				{
					for (INT ii = 0; ii < Component->EmitterInstances.Num(); ii++)
					{
						FParticleEmitterInstance* pkEmitInst = Component->EmitterInstances(ii);
						if (pkEmitInst && (pkEmitInst->SpriteTemplate->EmitterName == BeamModule_Source->SourceName))
						{
							SourceEmitter = pkEmitInst;
							break;
						}
					}
				}
				break;
			}
		}
	}
}

/**
 *	Resolve the target for the beam
 */
void FParticleBeam2EmitterInstance::ResolveTarget()
{
	if (BeamModule_Target)
	{
		if (BeamModule_Target->TargetName != NAME_None)
		{
			switch (BeamModule_Target->TargetMethod)
			{
			case PEB2STM_Actor:
				if (TargetActor == NULL)
				{
					FParticleSysParam Param;
					for (INT i = 0; i < Component->InstanceParameters.Num(); i++)
					{
						Param = Component->InstanceParameters(i);
						if (Param.Name == BeamModule_Target->TargetName)
						{
							TargetActor = Param.Actor;
							break;
						}
					}
				}
				break;
			case PEB2STM_Emitter:
				if (TargetEmitter == NULL)
				{
					for (INT ii = 0; ii < Component->EmitterInstances.Num(); ii++)
					{
						FParticleEmitterInstance* pkEmitInst = Component->EmitterInstances(ii);
						if (pkEmitInst && (pkEmitInst->SpriteTemplate->EmitterName == BeamModule_Target->TargetName))
						{
							TargetEmitter = pkEmitInst;
							break;
						}
					}
				}
				break;
			}
		}
	}
}

/**
 *	Determine the vertex and triangle counts for the emitter
 */
void FParticleBeam2EmitterInstance::DetermineVertexAndTriangleCount()
{
	// Need to determine # tris per beam...
	INT VerticesToRender = 0;
	INT TrianglesToRender = 0;

	check(BeamTypeData);
	INT Sheets = BeamTypeData->Sheets ? BeamTypeData->Sheets : 1;

	BeamTrianglesPerSheet.Empty(ActiveParticles);
	BeamTrianglesPerSheet.AddZeroed(ActiveParticles);
	for (INT i = 0; i < ActiveParticles; i++)
	{
		DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[i]);

		INT						CurrentOffset		= TypeDataOffset;
		FBeam2TypeDataPayload*	BeamData			= NULL;
		FVector*				InterpolatedPoints	= NULL;
		FLOAT*					NoiseRate			= NULL;
		FLOAT*					NoiseDelta			= NULL;
		FVector*				TargetNoisePoints	= NULL;
        FVector*				NextNoisePoints		= NULL;
		FLOAT*					TaperValues			= NULL;

		BeamTypeData->GetDataPointers(this, (const BYTE*)Particle, CurrentOffset, 
			BeamData, InterpolatedPoints, NoiseRate, NoiseDelta, TargetNoisePoints, 
			NextNoisePoints, TaperValues);

		BeamTrianglesPerSheet(i) = BeamData->TriangleCount;

		// Take sheets into account
		INT LocalTriangles = 0;
		if (BeamData->TriangleCount > 0)
		{
			// Stored triangle count is per sheet...
			LocalTriangles	+= BeamData->TriangleCount * Sheets;
			VerticesToRender += (BeamData->TriangleCount + 2) * Sheets;
			// 4 Degenerates Per Sheet (except for last one)
			LocalTriangles	+= (Sheets - 1) * 4;
			TrianglesToRender += LocalTriangles;
			// Multiple beams?
			if (i < (ActiveParticles - 1))
			{
				// 4 Degenerates Per Beam (except for last one)
				TrianglesToRender	+= 4;
			}
		}
	}

	VertexCount = VerticesToRender;
	TriangleCount = TrianglesToRender;
}

/**
 *	Retrieves the dynamic data for the emitter
 *	
 *	@param	bSelected					Whether the emitter is selected in the editor
 *
 *	@return	FDynamicEmitterDataBase*	The dynamic data, or NULL if it shouldn't be rendered
 */
FDynamicEmitterDataBase* FParticleBeam2EmitterInstance::GetDynamicData(UBOOL bSelected)
{
	// Validity check
	if (!SpriteTemplate)
	{
		return NULL;
	}

	// If the template is disabled, don't return data.
	UParticleLODLevel* LODLevel = SpriteTemplate->GetLODLevel(0);
	if ((LODLevel == NULL) ||
		(LODLevel->RequiredModule->bEnabled == FALSE))
	{
		return NULL;
	}

	// Allocate it for now, but we will want to change this to do some form
	// of caching
	if (ActiveParticles > 0)
	{
		// Get the material instance. If there is none, or the material isn't flagged for use with particle systems, use the DefaultMaterial.
		UMaterialInstance* MaterialInst = SpriteTemplate->Material;
		if (MaterialInst == NULL || !MaterialInst->UseWithParticleSystem())
		{
			MaterialInst = GEngine->DefaultMaterial;
		}

		// Allocate the data
		FDynamicBeam2EmitterData* NewEmitterData = ::new FDynamicBeam2EmitterData(ActiveParticles, ParticleStride, 
			MaterialInst->GetInstanceInterface(bSelected));
		check(NewEmitterData);
		check(NewEmitterData->ParticleData);

		// Fill it in...
		INT						ParticleCount	= ActiveParticles;
		UBOOL					bSorted			= FALSE;
		const INT				ScreenAlignment	= SpriteTemplate->ScreenAlignment;

		NewEmitterData->bUseLocalSpace = FALSE;
		NewEmitterData->Scale = FVector(1.0f, 1.0f, 1.0f);
		if (Component)
		{
			NewEmitterData->Scale *= Component->Scale * Component->Scale3D;
			AActor* Actor = Component->GetOwner();
			if (Actor && !Component->AbsoluteScale)
			{
				NewEmitterData->Scale *= Actor->DrawScale * Actor->DrawScale3D;
			}
		}

		DetermineVertexAndTriangleCount();

		NewEmitterData->TrianglesPerSheet.Empty(BeamTrianglesPerSheet.Num());
		NewEmitterData->TrianglesPerSheet.AddZeroed(BeamTrianglesPerSheet.Num());
		for (INT BeamIndex = 0; BeamIndex < BeamTrianglesPerSheet.Num(); BeamIndex++)
		{
			NewEmitterData->TrianglesPerSheet(BeamIndex) = BeamTrianglesPerSheet(BeamIndex);
		}
		NewEmitterData->ActiveParticleCount = ActiveParticles;
		NewEmitterData->VertexCount = VertexCount;
		NewEmitterData->PrimitiveCount = TriangleCount;

		NewEmitterData->ScreenAlignment = SpriteTemplate->ScreenAlignment;
		NewEmitterData->EmitterRenderMode = LODLevel->RequiredModule->EmitterRenderMode;

		NewEmitterData->bLockAxis = FALSE;

		BeamTypeData->GetDataPointerOffsets(this, NULL, TypeDataOffset, 
			NewEmitterData->BeamDataOffset, NewEmitterData->InterpolatedPointsOffset,
			NewEmitterData->NoiseRateOffset, NewEmitterData->NoiseDeltaTimeOffset,
			NewEmitterData->TargetNoisePointsOffset, NewEmitterData->NextNoisePointsOffset, 
			NewEmitterData->TaperCount, NewEmitterData->TaperValuesOffset);

		if (BeamModule_Source)
		{
			NewEmitterData->bUseSource = TRUE;
		}
		else
		{
			NewEmitterData->bUseSource = FALSE;
		}

		if (BeamModule_Target)
		{
			NewEmitterData->bUseTarget = TRUE;
		}
		else
		{
			NewEmitterData->bUseTarget = FALSE;
		}

		if (BeamModule_Noise)
		{
			NewEmitterData->bLowFreqNoise_Enabled = BeamModule_Noise->bLowFreq_Enabled;
			NewEmitterData->bHighFreqNoise_Enabled = FALSE;
			NewEmitterData->bSmoothNoise_Enabled = BeamModule_Noise->bSmooth;

		}
		else
		{
			NewEmitterData->bLowFreqNoise_Enabled = FALSE;
			NewEmitterData->bHighFreqNoise_Enabled = FALSE;
			NewEmitterData->bSmoothNoise_Enabled = FALSE;
		}
		NewEmitterData->TessFactor = (BeamTypeData->TessellationFactor > 0) ? BeamTypeData->TessellationFactor : 1;
		NewEmitterData->Sheets = (BeamTypeData->Sheets > 0) ? BeamTypeData->Sheets : 1;
		NewEmitterData->TextureTile = BeamTypeData->TextureTile;
		NewEmitterData->TextureTileDistance = BeamTypeData->TextureTileDistance;
//		NewEmitterData->TaperMethod = BeamTypeData->TaperMethod;
		NewEmitterData->TaperMethod = PEBTM_None;
		NewEmitterData->InterpolationPoints = BeamTypeData->InterpolationPoints;

		NewEmitterData->NoiseTessellation	= 0;
		NewEmitterData->Frequency			= 1;
		NewEmitterData->NoiseRangeScale		= 1.0f;
		NewEmitterData->NoiseTangentStrength= 1.0f;


		if ((BeamModule_Noise == NULL) || (BeamModule_Noise->bLowFreq_Enabled == FALSE))
		{
			NewEmitterData->TessFactor	= BeamTypeData->InterpolationPoints ? BeamTypeData->InterpolationPoints : 1;
		}
		else
		{
			if (BeamTypeData->InterpolationPoints > 0)
			{
				//@todo. HANDLE THIS CASE
				debugf(TEXT("Interpolated noisy beams not supported yet!"));
				appDebugBreak();
			}
			else
			{
				NewEmitterData->Frequency			= (BeamModule_Noise->Frequency > 0) ? BeamModule_Noise->Frequency : 1;
				NewEmitterData->NoiseTessellation	= (BeamModule_Noise->NoiseTessellation > 0) ? BeamModule_Noise->NoiseTessellation : 1;
				NewEmitterData->NoiseTangentStrength= BeamModule_Noise->NoiseTangentStrength.GetValue(EmitterTime);
				if (BeamModule_Noise->bNRScaleEmitterTime)
				{
					NewEmitterData->NoiseRangeScale = BeamModule_Noise->NoiseRangeScale.GetValue(EmitterTime, Component);
				}
				else
				{
					//@todo.SAS. Need to address this!!!!
//					check(0 && TEXT("NoiseRangeScale - No way to get per-particle setting at this time."));
//					NewEmitterData->NoiseRangeScale	= BeamModule_Noise->NoiseRangeScale.GetValue(Particle->RelativeTime, Component);
					NewEmitterData->NoiseRangeScale = BeamModule_Noise->NoiseRangeScale.GetValue(EmitterTime, Component);
				}
				NewEmitterData->NoiseSpeed = BeamModule_Noise->NoiseSpeed.GetValue(EmitterTime);
				NewEmitterData->NoiseLockTime = BeamModule_Noise->NoiseLockTime;
				NewEmitterData->NoiseLockRadius = BeamModule_Noise->NoiseLockRadius;
				NewEmitterData->bTargetNoise = BeamModule_Noise->bTargetNoise;
				NewEmitterData->NoiseTension = BeamModule_Noise->NoiseTension;
			}
		}

		INT MaxSegments	= ((NewEmitterData->TessFactor * NewEmitterData->Frequency) + 1 + 1);		// Tessellation * Frequency + FinalSegment + FirstEdge;

		// Determine the index count
		NewEmitterData->IndexCount	= 0;
		for (INT Beam = 0; Beam < ActiveParticles; Beam++)
		{
			DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[Beam]);

			INT						CurrentOffset		= TypeDataOffset;
			FBeam2TypeDataPayload*	BeamData			= NULL;
			FVector*				InterpolatedPoints	= NULL;
			FLOAT*					NoiseRate			= NULL;
			FLOAT*					NoiseDelta			= NULL;
			FVector*				TargetNoisePoints	= NULL;
			FVector*				NextNoisePoints		= NULL;
			FLOAT*					TaperValues			= NULL;

			BeamTypeData->GetDataPointers(this, (const BYTE*)Particle, CurrentOffset, BeamData, 
				InterpolatedPoints, NoiseRate, NoiseDelta, TargetNoisePoints, NextNoisePoints, TaperValues);

			if (BeamData->TriangleCount > 0)
			{
				if (NewEmitterData->IndexCount == 0)
				{
					NewEmitterData->IndexCount = 2;
				}
				NewEmitterData->IndexCount	+= BeamData->TriangleCount * NewEmitterData->Sheets;	// 1 index per triangle in the strip PER SHEET
				NewEmitterData->IndexCount	+= ((NewEmitterData->Sheets - 1) * 4);					// 4 extra indices per stitch (degenerates)
				if (Beam > 0)
				{
					NewEmitterData->IndexCount	+= 4;	// 4 extra indices per beam (degenerates)
				}
			}
		}

		if (NewEmitterData->IndexCount > 15000)
		{
			NewEmitterData->IndexStride	= sizeof(DWORD);
		}
		else
		{
			NewEmitterData->IndexStride	= sizeof(WORD);
		}
		
		//@todo. SORTING IS A DIFFERENT ISSUE NOW! 
		//		 GParticleView isn't going to be valid anymore?
		BYTE* PData = NewEmitterData->ParticleData;
		for (INT i = 0; i < ParticleCount; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			appMemcpy(PData, &Particle, ParticleStride);
			PData += ParticleStride;
		}

		return NewEmitterData;
	}
	return NULL;
}

/*-----------------------------------------------------------------------------
	ParticleTrail2EmitterInstance.
-----------------------------------------------------------------------------*/
/**
 *	Structure for trail emitter instances
 */

/** Constructor	*/
FParticleTrail2EmitterInstance::FParticleTrail2EmitterInstance() :
	  FParticleEmitterInstance()
	, TrailTypeData(NULL)
    , TrailModule_Source(NULL)
    , TrailModule_Source_Offset(0)
    , TrailModule_Spawn(NULL)
    , TrailModule_Spawn_Offset(0)
    , TrailModule_Taper(NULL)
    , TrailModule_Taper_Offset(0)
	, FirstEmission(0)
    , LastEmittedParticleIndex(-1)
    , LastSelectedParticleIndex(-1)
    , TickCount(0)
    , ForceSpawnCount(0)
    , VertexCount(0)
    , TriangleCount(0)
    , Tessellation(0)
    , TrailCount(0)
    , MaxTrailCount(0)
    , SourceActor(NULL)
    , SourceEmitter(NULL)
    , ActuallySpawned(0)
{
    TextureTiles.Empty();
	TrailSpawnTimes.Empty();
	SourcePosition.Empty();
	LastSourcePosition.Empty();
	CurrentSourcePosition.Empty();
	LastSpawnPosition.Empty();
	LastSpawnTangent.Empty();
	SourceDistanceTravelled.Empty();
	SourceOffsets.Empty();
}

/** Destructor	*/
FParticleTrail2EmitterInstance::~FParticleTrail2EmitterInstance()
{
    TextureTiles.Empty();
	TrailSpawnTimes.Empty();
	SourcePosition.Empty();
	LastSourcePosition.Empty();
	CurrentSourcePosition.Empty();
	LastSpawnPosition.Empty();
	LastSpawnTangent.Empty();
	SourceDistanceTravelled.Empty();
	SourceOffsets.Empty();
}

/**
 *	Initialize the parameters for the structure
 *
 *	@param	InTemplate		The ParticleEmitter to base the instance on
 *	@param	InComponent		The owning ParticleComponent
 *	@param	bClearResources	If TRUE, clear all resource data
 */
void FParticleTrail2EmitterInstance::InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources)
{
	FParticleEmitterInstance::InitParameters(InTemplate, InComponent, bClearResources);

	// We don't support LOD on trails
	UParticleLODLevel* LODLevel	= InTemplate->GetLODLevel(0);
	check(LODLevel);
	TrailTypeData	= CastChecked<UParticleModuleTypeDataTrail2>(LODLevel->TypeDataModule);
	check(TrailTypeData);

	TrailModule_Source			= NULL;
	TrailModule_Source_Offset	= 0;
	TrailModule_Spawn			= NULL;
	TrailModule_Spawn_Offset	= 0;
	TrailModule_Taper			= NULL;
	TrailModule_Taper_Offset	= 0;

	// Always have at least one trail
	if (TrailTypeData->MaxTrailCount == 0)
	{
		TrailTypeData->MaxTrailCount	= 1;
	}

	//@todo. Remove this statement once multiple trails per emitter is implemented. 
	TrailTypeData->MaxTrailCount	= 1;

	// Always have at least one particle per trail
	if (TrailTypeData->MaxParticleInTrailCount == 0)
	{
		// Doesn't make sense to have 0 for this...
		warnf(TEXT("TrailEmitter %s --> MaxParticleInTrailCount == 0!"), *(InTemplate->GetPathName()));
		TrailTypeData->MaxParticleInTrailCount	= 1;
	}

	MaxTrailCount				= TrailTypeData->MaxTrailCount;
	TrailSpawnTimes.AddZeroed(MaxTrailCount);
	SourceDistanceTravelled.AddZeroed(MaxTrailCount);
	SourcePosition.AddZeroed(MaxTrailCount);
	LastSourcePosition.AddZeroed(MaxTrailCount);
	CurrentSourcePosition.AddZeroed(MaxTrailCount);
	LastSpawnPosition.AddZeroed(MaxTrailCount);
	LastSpawnTangent.AddZeroed(MaxTrailCount);
	SourceDistanceTravelled.AddZeroed(MaxTrailCount);
	FirstEmission				= TRUE;
	LastEmittedParticleIndex	= -1;
	LastSelectedParticleIndex	= -1;
	TickCount					= 0;
	ForceSpawnCount				= 0;

	VertexCount					= 0;
	TriangleCount				= 0;

	TextureTiles.Empty();
	TextureTiles.AddItem(TrailTypeData->TextureTile);

	// Resolve any actors...
	ResolveSource();
}

/**
 *	Initialize the instance
 */
void FParticleTrail2EmitterInstance::Init()
{
	FParticleEmitterInstance::Init();
	// Setup the modules prior to initializing...
	SetupTrailModules();
}

/**
 *	Tick the instance.
 *
 *	@param	DeltaTime			The time slice to use
 *	@param	bSuppressSpawning	If TRUE, do not spawn during Tick
 */
void FParticleTrail2EmitterInstance::Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
	SCOPE_CYCLE_COUNTER(STAT_TrailTickTime);
	if (Component)
	{
		// Only support the high LOD
		UParticleLODLevel* LODLevel	= SpriteTemplate->GetLODLevel(0);
		check(LODLevel);

		// Make sure we don't try and do any interpolation on the first frame we are attached (OldLocation is not valid in this circumstance)
		if(Component->bJustAttached)
		{
			Location	= Component->LocalToWorld.GetOrigin();
			OldLocation	= Location;
		}
		else
		{
			// Keep track of location for world- space interpolation and other effects.
			OldLocation	= Location;
			Location	= Component->LocalToWorld.GetOrigin();
		}

		SecondsSinceCreation += DeltaTime;

		// Update time within emitter loop.
		EmitterTime = SecondsSinceCreation;

		if (EmitterDuration > KINDA_SMALL_NUMBER)
		{
			EmitterTime = appFmod(SecondsSinceCreation, EmitterDuration);
		}

		// Take delay into account
		FLOAT EmitterDelay = LODLevel->RequiredModule->EmitterDelay;

		// If looping, handle it
		if ((SecondsSinceCreation - (EmitterDuration * LoopCount)) >= EmitterDuration)
		{
			LoopCount++;
			ResetBurstList();

			if ((LoopCount == 1) && (LODLevel->RequiredModule->bDelayFirstLoopOnly == TRUE) && 
				((LODLevel->RequiredModule->EmitterLoops == 0) || (LODLevel->RequiredModule->EmitterLoops > 1)))
			{
				// Need to correct the emitter durations...
				for (INT LODIndex = 0; LODIndex < SpriteTemplate->LODLevels.Num(); LODIndex++)
				{
					UParticleLODLevel* TempLOD = SpriteTemplate->LODLevels(LODIndex);
					EmitterDurations(TempLOD->Level) -= TempLOD->RequiredModule->EmitterDelay;
				}
				EmitterDuration		= EmitterDurations(CurrentLODLevelIndex);
			}
		}

		// Don't delay unless required
		if ((LODLevel->RequiredModule->bDelayFirstLoopOnly == TRUE) && (LoopCount > 0))
		{
			EmitterDelay = 0;
		}

		// 'Reset' the emitter time so that the modules function correctly
		EmitterTime -= EmitterDelay;

		// Update the source data (position, etc.)
		UpdateSourceData(DeltaTime);

		// Kill before the spawn... Otherwise, we can get 'flashing'
		KillParticles();

		// We need to update the source travelled distance
		for (INT i=0; i<ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);

			INT						CurrentOffset		= TypeDataOffset;
			FTrail2TypeDataPayload*	TrailData			= NULL;
			FLOAT*					TaperValues			= NULL;

			TrailTypeData->GetDataPointers(this, (const BYTE*)&Particle, CurrentOffset, TrailData, TaperValues);

			if (TRAIL_EMITTER_IS_START(TrailData->Flags))
			{
				UBOOL	bGotSource	= FALSE;

				FVector LastPosition = SourcePosition(TrailData->TrailIndex);
				FVector Position;

				if (TrailModule_Source)
				{
					Position = CurrentSourcePosition(TrailData->TrailIndex);
					bGotSource	= TRUE;
				}

				if (!bGotSource)
				{
					// Assume it should be taken from the emitter...
					Position	= Component->LocalToWorld.GetOrigin();
				}

				FVector Travelled	= Position - LastPosition;
				FLOAT	Distance	= Travelled.Size();

				SourceDistanceTravelled(TrailData->TrailIndex) += Distance;
				if (Distance > KINDA_SMALL_NUMBER)
				{
					SourcePosition(TrailData->TrailIndex) = Position;
				}
			}
			else
			{
				// Nothing...
			}
		}

		// If not suppressing spawning...
		if (!bSuppressSpawning)
		{
			if ((LODLevel->RequiredModule->EmitterLoops == 0) || 
				(LoopCount < LODLevel->RequiredModule->EmitterLoops) ||
				(SecondsSinceCreation < (EmitterDuration * LODLevel->RequiredModule->EmitterLoops)))
			{
				// For Trails, we probably want to ignore the SpawnRate distribution,
				// and focus strictly on the BurstList...
				FLOAT SpawnRate = 0.0f;
				// Figure out spawn rate for this tick.
				SpawnRate = LODLevel->RequiredModule->SpawnRate.GetValue(EmitterTime, Component);
				
				// Take Bursts into account as well...
				INT		Burst		= 0;
				FLOAT	BurstTime	= GetCurrentBurstRateOffset(DeltaTime, Burst);
				SpawnRate += BurstTime;

				// Spawn new particles...

				//@todo. Fix the issue of 'blanking' Trails when the count drops...
				// This is a temporary hack!
				if ((ActiveParticles < MaxTrailCount) && (SpawnRate <= KINDA_SMALL_NUMBER))
				{
					// Force the spawn of a single Trail...
					SpawnRate = 1.0f / DeltaTime;
				}

				if (TrailModule_Spawn && TrailModule_Spawn->bEnabled)
				{
					INT	SpawnModCount = TrailModule_Spawn->GetSpawnCount(this, DeltaTime);
					INT	MaxParticlesAllowed	= MaxTrailCount * TrailTypeData->MaxParticleInTrailCount;
					if ((SpawnModCount + ActiveParticles) > MaxParticlesAllowed)
					{
						SpawnModCount	= MaxParticlesAllowed - ActiveParticles - 1;
						if (SpawnModCount < 0)
						{
							SpawnModCount = 0;
						}
					}

					if (ActiveParticles >= (TrailTypeData->MaxParticleInTrailCount * MaxTrailCount))
					{
						SpawnModCount = 0;
					}

					if (SpawnModCount)
					{
						//debugf(TEXT("SpawnModCount = %d"), SpawnModCount);
						// Set the burst for this, if there are any...
						SpawnFraction	= 0.0f;
						Burst			= SpawnModCount;
						SpawnRate		= Burst / DeltaTime;
					}
				}
				else
				{
					if ((ActiveParticles > 0) && (SourceDistanceTravelled(0) == 0.0f))
					{
						SpawnRate = 0.0f;
						//debugf(TEXT("Killing SpawnRate (no distance travelled)"));
					}
				}

				if (SpawnRate > 0.f)
				{
					SpawnFraction = Spawn(SpawnFraction, SpawnRate, DeltaTime, Burst, BurstTime);
				}
			}
		}

		// Reset velocity and size.
		for (INT i=0; i<ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			Particle.Velocity		= Particle.BaseVelocity;
			Particle.Size			= Particle.BaseSize;
			Particle.RotationRate	= Particle.BaseRotationRate;
			Particle.RelativeTime += Particle.OneOverMaxLifetime * DeltaTime;

			INC_DWORD_STAT(STAT_TrailParticlesUpdated);
		}

		UParticleModuleTypeDataBase* pkBase = 0;
		if (LODLevel->TypeDataModule)
		{
			pkBase = Cast<UParticleModuleTypeDataBase>(LODLevel->TypeDataModule);
			//@todo. Need to track TypeData offset into payload!
			pkBase->PreUpdate(this, TypeDataOffset, DeltaTime);
		}

		// Update existing particles (might respawn dying ones).
		for (INT i=0; i<LODLevel->UpdateModules.Num(); i++)
		{
			UParticleModule* ParticleModule	= LODLevel->UpdateModules(i);
			if (!ParticleModule || !ParticleModule->bEnabled)
			{
				continue;
			}

			UINT* Offset = ModuleOffsetMap.Find(ParticleModule);
			ParticleModule->Update(this, Offset ? *Offset : 0, DeltaTime);
		}

		//@todo. This should ALWAYS be true for Trails...
		if (pkBase)
		{
			// The order of the update here is VERY important
			if (TrailModule_Source && TrailModule_Source->bEnabled)
			{
				TrailModule_Source->Update(this, TrailModule_Source_Offset, DeltaTime);
			}
			if (TrailModule_Spawn && TrailModule_Spawn->bEnabled)
			{
				TrailModule_Spawn->Update(this, TrailModule_Spawn_Offset, DeltaTime);
			}
			if (TrailModule_Taper && TrailModule_Taper->bEnabled)
			{
				TrailModule_Taper->Update(this, TrailModule_Taper_Offset, DeltaTime);
			}

			//@todo. Need to track TypeData offset into payload!
			pkBase->Update(this, TypeDataOffset, DeltaTime);
			pkBase->PostUpdate(this, TypeDataOffset, DeltaTime);
		}

		// Calculate bounding box and simulate velocity.
		UpdateBoundingBox(DeltaTime);

		//DetermineVertexAndTriangleCount();

		if (!bSuppressSpawning)
		{
			// Ensure that we flip the 'FirstEmission' flag
			FirstEmission = false;
		}

		// Invalidate the contents of the vertex/index buffer.
		IsRenderDataDirty = 1;

		// Bump the tick count
		TickCount++;

		// 'Reset' the emitter time so that the delay functions correctly
		EmitterTime += EmitterDelay;
	}
	INC_DWORD_STAT(STAT_TrailParticlesUpdateCalls);
}

/**
 *	Tick the instance in the editor.
 *	This function will interpolate between the current LODLevels to allow for
 *	the designer to visualize how the selected LOD setting would look.
 *
 *	@param	HighLODLevel		The higher LOD level selected
 *	@param	LowLODLevel			The lower LOD level selected
 *	@param	Multiplier			The interpolation value to use between the two
 *	@param	DeltaTime			The time slice to use
 *	@param	bSuppressSpawning	If TRUE, do not spawn during Tick
 */
void FParticleTrail2EmitterInstance::TickEditor(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, FLOAT Multiplier, FLOAT DeltaTime, UBOOL bSuppressSpawning)
{
	Tick(DeltaTime, bSuppressSpawning);
}

/**
 *	Update the bounding box for the emitter
 *
 *	@param	DeltaTime		The time slice to use
 */
void FParticleTrail2EmitterInstance::UpdateBoundingBox(FLOAT DeltaTime)
{
	if (Component)
	{
		FLOAT MaxSizeScale	= 1.0f;
		ParticleBoundingBox.Init();

		// Handle local space usage
		UParticleLODLevel* LODLevel = SpriteTemplate->GetLODLevel(0);
		check(LODLevel);
		if (LODLevel->RequiredModule->bUseLocalSpace == false) 
		{
			ParticleBoundingBox += Component->LocalToWorld.GetOrigin();
		}

		// Take scale into account
		FVector Scale = FVector(1.0f, 1.0f, 1.0f);
		Scale *= Component->Scale * Component->Scale3D;
		AActor* Actor = Component->GetOwner();
		if (Actor && !Component->AbsoluteScale)
		{
			Scale *= Actor->DrawScale * Actor->DrawScale3D;
		}

		// As well as each particle
		for (INT i=0; i<ActiveParticles; i++)
		{
			DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			
			// Do linear integrator and update bounding box
			Particle->OldLocation = Particle->Location;
			Particle->Location	+= DeltaTime * Particle->Velocity;
			ParticleBoundingBox += Particle->Location;
			FVector Size = Particle->Size * Scale;
			ParticleBoundingBox += Particle->Location + Size;
			ParticleBoundingBox += Particle->Location - Size;

			// Do angular integrator, and wrap result to within +/- 2 PI
			Particle->Rotation	+= DeltaTime * Particle->RotationRate;
			Particle->Rotation	 = appFmod(Particle->Rotation, 2.f*(FLOAT)PI);
			MaxSizeScale		 = Max(MaxSizeScale, Size.GetAbsMax()); //@todo particles: this does a whole lot of compares that can be avoided using SSE/ Altivec.
		}
		ParticleBoundingBox = ParticleBoundingBox.ExpandBy(MaxSizeScale);

		// Transform bounding box into world space if the emitter uses a local space coordinate system.
		if (LODLevel->RequiredModule->bUseLocalSpace) 
		{
			ParticleBoundingBox = ParticleBoundingBox.TransformBy(Component->LocalToWorld);
		}
	}
}

/**
 *	Spawn particles for this instance
 *
 *	@param	OldLeftover		The leftover time from the last spawn
 *	@param	Rate			The rate at which particles should be spawned
 *	@param	DeltaTime		The time slice to spawn over
 *	@param	Burst			The number of burst particle
 *	@param	BurstTime		The burst time addition (faked time slice)
 *
 *	@return	FLOAT			The leftover fraction of spawning
 */
FLOAT FParticleTrail2EmitterInstance::Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst, FLOAT BurstTime)
{
	// If not a trail, get out
	if (!TrailTypeData)
	{
		return OldLeftover;
	}

	FLOAT	NewLeftover;

	SCOPE_CYCLE_COUNTER(STAT_TrailSpawnTime);

	UParticleLODLevel* LODLevel	= SpriteTemplate->GetLODLevel(0);
	check(LODLevel);

	// Ensure continous spawning... lots of fiddling.
	NewLeftover = OldLeftover + DeltaTime * Rate;

	INT		Number		= appFloor(NewLeftover);
	FLOAT	Increment	= 1.f / Rate;
	FLOAT	StartTime	= DeltaTime + OldLeftover * Increment - Increment;
	NewLeftover			= NewLeftover - Number;

	// Always at least match the burst
	if (Number < Burst)
	{
		Number = Burst;
	}

	// Offset burst time
	if (BurstTime > KINDA_SMALL_NUMBER)
	{
		NewLeftover -= BurstTime / Burst;
		NewLeftover	= Clamp<FLOAT>(NewLeftover, 0, NewLeftover);
	}

	// Determine if no particles are alive
	UBOOL bNoLivingParticles = false;
	if (ActiveParticles == 0)
	{
		bNoLivingParticles = true;
		if (Number == 0)
			Number = 1;
	}

	// Spawn for each trail
	if ((Number > 0) && (Number < TrailCount))
	{
		Number	= TrailCount;
	}

	// Don't allow more than TrailCount trails...
	INT	MaxParticlesAllowed	= MaxTrailCount * TrailTypeData->MaxParticleInTrailCount;
	if ((Number + ActiveParticles) > MaxParticlesAllowed)
	{
		Number	= MaxParticlesAllowed - ActiveParticles - 1;
		if (Number < 0)
		{
			Number = 0;
		}
	}

	// Handle growing arrays.
	INT NewCount = ActiveParticles + Number;
	if (NewCount >= MaxActiveParticles)
	{
		if (DeltaTime < 0.25f)
		{
			Resize(NewCount + appTrunc(appSqrt((FLOAT)NewCount)) + 1);
		}
		else
		{
			Resize((NewCount + appTrunc(appSqrt((FLOAT)NewCount)) + 1), FALSE);
		}
	}

	// Spawn particles.
	for (INT i = 0; (i < Number) && (((INT)ActiveParticles + 1) < MaxParticlesAllowed); i++)
	{
		INT		ParticleIndex	= ParticleIndices[ActiveParticles];

		DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndex);

		INT		TempOffset		= TypeDataOffset;

		INT						CurrentOffset		= TypeDataOffset;
		FTrail2TypeDataPayload*	TrailData			= NULL;
		FLOAT*					TaperValues			= NULL;

		TrailTypeData->GetDataPointers(this, (const BYTE*)Particle, CurrentOffset, 
			TrailData, TaperValues);

		FLOAT SpawnTime = StartTime - i * Increment;
	
		PreSpawn(Particle);
		for (INT n=0; n<LODLevel->SpawnModules.Num(); n++)
		{
			UParticleModule* SpawnModule = LODLevel->SpawnModules(n);
			if (!SpawnModule || !SpawnModule->bEnabled)
			{
				continue;
			}

			UINT* Offset = ModuleOffsetMap.Find(SpawnModule);
			SpawnModule->Spawn(this, Offset ? *Offset : 0, SpawnTime);
		}

		if ((1.0f / Particle->OneOverMaxLifetime) < 0.001f)
		{
			Particle->OneOverMaxLifetime = 1.f / 0.001f;
		}

		// The order of the Spawn here is VERY important as the modules may(will) depend on it occuring as such.
		if (TrailModule_Source && TrailModule_Source->bEnabled)
		{
			TrailModule_Source->Spawn(this, TrailModule_Source_Offset, DeltaTime);
		}
		if (TrailModule_Spawn && TrailModule_Spawn->bEnabled)
		{
			TrailModule_Spawn->Spawn(this, TrailModule_Spawn_Offset, DeltaTime);
		}
		if (TrailModule_Taper && TrailModule_Taper->bEnabled)
		{
			TrailModule_Taper->Spawn(this, TrailModule_Taper_Offset, DeltaTime);
		}
		if (LODLevel->TypeDataModule)
		{
			//@todo. Need to track TypeData offset into payload!
			LODLevel->TypeDataModule->Spawn(this, TypeDataOffset, SpawnTime);
		}

		PostSpawn(Particle, 1.f - FLOAT(i+1) / FLOAT(Number), SpawnTime);

		SourceDistanceTravelled(TrailData->TrailIndex) = 0.0f;
		LastSourcePosition(TrailData->TrailIndex)	= SourcePosition(TrailData->TrailIndex);
		SourcePosition(TrailData->TrailIndex) = Particle->Location;

		//debugf(TEXT("TrailEmitter: Spawn with tangent %s"), *(TrailData->Tangent.ToString()));
		FVector	SrcPos			= SourcePosition(TrailData->TrailIndex);
		FVector	LastSrcPos		= LastSourcePosition(TrailData->TrailIndex);
		FVector	CheckTangent	= SrcPos - LastSrcPos;
		CheckTangent.Normalize();
		//debugf(TEXT("TrailEmitter: CheckTangent       %s (%s - %s"), *CheckTangent.ToString(), *SrcPos.ToString(), *LastSrcPos.ToString());

		// Clear the next and previous - just to be safe
		TrailData->Flags = TRAIL_EMITTER_SET_NEXT(TrailData->Flags, TRAIL_EMITTER_NULL_NEXT);
		TrailData->Flags = TRAIL_EMITTER_SET_PREV(TrailData->Flags, TRAIL_EMITTER_NULL_PREV);

		// Set the tangents
		FVector	Dir	= Component->LocalToWorld.GetOrigin() - OldLocation;
		Dir.Normalize();
		TrailData->Tangent	=  Dir;

		UBOOL bAddedParticle = FALSE;
		// Determine which trail to attach to
		if (bNoLivingParticles)
		{
			// These are the first particles!
			// Tag it as the 'only'
			TrailData->Flags = TRAIL_EMITTER_SET_ONLY(TrailData->Flags);
			bNoLivingParticles	= FALSE;
			bAddedParticle		= TRUE;
		}
		else
		{
			INT iNextIndex = TRAIL_EMITTER_NULL_NEXT;
			INT iPrevIndex = TRAIL_EMITTER_NULL_PREV;
	
			// We need to check for existing particles, and 'link up' with them
			for (INT CheckIndex = 0; CheckIndex < ActiveParticles; CheckIndex++)
			{
				// Only care about 'head' particles...
				INT CheckParticleIndex = ParticleIndices[CheckIndex];

				// Don't check the particle of interest...
				// although this should never happen...
				if (ParticleIndex == CheckParticleIndex)
				{
					continue;
				}

				// Grab the particle and its associated trail data
				DECLARE_PARTICLE_PTR(CheckParticle, ParticleData + ParticleStride * CheckParticleIndex);
				
				CurrentOffset		= TypeDataOffset;
				
				FTrail2TypeDataPayload*	CheckTrailData		= NULL;
				FLOAT*					CheckTaperValues	= NULL;

				TrailTypeData->GetDataPointers(this, (const BYTE*)CheckParticle, CurrentOffset, 
					CheckTrailData, CheckTaperValues);

				//@todo. Determine how to handle multiple trails...
				if (TRAIL_EMITTER_IS_ONLY(CheckTrailData->Flags))
				{
					CheckTrailData->Flags	= TRAIL_EMITTER_SET_END(CheckTrailData->Flags);
					CheckTrailData->Flags	= TRAIL_EMITTER_SET_NEXT(CheckTrailData->Flags, TRAIL_EMITTER_NULL_NEXT);
					CheckTrailData->Flags	= TRAIL_EMITTER_SET_PREV(CheckTrailData->Flags, ParticleIndex);

					// Now, 'join' them
					TrailData->Flags		= TRAIL_EMITTER_SET_PREV(TrailData->Flags, TRAIL_EMITTER_NULL_NEXT);
					TrailData->Flags		= TRAIL_EMITTER_SET_NEXT(TrailData->Flags, CheckParticleIndex);
					TrailData->Flags		= TRAIL_EMITTER_SET_START(TrailData->Flags);

					bAddedParticle = TRUE;
					break;
				}
				else
				// ISSUE: How do we determine which 'trail' to join up with????
				if (TRAIL_EMITTER_IS_START(CheckTrailData->Flags))
				{
					check(TRAIL_EMITTER_GET_NEXT(CheckTrailData->Flags) != TRAIL_EMITTER_NULL_NEXT);
					
					CheckTrailData->Flags	= TRAIL_EMITTER_SET_MIDDLE(CheckTrailData->Flags);
					CheckTrailData->Flags	= TRAIL_EMITTER_SET_PREV(CheckTrailData->Flags, ParticleIndex);
					// Now, 'join' them
					TrailData->Flags		= TRAIL_EMITTER_SET_PREV(TrailData->Flags, TRAIL_EMITTER_NULL_NEXT);
					TrailData->Flags		= TRAIL_EMITTER_SET_NEXT(TrailData->Flags, CheckParticleIndex);
					TrailData->Flags		= TRAIL_EMITTER_SET_START(TrailData->Flags);

					//SourceDistanceTravelled(TrailData->TrailIndex) += SourceDistanceTravelled(CheckTrailData->TrailIndex);

					bAddedParticle = TRUE;
					break;
				}
			}
		}

		if (bAddedParticle)
		{
			TrailData->Tangent	= FVector(0.0f);
			ActiveParticles++;

			check((INT)ActiveParticles < TrailTypeData->MaxParticleInTrailCount);

			INC_DWORD_STAT(STAT_TrailParticles);
			INC_DWORD_STAT(STAT_TrailParticlesSpawned);

			LastEmittedParticleIndex = ParticleIndex;
		}
		else
		{
			check(TEXT("Failed to add particle to trail!!!!"));
		}
	}

	if (ForceSpawnCount > 0)
	{
		ForceSpawnCount = 0;
	}

	return NewLeftover;
}

/**
 *	Spawn particles for this instance
 *	Editor version for interpolation
 *
 *	@param	HighLODLevel	The higher LOD level
 *	@param	LowLODLevel		The lower LOD level
 *	@param	Multiplier		The interpolation value to use
 *	@param	OldLeftover		The leftover time from the last spawn
 *	@param	Rate			The rate at which particles should be spawned
 *	@param	DeltaTime		The time slice to spawn over
 *	@param	Burst			The number of burst particle
 *	@param	BurstTime		The burst time addition (faked time slice)
 *
 *	@return	FLOAT			The leftover fraction of spawning
 */
FLOAT FParticleTrail2EmitterInstance::SpawnEditor(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, FLOAT Multiplier, 
	FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, INT Burst, FLOAT BurstTime)
{
	return Spawn(OldLeftover, Rate, DeltaTime, Burst, BurstTime);
}

/**
 *	Handle any pre-spawning actions required for particles
 *
 *	@param	Particle	The particle being spawned.
 */
void FParticleTrail2EmitterInstance::PreSpawn(FBaseParticle* Particle)
{
	FParticleEmitterInstance::PreSpawn(Particle);
	if (TrailTypeData)
	{
		TrailTypeData->PreSpawn(this, Particle);
	}
}

/**
 *	Kill off any dead particles. (Remove them from the active array)
 */
void FParticleTrail2EmitterInstance::KillParticles()
{
	if (ActiveParticles)
	{
		// Loop over the active particles... If their RelativeTime is > 1.0f (indicating they are dead),
		// move them to the 'end' of the active particle list.
		for (INT i = ActiveParticles - 1; i >= 0; i--)
		{
			const INT	CurrentIndex	= ParticleIndices[i];

			DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * CurrentIndex);
			INT						CurrentOffset		= TypeDataOffset;
			FTrail2TypeDataPayload*	TrailData			= NULL;
			FLOAT*					TaperValues			= NULL;

			TrailTypeData->GetDataPointers(this, (const BYTE*)Particle, CurrentOffset, 
				TrailData, TaperValues);

			if (Particle->RelativeTime > 1.0f)
			{
#if defined(_TRAILS_DEBUG_KILL_PARTICLES_)
				debugf(TEXT("Killing Particle %4d - Next = %4d, Prev = %4d, Type = %8s"), 
					CurrentIndex, 
					TRAIL_EMITTER_GET_NEXT(TrailData->Flags),
					TRAIL_EMITTER_GET_PREV(TrailData->Flags),
					TRAIL_EMITTER_IS_ONLY(TrailData->Flags) ? TEXT("ONLY") :
						TRAIL_EMITTER_IS_START(TrailData->Flags) ? TEXT("START") :
							TRAIL_EMITTER_IS_END(TrailData->Flags) ? TEXT("END") :
								TRAIL_EMITTER_IS_MIDDLE(TrailData->Flags) ? TEXT("MIDDLE") :
									TEXT("????")
					);
#endif	//#if defined(_TRAILS_DEBUG_KILL_PARTICLES_)

				if (TRAIL_EMITTER_IS_START(TrailData->Flags) ||
					TRAIL_EMITTER_IS_ONLY(TrailData->Flags))
				{
					// Set the 'next' one in the list to the start
					INT Next = TRAIL_EMITTER_GET_NEXT(TrailData->Flags);
					if (Next != TRAIL_EMITTER_NULL_NEXT)
					{
						DECLARE_PARTICLE_PTR(NextParticle, ParticleData + ParticleStride * Next);

						CurrentOffset		= TypeDataOffset;
						
						FTrail2TypeDataPayload*	NextTrailData	= NULL;
						FLOAT*					NextTaperValues	= NULL;

						TrailTypeData->GetDataPointers(this, (const BYTE*)NextParticle, CurrentOffset, 
							NextTrailData, NextTaperValues);

						if (TRAIL_EMITTER_IS_END(NextTrailData->Flags))
						{
							NextTrailData->Flags = TRAIL_EMITTER_SET_ONLY(NextTrailData->Flags);
							check(TRAIL_EMITTER_GET_NEXT(NextTrailData->Flags) == TRAIL_EMITTER_NULL_NEXT);
						}
						else
						{
							NextTrailData->Flags = TRAIL_EMITTER_SET_START(NextTrailData->Flags);
						}
						NextTrailData->Flags = TRAIL_EMITTER_SET_PREV(NextTrailData->Flags, TRAIL_EMITTER_NULL_PREV);
					}
				}
				else
				if (TRAIL_EMITTER_IS_END(TrailData->Flags))
				{
					// See if there is a 'prev'
					INT Prev = TRAIL_EMITTER_GET_PREV(TrailData->Flags);
					if (Prev != TRAIL_EMITTER_NULL_PREV)
					{
						DECLARE_PARTICLE_PTR(PrevParticle, ParticleData + ParticleStride * Prev);
						CurrentOffset		= TypeDataOffset;
						
						FTrail2TypeDataPayload*	PrevTrailData	= NULL;
						FLOAT*					PrevTaperValues	= NULL;

						TrailTypeData->GetDataPointers(this, (const BYTE*)PrevParticle, CurrentOffset, 
							PrevTrailData, PrevTaperValues);
						if (TRAIL_EMITTER_IS_START(PrevTrailData->Flags))
						{
							PrevTrailData->Flags = TRAIL_EMITTER_SET_ONLY(PrevTrailData->Flags);
						}
						else
						{
							PrevTrailData->Flags = TRAIL_EMITTER_SET_END(PrevTrailData->Flags);
						}
						PrevTrailData->Flags = TRAIL_EMITTER_SET_NEXT(PrevTrailData->Flags, TRAIL_EMITTER_NULL_NEXT);
					}
				}
				else
				if (TRAIL_EMITTER_IS_MIDDLE(TrailData->Flags))
				{
					// Break the trail? Or kill off from here to the end

					INT	Next	= TRAIL_EMITTER_GET_NEXT(TrailData->Flags);
					INT	Prev	= TRAIL_EMITTER_GET_PREV(TrailData->Flags);

#define _TRAIL_KILL_BROKEN_SEGMENT_
#if defined(_TRAIL_KILL_BROKEN_SEGMENT_)
					if (Prev != TRAIL_EMITTER_NULL_PREV)
					{
						DECLARE_PARTICLE_PTR(PrevParticle, ParticleData + ParticleStride * Prev);
						CurrentOffset		= TypeDataOffset;
						
						FTrail2TypeDataPayload*	PrevTrailData	= NULL;
						FLOAT*					PrevTaperValues	= NULL;

						TrailTypeData->GetDataPointers(this, (const BYTE*)PrevParticle, CurrentOffset, 
							PrevTrailData, PrevTaperValues);
						if (TRAIL_EMITTER_IS_START(PrevTrailData->Flags))
						{
							PrevTrailData->Flags = TRAIL_EMITTER_SET_ONLY(PrevTrailData->Flags);
						}
						else
						{
							PrevTrailData->Flags = TRAIL_EMITTER_SET_END(PrevTrailData->Flags);
						}
						PrevTrailData->Flags = TRAIL_EMITTER_SET_NEXT(PrevTrailData->Flags, TRAIL_EMITTER_NULL_NEXT);
					}

					while (Next != TRAIL_EMITTER_NULL_NEXT)
					{
						DECLARE_PARTICLE_PTR(NextParticle, ParticleData + ParticleStride * Next);

						CurrentOffset		= TypeDataOffset;
						
						FTrail2TypeDataPayload*	NextTrailData	= NULL;
						FLOAT*					NextTaperValues	= NULL;

						TrailTypeData->GetDataPointers(this, (const BYTE*)NextParticle, CurrentOffset, 
							NextTrailData, NextTaperValues);

						Next	= TRAIL_EMITTER_GET_NEXT(NextTrailData->Flags);
						TRAIL_EMITTER_SET_FORCEKILL(NextTrailData->Flags);
					}
#else	//#if defined(_TRAIL_KILL_BROKEN_SEGMENT_)
					//@todo. Fill in code to make the broken segment a new trail??
					if (Next != TRAIL_EMITTER_NULL_NEXT)
					{
					}
#endif	//#if defined(_TRAIL_KILL_BROKEN_SEGMENT_)
				}
				else
				if (TRAIL_EMITTER_IS_FORCEKILL(TrailData->Flags))
				{
				}
				else
				{
					check(!TEXT("What the hell are you doing in here?"));
				}

				// Clear it out...
				TrailData->Flags	= TRAIL_EMITTER_SET_NEXT(TrailData->Flags, TRAIL_EMITTER_NULL_NEXT);
				TrailData->Flags	= TRAIL_EMITTER_SET_PREV(TrailData->Flags, TRAIL_EMITTER_NULL_PREV);

				ParticleIndices[i]	= ParticleIndices[ActiveParticles-1];
				ParticleIndices[ActiveParticles-1]	= CurrentIndex;
				ActiveParticles--;

				DEC_DWORD_STAT(STAT_TrailParticles);
				INC_DWORD_STAT(STAT_TrailParticlesKilled);
			}
		}
	}
}

/**
 *	Setup the modules for the trail emitter
 */
void FParticleTrail2EmitterInstance::SetupTrailModules()
{
	// Trails are a special case... 
	// We don't want standard Spawn/Update calls occuring on Trail-type modules.
	UParticleLODLevel* LODLevel = SpriteTemplate->GetLODLevel(0);
	check(LODLevel);
	for (INT ii = 0; ii < LODLevel->Modules.Num(); ii++)
	{
		UParticleModule* CheckModule = LODLevel->Modules(ii);
		if (CheckModule->GetModuleType() == EPMT_Trail)
		{
			UBOOL bRemove = FALSE;

			UINT* Offset;
			if (CheckModule->IsA(UParticleModuleTrailSource::StaticClass()))
			{
				if (TrailModule_Source)
				{
					debugf(TEXT("Warning: Multiple Trail Source modules!"));
				}
				TrailModule_Source	= Cast<UParticleModuleTrailSource>(CheckModule);
				Offset = ModuleOffsetMap.Find(TrailModule_Source);
				if (Offset)
				{
					TrailModule_Source_Offset	= *Offset;
				}
				bRemove	= TRUE;
			}
			else
			if (CheckModule->IsA(UParticleModuleTrailSpawn::StaticClass()))
			{
				if (TrailModule_Spawn)
				{
					debugf(TEXT("Warning: Multiple Trail spawn modules!"));
				}
				TrailModule_Spawn	= Cast<UParticleModuleTrailSpawn>(CheckModule);
				Offset = ModuleOffsetMap.Find(TrailModule_Spawn);
				if (Offset)
				{
					TrailModule_Spawn_Offset	= *Offset;
				}
				bRemove = TRUE;
			}
			else
			if (CheckModule->IsA(UParticleModuleTrailTaper::StaticClass()))
			{
				if (TrailModule_Taper)
				{
					debugf(TEXT("Warning: Multiple Trail taper modules!"));
				}
				TrailModule_Taper	= Cast<UParticleModuleTrailTaper>(CheckModule);
				Offset = ModuleOffsetMap.Find(TrailModule_Taper);
				if (Offset)
				{
					TrailModule_Taper_Offset	= *Offset;
				}
				bRemove = TRUE;
			}

			//@todo. Remove from the Update/Spawn lists???
			if (bRemove)
			{
				for (INT jj = 0; jj < LODLevel->UpdateModules.Num(); jj++)
				{
					if (LODLevel->UpdateModules(jj) == CheckModule)
					{
						LODLevel->UpdateModules.Remove(jj);
						break;
					}
				}

				for (INT kk = 0; kk < LODLevel->SpawnModules.Num(); kk++)
				{
					if (LODLevel->SpawnModules(kk) == CheckModule)
					{
						LODLevel->SpawnModules.Remove(kk);
						break;
					}
				}
			}
		}
	}
}

/**
 *	Resolve the source of the trail
 */
void FParticleTrail2EmitterInstance::ResolveSource()
{
	if (TrailModule_Source)
	{
		if (TrailModule_Source->SourceName != NAME_None)
		{
			switch (TrailModule_Source->SourceMethod)
			{
			case PET2SRCM_Actor:
				if (SourceActor == NULL)
				{
					FParticleSysParam Param;
					for (INT i = 0; i < Component->InstanceParameters.Num(); i++)
					{
						Param = Component->InstanceParameters(i);
						if (Param.Name == TrailModule_Source->SourceName)
						{
							SourceActor = Param.Actor;
							break;
						}
					}

					if (TrailModule_Source->SourceOffsetCount > 0)
					{
						for (INT i = 0; i < Component->InstanceParameters.Num(); i++)
						{
							Param = Component->InstanceParameters(i);
							FString ParamName = Param.Name.ToString();
							TCHAR* TrailSourceOffset	= appStrstr(*ParamName, TEXT("TrailSourceOffset"));
							if (TrailSourceOffset)
							{
								// Parse off the digit
								INT	Index	= appAtoi(TrailSourceOffset);
								if (Index >= 0)
								{
									if (Param.ParamType	== PSPT_Vector)
									{
										SourceOffsets.Insert(Index);
										SourceOffsets(Index)	= Param.Vector;
									}
									else
									if (Param.ParamType == PSPT_Scalar)
									{
										SourceOffsets.InsertZeroed(Index);
										SourceOffsets(Index)	= FVector(Param.Scalar, 0.0f, 0.0f);
									}
								}
							}
						}
					}
				}
				break;
			case PET2SRCM_Particle:
				if (SourceEmitter == NULL)
				{
					for (INT ii = 0; ii < Component->EmitterInstances.Num(); ii++)
					{
						FParticleEmitterInstance* pkEmitInst = Component->EmitterInstances(ii);
						if (pkEmitInst && (pkEmitInst->SpriteTemplate->EmitterName == TrailModule_Source->SourceName))
						{
							SourceEmitter = pkEmitInst;
							break;
						}
					}
				}
				break;
			}
		}
	}
}

/**
 *	Update the source data for the trail
 *
 *	@param	DeltaTime		The time slice to use for the update
 */
void FParticleTrail2EmitterInstance::UpdateSourceData(FLOAT DeltaTime)
{
	FVector	Position = Component->LocalToWorld.GetOrigin();
	FVector	Dir	= Component->LocalToWorld.GetAxis(0);
	if (TrailModule_Source == NULL)
	{
		Dir.Normalize();
	}

	for (INT i = 0; i < ActiveParticles; i++)
	{
		DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[i]);

		INT						CurrentOffset		= TypeDataOffset;
		FTrail2TypeDataPayload*	TrailData			= NULL;
		FLOAT*					TaperValues			= NULL;

		TrailTypeData->GetDataPointers(this, (const BYTE*)Particle, CurrentOffset, TrailData, TaperValues);
		if (TRAIL_EMITTER_IS_START(TrailData->Flags))
		{
			FVector	Tangent;
			if (TrailModule_Source)
			{
				TrailModule_Source->ResolveSourcePoint(this, *Particle, *TrailData, Position, Tangent);
			}
			else
			{
				Tangent		=  Dir;
			}

			//FVector	Delta = Position - CurrentSourcePosition(TrailData->TrailIndex);
#if 0
			FVector	Delta	= CurrentSourcePosition(TrailData->TrailIndex) - LastSourcePosition(TrailData->TrailIndex);
			debugf(TEXT("\tTrail %d (0x%08x) --> %s - Distance = %s (%f) | %s vs %s"), 
				TrailData->TrailIndex, (DWORD)this,
				*Position.ToString(),
				*Delta.ToString(), Delta.Size(),
				*CurrentSourcePosition(TrailData->TrailIndex).ToString(),
				*LastSourcePosition(TrailData->TrailIndex).ToString()
				);
#endif
			CurrentSourcePosition(TrailData->TrailIndex)	= Position;
		}
	}
}

/**
 *	Determine the vertex and triangle counts for the emitter
 */
void FParticleTrail2EmitterInstance::DetermineVertexAndTriangleCount()
{
	UINT	NewSize		= 0;
	INT		TessFactor	= TrailTypeData->TessellationFactor ? TrailTypeData->TessellationFactor : 1;
	INT		Sheets		= TrailTypeData->Sheets ? TrailTypeData->Sheets : 1;
	INT		TrailCount	= 0;
	INT		IndexCount	= 0;
	
	VertexCount		= 0;
	TriangleCount	= 0;

	FVector	TessDistCheck;
	FLOAT	TessRatio;
	INT		SegmentTessFactor;
	INT		CheckParticleCount = 0;

	for (INT ii = 0; ii < ActiveParticles; ii++)
	{
		INT		LocalVertexCount	= 0;
		INT		LocalIndexCount		= 0;

		DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[ii]);

		INT						CurrentOffset		= TypeDataOffset;
		FTrail2TypeDataPayload*	TrailData			= NULL;
		FLOAT*					TaperValues			= NULL;

		TrailTypeData->GetDataPointers(this, (const BYTE*)Particle, CurrentOffset, 
			TrailData, TaperValues);

		FTrail2TypeDataPayload*	StartTrailData	= NULL;

		if (TRAIL_EMITTER_IS_START(TrailData->Flags))
		{
			StartTrailData		 = TrailData;

			// Count the number of particles in this trail
			INT	ParticleCount	 = 1;
			CheckParticleCount++;

			LocalVertexCount	+= 2;
			VertexCount			+= 2;
			LocalIndexCount		+= 2;

			UBOOL	bDone	= FALSE;

#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
			if (TrailTypeData->TessellationFactorDistance <= 0.0f)
#else	//#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
			if (1)
#endif	//#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
			{
				while (!bDone)
				{
					ParticleCount++;
					CheckParticleCount++;

#if !defined(_TRAIL2_TESSELLATE_TO_SOURCE_)
					if (TRAIL_EMITTER_IS_START(TrailData->Flags))
					{
						LocalVertexCount	+= 2 * Sheets;
						VertexCount			+= 2 * Sheets;
						LocalIndexCount		+= 2 * Sheets;
					}
					else
#endif	//#if !defined(_TRAIL2_TESSELLATE_TO_SOURCE_)
					{
						LocalVertexCount	+= 2 * TessFactor * Sheets;
						VertexCount			+= 2 * TessFactor * Sheets;
						LocalIndexCount		+= 2 * TessFactor * Sheets;
					}

					// The end will have Next set to the NULL flag...
					INT	Next	= TRAIL_EMITTER_GET_NEXT(TrailData->Flags);
					if (Next == TRAIL_EMITTER_NULL_NEXT)
					{
						bDone = TRUE;
					}
					else
					{
						DECLARE_PARTICLE_PTR(NextParticle, ParticleData + ParticleStride * Next);

						CurrentOffset		= TypeDataOffset;
						TrailData			= NULL;
						TaperValues			= NULL;

						TrailTypeData->GetDataPointers(this, (const BYTE*)NextParticle, CurrentOffset, 
							TrailData, TaperValues);
					}
				}

				StartTrailData->TriangleCount	= LocalIndexCount - 2;

				// Handle degenerates - 4 tris per stitch
				LocalIndexCount	+= ((Sheets - 1) * 4);

				IndexCount	+= LocalIndexCount;

				TrailCount++;
			}
			else
			{
				while (!bDone)
				{
					SegmentTessFactor	= TessFactor;

					// The end will have Next set to the NULL flag...
					INT	Next	= TRAIL_EMITTER_GET_NEXT(TrailData->Flags);
					if (Next != TRAIL_EMITTER_NULL_NEXT)
					{
						DECLARE_PARTICLE_PTR(NextParticle, ParticleData + ParticleStride * Next);
						if (NextParticle->RelativeTime < 1.0f)
						{
							TessDistCheck		= (Particle->Location - NextParticle->Location);
							TessRatio			= TessDistCheck.Size() / TrailTypeData->TessellationFactorDistance;
							if (TessRatio <= KINDA_SMALL_NUMBER)
							{
								SegmentTessFactor	= 1;
							}
							else
							if (TessRatio < 1.0f)
							{
								SegmentTessFactor	= appTrunc(TessFactor * TessRatio);
							}
						}
						else
						{
							SegmentTessFactor	= 0;
						}
					}

					ParticleCount++;
					CheckParticleCount++;
					LocalVertexCount	+= 2 * SegmentTessFactor * Sheets;
					VertexCount			+= 2 * SegmentTessFactor * Sheets;
					LocalIndexCount		+= 2 * SegmentTessFactor * Sheets;

					// The end will have Next set to the NULL flag...
					if (Next == TRAIL_EMITTER_NULL_NEXT)
					{
						bDone = TRUE;
					}
					else
					{
						DECLARE_PARTICLE_PTR(NParticle, ParticleData + ParticleStride * Next);

						CurrentOffset		= TypeDataOffset;
						TrailData			= NULL;
						TaperValues			= NULL;

						TrailTypeData->GetDataPointers(this, (const BYTE*)NParticle, CurrentOffset, 
							TrailData, TaperValues);
					}
				}

				StartTrailData->TriangleCount	= LocalIndexCount - 2;

				// Handle degenerates - 4 tris per stitch
				LocalIndexCount	+= ((Sheets - 1) * 4);

				IndexCount	+= LocalIndexCount;

				TrailCount++;
			}
		}
	}

	if (TrailCount > 0)
	{
		IndexCount		+= 4 * (TrailCount - 1);	// 4 extra indices per Trail (degenerates)
		TriangleCount	 = IndexCount - 2;
	}
	else
	{
		IndexCount		= 0;
		TriangleCount	= 0;
	}
//	TriangleCount	-= 1;

//#define _TRAILS_DEBUG_VERT_TRI_COUNTS_
#if defined(_TRAILS_DEBUG_VERT_TRI_COUNTS_)
	debugf(TEXT("Trail VertexCount = %3d, TriangleCount = %3d"), VertexCount, TriangleCount);
#endif	//#if defined(_TRAILS_DEBUG_VERT_TRI_COUNTS_)
}

/**
 *	Retrieves the dynamic data for the emitter
 *	
 *	@param	bSelected					Whether the emitter is selected in the editor
 *
 *	@return	FDynamicEmitterDataBase*	The dynamic data, or NULL if it shouldn't be rendered
 */
FDynamicEmitterDataBase* FParticleTrail2EmitterInstance::GetDynamicData(UBOOL bSelected)
{
	// Validity check
	if (!SpriteTemplate)
	{
		return NULL;
	}

	// If the template is disabled, don't return data.
	UParticleLODLevel* LODLevel = SpriteTemplate->GetLODLevel(0);
	if ((LODLevel == NULL) ||
		(LODLevel->RequiredModule->bEnabled == FALSE))
	{
		return NULL;
	}

	DetermineVertexAndTriangleCount();
	if (TriangleCount <= 0)
	{
		if (ActiveParticles > 0)
		{
			warnf(TEXT("TRAIL: GetDynamicData -- TriangleCount == 0 (APC = %4d) for PSys %s"),
				ActiveParticles, 
				Component ? (Component->Template ? *Component->Template->GetName() : 
						TEXT("No Template")) : TEXT("No Component"));
		}
		return NULL;
	}

	// Allocate it for now, but we will want to change this to do some form
	// of caching
	if (ActiveParticles > 0)
	{
		check(MaxActiveParticles >= ActiveParticles);	// TTP #33385

		// Get the material instance. If there is none, or the material isn't flagged for use with particle systems, use the DefaultMaterial.
		UMaterialInstance* MaterialInst = SpriteTemplate->Material;
		if (MaterialInst == NULL || !MaterialInst->UseWithParticleSystem())
		{
			MaterialInst = GEngine->DefaultMaterial;
		}

		FDynamicTrail2EmitterData* NewEmitterData = ::new FDynamicTrail2EmitterData(MaxActiveParticles, ParticleStride, 
			MaterialInst->GetInstanceInterface(bSelected));
		check(NewEmitterData);
		check(NewEmitterData->ParticleData);
		check(NewEmitterData->ParticleIndices);

		// Fill it in...
		INT						ParticleCount	= ActiveParticles;
		UBOOL					bSorted			= FALSE;
		const INT				ScreenAlignment	= SpriteTemplate->ScreenAlignment;

		NewEmitterData->Scale = FVector(1.0f, 1.0f, 1.0f);
		if (Component)
		{
			NewEmitterData->Scale *= Component->Scale * Component->Scale3D;
			AActor* Actor = Component->GetOwner();
			if (Actor && !Component->AbsoluteScale)
			{
				NewEmitterData->Scale *= Actor->DrawScale * Actor->DrawScale3D;
			}
		}

		NewEmitterData->ActiveParticleCount = ActiveParticles;
		NewEmitterData->ScreenAlignment = SpriteTemplate->ScreenAlignment;
		NewEmitterData->bUseLocalSpace = FALSE;
		NewEmitterData->EmitterRenderMode = LODLevel->RequiredModule->EmitterRenderMode;
		NewEmitterData->bLockAxis = FALSE;
		NewEmitterData->TessFactor = TrailTypeData->TessellationFactor ? TrailTypeData->TessellationFactor : 1;
		NewEmitterData->TessStrength = appTrunc(TrailTypeData->TessellationStrength);
		NewEmitterData->TessFactorDistance = TrailTypeData->TessellationFactorDistance;
		NewEmitterData->Sheets = TrailTypeData->Sheets ? TrailTypeData->Sheets : 1;

		INT CheckActiveParticles = ActiveParticles;

		NewEmitterData->VertexCount = VertexCount;
		NewEmitterData->IndexCount = TriangleCount + 2;
		NewEmitterData->PrimitiveCount = TriangleCount;
		NewEmitterData->TrailCount = TrailCount;

		//@todo.SAS. Check for requiring DWORD sized indices?
		NewEmitterData->IndexStride = sizeof(WORD);

		TrailTypeData->GetDataPointerOffsets(this, NULL, TypeDataOffset,
			NewEmitterData->TrailDataOffset, NewEmitterData->TaperValuesOffset);
		NewEmitterData->ParticleSourceOffset = -1;
		if (TrailModule_Source)
		{
			TrailModule_Source->GetDataPointerOffsets(this, NULL, 
				TrailModule_Source_Offset, NewEmitterData->ParticleSourceOffset);
		}

		//@todo. SORTING IS A DIFFERENT ISSUE NOW! 
		//		 GParticleView isn't going to be valid anymore?

		// For trails, we need the complete array of particle data and indices...
		check(NewEmitterData->ParticleData);		// TTP #33385
		appMemcpy(NewEmitterData->ParticleData, ParticleData, MaxActiveParticles * ParticleStride);
		check(NewEmitterData->ParticleIndices);		// TTP #33385
		appMemcpy(NewEmitterData->ParticleIndices, ParticleIndices, MaxActiveParticles * sizeof(WORD));

		//@todo.SAS. Optimize this nonsense...
		INT Index;
		NewEmitterData->TrailSpawnTimes.Empty(TrailSpawnTimes.Num());
		for (Index = 0; Index < TrailSpawnTimes.Num(); Index++)
		{
			NewEmitterData->TrailSpawnTimes.Add(appTrunc(TrailSpawnTimes(Index)));
		}
		NewEmitterData->SourcePosition.Empty(SourcePosition.Num());
		for (Index = 0; Index < SourcePosition.Num(); Index++)
		{
			NewEmitterData->SourcePosition.AddItem(SourcePosition(Index));
		}
		NewEmitterData->LastSourcePosition.Empty(LastSourcePosition.Num());
		for (Index = 0; Index < LastSourcePosition.Num(); Index++)
		{
			NewEmitterData->LastSourcePosition.AddItem(LastSourcePosition(Index));
		}
		NewEmitterData->CurrentSourcePosition.Empty(CurrentSourcePosition.Num());
		for (Index = 0; Index < CurrentSourcePosition.Num(); Index++)
		{
			NewEmitterData->CurrentSourcePosition.AddItem(CurrentSourcePosition(Index));
		}
		NewEmitterData->LastSpawnPosition.Empty(LastSpawnPosition.Num());
		for (Index = 0; Index < LastSpawnPosition.Num(); Index++)
		{
			NewEmitterData->LastSpawnPosition.AddItem(LastSpawnPosition(Index));
		}
		NewEmitterData->LastSpawnTangent.Empty(LastSpawnTangent.Num());
		for (Index = 0; Index < LastSpawnTangent.Num(); Index++)
		{
			NewEmitterData->LastSpawnTangent.AddItem(LastSpawnTangent(Index));
		}
		NewEmitterData->SourceDistanceTravelled.Empty(SourceDistanceTravelled.Num());
		for (Index = 0; Index < SourceDistanceTravelled.Num(); Index++)
		{
			NewEmitterData->SourceDistanceTravelled.AddItem(SourceDistanceTravelled(Index));
		}
		NewEmitterData->SourceOffsets.Empty(SourceOffsets.Num());
		for (Index = 0; Index < SourceOffsets.Num(); Index++)
		{
			NewEmitterData->SourceOffsets.AddItem(SourceOffsets(Index));
		}

		return NewEmitterData;
	}

	return NULL;
}

