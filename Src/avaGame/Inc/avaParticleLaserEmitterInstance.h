#pragma once

struct FParticleLaserVertexFactory : FParticleVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FParticleLaserVertexFactory);
};

//
//	ParticleLaserEmitterInstance
//
class UavaParticleLaserEmitterInstance : public FParticleEmitterInstance
{
	DECLARE_CLASS(UavaParticleLaserEmitterInstance,FParticleEmitterInstance,CLASS_NoExport,avaGame)

	DECLARE_FUNCTION(execSetLaserType);
	DECLARE_FUNCTION(execSetTessellationFactor);
	DECLARE_FUNCTION(execSetEndPoint);
	DECLARE_FUNCTION(execSetDistance);
	DECLARE_FUNCTION(execSetSourcePoint);
	DECLARE_FUNCTION(execSetSourceTangent);
	DECLARE_FUNCTION(execSetSourceStrength);	

	UParticleSpriteEmitter*				SpriteTemplate;		// Sprite emitter template.
	UavaParticleModuleTypeDataLaser*	LaserTypeData;	

	UBOOL                               FirstEmission;
	INT									LastEmittedParticleIndex;
	INT									TickCount;
	INT									ForceSpawnCount;
		
	/** The number of live Lasers												*/
	INT									LaserCount;	

	FParticleLaserVertexBuffer*			VertexBuffer;		// Vertex buffer.
	//	FQuadIndexBuffer*					IndexBuffer;		// Index buffer.
	FLaserIndexBuffer*					IndexBuffer;
	FParticleLaserVertexFactory*		VertexFactory;		// specific vertex factory

	FVector								TargetPoint;

	//virtual void Destroy();
	//virtual void InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources = TRUE);
	//virtual void Init();
	//virtual void Resize(UINT NewMaxActiveParticles);
	//virtual void Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning);
	//virtual void TickEditor(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, FLOAT Multiplier, FLOAT DeltaTime, UBOOL bSuppressSpawning);
	//virtual void Render(FScene* Scene, const FSceneContext& Context,FPrimitiveRenderInterface* PRI);	
	//virtual void UpdateBoundingBox(FLOAT DeltaTime);
	//virtual UINT RequiredBytes();
	//virtual UINT CalculateParticleStride(UINT ParticleSize);
	//virtual FLOAT Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, UINT Burst = 0, FLOAT BurstTime = 0.0f);
	//virtual FLOAT SpawnEditor(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, FLOAT Multiplier, 
	//	FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, UINT Burst = 0, FLOAT BurstTime = 0.0f);
	//virtual void PreSpawn(FBaseParticle* Particle);
	//virtual UBOOL HasCompleted();
	//virtual void PostSpawn(FBaseParticle* Particle, FLOAT InterpolationPercentage, FLOAT SpawnTime);
	//virtual void KillParticles();
	//void SetupLaserModules();	
};