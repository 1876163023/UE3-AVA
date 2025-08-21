//=============================================================================
// Copyright 2004-2005 Epic Games - All Rights Reserved.
// Confidential.
//=============================================================================
#include "PrecompiledHeaders.h"
#include "avaGame.h"
//#include "EngineParticleClasses.h"
#include "EngineMaterialClasses.h"
#include "UnPath.h"

// {{ dEAthcURe|HM
#ifdef EnableHostMigration
#include "hmMacro.h"
#endif
// }} dEAthcURe|HM

IMPLEMENT_CLASS(AavaWeapon);
IMPLEMENT_CLASS(AavaWeaponAttachment);
IMPLEMENT_CLASS(AavaWeaponShield);
IMPLEMENT_CLASS(AavaWeaponPickupFactory);
IMPLEMENT_CLASS(AavaProjectile);
IMPLEMENT_CLASS(AavaKProjectile);
IMPLEMENT_CLASS(UavaExplosionLight);
IMPLEMENT_CLASS(UavaGunMuzzleFlashLight);
IMPLEMENT_CLASS(AavaFixedHeavyWeapon);
IMPLEMENT_CLASS(AavaWeap_BaseRifle);
IMPLEMENT_CLASS(UavaLightStickComponent)

// {{ dEAthcURe|HM
#ifdef EnableHostMigration
void AavaWeapon::hmSerialize(FArchive& Ar)
{
	//check 20070207 ammocount 이외의 수치도 HM이 필요한지 확인해보3
	Super::hmSerialize(Ar);

	/* test disable 20070323 검증후삭제할것
	if ( Ar.IsLoading() ) {
	}
	else {
	}

	Ar << AmmoCount;
    //Ar << LockerAmmoCount;
    Ar << MaxAmmoCount;    
	*/
}
#endif
// }} dEAthcURe|HM

void UavaGunMuzzleFlashLight::ResetLight()
{
	LastRenderTime = GWorld->GetWorldInfo()->TimeSeconds;
	LastReallyRenderedTime = -1;	

	if ( !bEnabled)
	{
		bEnabled = TRUE;
		// flag as dirty to guarantee an update this frame
		BeginDeferredReattach();
	}		
}

void UavaGunMuzzleFlashLight::Tick( FLOAT DeltaTime )
{
	Super::Tick(DeltaTime);

	FLOAT CurTime = GWorld->GetWorldInfo()->TimeSeconds;

	if (CurTime != LastRenderTime)
	{
		if (LastReallyRenderedTime<0)
			LastReallyRenderedTime = CurTime;
		else
		{
			if (CurTime > LastReallyRenderedTime)
			{
				if (bEnabled)
				{
					//debugf( NAME_Default, TEXT("TurnOff %f %f %f"), LastRenderTime, LastReallyRenderedTime, CurTime );					
					bEnabled = FALSE;

					BeginDeferredReattach();
				}		
			}
		}		
	}	
}

void UavaExplosionLight::ResetLight()
{
	if ( !bEnabled)
	{
		bEnabled = TRUE;
		// flag as dirty to guarantee an update this frame
		BeginDeferredReattach();
	}

	TimeShiftIndex = 0;
	Lifetime = 0.f;
}

void UavaLightStickComponent::Tick(FLOAT DeltaTime) 
{
	Super::Tick(DeltaTime);
	ElapsedTime += DeltaTime;
	if ( ElapsedTime < IncTime )
	{
		FLOAT			DefaultRadius	=	GetClass()->GetDefaultObject<UavaLightStickComponent>()->TargetRadius;
		FLinearColor	DefaultColor	=	GetClass()->GetDefaultObject<UavaLightStickComponent>()->LightColor;
		Radius = DefaultRadius * ElapsedTime / IncTime;
		LightColor = DefaultColor * ElapsedTime / IncTime;
		BeginDeferredReattach();
	}
	else if ( ElapsedTime >= IncTime && ElapsedTime <= DurationTime )
	{
		Radius		=	GetClass()->GetDefaultObject<UavaLightStickComponent>()->TargetRadius;
		LightColor	=	GetClass()->GetDefaultObject<UavaLightStickComponent>()->LightColor;
		BeginDeferredReattach();
	}
	else if ( ElapsedTime > DurationTime )
	{
		FLOAT T =	FallOffTime - ( ElapsedTime - DurationTime );
		if ( T >= 0.0 )
		{
			FLOAT			DefaultRadius	=	GetClass()->GetDefaultObject<UavaLightStickComponent>()->TargetRadius;
			FLinearColor	DefaultColor	=	GetClass()->GetDefaultObject<UavaLightStickComponent>()->LightColor;
			Radius = DefaultRadius * T / FallOffTime;
			LightColor = DefaultColor * T / FallOffTime;
			BeginDeferredReattach();
		}
		else
		{
			bEnabled = FALSE;
		}
	}
}

/**
* Updates time dependent state for this component.
* Requires bAttached == true.
* @param DeltaTime - The time since the last tick.
*/
void UavaExplosionLight::Tick(FLOAT DeltaTime)
{
	Super::Tick(DeltaTime);

	if ( bCheckFrameRate )
	{
		bCheckFrameRate = FALSE;
		if ( DeltaTime < HighDetailFrameTime )
		{
			// if superhighdetail 
			CastShadows = TRUE;
			if ( TimeShift.Num() > 0 )
				TimeShift(0).Radius *= 1.5f;
		}
	}

	if ( bEnabled )
	{
		if ( TimeShift.Num() <= TimeShiftIndex + 1 )
		{			
			bEnabled = FALSE;
		}
		else
		{
			Lifetime += DeltaTime;
			if ( Lifetime > TimeShift(TimeShiftIndex+1).StartTime )
			{
				TimeShiftIndex++;
				if ( TimeShift.Num() <= TimeShiftIndex + 1 )
				{					
					bEnabled = FALSE;
				}
			}
			if ( bEnabled )
			{
				// fade and color shift
				FLOAT InterpFactor = (Lifetime - TimeShift(TimeShiftIndex).StartTime)/(TimeShift(TimeShiftIndex+1).StartTime - TimeShift(TimeShiftIndex).StartTime);
				Radius = TimeShift(TimeShiftIndex).Radius * (1.f - InterpFactor) + TimeShift(TimeShiftIndex+1).Radius * InterpFactor;
				Brightness = TimeShift(TimeShiftIndex).Brightness * (1.f - InterpFactor) + TimeShift(TimeShiftIndex+1).Brightness * InterpFactor;
				LightColor.R = (BYTE)appTrunc(FLOAT(TimeShift(TimeShiftIndex).LightColor.R) * (1.f - InterpFactor) + FLOAT(TimeShift(TimeShiftIndex+1).LightColor.R) * InterpFactor);
				LightColor.G = (BYTE)appTrunc(FLOAT(TimeShift(TimeShiftIndex).LightColor.G) * (1.f - InterpFactor) + FLOAT(TimeShift(TimeShiftIndex+1).LightColor.G) * InterpFactor);
				LightColor.B = (BYTE)appTrunc(FLOAT(TimeShift(TimeShiftIndex).LightColor.B) * (1.f - InterpFactor) + FLOAT(TimeShift(TimeShiftIndex+1).LightColor.B) * InterpFactor);
				LightColor.A = (BYTE)appTrunc(FLOAT(TimeShift(TimeShiftIndex).LightColor.A) * (1.f - InterpFactor) + FLOAT(TimeShift(TimeShiftIndex+1).LightColor.A) * InterpFactor);
			}
			BeginDeferredReattach();
		}
	}
}

// {{ 20070212 dEAthcURe|HM
#ifdef EnableHostMigration
void AavaKProjectile::hmSerialize(FArchive& Ar)
{
	//Autogenerated code by HmSerializeGenerator/generateSerializer.rb

	Super::hmSerialize(Ar);

	/* test disable 20070323 검증후삭제할것
	if (Ar.IsLoading() ) {
		_hms_defLoading;
		_hms_loadValue(bCheckAutoMessage); // BITFIELD bCheckAutoMessage:1;
		_hms_loadValue(bSuppressExplosionFX); // BITFIELD bSuppressExplosionFX:1;
	}
	else {
		_hms_defSaving;
		_hms_saveValue(bCheckAutoMessage); // BITFIELD bCheckAutoMessage:1;
		_hms_saveValue(bSuppressExplosionFX); // BITFIELD bSuppressExplosionFX:1;
	}

	Ar << FiringWeaponMode; // INT FiringWeaponMode;
	Ar << FiringWeaponStatsID; // INT FiringWeaponStatsID;
	Ar << DamageRadius; // FLOAT DamageRadius;
	Ar << StartImpulse; // FLOAT StartImpulse;
	Ar << DecalWidth; // FLOAT DecalWidth;
	Ar << LastCollisionSoundTime; // FLOAT LastCollisionSoundTime;
	Ar << FiringOwnerStatsID; // INT FiringOwnerStatsID;
	Ar << CollisionIntervalSecs; // FLOAT CollisionIntervalSecs;
	Ar << MomentumTransfer; // FLOAT MomentumTransfer;
	Ar << Damage; // FLOAT Damage;
	Ar << DecalHeight; // FLOAT DecalHeight;
	Ar << MaxEffectDistance; // FLOAT MaxEffectDistance;
	Ar << FullDamageMinRadius; // FLOAT FullDamageMinRadius;
	*/

	//{{other values
	// class UClass* MyDamageType;
	// class AController* InstigatorController;
	// class AActor* ImpactedActor;
	// class AEmitter* ProjExplosion;
	// class USoundCue* AmbientSound;
	// class USoundCue* ExplosionSound;
	// class UParticleSystemComponent* ProjEffects;
	// class UParticleSystem* ProjFlightTemplate;
	// class UParticleSystem* ProjExplosionTemplate;
	// class UMaterialInstance* ExplosionDecal;
	// class USoundCue* CollideSound;
	//}}other values
}
#endif
// }} 20070212 dEAthcURe|HM

FLOAT AavaKProjectile::GetGravityZ()
{
	return Super::GetGravityZ() * 0.5f;
}

void AavaWeapon::TickSpecial(FLOAT DeltaTime)
{
	Super::TickSpecial(DeltaTime);	
}

FVector	AavaWeapon::GetWeaponBob(class AavaPawn* Holder)
{
	static FName DashState( TEXT("WeaponDash") );
	FStateFrame* StateFrame = GetStateFrame();
	if (StateFrame && StateFrame->StateNode && StateFrame->StateNode->GetFName() == DashState )
	{
		return Holder->WeaponBobNative( BobDampingInDash, JumpDamping );
	}
	return Holder->WeaponBobNative( BobDamping, JumpDamping );
}

void AavaWeapon::SetPositionEx(class AavaPawn* Holder)
{
	if ( Instigator == NULL )					return;

	AavaPawn*			  avaPawn = Cast<AavaPawn>( Holder );
	AavaPlayerController* avaPC = Cast<AavaPlayerController>( Holder->Controller );
	if ( avaPC == NULL || avaPC->bBehindView )	return;
	if ( avaPawn->WeaponHand == HAND_Hidden )
	{
		SetHidden( true );
		return;
	}

	SetHidden( false );
	
	FVector	ViewOffset = PlayerViewOffset;
	FVector	DrawOffset(0.f);
	FLOAT	EyeHeight = Holder->IsLocallyControlled() ? Holder->EyeHeight : Holder->BaseEyeHeight;
	if ( Holder->Controller == NULL )
	{
		DrawOffset	=	FRotationMatrix(Rotation).TransformNormal(ViewOffset) + FVector(0,0,EyeHeight);
	}
	else
	{
		DrawOffset.Z = EyeHeight;
		if ( Holder->bWeaponBob )	
			DrawOffset += GetWeaponBob(Holder);

		DrawOffset += FRotationMatrix(avaPC->Rotation).TransformNormal( avaPC->ShakeOffset );
		DrawOffset = DrawOffset + FRotationMatrix(avaPC->Rotation).TransformNormal( ViewOffset );
	}

	SetLocation( avaPawn->Location + DrawOffset );

	FRotator	NewRotation;
	NewRotation			 =	avaPC->Rotation;
	NewRotation.Yaw		+=	avaPawn->PunchAngle.Y * 182.04167f / 2.f;
	NewRotation.Pitch	+=	avaPawn->PunchAngle.X * 182.04167f / 2.f;
	NewRotation.Roll	+=	avaPawn->PunchAngle.Z * 182.04167f / 2.f;
	NewRotation			+=	avaPC->ShakeRot;

	//if ( avaPawn->bWeaponBob )
	//{
	//	// if bWeaponBob, then add some rotation lag
	//	NewRotation.Yaw		= LagRot(NewRotation.Yaw & 0xFFFF, Rotation.Yaw & 0xFFFF, MaxYawLag);
	//	NewRotation.Pitch	= LagRot(NewRotation.Pitch & 0xFFFF, Rotation.Pitch & 0xFFFF, MaxPitchLag);
	//}

	SetRotation( NewRotation );
}

INT AavaWeapon::LagRot( INT NewValue, INT OldValue, INT MaxDiff)
{
	if ( ( Abs(NewValue - OldValue) > 32768 ) ? ( NewValue < OldValue ) : ( NewValue > OldValue ) )
	{
		if ( OldValue > NewValue )
		{
			OldValue -= 65536;
		}
		if ( NewValue - OldValue > MaxDiff )
			OldValue = NewValue - MaxDiff;
	}
	else
	{
		if ( NewValue > OldValue )
		{
			NewValue -= 65536;
		}
		if ( OldValue - NewValue > MaxDiff )
			OldValue = NewValue + MaxDiff;
	}
	return OldValue + RotLagSpeed * (NewValue - OldValue);
}

UBOOL AavaWeaponShield::IgnoreBlockingBy(const AActor* Other) const
{
	return Other->GetAProjectile() != NULL ? false : true;
}

UBOOL AavaWeaponShield::ShouldTrace(UPrimitiveComponent* Primitive, AActor* SourceActor, DWORD TraceFlags)
{
	// FIXME: what about weapon-related traces that don't come from a weapon (bot checking for clear shot, etc) ?
	return (SourceActor != NULL && (SourceActor->GetAProjectile() || SourceActor->IsA(AWeapon::StaticClass()))) ? true : false;
}

//--------------------------------------------------------------


void AavaWeaponPickupFactory::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);
}
//--------------------------------------------------------------


void AavaWeap_BaseGun::TickSpecial(FLOAT DeltaTime)
{
	UDynamicLightEnvironmentComponent*	dynamiclightec;
	FVector								out_location;
	FRotator							out_rotation;
	static const FName					LightDir(TEXT("LightDir"));	
	static const FName					SkyDirection(TEXT("SkyDirection"));
	static const FName					LightColor(TEXT("LightColor"));
	static const FName					UpperColor(TEXT("UpperColor"));
	static const FName					LowerColor(TEXT("LowerColor"));
	if ( SightMode >= 1 && ScopeMIC != NULL && Mesh != NULL && Instigator != NULL && Instigator->Controller != NULL )
	{
		dynamiclightec = Cast<UDynamicLightEnvironmentComponent>( Mesh->LightEnvironment );
		Instigator->Controller->eventGetPlayerViewPoint( out_location, out_rotation );
		FVector last_direction = TransformTest( FVector(0,0,0), out_rotation, dynamiclightec->LastDirection );
		FVector sky_direction  = TransformTest( FVector(0,0,0), out_rotation, FVector(0,0,-1) );
		ScopeMIC->SetVectorParameterValue(LightDir,		FLinearColor(-last_direction.X,last_direction.Y,last_direction.Z,0.0f));
		ScopeMIC->SetVectorParameterValue(SkyDirection,	FLinearColor(-sky_direction.X,sky_direction.Y,sky_direction.Z));
		ScopeMIC->SetVectorParameterValue(LightColor,	FLinearColor(dynamiclightec->LastColor) * dynamiclightec->LastBrightness );
		ScopeMIC->SetVectorParameterValue(UpperColor,	FLinearColor(dynamiclightec->LastUpperSkyColor) * dynamiclightec->LastUpperSkyBrightness );
		ScopeMIC->SetVectorParameterValue(LowerColor,	FLinearColor(dynamiclightec->LastLowerSkyColor) * dynamiclightec->LastLowerSkyBrightness );
	}

	RecalculateAccuracy( DeltaTime );

	if ( fov_target != fov_current )
	{
		fov_transitionElapsedTime += DeltaTime;
		if ( fov_transitionElapsedTime >= fov_transitionTime )
		{
			ChangeFOV( fov_target, 0.0 );		// Change FOV Done
			return;
		}
		fov_current = fov_start + ( fov_target - fov_start ) * fov_transitionElapsedTime / fov_transitionTime;
	}

	FLOAT TempAccuracy	= CalcAccuracyEx( iShotsFired + 1, LastFire == 0 ? 0 : WorldInfo->TimeSeconds - LastFire );
	ClampAccuracyEx( TempAccuracy );
	CurrentSpread	= CalcSpreadEx( TempAccuracy );	

	Super::TickSpecial(	DeltaTime );
}

FVector AavaWeap_BaseGun::GetWeaponBob(class AavaPawn* Holder)
{
	if ( SightMode > 0 )
		return Holder->WeaponBobNative( BobDampingInSight, JumpDamping );
	return Super::GetWeaponBob( Holder );
}

void AavaWeap_BaseGun::ClampAccuracyEx( FLOAT& result )
{
	if ( SightMode > 0 )	
	{
		if ( result > MaxInaccuracyA )
			result = MaxInaccuracyA;
	}
	else
	{
		if ( result > MaxInaccuracy )
			result = MaxInaccuracy;
	}
}

FLOAT AavaWeap_BaseGun::CalcAccuracyEx( int ShotsFired, float DeltaTime )
{
	FLOAT result = 0.0f;

	if ( SightMode > 0 )	
	{
		if (AccuracyDivisorA != 0)	result = ((ShotsFired * ShotsFired * ShotsFired) / AccuracyDivisorA) + AccuracyOffsetA;			
	}
	else
	{
		if (AccuracyDivisor != 0)	result = ((ShotsFired * ShotsFired * ShotsFired) / AccuracyDivisor) + AccuracyOffset;		
	}
	return result;
}

FLOAT AavaWeap_BaseGun::DecayAccuracyEx( FLOAT Acc )
{
	if (SpreadDecayTime > 0)
		return AccumulatedAccuracy * (1 - Min( 1.0f, (WorldInfo->TimeSeconds - AccumulatedAccurayQueuedTime) / SpreadDecayTime ));
	else
		return 0;
}

FLOAT AavaWeap_BaseGun::CalcSpreadEx(float fAccuracy)
{
	FLOAT	SpreadResult = 0.0;
	if ( Instigator == NULL )	return 0.0;

	AavaPawn* avaPawn = Cast<AavaPawn>(Instigator);

	if ( avaPawn == NULL )
	{
		AavaWeaponPawn* WeaponPawn = Cast<AavaWeaponPawn>(Instigator);
		avaPawn = WeaponPawn != NULL ? Cast<AavaPawn>(WeaponPawn->Driver) : NULL;
	}

	if ( avaPawn != NULL )
	{
		if ( SightMode > 0 )	
		{
			if (Instigator->Physics == PHYS_Falling ||
				Instigator->Physics == PHYS_Ladder)		SpreadResult = Spread_WhenFallingA.param1 + Spread_WhenFallingA.param2 * fAccuracy + avaPawn->WeapTypeAdd[WeaponType].SpreadFallingAdd;
			else if( Instigator->Velocity.Size() > 140 )	SpreadResult = Spread_WhenMovingA.param1 + Spread_WhenMovingA.param2 * fAccuracy   + avaPawn->WeapTypeAdd[WeaponType].SpreadMovingAdd;
			else if( Instigator->bIsCrouched )				SpreadResult = Spread_WhenDuckingA.param1 + Spread_WhenDuckingA.param2 * fAccuracy + avaPawn->WeapTypeAdd[WeaponType].SpreadDuckingAdd;
			else											SpreadResult = Spread_WhenSteadyA.param1 + Spread_WhenSteadyA.param2 * fAccuracy   + avaPawn->WeapTypeAdd[WeaponType].SpreadSteadyAdd;
		}
		else
		{
			if (Instigator->Physics == PHYS_Falling ||
				Instigator->Physics == PHYS_Ladder)		SpreadResult = Spread_WhenFalling.param1 + Spread_WhenFalling.param2 * fAccuracy + avaPawn->WeapTypeAdd[WeaponType].SpreadFallingAdd;
			else if( Instigator->Velocity.Size() > 140 )	SpreadResult = Spread_WhenMoving.param1	 + Spread_WhenMoving.param2 * fAccuracy	 + avaPawn->WeapTypeAdd[WeaponType].SpreadMovingAdd;
			else if( Instigator->bIsCrouched )				SpreadResult = Spread_WhenDucking.param1 + Spread_WhenDucking.param2 * fAccuracy + avaPawn->WeapTypeAdd[WeaponType].SpreadDuckingAdd;
			else											SpreadResult = Spread_WhenSteady.param1	 + Spread_WhenSteady.param2 * fAccuracy  + avaPawn->WeapTypeAdd[WeaponType].SpreadSteadyAdd;
		}

		if ( SightMode == 0 )						
		{
			SpreadResult += UnZoomPenalty + avaPawn->WeapTypeAdd[WeaponType].UnZoomSpreadAdd;
			SpreadResult *= UnZoomSpreadAmp;
		}
		else
		{
			SpreadResult += avaPawn->WeapTypeAdd[WeaponType].ZoomSpreadAdd;
			SpreadResult *= ZoomSpreadAmp;
		}
	}

	return Max( SpreadResult, 0.0f );

}

void AavaWeap_BaseGun::RecalculateAccuracy( FLOAT DeltaTime )
{
	FLOAT	ElapsedTime		= WorldInfo->TimeSeconds - LastFire;
	FLOAT	TargetAccuracy	= CalcAccuracyEx(iShotsFired+1,ElapsedTime);
	
	TargetAccuracy += DecayAccuracyEx( AccumulatedAccuracy );

	ClampAccuracyEx( TargetAccuracy );		
	
	TargetAccuracy = CalcSpreadEx( TargetAccuracy ) * CrossHairSpeed;

	FLOAT FinalAccuracy = TargetAccuracy;

	if (iShotsFired)
	{
		const FLOAT Interval = FireInterval(0);

		static FLOAT Size = 0.005f;

		static const FLOAT InvTau = 2.0f;

		FinalAccuracy += Clamp( 1 - appExp( - ElapsedTime / Interval * InvTau ), 0.0f, 1.0f ) * CrossHairSpeed * Size * 2;
	}
	
	if (Inaccuracy < FinalAccuracy)
	{
		Inaccuracy = FinalAccuracy;//Min( FinalAccuracy, Inaccuracy + 8 * DeltaTime );			
	}
	else if (Inaccuracy > FinalAccuracy)
	{
		Inaccuracy = Max( FinalAccuracy, Inaccuracy - DeltaTime );
	}		
}

void AavaWeap_BaseGun::AccumulateAccuracy()
{
	AccumulatedAccuracy = CalcAccuracyEx( iShotsFired, LastFire == 0 ? 0 : WorldInfo->TimeSeconds - LastFire ) + DecayAccuracyEx( AccumulatedAccuracy );

	AccumulatedAccurayQueuedTime = WorldInfo->TimeSeconds;
}

void AavaWeap_BaseGun::InstantFireEx()
{
	iShotsFired ++;

	Accuracy	= CalcAccuracyEx( iShotsFired, LastFire == 0 ? 0 : WorldInfo->TimeSeconds - LastFire ) + DecayAccuracyEx( AccumulatedAccuracy );

	ClampAccuracyEx( Accuracy );

	CurrentSpread	= CalcSpreadEx( Accuracy );	
	LastFire		= WorldInfo->TimeSeconds;	
	LastSpread		= CurrentSpread;

	for (int i=0; i<NumFiresPerShot; ++i)
	{
		eventRifleFire( i );
	}

	ApplyKickback();
}

void AavaWeap_BaseGun::ApplyKickback()
{
	if ( Instigator == NULL )	return;
	if ( SightMode > 0 )	
	{
		/// 멈춘 상태
		if (!Instigator->Velocity.IsNearlyZero())		KickBack( Kickback_WhenMovingA );
		else if (Instigator->Physics == PHYS_Falling ||
				 Instigator->Physics == PHYS_Ladder)	KickBack( Kickback_WhenFallingA );
		else if (Instigator->bIsCrouched)				KickBack( Kickback_WhenDuckingA );
		else											KickBack( Kickback_WhenSteadyA );
	}
	else
	{
		/// 멈춘 상태
		if (!Instigator->Velocity.IsNearlyZero())		KickBack( Kickback_WhenMoving );
		else if (Instigator->Physics == PHYS_Falling ||
				 Instigator->Physics == PHYS_Ladder)	KickBack( Kickback_WhenFalling );
		else if (Instigator->bIsCrouched)				KickBack( Kickback_WhenDucking );
		else											KickBack( Kickback_WhenSteady );
	}
}

FVector AavaWeap_BaseGun::TransformTest(FVector Loc, FRotator Rot,FVector Src)
{
	FMatrix ViewMatrix = FTranslationMatrix(-Loc);
	ViewMatrix = ViewMatrix * FInverseRotationMatrix(Rot);
	ViewMatrix = ViewMatrix * FMatrix(
		FPlane(0,	0,	1,	0),
		FPlane(1,	0,	0,	0),
		FPlane(0,	1,	0,	0),
		FPlane(0,	0,	0,	1));
	return ViewMatrix.TransformNormal( Src );
}

// Trace 를 해서 거리를 산정한다...
// Scope 의 Distance 와 방위각을 바꿔준다...
VOID AavaWeap_Binocular::TestTrace()
{
	static const FName	UV(TEXT("UV"));	
	static const FName	Color(TEXT("Color"));
	static const FName  Active( TEXT("Active") );
	if ( Instigator == NULL || Instigator->Controller == NULL )	return;

	if ( DistanceMIC[0] != NULL && DistanceMIC[1] != NULL && DistanceMIC[2] != NULL )
	{
		FVector ViewPos;	
		ViewPos = Instigator->Location + FVector( 0, 0, Instigator->BaseEyeHeight );
		FCheckResult Hit(1.f);
		GWorld->SingleLineCheck(Hit, Instigator, ViewPos+10000*Instigator->Controller->Rotation.Vector(),ViewPos, TRACE_ProjTargets|TRACE_Tesselation|TRACE_ComplexCollision );
		if ( Hit.Actor != NULL )
		{
			FLOAT	Dist = ( Hit.Location - ViewPos ).Size() / 16.0f * 0.3f;
			Dist		 = PrvDistance + 0.3f * ( Dist - PrvDistance );
			INT		D1,D2;
			D1			 = Dist/100;
			DistanceMIC[0]->SetVectorParameterValue( UV, FLinearColor(D1*0.0625 ,0.5,0.0,0.0) );
			D1			 = INT(Dist)%100;
			D2			 = D1/10;
			DistanceMIC[1]->SetVectorParameterValue( UV, FLinearColor(D2*0.0625 ,0.5,0.0,0.0) );
			D2			 = D1%10;
			DistanceMIC[2]->SetVectorParameterValue( UV, FLinearColor(D2*0.0625 ,0.5,0.0,0.0 ) );
			PrvDistance	 = Dist;
		}
		else
		{
			PrvDistance	=	999.0;
			DistanceMIC[0]->SetVectorParameterValue( UV, FLinearColor( 0.625,0.5,0.0,0.0 ) );
			DistanceMIC[1]->SetVectorParameterValue( UV, FLinearColor( 0.625,0.5,0.0,0.0 ) );
			DistanceMIC[2]->SetVectorParameterValue( UV, FLinearColor( 0.625,0.5,0.0,0.0 ) );
		}
	}

	if ( DistanceMIC != NULL )
		DirectionMIC->SetVectorParameterValue( UV, FLinearColor( 0.125 + Instigator->Rotation.Yaw/65535.0,0.0,0.0,0.0 ) );

	if ( TargetMIC != NULL )
	{
		FStateFrame* StateFrame = GetStateFrame();
		if ( bCanFire && StateFrame && StateFrame->StateNode && StateFrame->StateNode->GetFName() == Active )
			TargetMIC->SetVectorParameterValue( Color, FLinearColor( 0, 1.0, 0, 1.0 ) );
		else
			TargetMIC->SetVectorParameterValue( Color, FLinearColor( 1.0, 0, 0, 1.0 ) );	
	}
}

VOID AavaWeap_Binocular::TickSpecial(FLOAT DeltaTime)
{
	static FName Inactive( TEXT("Inactive") );
	Super::TickSpecial(DeltaTime);
	if ( Instigator == NULL )					return;
	if ( SightMode == 0 )						return;
	if ( !Instigator->IsLocallyControlled() )	return;
	FStateFrame* StateFrame = GetStateFrame();
	if (StateFrame && StateFrame->StateNode)
	{
		if (StateFrame->StateNode->GetFName() != Inactive )
			TestTrace();
	}
}

void AavaWeap_BaseGun::ChangeFOV( FLOAT targetFOV, FLOAT transition_time )
{
	FLOAT	OldFOVTarget;

	OldFOVTarget				= fov_target;
	fov_target					= targetFOV;
	fov_transitionElapsedTime	= 0;
	if ( transition_time != 0.0 )
	{
		if ( ( fov_current > OldFOVTarget && fov_current < fov_target ) || ( fov_current > fov_target && fov_current < OldFOVTarget ) )
			fov_transitionTime		= transition_time;	
		else
			fov_transitionTime		+= transition_time;
	}
	else
	{
		fov_transitionTime		= 0.0;
		fov_current				= fov_target;
	}
	fov_start					= fov_current;
}