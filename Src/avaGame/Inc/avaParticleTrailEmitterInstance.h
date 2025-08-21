#pragma once

struct FavaParticleTrailVertexFactory : FParticleVertexFactory
{
	DECLARE_RESOURCE_TYPE(FavaParticleTrailVertexFactory,FParticleVertexFactory);
};

//
//	ParticleTrailEmitterInstance
//
class UavaParticleTrailEmitterInstance : public UParticleEmitterInstance
{
	DECLARE_CLASS(UavaParticleTrailEmitterInstance,UParticleEmitterInstance,CLASS_NoExport,avaGame)

	DECLARE_FUNCTION(execSetTrailType);
	DECLARE_FUNCTION(execSetTessellationFactor);
	DECLARE_FUNCTION(execSetEndPoint);
	DECLARE_FUNCTION(execSetDistance);
	DECLARE_FUNCTION(execSetSourcePoint);
	DECLARE_FUNCTION(execSetSourceTangent);
	DECLARE_FUNCTION(execSetSourceStrength);	

	UParticleSpriteEmitter*				SpriteTemplate;		// Sprite emitter template.
	UavaParticleModuleTypeDataTrail*	TrailTypeData;	

	UBOOL                               FirstEmission;
	INT									LastEmittedParticleIndex;
	INT									TickCount;
	INT									ForceSpawnCount;

	/** The number of live Trails												*/
	INT									TrailCount;	

	FavaParticleTrailVertexBuffer*			VertexBuffer;		// Vertex buffer.
	//	FQuadIndexBuffer*					IndexBuffer;		// Index buffer.
	FavaTrailIndexBuffer*					IndexBuffer;
	FavaParticleTrailVertexFactory*		VertexFactory;		// specific vertex factory

	FVector								TargetPoint;

	virtual void Destroy();
	virtual void InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources = TRUE);
	virtual void Init();
	virtual void Resize(UINT NewMaxActiveParticles);
	virtual void Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning);
	virtual void TickEditor(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, FLOAT Multiplier, FLOAT DeltaTime, UBOOL bSuppressSpawning);
	virtual void Render(FScene* Scene, const FSceneContext& Context,FPrimitiveRenderInterface* PRI);	
	virtual void UpdateBoundingBox(FLOAT DeltaTime);
	virtual UINT RequiredBytes();
	virtual UINT CalculateParticleStride(UINT ParticleSize);
	virtual FLOAT Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, UINT Burst = 0, FLOAT BurstTime = 0.0f);
	virtual FLOAT SpawnEditor(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, FLOAT Multiplier, 
		FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, UINT Burst = 0, FLOAT BurstTime = 0.0f);
	virtual void PreSpawn(FBaseParticle* Particle);
	virtual UBOOL HasCompleted();
	virtual void PostSpawn(FBaseParticle* Particle, FLOAT InterpolationPercentage, FLOAT SpawnTime);
	virtual void KillParticles();
	void SetupTrailModules();	
};