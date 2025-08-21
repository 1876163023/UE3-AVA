/*-----------------------------------------------------------------------------
UnParticleLaserComponents.cpp: Particle Laser related code.
Copyright ?2005 Epic Games, Inc. All Rights Reserved.
-----------------------------------------------------------------------------*/
#include "PrecompiledHeaders.h"
#include "avaGame.h"
#include "avaLaser.h"
#include "avaParticleLaserEmitterInstance.h"
#include "LevelUtils.h"

#pragma warning( disable:4018 )

//----------------------------------------------------------------------------
extern FSceneView*			GParticleView;

/** Laser particle stat objects */
/*DECLARE_STATS_GROUP(TEXT("LaserParticles"),STATGROUP_LaserParticles);

DECLARE_DWORD_COUNTER_STAT(TEXT("Laser Particles"),STAT_LaserParticles,STATGROUP_LaserParticles);
DECLARE_DWORD_COUNTER_STAT(TEXT("Laser Ptcl Render Calls"),STAT_LaserParticlesRenderCalls,STATGROUP_LaserParticles);
DECLARE_DWORD_COUNTER_STAT(TEXT("Laser Ptcl Render Calls Completed"),STAT_LaserParticlesRenderCallsCompleted,STATGROUP_LaserParticles);
DECLARE_DWORD_COUNTER_STAT(TEXT("Laser Ptcls Spawned"),STAT_LaserParticlesSpawned,STATGROUP_LaserParticles);
DECLARE_DWORD_COUNTER_STAT(TEXT("Laser Ptcl Update Calls"),STAT_LaserParticlesUpdateCalls,STATGROUP_LaserParticles);
DECLARE_DWORD_COUNTER_STAT(TEXT("Laser Ptcls Updated"),STAT_LaserParticlesUpdated,STATGROUP_LaserParticles);
DECLARE_DWORD_COUNTER_STAT(TEXT("Laser Ptcls Killed"),STAT_LaserParticlesKilled,STATGROUP_LaserParticles);

DECLARE_CYCLE_STAT(TEXT("Laser Spawn Time"),STAT_LaserSpawnTime,STATGROUP_LaserParticles);
DECLARE_CYCLE_STAT(TEXT("Laser VB Pack Time"),STAT_LaserVBPackingTime,STATGROUP_LaserParticles);
DECLARE_CYCLE_STAT(TEXT("Laser IB Pack Time"),STAT_LaserIBPackingTime,STATGROUP_LaserParticles);
DECLARE_CYCLE_STAT(TEXT("Laser Render Time"),STAT_LaserRenderingTime,STATGROUP_LaserParticles);
DECLARE_CYCLE_STAT(TEXT("Laser Res Update Time"),STAT_LaserResourceUpdateTime,STATGROUP_LaserParticles);
DECLARE_CYCLE_STAT(TEXT("Laser Tick Time"),STAT_LaserTickTime,STATGROUP_LaserParticles);
DECLARE_CYCLE_STAT(TEXT("Laser Update Time"),STAT_LaserUpdateTime,STATGROUP_LaserParticles);
DECLARE_CYCLE_STAT(TEXT("Laser PSys Comp Tick Time"),STAT_LaserPSysCompTickTime,STATGROUP_LaserParticles);*/

/*-----------------------------------------------------------------------------
FParticleLaserVertexBuffer implementation.
-----------------------------------------------------------------------------*/
//FParticleLaserVertexBuffer::FParticleLaserVertexBuffer(UavaParticleLaserEmitterInstance* InOwner) :
//FVertexBuffer(),
//Owner(InOwner)
//{
//	// We don't pack the data that is stored in the 'vertex'...
//	Stride	            = sizeof(FParticleSpriteVertex);
//	Size	            = 0;
//	UpdateHint			= RUH_Dynamic;
//}
//
//void FParticleLaserVertexBuffer::SetSize(UINT InNumQuads)
//{
//	if (InNumQuads)
//	{
//		check(Owner);
//		check(Owner->LaserTypeData);
//
//		INT		MaxQuadsPerLaser	= 0;
//		INT		TessFactor		= 1;
//		INT		Frequency		= 1;
//		INT		Sheets			= Owner->LaserTypeData->Sheets ? Owner->LaserTypeData->Sheets : 1;
//
//		TessFactor	= Owner->LaserTypeData->MaxLaserSegments ? Owner->LaserTypeData->MaxLaserSegments : 1;
//		
//		UINT	MaxSegments	= ((TessFactor * Frequency) + 1 + 1);		// Tessellation * Frequency + FinalSegment + FirstEdge;
//		UINT	VertexCount;
//
//		VertexCount	 = InNumQuads;								// Number of Lasers
//		VertexCount	*= MaxSegments;
//		VertexCount	*= Sheets;									// Per-sheet
//		VertexCount *= 2;										// 2 triangles per segment
//
//		UINT	NewSize	= VertexCount * Stride;					// Take stride into account
//
//		if (NewSize > Size)
//		{
//			Size = NewSize;
//		}
//	}
//	else
//	{
//		// Don't re-size to zero...
//		//		Size	= 0;
//	}
//}
//
//void FParticleLaserVertexBuffer::GetData(void* Buffer)
//{
//	//SCOPE_CYCLE_COUNTER(STAT_LaserVBPackingTime);
//
//	FParticleSpriteVertex*			Vertex			= (FParticleSpriteVertex*)Buffer;
//	const INT						ScreenAlignment	= ((UParticleSpriteEmitter*)(Owner->Template))->ScreenAlignment;
//	FMatrix							CameraToWorld	= GParticleView->ViewMatrix.Inverse();
//	UavaParticleModuleTypeDataLaser*	LaserTypeData	= Owner->LaserTypeData;	
//	INT								Sheets			= LaserTypeData->Sheets ? LaserTypeData->Sheets : 1;
//	
//	FVector	ViewOrigin	= CameraToWorld.GetOrigin();	
//
//	{
//		FVector	Offset;
//		FVector	Size;
//
//		FLOAT	fTextureScale = 1.0f;
//
//		for (UINT i = 0; i < Owner->ActiveParticles; i++)
//		{
//			DECLARE_PARTICLE_PTR(Particle, Owner->ParticleData + Owner->ParticleStride * Owner->ParticleIndices[i]);
//
//			INT						CurrentOffset		= Owner->TypeDataOffset;
//			FLaserTypeDataPayload*	LaserData			= NULL;
//			FLaserPoint*				LaserPoints	= NULL;						
//			INT* pnLaserPoints;
//
//			LaserTypeData->GetDataPointers(Owner, (const BYTE*)Particle, CurrentOffset, LaserData, LaserPoints, pnLaserPoints);
//
//			if (LaserData->TriangleCount == 0)
//			{
//				continue;
//			}
//			
//			/*FCheckResult	Hit;
//			AActor* Actor = Owner->Component->GetOwner();
//			if (!Owner->Component->SingleLineCheck(Hit, Actor, LaserData->TargetPoint, LaserData->SourcePoint, TRACE_ProjTargets | TRACE_AllBlocking, FVector(0,0,0)))
//			{
//				LaserData->TargetPoint = Hit.Location;
//			}*/
//
//			if (LaserTypeData->TextureTileDistance > 0.0f)
//			{
//				fTextureScale = (LaserData->SourcePoint - LaserData->TargetPoint).Size() / LaserTypeData->TextureTileDistance;
//			}			
//
//			// Pin the size to the X component
//			Size	= FVector(Particle->Size.X);
//
//			FLOAT	Angle;
//			FQuat	QuatRotator;
//
//			FVector Location;
//			FVector EndPoint;
//			FVector Right;
//			FVector Up;
//			FVector WorkingUp;
//			FLOAT	fU;
//			
//			// For the direct case, this isn't a big deal, as it will not require much work per sheet.
//			for (INT SheetIndex = 0; SheetIndex < Sheets; SheetIndex++)
//			{
//				fU			= LaserPoints[0].Alpha * fTextureScale;
//				Location	= LaserData->SourcePoint;
//				EndPoint	= LaserPoints[(*pnLaserPoints)-1].Location;
//				Right		= Location - EndPoint;
//				Right.Normalize();
//				Up			= Right ^  (Location - ViewOrigin);
//				Up.Normalize();
//
//				if (SheetIndex)
//				{
//					Angle		= ((FLOAT)PI / (FLOAT)Sheets) * SheetIndex;
//					QuatRotator	= FQuat(Right, Angle);
//
//					WorkingUp	= QuatRotator.RotateVector(Up);
//				}
//				else
//				{
//					WorkingUp	= Up;
//				}				
//				
//				Offset.X	= WorkingUp.X * Size.X;
//				Offset.Y	= WorkingUp.Y * Size.Y;
//				Offset.Z	= WorkingUp.Z * Size.Z;
//
//				/*AActor* Actor = Owner->Component->GetOwner();
//				Actor->DrawDebugLine( LaserPoints[0].Location, LaserPoints[1].Location, 255, 255, 0, 0 );*/
//
//				// 'Lead' edge
//				Vertex->Position	= Location + Offset;
//				Vertex->OldPosition	= Location;
//				Vertex->Size		= Size;
//				Vertex->U			= fU;
//				Vertex->V			= 0.0f;
//				Vertex->Rotation	= Particle->Rotation;
//				Vertex->Color		= Particle->Color;
//				Vertex->Color.A		= LaserPoints[0].Intensity;
//				Vertex++;
//
//				Vertex->Position	= Location - Offset;
//				Vertex->OldPosition	= Location;
//				Vertex->Size		= Size;
//				Vertex->U			= fU;
//				Vertex->V			= 1.0f;
//				Vertex->Rotation	= Particle->Rotation;
//				Vertex->Color		= Particle->Color;
//				Vertex->Color.A		= LaserPoints[0].Intensity;
//				Vertex++;
//
//				for (INT StepIndex = 1; StepIndex < *pnLaserPoints; StepIndex++)
//				{
//					fU			= LaserPoints[StepIndex].Alpha * fTextureScale;
//					EndPoint	= LaserPoints[StepIndex].Location;
//					Up			= Right ^  (Location - ViewOrigin);
//					Up.Normalize();
//					if (SheetIndex)
//					{
//						WorkingUp		= QuatRotator.RotateVector(Up);
//					}
//					else
//					{
//						WorkingUp	= Up;
//					}					
//
//					Offset.X		= WorkingUp.X * Size.X;
//					Offset.Y		= WorkingUp.Y * Size.Y;
//					Offset.Z		= WorkingUp.Z * Size.Z;
//
//					//
//					Vertex->Position	= EndPoint + Offset;
//					Vertex->OldPosition	= EndPoint;
//					Vertex->Size		= Size;
//					Vertex->U			= fU;
//					Vertex->V			= 0.0f;
//					Vertex->Rotation	= Particle->Rotation;
//					Vertex->Color		= Particle->Color;
//					Vertex->Color.A		= LaserPoints[StepIndex].Intensity;
//					Vertex++;
//
//					Vertex->Position	= EndPoint - Offset;
//					Vertex->OldPosition	= EndPoint;
//					Vertex->Size		= Size;
//					Vertex->U			= fU;
//					Vertex->V			= 1.0f;
//					Vertex->Rotation	= Particle->Rotation;
//					Vertex->Color		= Particle->Color;
//					Vertex->Color.A		= LaserPoints[StepIndex].Intensity;
//					Vertex++;
//
//					Location			 = EndPoint;					
//				}				
//			}
//		}
//	}
//}
//
//
////=============================================================================
////	FLaserIndexBuffer.
////=============================================================================
//FLaserIndexBuffer::FLaserIndexBuffer(UavaParticleLaserEmitterInstance* InOwner) :
//FQuadIndexBuffer(),
//Owner(InOwner)
//{
//	//
//}
//
//void FLaserIndexBuffer::SetSize(UINT InNumQuads)
//{
//	NumQuads	= InNumQuads;
//	Stride		= InNumQuads > 5000 ? sizeof(DWORD) : sizeof(WORD);
//
//	check(Owner);
//
//	UINT NewSize	= 0;
//	INT Sheets		= Owner->LaserTypeData->Sheets ? Owner->LaserTypeData->Sheets : 1;
//
//	INT		IndexCount	= 0;
//	for (UINT Laser = 0; Laser < Owner->ActiveParticles; Laser++)
//	{
//		DECLARE_PARTICLE_PTR(Particle, Owner->ParticleData + Owner->ParticleStride * Owner->ParticleIndices[Laser]);
//
//		INT						CurrentOffset		= Owner->TypeDataOffset;
//		FLaserTypeDataPayload*	LaserData			= NULL;
//		FLaserPoint*				LaserPoints	= NULL;				
//		INT* pnLaserPoints;
//
//		Owner->LaserTypeData->GetDataPointers(Owner, (const BYTE*)Particle, CurrentOffset, LaserData, LaserPoints, pnLaserPoints);
//
//		if (LaserData->TriangleCount > 0)
//		{
//			IndexCount	+= 2 * Sheets;							// First two indices of each strip
//			IndexCount	+= LaserData->TriangleCount * Sheets;	// 1 index per triangle in the strip PER SHEET
//			IndexCount	+= ((Sheets - 1) * 2);					// 2 extra indices per stitch (degenerates)
//			if ((Laser + 1) < Owner->ActiveParticles)
//			{
//				IndexCount	+= 2;								// 2 extra indices per Laser (degenerates)
//			}
//		}
//	}
//
//	if (IndexCount > 15000)
//	{
//		Stride		= sizeof(DWORD);
//	}
//	NewSize		 = IndexCount * Stride;
//
//	if (Size < NewSize)
//	{
//		Size = NewSize;
//	}
//}
//
//void FLaserIndexBuffer::GetData(void* Buffer)
//{
//	//SCOPE_CYCLE_COUNTER(STAT_LaserIBPackingTime);
//
//	// Laser polygons are packed and joined as follows:
//	//
//	// 1--3--5--7--9-...
//	// |\ |\ |\ |\ |\...
//	// | \| \| \| \| ...
//	// 0--2--4--6--8-...
//	//
//	// (ie, the 'leading' edge of polygon (n) is the trailing edge of polygon (n+1)
//	//
//	// NOTE: This is primed for moving to tri-strips...
//	//
//	check(Owner);
//
//	UavaParticleLaserEmitterInstance*	LaserInst	= Cast<UavaParticleLaserEmitterInstance>(Owner);	
//	UavaParticleModuleTypeDataLaser*	LaserTD		= Owner->LaserTypeData;
//	INT								Sheets		= LaserTD->Sheets ? LaserTD->Sheets : 1;
//	INT								TessFactor	= LaserTD->MaxLaserSegments ? LaserTD->MaxLaserSegments : 1;
//
//	if (Stride == sizeof(WORD))
//	{
//		WORD*	Index		= (WORD*) Buffer;
//		WORD	VertexIndex	= 0;
//
//		for (UINT Laser = 0; Laser < Owner->ActiveParticles; Laser++)
//		{
//			DECLARE_PARTICLE_PTR(Particle, Owner->ParticleData + Owner->ParticleStride * Owner->ParticleIndices[Laser]);
//
//			INT						CurrentOffset		= Owner->TypeDataOffset;
//			FLaserTypeDataPayload*	LaserData			= NULL;
//			FLaserPoint*				LaserPoints	= NULL;						
//
//			INT* pnLaserPoints;
//			Owner->LaserTypeData->GetDataPointers(Owner, (const BYTE*)Particle, CurrentOffset, LaserData, LaserPoints, pnLaserPoints);
//
//			if (LaserData->TriangleCount == 0)
//			{
//				continue;
//			}
//
//			for (INT SheetIndex = 0; SheetIndex < Sheets; SheetIndex++)
//			{
//				INT TrianglesToRender = 0;
//
//				// 2 triangles per tessellation factor
//				TrianglesToRender	= LaserData->TriangleCount;
//
//				// 
//				*(Index++) = VertexIndex++;	// SheetIndex + 0
//				*(Index++) = VertexIndex++;	// SheetIndex + 1
//
//				// Sequentially step through each triangle - 1 vertex per triangle
//				for (INT i=0; i<TrianglesToRender; i++)
//				{
//					*(Index++) = VertexIndex++;
//				}
//
//				// Degenerate tris
//				if ((SheetIndex + 1) < Sheets)
//				{
//					*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
//					*(Index++) = VertexIndex;		// First vertex of the next sheet
//				}
//			}
//			if ((Laser + 1) < Owner->ActiveParticles)
//			{
//				*(Index++) = VertexIndex - 1;
//				*(Index++) = VertexIndex;
//			}
//		}
//	}
//	else
//	{
//		DWORD*	Index		= (DWORD*) Buffer;
//		DWORD	VertexIndex	= 0;
//		for (UINT Laser = 0; Laser < Owner->ActiveParticles; Laser++)
//		{
//			DECLARE_PARTICLE_PTR(Particle, Owner->ParticleData + Owner->ParticleStride * Owner->ParticleIndices[Laser]);
//
//			INT						CurrentOffset		= Owner->TypeDataOffset;
//			FLaserTypeDataPayload*	LaserData			= NULL;
//			FLaserPoint*				LaserPoints	= NULL;						
//
//			INT* pnLaserPoints;
//			Owner->LaserTypeData->GetDataPointers(Owner, (const BYTE*)Particle, CurrentOffset, LaserData, LaserPoints, pnLaserPoints);
//
//			if (LaserData->TriangleCount == 0)
//			{
//				continue;
//			}
//
//			for (INT SheetIndex = 0; SheetIndex < Sheets; SheetIndex++)
//			{
//				INT TrianglesToRender = 0;
//
//				// 2 triangles per tessellation factor
//				TrianglesToRender	= LaserData->TriangleCount;
//
//				// 
//				*(Index++) = VertexIndex++;	// SheetIndex + 0
//				*(Index++) = VertexIndex++;	// SheetIndex + 1
//
//				// Sequentially step through each triangle - 1 vertex per triangle
//				for (INT i=0; i<TrianglesToRender; i++)
//				{
//					*(Index++) = VertexIndex++;
//				}
//
//				// Degenerate tris
//				if ((SheetIndex + 1) < Sheets)
//				{
//					*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
//					*(Index++) = VertexIndex;		// First vertex of the next sheet
//				}
//			}
//			if ((Laser + 1) < Owner->ActiveParticles)
//			{
//				*(Index++) = VertexIndex - 1;
//				*(Index++) = VertexIndex;
//			}
//		}
//	}
//}
//
////=============================================================================
//// ParticleLaserEmitterInstance
////=============================================================================
IMPLEMENT_CLASS(UavaParticleLaserEmitterInstance);
//
//void UavaParticleLaserEmitterInstance::Destroy()
//{
//	// don't execute this code for the the class default object 
//	if ( !IsTemplate(RF_ClassDefaultObject) )
//	{
//		if (VertexBuffer)
//		{
//			delete VertexBuffer;
//			VertexBuffer = 0;
//		}
//		if (IndexBuffer)
//		{
//			delete IndexBuffer;
//			IndexBuffer = 0;
//		}
//		if (VertexFactory)
//		{
//			delete VertexFactory;
//			VertexFactory = 0;
//		}
//	}
//	Super::Destroy();
//}
//
//void UavaParticleLaserEmitterInstance::InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent, UBOOL bClearResources)
//{
//	Super::InitParameters(InTemplate, InComponent, bClearResources);
//	SpriteTemplate	= CastChecked<UParticleSpriteEmitter>(InTemplate);
//	check(SpriteTemplate);
//
//	UParticleLODLevel* LODLevel	= InTemplate->GetLODLevel(0);
//	LaserTypeData	= CastChecked<UavaParticleModuleTypeDataLaser>(LODLevel->TypeDataModule);
//	check(LaserTypeData);
//
//	//@todo. Determine if we need to support local space.
//	if (LODLevel->RequiredModule->bUseLocalSpace)
//	{
//		LODLevel->RequiredModule->bUseLocalSpace	= FALSE;
//	}
//
//	if (LaserTypeData->MaxLaserCount == 0)
//	{
//		LaserTypeData->MaxLaserCount	= 1;
//	}
//
//	LaserCount					= LaserTypeData->MaxLaserCount;
//	FirstEmission				= TRUE;
//	LastEmittedParticleIndex	= -1;
//	TickCount					= 0;
//	ForceSpawnCount				= 0;
//
//	if (bClearResources)
//	{
//		VertexBuffer	= NULL;
//		IndexBuffer		= NULL;
//		VertexFactory	= NULL;
//	}
//}
//
//void UavaParticleLaserEmitterInstance::Init()
//{
//	// Setup the modules prior to initializing...
//	SetupLaserModules();
//
//	FParticleEmitterInstance::Init();
//	if (VertexBuffer == 0)
//	{
//		VertexBuffer = new FParticleLaserVertexBuffer(this);
//	}
//	if (IndexBuffer == 0)
//	{
//		IndexBuffer = new FLaserIndexBuffer(this);
//	}
//	if (VertexFactory == 0)
//	{
//		VertexFactory = new FParticleLaserVertexFactory();
//	}
//
//	VertexFactory->PositionComponent	= FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FParticleSpriteVertex, Position		), VCT_Float3);
//	VertexFactory->OldPositionComponent	= FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FParticleSpriteVertex, OldPosition	), VCT_Float3);
//	VertexFactory->SizeComponent		= FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FParticleSpriteVertex, Size			), VCT_Float3);
//	VertexFactory->UVComponent			= FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FParticleSpriteVertex, U				), VCT_Float2);
//	VertexFactory->RotationComponent	= FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FParticleSpriteVertex, Rotation		), VCT_Float1);
//	VertexFactory->ColorComponent		= FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FParticleSpriteVertex, Color			), VCT_Float4);
//
//	VertexFactory->ScreenAlignment		= SpriteTemplate->ScreenAlignment;
//
//#if XBOX
//	VertexFactory->UpdateHint = RUH_Dynamic;
//#endif
//}
//
//void UavaParticleLaserEmitterInstance::Resize(UINT NewMaxActiveParticles)
//{
//	FParticleEmitterInstance::Resize(NewMaxActiveParticles);
//}
//
//void UavaParticleLaserEmitterInstance::Tick(FLOAT DeltaTime, UBOOL bSuppressSpawning)
//{
//	//SCOPE_CYCLE_COUNTER(STAT_LaserTickTime);
//
//	// Keep track of location for world- space interpolation and other effects.
//	OldLocation	= Location;
//	Location	= Component->LocalToWorld.GetOrigin();
//
//	SecondsSinceCreation += DeltaTime;
//
//	// Update time within emitter loop.
//	EmitterTime = SecondsSinceCreation;
//
//	UParticleLODLevel* LODLevel	= Template->GetLODLevel(0);
//	if (EmitterDuration > KINDA_SMALL_NUMBER)
//	{
//		EmitterTime = appFmod(SecondsSinceCreation, EmitterDuration);
//	}
//
//	FLOAT EmitterDelay = LODLevel->RequiredModule->EmitterDelay;
//
//	if ((SecondsSinceCreation - (EmitterDuration * LoopCount)) >= EmitterDuration)
//	{
//		LoopCount++;
//		ResetBurstList();
//
//		if ((LoopCount == 1) && (LODLevel->RequiredModule->bDelayFirstLoopOnly == TRUE) && 
//			((LODLevel->RequiredModule->EmitterLoops == 0) || (LODLevel->RequiredModule->EmitterLoops > 1)))
//		{
//			// Need to correct the emitter durations...
//			for (INT LODIndex = 0; LODIndex < Template->LODLevels.Num(); LODIndex++)
//			{
//				UParticleLODLevel* TempLOD = Template->LODLevels(LODIndex);
//				EmitterDurations(TempLOD->Level) -= TempLOD->RequiredModule->EmitterDelay;
//			}
//			EmitterDuration		= EmitterDurations(CurrentLODLevelIndex);
//		}
//	}
//
//	if ((LODLevel->RequiredModule->bDelayFirstLoopOnly == TRUE) && (LoopCount > 0))
//	{
//		EmitterDelay = 0;
//	}
//
//	// 'Reset' the emitter time so that the modules function correctly
//	EmitterTime -= EmitterDelay;
//
//	// Kill before the spawn... Otherwise, we can get 'flashing'
//	//@todo. We should do this for ALL emitters...
//	KillParticles();
//
//	// If not suppressing spawning...
//	if (!bSuppressSpawning && (EmitterTime >= 0.0f))
//	{
//		if ((LODLevel->RequiredModule->EmitterLoops == 0) || 
//			(LoopCount < LODLevel->RequiredModule->EmitterLoops) ||
//			(SecondsSinceCreation < (EmitterDuration * LODLevel->RequiredModule->EmitterLoops)))
//		{
//			// For Lasers, we probably want to ignore the SpawnRate distribution,
//			// and focus strictly on the BurstList...
//			FLOAT SpawnRate = 0.0f;
//			// Figure out spawn rate for this tick.
//			SpawnRate = LODLevel->RequiredModule->SpawnRate->GetValue(EmitterTime, Component);
//			// Take Bursts into account as well...
//			INT	Burst		= 0;
//			FLOAT	BurstTime	= GetCurrentBurstRateOffset(DeltaTime, Burst);
//			SpawnRate += BurstTime;
//
//			// Spawn new particles...
//
//			//@todo. Fix the issue of 'blanking' Lasers when the count drops...
//			// This is a temporary hack!
//			if ((ActiveParticles < (UINT)LaserCount) && SpawnRate <= 0.0f)
//			{
//				// Force the spawn of a single Laser...
//				SpawnRate = 1.0f / DeltaTime;
//			}
//
//			// Force Lasers if the emitter is marked "AlwaysOn"
//			if ((ActiveParticles < (UINT)LaserCount) && LaserTypeData->bAlwaysOn)
//			{
//				Burst		= LaserCount;
//				if (DeltaTime > 0.0f)
//				{
//					BurstTime	 = Burst / DeltaTime;
//					SpawnRate	+= BurstTime;
//				}
//			}
//
//			if (SpawnRate > 0.f)
//			{
//				SpawnFraction = Spawn(SpawnFraction, SpawnRate, DeltaTime, Burst, BurstTime);
//			}
//		}
//	}
//
//	// Reset velocity and size.
//	for (UINT i=0; i<ActiveParticles; i++)
//	{
//		DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
//		Particle.Velocity		= Particle.BaseVelocity;
//		Particle.Size			= Particle.BaseSize;
//		Particle.RotationRate	= Particle.BaseRotationRate;
//		Particle.RelativeTime += Particle.OneOverMaxLifetime * DeltaTime;
//
//		//INC_DWORD_STAT(STAT_LaserParticlesUpdated);
//	}
//
//	UParticleModuleTypeDataBase* pkBase = 0;
//	if (LODLevel->TypeDataModule)
//	{
//		pkBase = Cast<UParticleModuleTypeDataBase>(LODLevel->TypeDataModule);
//		//@todo. Need to track TypeData offset into payload!
//		pkBase->PreUpdate(this, TypeDataOffset, DeltaTime);
//	}
//
//	// Update existing particles (might respawn dying ones).
//	for (INT i = 0; i < LODLevel->UpdateModules.Num(); i++)
//	{
//		UParticleModule* ParticleModule	= LODLevel->UpdateModules(i);
//		if (!ParticleModule || !ParticleModule->bEnabled)
//		{
//			continue;
//		}
//		UINT* Offset = ModuleOffsetMap.Find(ParticleModule);
//		ParticleModule->Update(this, Offset ? *Offset : 0, DeltaTime);
//	}
//
//	//@todo. This should ALWAYS be true for Lasers...
//	if (pkBase)
//	{
//		//@todo. Need to track TypeData offset into payload!
//		pkBase->Update(this, TypeDataOffset, DeltaTime);
//		pkBase->PostUpdate(this, TypeDataOffset, DeltaTime);
//	}
//
//	// Calculate bounding box and simulate velocity.
//	UpdateBoundingBox(DeltaTime);
//
//	if (!bSuppressSpawning)
//	{
//		// Ensure that we flip the 'FirstEmission' flag
//		FirstEmission = false;
//	}
//
//	// Invalidate the contents of the vertex/index buffer.
//	IsRenderDataDirty = 1;
//
//	// Bump the tick count
//	TickCount++;
//
//	// 'Reset' the emitter time so that the delay functions correctly
//	EmitterTime += EmitterDelay;
//
//	//INC_DWORD_STAT(STAT_LaserParticlesUpdateCalls);
//}
//
//void UavaParticleLaserEmitterInstance::TickEditor(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, FLOAT Multiplier, FLOAT DeltaTime, UBOOL bSuppressSpawning)
//{
//	Tick(DeltaTime, bSuppressSpawning);
//}
//
//void UavaParticleLaserEmitterInstance::Render(FScene* Scene, const FSceneContext& Context,FPrimitiveRenderInterface* PRI)
//{
//	//INC_DWORD_STAT(STAT_LaserParticlesRenderCalls);
//
//	if (ActiveParticles == 0)
//		return;
//
//	//SCOPE_CYCLE_COUNTER(STAT_LaserRenderingTime);
//
//	//@todo particles: move to PostEditChange
//	GParticleView = Context.View;
//
//	INT		Sheets		= LaserTypeData->Sheets ? LaserTypeData->Sheets : 1;
//
//	UParticleLODLevel* LODLevel	= Template->GetLODLevel(0);
//
//	// Need to determine # tris per Laser...
//	TrianglesToRender	= 0;
//	for (UINT i = 0; i < ActiveParticles; i++)
//	{
//		DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
//
//		INT						CurrentOffset		= TypeDataOffset;
//		FLaserTypeDataPayload*	LaserData			= NULL;
//		FLaserPoint*				LaserPoints	= NULL;				
//
//		INT* pnLaserPoints;
//		LaserTypeData->GetDataPointers(this, (const BYTE*)Particle, CurrentOffset, LaserData, LaserPoints, pnLaserPoints);
//		// Take sheets into account
//		if (LaserData->TriangleCount > 0)
//		{
//			// Stored triangle count is per sheet...
//			TrianglesToRender	+= LaserData->TriangleCount * Sheets;
//			// 4 Degenerates Per Sheet (except for last one)
//			TrianglesToRender	+= (Sheets - 1) * 4;
//			// Multiple Lasers?
//			if (i < (ActiveParticles - 1))
//			{
//				// 4 Degenerates Per Laser (except for last one)
//				TrianglesToRender	+= 4;
//			}
//		}
//	}
//
//	if (TrianglesToRender > 0)
//	{
//		MaxVertexIndex		= TrianglesToRender;
//
//		if (IsRenderDataDirty)
//		{
//			//@todo particles: move to PostEditChange
//			GParticleView = Context.View;
//
//			VertexFactory->ScreenAlignment	= SpriteTemplate->ScreenAlignment;
//			VertexFactory->bLockAxis		= false;
//			VertexBuffer->SetSize(LODLevel->PeakActiveParticles);
//			//IndexBuffer->SetSize(ActiveParticles * (TessFactor + 1) * LaserTypeData->Sheets);
//			IndexBuffer->SetSize(TrianglesToRender / 2);
//
//			{
//				//SCOPE_CYCLE_COUNTER(STAT_LaserResourceUpdateTime);
//				GResourceManager->UpdateResource(VertexBuffer);
//				GResourceManager->UpdateResource(IndexBuffer);
//				GResourceManager->UpdateResource(VertexFactory);
//			}
//
//			IsRenderDataDirty = 0;
//		}
//
//		//INC_DWORD_STAT_BY(STAT_SpriteParticles,ActiveParticles);
//
//		if (LODLevel->RequiredModule->EmitterRenderMode == ERM_Normal)
//		{
//			if (LaserTypeData->RenderGeometry)
//			{
//				// Try to find a color for level coloration.
//				FColor* LevelColor = NULL;
//				if ( Context.View->ShowFlags & SHOW_LevelColoration )
//				{
//					AActor* Owner = Component->GetOwner();
//					if ( Owner )
//					{
//						ULevel* Level = Owner->GetLevel();
//						ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel( Level );
//						if ( LevelStreaming )
//						{
//							LevelColor = &LevelStreaming->DrawColor;
//						}
//					}
//				}
//
//				PRI->DrawMesh(
//					VertexFactory,
//					IndexBuffer,
//					Context.GetRenderMaterial(SpriteTemplate->Material,FColor(255,0,0),Component->IsOwnerSelected(),&Component->Lights,LevelColor),
//					//					Template->UseLocalSpace ? &Component->LocalToWorld : &FMatrix::Identity,
//					//					Template->UseLocalSpace ? &Component->WorldToLocal : &FMatrix::Identity,
//					&FMatrix::Identity,
//					&FMatrix::Identity,
//					&Component->Lights,
//					NULL,
//					NULL,/*AmbientCube*/
//					0,
//					TrianglesToRender,
//					0,
//					MaxVertexIndex, 
//					0, 
//					Component->CastShadow, 
//					MET_TriStrip,
//					(ESceneDepthPriorityGroup)Component->DepthPriorityGroup
//					);
//			}
//		}
//		else
//			if (LODLevel->RequiredModule->EmitterRenderMode == ERM_Point)
//			{
//				RenderDebug(Scene, Context, PRI, false);
//			}
//			else
//				if (LODLevel->RequiredModule->EmitterRenderMode == ERM_Cross)
//				{
//					RenderDebug(Scene, Context, PRI, true);
//				}
//	}
//
//	//INC_DWORD_STAT(STAT_LaserParticlesRenderCallsCompleted);	
//}
//
//
//
//
//void UavaParticleLaserEmitterInstance::UpdateBoundingBox(FLOAT DeltaTime)
//{
//	FLOAT MaxSizeScale	= 1.0f;
//	ParticleBoundingBox.Init();
//	//	if (Template->UseLocalSpace == false) 
//	{
//		ParticleBoundingBox += Component->LocalToWorld.GetOrigin();
//	}
//
//	for (UINT i=0; i<ActiveParticles; i++)
//	{
//		DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
//
//		INT						CurrentOffset		= TypeDataOffset;
//		FLaserTypeDataPayload*	LaserData			= NULL;
//		FLaserPoint*				LaserPoints	= NULL;				
//		INT* pnLaserPoints;
//
//		LaserTypeData->GetDataPointers(this, (const BYTE*)Particle, CurrentOffset, LaserData, LaserPoints, pnLaserPoints);
//
//		// Do linear integrator and update bounding box
//		Particle->OldLocation = Particle->Location;
//		Particle->Location	+= DeltaTime * Particle->Velocity;
//		ParticleBoundingBox += Particle->Location;		
//		ParticleBoundingBox += LaserData->SourcePoint;		
//		ParticleBoundingBox += LaserData->TargetPoint;		
//
//		// Do angular integrator, and wrap result to within +/- 2 PI
//		Particle->Rotation	+= DeltaTime * Particle->RotationRate;
//		Particle->Rotation	 = appFmod(Particle->Rotation, 2.f*(FLOAT)PI);
//		MaxSizeScale		 = Max(MaxSizeScale, Particle->Size.GetAbsMax()); //@todo particles: this does a whole lot of compares that can be avoided using SSE/ Altivec.
//	}
//	ParticleBoundingBox = ParticleBoundingBox.ExpandBy(MaxSizeScale);
//
//	// Transform bounding box into world space if the emitter uses a local space coordinate system.
//	/***
//	if (Template->UseLocalSpace) 
//	{
//	ParticleBoundingBox = ParticleBoundingBox.TransformBy(Component->LocalToWorld);
//	}
//	***/
//}
//
//UINT UavaParticleLaserEmitterInstance::RequiredBytes()
//{
//	UINT uiBytes = FParticleEmitterInstance::RequiredBytes();
//
//	// Flag bits indicating particle 
//	uiBytes += sizeof(INT);
//
//	return uiBytes;
//}
//
//UINT UavaParticleLaserEmitterInstance::CalculateParticleStride(UINT ParticleSize)
//{
//	return ParticleSize;
//}
//
//FLOAT UavaParticleLaserEmitterInstance::Spawn(FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, UINT Burst, FLOAT BurstTime)
//{
//	FLOAT	NewLeftover = OldLeftover + DeltaTime * Rate;
//
//	//SCOPE_CYCLE_COUNTER(STAT_LaserSpawnTime);
//
//	// Ensure continous spawning... lots of fiddling.
//	UINT	Number		= appFloor(NewLeftover);
//	FLOAT	Increment	= 1.f / Rate;
//	FLOAT	StartTime	= DeltaTime + OldLeftover * Increment - Increment;
//	NewLeftover			= NewLeftover - Number;
//
//	if (Number < Burst)
//	{
//		Number = Burst;
//	}
//
//	if (BurstTime > 0.0f)
//	{
//		NewLeftover -= BurstTime / Burst;
//		NewLeftover	= Clamp<FLOAT>(NewLeftover, 0, NewLeftover);
//	}
//
//	UBOOL bNoLivingParticles = false;
//	if (ActiveParticles == 0)
//	{
//		bNoLivingParticles = true;
//		if (Number == 0)
//			Number = 1;
//	}
//
//	// Don't allow more than LaserCount Lasers...
//	if (Number + ActiveParticles > (UINT)LaserCount)
//	{
//		Number	= LaserCount - ActiveParticles;
//	}
//
//	// Handle growing arrays.
//	UINT	NewCount	= ActiveParticles + Number;
//	if (NewCount >= MaxActiveParticles)
//	{
//		Resize(NewCount + appTrunc(appSqrt((FLOAT)NewCount)) + 1);
//	}
//
//	UParticleLODLevel* LODLevel	= Template->GetLODLevel(0);
//
//	// Spawn particles.
//	for (UINT i=0; i<Number; i++)
//	{
//		INT iParticleIndex = ParticleIndices[ActiveParticles];
//
//		DECLARE_PARTICLE_PTR(pkParticle, ParticleData + ParticleStride * iParticleIndex);
//
//		FLOAT SpawnTime = StartTime - i * Increment;
//
//		PreSpawn(pkParticle);
//		for (INT n = 0; n < LODLevel->SpawnModules.Num(); n++)
//		{
//			UParticleModule* SpawnModule	= LODLevel->SpawnModules(n);
//			UINT* Offset = ModuleOffsetMap.Find(SpawnModule);
//			if (SpawnModule->bEnabled)
//			{
//				SpawnModule->Spawn(this, Offset ? *Offset : 0, SpawnTime);
//			}
//		}
//
//		// The order of the Spawn here is VERY important as the modules may(will) depend on it occuring as such.
//		if (LODLevel->TypeDataModule)
//		{
//			//@todo. Need to track TypeData offset into payload!
//			LODLevel->TypeDataModule->Spawn(this, TypeDataOffset, SpawnTime);
//		}
//
//		PostSpawn(pkParticle, 1.f - FLOAT(i+1) / FLOAT(Number), SpawnTime);
//
//		ActiveParticles++;
//
//		//INC_DWORD_STAT(STAT_LaserParticles);
//		//INC_DWORD_STAT(STAT_LaserParticlesSpawned);
//
//		LastEmittedParticleIndex = iParticleIndex;
//	}
//
//	if (ForceSpawnCount > 0)
//	{
//		ForceSpawnCount = 0;
//	}
//
//	return NewLeftover;
//}
//
//FLOAT UavaParticleLaserEmitterInstance::SpawnEditor(UParticleLODLevel* HighLODLevel, UParticleLODLevel* LowLODLevel, FLOAT Multiplier, 
//												 FLOAT OldLeftover, FLOAT Rate, FLOAT DeltaTime, UINT Burst, FLOAT BurstTime)
//{
//	return Spawn(OldLeftover, Rate, DeltaTime, Burst, BurstTime);
//}
//
//void UavaParticleLaserEmitterInstance::PreSpawn(FBaseParticle* Particle)
//{
//	FParticleEmitterInstance::PreSpawn(Particle);
//	if (LaserTypeData)
//	{
//		LaserTypeData->PreSpawn(this, Particle);
//	}
//}
//
//UBOOL UavaParticleLaserEmitterInstance::HasCompleted()
//{
//	return FParticleEmitterInstance::HasCompleted();
//}
//
//void UavaParticleLaserEmitterInstance::PostSpawn(FBaseParticle* Particle, FLOAT InterpolationPercentage, FLOAT SpawnTime)
//{
//	// Do nothing for Laser types...
//	//	FParticleEmitterInstance::PostSpawn(Particle, InterpolationPercentage, SpawnTime);
//}
//
//void UavaParticleLaserEmitterInstance::KillParticles()
//{
//	//	FParticleEmitterInstance::KillParticles();
//	// Loop over the active particles... If their RelativeTime is > 1.0f (indicating they are dead),
//	// move them to the 'end' of the active particle list.
//	for (INT i=ActiveParticles-1; i>=0; i--)
//	{
//		const INT	CurrentIndex	= ParticleIndices[i];
//		const BYTE* ParticleBase	= ParticleData + CurrentIndex * ParticleStride;
//		FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);
//		if (Particle.RelativeTime > 1.0f)
//		{
//			ParticleIndices[i]	= ParticleIndices[ActiveParticles-1];
//			ParticleIndices[ActiveParticles-1]	= CurrentIndex;
//			ActiveParticles--;
//
//			//DEC_DWORD_STAT(STAT_LaserParticles);
//			//INC_DWORD_STAT(STAT_LaserParticlesKilled);
//		}
//	}
//}
//
//void UavaParticleLaserEmitterInstance::SetupLaserModules()
//{
//	// Lasers are a special case... 
//	// We don't want standard Spawn/Update calls occuring on Laser-type modules.
//	UParticleLODLevel* LODLevel	= Template->GetLODLevel(0);
//
//	for (INT ii = 0; ii < LODLevel->Modules.Num(); ii++)
//	{
//		UParticleModule* CheckModule = LODLevel->Modules(ii);
//
//		UBOOL bRemove = FALSE;
//
//		if (!CheckModule) 
//		{
//			bRemove = TRUE;
//
//			LODLevel->Modules.Empty();
//		} 
//		else if (CheckModule->GetModuleType() == EPMT_Beam)
//		{
//		}
//			
//
//		//@todo. Remove from the Update/Spawn lists???
//		if (bRemove)
//		{			
//			for (INT jj = 0; jj < LODLevel->UpdateModules.Num(); jj++)
//			{
//				if (LODLevel->UpdateModules(jj) == CheckModule)
//				{
//					LODLevel->UpdateModules.Remove(jj);
//					break;
//				}
//			}
//
//			for (INT kk = 0; kk < LODLevel->SpawnModules.Num(); kk++)
//			{
//				if (LODLevel->SpawnModules(kk) == CheckModule)
//				{
//					LODLevel->SpawnModules.Remove(kk);
//					break;
//				}
//			}
//		}
//	}
//}


// Native script functions
void UavaParticleLaserEmitterInstance::execSetLaserType(FFrame& Stack, RESULT_DECL)
{
	P_GET_INT(NewMethod);
	P_FINISH;
}

void UavaParticleLaserEmitterInstance::execSetTessellationFactor(FFrame& Stack, RESULT_DECL)
{
	P_GET_FLOAT(NewFactor);
	P_FINISH;
}

void UavaParticleLaserEmitterInstance::execSetDistance(FFrame& Stack, RESULT_DECL)
{
	P_GET_FLOAT(Distance);
	P_FINISH;
}


