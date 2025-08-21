#include "PrecompiledHeaders.h"
#include "avaGame.h"
#include "avaLaser.h"
#include "avaParticleLaserEmitterInstance.h"

IMPLEMENT_RESOURCE_TYPE(FParticleLaserVertexFactory);


/*-----------------------------------------------------------------------------
UavaParticleModuleTypeDataLaser implementation.
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UavaParticleModuleTypeDataLaser);

void UavaParticleModuleTypeDataLaser::PreSpawn(FParticleEmitterInstance* Owner, FBaseParticle* Particle)
{
	UParticleSystemComponent*		Component	= Owner->Component;
	UavaParticleLaserEmitterInstance*	LaserInst	= Cast<UavaParticleLaserEmitterInstance>(Owner);

	FLaserTypeDataPayload*	LaserData			= NULL;
	FLaserPoint*				LaserPoints	= NULL;		

	INT						TempOffset	= (INT)LaserInst->TypeDataOffset;
	INT* pnLaserPoints;
	GetDataPointers(Owner, (const BYTE*)Particle, TempOffset, LaserData, LaserPoints, pnLaserPoints);

	appMemzero((void*)LaserData, sizeof(FLaserTypeDataPayload));
}

void UavaParticleModuleTypeDataLaser::Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime)
{
	UavaParticleLaserEmitterInstance*	LaserInst	= Cast<UavaParticleLaserEmitterInstance>(Owner);
	if (!LaserInst)
	{
		return;
	}

	UParticleSystemComponent*		Component	= Owner->Component;

	SPAWN_INIT;

	FLaserTypeDataPayload*	LaserData			= NULL;
	FLaserPoint*				LaserPoints	= NULL;		

	INT						TempOffset	= (INT)CurrentOffset;
	INT* pnLaserPoints;
	GetDataPointers(Owner, ParticleBase, TempOffset, LaserData, LaserPoints, pnLaserPoints);
	CurrentOffset	= TempOffset;

	LaserData->SourcePoint	= Component->LocalToWorld.GetOrigin();	

	// Set the particle target based on the distance
	FLOAT	TotalDistance	= Distance->GetValue(Particle.RelativeTime, Component);
	FVector	Direction		= Component->LocalToWorld.GetAxis(0);
	Direction.Normalize();
	LaserData->TargetPoint	= LaserData->SourcePoint + Direction * TotalDistance;		
}

void UavaParticleModuleTypeDataLaser::PreUpdate(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)
{
}

struct FDust
{
	FDust( const FVector& L, float Intensity, float Alpha )
		: Location(L), Intensity(Intensity), Alpha(Alpha)
	{
	}

	FVector Location;
	float Intensity;
	float Alpha;
	FDust* Next;
};

static void InsertDust( FDust*& RootItem, FDust* NewItem )
{
	FDust* prev = NULL;
	for (FDust* cur = RootItem; cur && cur->Alpha < NewItem->Alpha; cur = cur->Next)
	{
		prev = cur;
	}

	if (!prev)
	{
		NewItem->Next = RootItem;
		RootItem = NewItem;		
	}
	else
	{		
		NewItem->Next = prev->Next;		
		prev->Next = NewItem;
	}
}

void UavaParticleModuleTypeDataLaser::Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)
{
	UParticleSystemComponent*		Component	= Owner->Component;
	UavaParticleLaserEmitterInstance*	LaserInst	= Cast<UavaParticleLaserEmitterInstance>(Owner);
		

	FLOAT	LockRadius	= 1.0f;	

	BEGIN_UPDATE_LOOP;
	{
		FLaserTypeDataPayload*	LaserData			= NULL;
		FLaserPoint*				LaserPoints	= NULL;				

		INT						TempOffset	= (INT)CurrentOffset;
		INT* pnLaserPoints;
		GetDataPointers(Owner, ParticleBase, TempOffset, LaserData, LaserPoints, pnLaserPoints);

		LaserData->SourcePoint	= Component->LocalToWorld.GetOrigin();		

		// Set the particle target based on the distance
		FLOAT	TotalDistance	= Distance->GetValue(Particle.RelativeTime, Component);
		FVector	Direction		= Component->LocalToWorld.GetAxis(0);
		Direction.Normalize();
		LaserData->TargetPoint	= Cast<UavaParticleLaserEmitterInstance>(Owner)->TargetPoint; //LaserData->SourcePoint + Direction * TotalDistance;

		FCheckResult	Hit;
		AActor* Actor = Owner->Component->GetOwner();
		
		*pnLaserPoints = 0;
		
		UBOOL	bLowFreqNoise		= FALSE;

		Particle.Location = LaserData->SourcePoint;		

		// Determine the step size, count, and travelled ratio
		LaserData->Direction		= LaserData->TargetPoint - LaserData->SourcePoint;
		FLOAT	FullMagnitude	= LaserData->Direction.Size();
		LaserData->Direction.Normalize();

		FVector	TrueDistance	= Particle.Location - LaserData->SourcePoint;
		FLOAT	TrueMagnitude	= TrueDistance.Size();
				
		// Readjust the travel ratio		

		float Distance = (LaserData->TargetPoint - LaserData->SourcePoint).Size();

		FVector TraceStart = LaserData->SourcePoint;
		FVector TraceEnd = LaserData->TargetPoint;

		// Get list of hit actors.
		FMemMark Mark(GMem);
		
#define TRACE_Dust 0x80000000

		

		FDust* dusts = NULL;

		FCheckResult* FirstHit = GWorld->MultiLineCheck( GMem, TraceEnd, TraceStart, FVector(0,0,0), TRACE_AllBlocking | TRACE_Dust, Actor );						

		/// Dust Volume의 경우 빙글 빙글~
		while (FirstHit && FirstHit->Actor->IsA( AavaDustVolume::StaticClass()))
		{
			InsertDust( dusts, new(GMem)FDust( FirstHit->Location, Cast<AavaDustVolume>( FirstHit->Actor )->DustIntensity, (FirstHit->Location - LaserData->SourcePoint).Size() / Distance ) );

			FirstHit = FirstHit->GetNext();			
		}

		if (!dusts || dusts->Alpha > 0)
		{
			InsertDust( dusts, new(GMem)FDust( TraceStart, 0.0f, 0 ) );
		}

		/// 어어... 끝까지 가보자!
		if (!FirstHit)
		{
			InsertDust( dusts, new(GMem)FDust( TraceEnd, 0, 1 ) );			
		}
		else /// DustVolume이 아닌 것에 의해 막혔다!
		{
			TraceEnd = FirstHit->Location;

			InsertDust( dusts, new(GMem)FDust( TraceEnd, 0, (FirstHit->Location - LaserData->SourcePoint).Size() / Distance ) );			
		};

		FirstHit = GWorld->MultiLineCheck( GMem, TraceStart, TraceEnd, FVector(0,0,0), TRACE_AllBlocking | TRACE_Dust, Actor );						

		/// Dust Volume의 경우 빙글 빙글~
		while (FirstHit && FirstHit->Actor->IsA( AavaDustVolume::StaticClass()))
		{
			InsertDust( dusts, new(GMem)FDust( FirstHit->Location, -(Cast<AavaDustVolume>( FirstHit->Actor )->DustIntensity), (FirstHit->Location - LaserData->SourcePoint).Size() / Distance ) );

			FirstHit = FirstHit->GetNext();			
		}

		float Intensity = 0.01f;
		for (;dusts;dusts = dusts->Next)
		{
			/// 여기가 바뀌는 부분이니.. -_-;; interpolation해야 한다. -_- 일단은 대강 삽시다:)
			if (dusts->Intensity != 0)
			{				
				LaserPoints[*pnLaserPoints].Location = dusts->Location;
				LaserPoints[*pnLaserPoints].Intensity = Intensity;
				LaserPoints[(*pnLaserPoints)++].Alpha = dusts->Alpha;

				if (*pnLaserPoints == MaxLaserSegments) break;
			}

			Intensity += dusts->Intensity * 10;
			
			LaserPoints[*pnLaserPoints].Location = dusts->Location;
			LaserPoints[*pnLaserPoints].Intensity = Intensity;
			LaserPoints[(*pnLaserPoints)++].Alpha = dusts->Alpha;

			if (*pnLaserPoints == MaxLaserSegments) break;
		}		

		Mark.Pop();				

		LaserData->TriangleCount = (*pnLaserPoints - 1) * 2;		
	}
	END_UPDATE_LOOP;
}

UINT UavaParticleModuleTypeDataLaser::RequiredBytes(FParticleEmitterInstance* Owner)
{
	INT	Size		= 0;	

	UavaParticleLaserEmitterInstance* LaserInst = Cast<UavaParticleLaserEmitterInstance>(Owner);

	Size	+= sizeof(FLaserTypeDataPayload);		// Laser payload data

	// Store the interpolated points for each Laser.
	if (MaxLaserSegments >= 0)
	{
		Size		+= sizeof(FLaserPoint) * MaxLaserSegments;
	}	

	Size += sizeof(INT);
	

	return Size;
}

void UavaParticleModuleTypeDataLaser::SetToSensibleDefaults()
{
}

void UavaParticleModuleTypeDataLaser::PostEditChange(UProperty* PropertyThatChanged)
{
	UParticleSystem* PartSys = CastChecked<UParticleSystem>(GetOuter());
	if (PartSys && PropertyThatChanged)
	{
		PartSys->PostEditChange(PropertyThatChanged);
	}
}

FParticleEmitterInstance* UavaParticleModuleTypeDataLaser::CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent)
{
	SetToSensibleDefaults();
	FParticleEmitterInstance* Instance = ConstructObject<UavaParticleLaserEmitterInstance>(
		UavaParticleLaserEmitterInstance::StaticClass(), InComponent);
	check(Instance);

	Instance->InitParameters(InEmitterParent, InComponent);

	return Instance;
}

void UavaParticleModuleTypeDataLaser::SpawnEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime, UParticleModule* LowerLODLevel, FLOAT Multiplier)
{
	//@todo. Support LOD for Trails??
	Spawn(Owner, Offset, SpawnTime);
}

void UavaParticleModuleTypeDataLaser::UpdateEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime, UParticleModule* LowerLODLevel, FLOAT Multiplier)
{
	//@todo. Support LOD for Trails??
	Update(Owner, Offset, DeltaTime);
}

/** Add all curve-editable Objects within this module to the curve. */
void UavaParticleModuleTypeDataLaser::AddModuleCurvesToEditor(UInterpCurveEdSetup* EdSetup)
{
	//@todo. Once the old members are deprecated, open these functions back up...
	// Until then, any new distributions added to this module will have to be
	// hand-checked for in this function!!!!
	EdSetup->AddCurveToCurrentTab(Distance, FString(TEXT("Distance")), ModuleEditorColor);	
}

void UavaParticleModuleTypeDataLaser::GetDataPointers(FParticleEmitterInstance* Owner, 
												   const BYTE* ParticleBase, INT& CurrentOffset, FLaserTypeDataPayload*& LaserData, 
												   FLaserPoint*& LaserPoints, INT*& nLaserPoints)
{
	UavaParticleLaserEmitterInstance*	LaserInst	= Cast<UavaParticleLaserEmitterInstance>(Owner);	

	PARTICLE_ELEMENT(FLaserTypeDataPayload, Data);
	LaserData	= &Data;

	if (MaxLaserSegments > 0)
	{
		PARTICLE_ELEMENT(FLaserPoint, InterpPoints);
		LaserPoints	 = &InterpPoints; 
		CurrentOffset		+= sizeof(FLaserPoint) * (MaxLaserSegments - 1);
	}	

	PARTICLE_ELEMENT(INT, N);
	nLaserPoints = &N;
}


