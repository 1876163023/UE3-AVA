#include "avaGame.h"

/*-----------------------------------------------------------------------------
UavaParticleModuleAirResistance implementation.
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UavaParticleModuleAirResistance);

void UavaParticleModuleAirResistance::Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)
{
	BEGIN_UPDATE_LOOP;
	// Acceleration should always be in world space...
	FVector BySize = AirResistanceBySize.GetValue(Particle.RelativeTime, Owner->Component);
	FVector ByVelocity = AirResistanceByVelocity.GetValue(Particle.RelativeTime, Owner->Component);

	FLOAT CurrentSize = Particle.Size.Size();	
	
	FLOAT ArBySize = CurrentSize * ( BySize.X + CurrentSize * ( BySize.Y + CurrentSize * BySize.Z ) );

	{
		FLOAT CurrentVelocity = Particle.Velocity.Size();
		FLOAT ArByVelocity = CurrentVelocity * ( ByVelocity.X + CurrentVelocity * ( ByVelocity.Y + CurrentVelocity * ByVelocity.Z ) );

		FLOAT Ar = ArBySize + ArByVelocity / 128;

		FLOAT w = (1 - Ar * DeltaTime / CurrentVelocity);
		if (w < 0.0f) 	Particle.Velocity = FVector( 0.0f, 0.0f, 0.0f );				
		else			Particle.Velocity *= w;
	}

	{
		FLOAT CurrentVelocity = Particle.BaseVelocity.Size();
		FLOAT ArByVelocity = CurrentVelocity * ( ByVelocity.X + CurrentVelocity * ( ByVelocity.Y + CurrentVelocity * ByVelocity.Z ) );

		FLOAT Ar = ArBySize + ArByVelocity / 128;

		FLOAT w = (1 - Ar * DeltaTime / CurrentVelocity);
		if (w < 0.0f) 	Particle.BaseVelocity = FVector( 0.0f, 0.0f, 0.0f );				
		else			Particle.BaseVelocity *= w;
	}	
	
	END_UPDATE_LOOP;
}

void UavaParticleModuleAirResistance::UpdateEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime, UParticleModule* LowerLODModule, FLOAT Multiplier)
{
	UParticleLODLevel*							LODLevel	= Owner->SpriteTemplate->GetCurrentLODLevel(Owner);;
	UavaParticleModuleAirResistance*			LowModule	= Cast<UavaParticleModuleAirResistance>(LowerLODModule);

	BEGIN_UPDATE_LOOP;	

	FVector BySizeA = AirResistanceBySize.GetValue(Particle.RelativeTime, Owner->Component);
	FVector ByVelocityA = AirResistanceByVelocity.GetValue(Particle.RelativeTime, Owner->Component);

	FVector BySizeB = LowModule->AirResistanceBySize.GetValue(Particle.RelativeTime, Owner->Component);
	FVector ByVelocityB = LowModule->AirResistanceByVelocity.GetValue(Particle.RelativeTime, Owner->Component);

	FVector BySize = (BySizeA * Multiplier) +  (BySizeB * (1.0f - Multiplier));
	FVector ByVelocity = (ByVelocityA * Multiplier) +  (ByVelocityB * (1.0f - Multiplier));

	FLOAT CurrentSize = Particle.Size.Size();	

	FLOAT ArBySize = CurrentSize * ( BySize.X + CurrentSize * ( BySize.Y + CurrentSize * BySize.Z ) );

	{
		FLOAT CurrentVelocity = Particle.Velocity.Size();
		FLOAT ArByVelocity = CurrentVelocity * ( ByVelocity.X + CurrentVelocity * ( ByVelocity.Y + CurrentVelocity * ByVelocity.Z ) );

		FLOAT Ar = ArBySize + ArByVelocity / 128;

		FLOAT w = (1 - Ar * DeltaTime / CurrentVelocity);
		if (w < 0.0f) 	Particle.Velocity = FVector( 0.0f, 0.0f, 0.0f );				
		else			Particle.Velocity *= w;
	}

	{
		FLOAT CurrentVelocity = Particle.BaseVelocity.Size();
		FLOAT ArByVelocity = CurrentVelocity * ( ByVelocity.X + CurrentVelocity * ( ByVelocity.Y + CurrentVelocity * ByVelocity.Z ) );

		FLOAT Ar = ArBySize + ArByVelocity / 128;

		FLOAT w = (1 - Ar * DeltaTime / CurrentVelocity);
		if (w < 0.0f) 	Particle.BaseVelocity = FVector( 0.0f, 0.0f, 0.0f );				
		else			Particle.BaseVelocity *= w;
	}
	END_UPDATE_LOOP;
}


/*-----------------------------------------------------------------------------
UavaParticleModuleAirResistance implementation.
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UavaParticleModuleGravity);

void UavaParticleModuleGravity::Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)
{
	BEGIN_UPDATE_LOOP;
	// Acceleration should always be in world space...
	FLOAT Amount = GravityAmount.GetValue(Particle.RelativeTime, Owner->Component);	

	const FMatrix& LocalToWorld = Owner->Component->LocalToWorld;	
	FVector Axis( LocalToWorld.M[0][2], LocalToWorld.M[1][2], LocalToWorld.M[2][2] );

	UParticleLODLevel* LODLevel = Owner->SpriteTemplate->GetCurrentLODLevel(Owner);;
	if (LODLevel->RequiredModule->bUseLocalSpace == FALSE)
	{
		Axis = FVector( 0, 0, 1 );
	}

	Particle.Velocity += (Amount * DeltaTime) * Axis;		
	Particle.BaseVelocity += (Amount * DeltaTime) * Axis;		

	END_UPDATE_LOOP;
}

void UavaParticleModuleGravity::UpdateEditor(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime, UParticleModule* LowerLODModule, FLOAT Multiplier)
{
	UParticleLODLevel*							LODLevel	= Owner->SpriteTemplate->GetCurrentLODLevel(Owner);;
	UavaParticleModuleGravity*					LowModule	= Cast<UavaParticleModuleGravity>(LowerLODModule);

	BEGIN_UPDATE_LOOP;	

	const FMatrix& LocalToWorld = Owner->Component->LocalToWorld;	
	FVector Axis( LocalToWorld.M[0][2], LocalToWorld.M[1][2], LocalToWorld.M[2][2] );

	UParticleLODLevel* LODLevel = Owner->SpriteTemplate->GetCurrentLODLevel(Owner);;
	if (LODLevel->RequiredModule->bUseLocalSpace == FALSE)
	{
		Axis = FVector( 0, 0, 1 );
	}

	FLOAT AmountA = GravityAmount.GetValue(Particle.RelativeTime, Owner->Component);	
	FLOAT AmountB = LowModule->GravityAmount.GetValue(Particle.RelativeTime, Owner->Component);	
	
	Particle.Velocity += (((AmountA * Multiplier) + (AmountB * (1.0f - Multiplier))) * DeltaTime) * Axis;
	Particle.BaseVelocity += (((AmountA * Multiplier) + (AmountB * (1.0f - Multiplier))) * DeltaTime) * Axis;

	END_UPDATE_LOOP;
}


/*----------------------------------------------------------
UavaParticleModuleColorScaleByAmbient implementation.
2007. 04. 25 changmin
-----------------------------------------------------------*/

IMPLEMENT_CLASS(UavaParticleModuleColorScaleByAmbient);
void UavaParticleModuleColorScaleByAmbient::Update(FParticleEmitterInstance *Owner, INT Offset, FLOAT DeltaTime )
{
	extern void BeginFetchIrradiance();
	BeginFetchIrradiance();

	BEGIN_UPDATE_LOOP

	if( GWorld && GWorld->GetWorldInfo() )
	{
		FVector SamplePosition = Particle.Location;

		if( Owner && Owner->SpriteTemplate )
		{
			const FMatrix& LocalToWorld = Owner->Component->LocalToWorld;	
			UParticleLODLevel* LODLevel = Owner->SpriteTemplate->GetCurrentLODLevel(Owner);;
			if (LODLevel->RequiredModule->bUseLocalSpace == TRUE)
			{
				SamplePosition = LocalToWorld.TransformFVector( SamplePosition );
			}
		}

		//FAmbientSH &Ambient = (FAmbientSH&)(GWorld->GetWorldInfo()->GetAmbientCube( SamplePosition ));
		FAmbientSH Ambient;
		GWorld->GetWorldInfo()->InterpolateAmbientCube( SamplePosition, Ambient );

		BYTE *Irradiance = Ambient.Irradiance;
		signed char Exponent = ((signed char)Irradiance[3]);

		FVector IrradianceVec;
		IrradianceVec.X = (FLOAT)Irradiance[0] * pow( 2.0f, Exponent);
		IrradianceVec.Y = (FLOAT)Irradiance[1] * pow( 2.0f, Exponent);
		IrradianceVec.Z = (FLOAT)Irradiance[2] * pow( 2.0f, Exponent);

		FVector IrradianceBaseVec(IrradianceVec);
		IrradianceBaseVec.Normalize();
		IrradianceBaseVec *= AmbientBase;

		IrradianceVec += IrradianceBaseVec;

		Particle.Color.R = Particle.Color.R * IrradianceVec.X + ParticleColorOffset.X;
		Particle.Color.G = Particle.Color.G * IrradianceVec.Y + ParticleColorOffset.Y;
		Particle.Color.B = Particle.Color.B * IrradianceVec.Z + ParticleColorOffset.Z;

	}
	END_UPDATE_LOOP
}

void UavaParticleModuleColorScaleByAmbient::UpdateEditor(FParticleEmitterInstance *Owner, INT Offset, FLOAT DeltaTime, UParticleModule *LowerLODModule, FLOAT Multiplier )
{
	extern void BeginFetchIrradiance();
	BeginFetchIrradiance();

	BEGIN_UPDATE_LOOP

	if( GWorld && GWorld->GetWorldInfo() )
	{
		FVector SamplePosition = Particle.Location;

		if( Owner && Owner->SpriteTemplate )
		{
			const FMatrix& LocalToWorld = Owner->Component->LocalToWorld;	
			UParticleLODLevel* LODLevel = Owner->SpriteTemplate->GetCurrentLODLevel(Owner);;
			if (LODLevel->RequiredModule->bUseLocalSpace == TRUE)
			{
				SamplePosition = LocalToWorld.TransformFVector( SamplePosition );
			}
		}

		//FAmbientSH &Ambient = (FAmbientSH&)(GWorld->GetWorldInfo()->GetAmbientCube( SamplePosition ));
		FAmbientSH Ambient;
		GWorld->GetWorldInfo()->InterpolateAmbientCube( SamplePosition, Ambient );

		BYTE *Irradiance = Ambient.Irradiance;
		signed char Exponent = ((signed char)Irradiance[3]);

		FVector IrradianceVec;
		IrradianceVec.X = (FLOAT)Irradiance[0] * pow( 2.0f, Exponent);
		IrradianceVec.Y = (FLOAT)Irradiance[1] * pow( 2.0f, Exponent);
		IrradianceVec.Z = (FLOAT)Irradiance[2] * pow( 2.0f, Exponent);

		FLOAT Base = AmbientBase;
		FVector ColorOffset(ParticleColorOffset);

		FVector IrradianceBaseVec(IrradianceVec);
		IrradianceBaseVec.Normalize();
		IrradianceBaseVec *= AmbientBase;

		IrradianceVec += IrradianceBaseVec;

		Particle.Color.R = Particle.Color.R * IrradianceVec.X + ParticleColorOffset.X;
		Particle.Color.G = Particle.Color.G * IrradianceVec.Y + ParticleColorOffset.Y;
		Particle.Color.B = Particle.Color.B * IrradianceVec.Z + ParticleColorOffset.Z;
	}

	END_UPDATE_LOOP
}