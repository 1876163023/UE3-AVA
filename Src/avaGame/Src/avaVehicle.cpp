#include "avaGame.h"
#include "EngineMaterialClasses.h"
#include "UnNet.h"

#define _NO_SYS_GUID_OPERATOR_EQ_

IMPLEMENT_CLASS(AavaVehicle);
IMPLEMENT_CLASS(AavaVehicleBase);
IMPLEMENT_CLASS(AavaVehicleFactory);
IMPLEMENT_CLASS(AavaVehicleWeapon);
IMPLEMENT_CLASS(AavaDestroyedVehicleHitProxy);
IMPLEMENT_CLASS(UavaVehicleSimCar);
IMPLEMENT_CLASS(UavaVehicleSimTank);
IMPLEMENT_CLASS(AavaWeaponPawn);

// test
IMPLEMENT_CLASS(AavaVehicle_Ural);
IMPLEMENT_CLASS(AavaVehicle_Leopard);

#ifndef RAD_TO_DEG
	#define RAD_TO_DEG 57.2957795132
#endif


//-----------------------------------------------------------------------------
//	class AavaWeaponPawn
//-----------------------------------------------------------------------------
void AavaWeaponPawn::TickSpecial( FLOAT DeltaSeconds )
{
	if (Controller)
	{
		// Don't Do anything if the pawn is in fixed view
		AavaPawn* const DriverPawn = Cast<AavaPawn>(Driver);

		if ( DriverPawn && !DriverPawn->bFixedView )
		{
			AavaPlayerController* const PC = Cast<AavaPlayerController>(Controller);
			if (PC)// && !PC->bDebugFreeCam)
			{
				FRotator Rot = Controller->Rotation;
				MyVehicle->ApplyWeaponRotation(MySeatIndex, Rot);
			}
		}
	}
}

AVehicle* AavaWeaponPawn::GetVehicleBase()
{
	return MyVehicle;
}


//-----------------------------------------------------------------------------
//	class AavaVehicle
//-----------------------------------------------------------------------------

FVector AavaVehicle::GetDampingForce(const FVector& InForce)
{
	checkSlow(AirSpeed > 0.f );

	FVector DampedVelocity = Velocity;
	// perhaps don't damp downward z velocity if vehicle isn't touching ground
	DampedVelocity.Z = (bNoZDamping || (bNoZDampingInAir && !HasWheelsOnGround())) ? 0.f : DampedVelocity.Z;

	return InForce.Size() * ::Min(DampedVelocity.SizeSquared()/Square(1.03f*AirSpeed), 2.f) * DampedVelocity.SafeNormal();
}

void AavaVehicle::RequestTrackingFor(AavaBot *Bot)
{
	Trackers.AddItem(Bot);
}

void AavaVehicle::TickSpecial( FLOAT DeltaSeconds )
{
	Super::TickSpecial(DeltaSeconds);

	// Reset frontal collision flags.
	bFrontalCollision = FALSE;
	bFrontalCollisionWithFixed = FALSE;

	// use appropriate physical material depending on whether being driven

	if ( WorldInfo->NetMode != NM_DedicatedServer )
	{
/*
		if ( bDeadVehicle )
		{
			if (bIsBurning && BurnOutMaterialInstances.Num() > 0)
			{
				RemainingBurn += DeltaSeconds * 10.f/BurnOutTime;
				if ( RemainingBurn < 10.f )
				{
					for (INT i = 0; i < BurnOutMaterialInstances.Num(); i++)
					{
						if (BurnOutMaterialInstances(i) != NULL)
						{
							BurnOutMaterialInstances(i)->SetScalarParameterValue(BurnTimeParameterName, RemainingBurn); 
							if (RemainingBurn > 7.5f)
							{
								BurnOutMaterialInstances(i)->SetScalarParameterValue(BurnValueParameterName, 20.f * (10.f - RemainingBurn) / 2.5f); 
							}
						}
					}
				}
				else
				{
					bHidden = true;
				}
			}
			return;
		}
*/

		// deal with wheel sounds/effects
		if (bVehicleOnGround) // on the ground, with tire sounds, moving anywhere but straight up/down
		{
			if (TireAudioComp != NULL || WheelParticleEffects.Num() > 0)
			{
				FLOAT curSpd = Velocity.Size2D();

				// while moving:
				if(curSpd > 1.0f) // if we're moving or we're on the gas.
				{
					FCheckResult HitRes(1.0f);
					FTraceHitInfo HitInfo;
					FVector TraceStart(Location.X,Location.Y,Location.Z);
					if (CylinderComponent != NULL)
					{
						TraceStart.Z -= CylinderComponent->CollisionHeight - CylinderComponent->Translation.Z;
					}
					FVector EndPt(TraceStart.X,TraceStart.Y,TraceStart.Z-32);  // got these numbers from UTPawn's GetMaterialBelowFeet()

					GWorld->SingleLineCheck(HitRes, this, EndPt, TraceStart, TRACE_World | TRACE_Material);
/*
					DetermineCorrectPhysicalMaterial<FCheckResult, FTraceHitInfo>( HitRes, HitInfo );
					// we now have a phys material so we can see if we need to update the sound
					HitInfo.Material = HitRes.Material ? HitRes.Material->GetMaterial() : NULL;
					UavaPhysicalMaterialProperty* avaPMP = NULL;
					if (HitInfo.Material && HitInfo.Material->PhysMaterial)
					{
						avaPMP = Cast<UavaPhysicalMaterialProperty>(HitInfo.Material->PhysMaterial->PhysicalMaterialProperty);
						if (TireAudioComp != NULL && avaPMP != NULL && avaPMP->MaterialType != CurrentTireMaterial) // if we're on a material that's not what we're on already.
						{
							INT match = -1;
							for(INT i=0;i<TireSoundList.Num();++i)
							{
								if(TireSoundList(i).MaterialType == avaPMP->MaterialType)
								{
									match = i;
									CurrentTireMaterial = avaPMP->MaterialType;
									break;
								}
							}
							if(match != -1)
							{
								if(TireAudioComp->bWasPlaying) // we have a new match, so fade out the old one and let the garbage collector take care of it.
								{
									TireAudioComp->FadeOut(0.3f,0.0f);
								}
								TireAudioComp = CreateAudioComponent(TireSoundList(match).Sound, FALSE, TRUE, FALSE);
							}
						}
					}

					if (WheelParticleEffects.Num() > 0 && HitRes.Time < 1.0f)
					{
						// figure out the material type, then check any wheels requesting material specific effects and change any that are different
						FName MaterialType = NAME_None;
						// if the trace endpoint is underwater, override with 'water' type
						FMemMark Mark(GMem);
						UBOOL bNowInWater = FALSE;
						for (FCheckResult* Link = GWorld->Hash->ActorPointCheck(GMem, HitRes.Location, FVector(0.f,0.f,0.f), TRACE_PhysicsVolumes); Link != NULL; Link = Link->GetNext())
						{
							APhysicsVolume* Volume = Cast<APhysicsVolume>(Link->Actor);
							if (Volume != NULL && Volume->bWaterVolume)
							{
								bNowInWater = TRUE;
								break;
							}
						}
						Mark.Pop();

						if (bNowInWater)
						{
							MaterialType = FName(TEXT("Water"));
						}
						else if (avaPMP != NULL)
						{
							MaterialType = avaPMP->MaterialType;
						}
						INT EffectIndex = 0;
						if (MaterialType != NAME_None)
						{
							for (INT i = 0; i < WheelParticleEffects.Num(); i++)
							{
								if (WheelParticleEffects(i).MaterialType == MaterialType)
								{
									EffectIndex = i;
									break;
								}
							}
						}

//						for (INT i = 0; i < Wheels.Num(); i++)
//						{
//							UavaVehicleWheel* Wheel = Cast<UavaVehicleWheel>(Wheels(i));
//							if ( Wheel != NULL && Wheel->bUseMaterialSpecificEffects && Wheel->WheelParticleComp != NULL
//								&& Wheel->WheelParticleComp->Template != WheelParticleEffects(EffectIndex).ParticleTemplate )
//							{
//								Wheel->eventSetParticleEffect(this, WheelParticleEffects(EffectIndex).ParticleTemplate);
//							}
//						}
					}
*/
					if (TireAudioComp != NULL)
					{
						if(!TireAudioComp->bWasPlaying)
						{
							TireAudioComp->Play();
						}
						TireAudioComp->AdjustVolume(0.1f, Min<float>(1.0,curSpd/(AirSpeed*0.10f)) ); // go to full volume if >10%, else to the % of 10%
						TireAudioComp->PitchMultiplier = 0.5f + 1.25f*(curSpd/AirSpeed); // 0 = 0.5, 40% = 1.0, 80% = 1.5
					}
				}
				else if (TireAudioComp != NULL) // not moving, stop tires.
				{
					TireAudioComp->Stop();//TireAudioComp->FadeOu(1.0f,0.0f);
				}
			}
		}

		// toggle any wheel effects that only want to be played when the wheel is moving a certain direction
//		for (INT i = 0; i < Wheels.Num(); i++)
//		{
//			UavaVehicleWheel* Wheel = Cast<UavaVehicleWheel>(Wheels(i));
//			if (Wheel != NULL && Wheel->WheelParticleComp != NULL && Wheel->EffectDesiredSpinDir != 0.0f)
//			{
//				UBOOL bActivate = (Wheel->SpinVel / Wheel->EffectDesiredSpinDir > 0.0f);
//				if (bActivate != Wheel->WheelParticleComp->bIsActive)
//				{
//					if (bActivate)
//					{
//						Wheel->WheelParticleComp->DeactivateSystem();
//					}
//					else
//					{
//						Wheel->WheelParticleComp->ActivateSystem();
//					}
//				}
//			}
//		}
	}

	if ( Role == ROLE_Authority )
	{
/*
		// if being tracked, update trackers
		if ( Trackers.Num() > 0 )
		{
			FEnemyPosition NewPosition;
			NewPosition.Position = GetTargetLocation();
			NewPosition.Velocity = Velocity;
			NewPosition.Time = WorldInfo->TimeSeconds;

			for ( INT i=0; i<Trackers.Num(); i++ )
			{
				if ( !Trackers(i) || Trackers(i)->bDeleteMe || (Trackers(i)->CurrentlyTrackedEnemy != this) )
				{
					Trackers.Remove(i,1);
					i--;
				}
				else
				{
					Trackers(i)->SavedPositions.AddItem(NewPosition);
				}
			}
		}
*/
		if ( PhysicsVolume && PhysicsVolume->bWaterVolume && (WaterDamage > 0.f) )
		{
			//@todo steve - check that cylinder center is in water
			eventTakeWaterDamage(DeltaSeconds);
			if ( bDeleteMe )
				return;
		}

		// check if vehicle is upside down and on ground
		if ( bIsInverted && bWasChassisTouchingGroundLastTick )
		{
			if ( WorldInfo->TimeSeconds - LastCheckUpsideDownTime > 0.5f )
			{
				if (WorldInfo->TimeSeconds - LastCheckUpsideDownTime > 1.f)
				{
					if ( bIsScraping && ScrapeSound )
					{
						ScrapeSound->Stop();
						bIsScraping = FALSE;
					}
				}

				// Check if we are upside down and touching the level every 0.5 seconds.
				if ( bEjectPassengersWhenFlipped )
				{
					FlippedCount++;
					if ( FlippedCount > 2 )
					{
						if (Driver)
							eventDriverLeave(TRUE);

						for ( INT i=0; i<Seats.Num(); i++ )
							if ( Seats(i).SeatPawn )
								Seats(i).SeatPawn->eventDriverLeave(true);

						FlippedCount = 0;
					}
					LastCheckUpsideDownTime = WorldInfo->TimeSeconds;
				}
			}
			if ( !bCanFlip )
			{
				AccruedFireDamage += UpsideDownDamagePerSec * DeltaSeconds;
			}
		}	
		else
		{
			if ( ScrapeSound )
			{
				if ( bWasChassisTouchingGroundLastTick && (Velocity.SizeSquared() > 200000.f) && (WorldInfo->TimeSeconds - LastCollisionSoundTime > CollisionIntervalSecs) )
				{
					if ( !bIsScraping )
					{
						ScrapeSound->Play();
						bIsScraping = TRUE;
					}
				}
				else if ( bIsScraping )
				{
					ScrapeSound->Stop();
					bIsScraping = FALSE;
				}
			}
			FlippedCount = 0;
		}

		if ( !Controller && (WorldInfo->TimeSeconds - DeltaSeconds < ResetTime) && (WorldInfo->TimeSeconds >= ResetTime) )
		{
			eventCheckReset();
			if ( bDeleteMe )
				return;
		}
//! @comment avaBot은 구현 안됨.
/*
		//check for bots in danger of being run over every half a second
		if ( WorldInfo->TimeSeconds - LastRunOverWarningTime > 0.5f )
		{
			FLOAT SpeedSquared = Velocity.SizeSquared();
			if (SpeedSquared > MinRunOverSpeed * MinRunOverSpeed)
			{
				FVector VelNormal = Velocity.SafeNormal();
				FLOAT WarningDistSquared = SpeedSquared * 2.f;
				for ( AController* C=WorldInfo->ControllerList; C!=NULL; C=C->NextController )
					if ( (C != Controller) && C->Pawn && C->PlayerReplicationInfo && !C->Pawn->IsA(AVehicle::StaticClass()) )
					{
						// warn friendly and enemy bots about potentially being run over
						FVector Dir = C->Pawn->Location - Location;
						if ( Dir.SizeSquared() < WarningDistSquared )
						{
							AavaBot *B = Cast<AavaBot>(C);
							if ( B && ((VelNormal | Dir.SafeNormal()) > MinRunOverWarningAim) )
								B->eventReceiveRunOverWarning(this, appSqrt(SpeedSquared), VelNormal);
						}
					}
					LastRunOverWarningTime = WorldInfo->TimeSeconds;
			}
		}
*/
		// fire damage to empty burning vehicles
		if ( Health < FireDamageThreshold )
		{
			// check if vehicle is empty
			UBOOL bIsEmpty = TRUE;
			for ( INT i=0; i<Seats.Num(); i++ )
			{
				if ( Seats(i).SeatPawn && Seats(i).SeatPawn->Controller )
				{
					bIsEmpty = FALSE;
					break;
				}
			}
			if ( bIsEmpty )
			{
				AccruedFireDamage += FireDamagePerSec * DeltaSeconds;
			}
		}
	}

	if ( AccruedFireDamage > 1.f )
	{
		eventTakeFireDamage();
		if ( bDeleteMe )
			return;
	}

	if ( Controller && (Role == ROLE_Authority || IsLocallyControlled()) && Driver )
	{
		// Don't do anything if the pawn is in fixed view
		AavaPawn *avaDriverPawn = Cast<AavaPawn>(Driver);
		if ( avaDriverPawn && !avaDriverPawn->bFixedView )
		{
			FRotator Rot = Controller->Rotation;
			ApplyWeaponRotation(0, Rot);
		}
	}
}

UBOOL AavaVehicle::JumpOutCheck(AActor *GoalActor, FLOAT Distance, FLOAT ZDiff)
{
	if ( GoalActor && (ZDiff > -500.f) && (WorldInfo->TimeSeconds - LastJumpOutCheck > 1.f) )
	{
		FLOAT GoalRadius, GoalHeight;
		GoalActor->GetBoundingCylinder(GoalRadius, GoalHeight);
		if ( Distance < ::Min(2.f*GoalRadius,ObjectiveGetOutDist) )
		{
			LastJumpOutCheck = WorldInfo->TimeSeconds;
			eventJumpOutCheck();
			return (Controller == NULL);
		}
	}
	return false;
}

FLOAT AavaVehicle::GetMaxRiseForce()
{
	return 100.f;
}

void AavaVehicle::OnRigidBodyCollision(const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const FCollisionImpactData& RigidCollisionData)
{
	Super::OnRigidBodyCollision(Info0, Info1, RigidCollisionData);

	AActor* OtherActor = (Info0.Actor != this) ? Info0.Actor : Info1.Actor;
	FLOAT ImpactMag=RigidCollisionData.TotalNormalForceVector.Size();
/*
	// update LastBlockingVehicle, so bots detect when another vehicle is on top of their destination
	AavaBot* B = Cast<AavaBot>(Controller);
	if (B != NULL)
	{
		B->LastBlockingVehicle = Cast<AVehicle>(OtherActor);
	}
*/

	// If the impact force is non-zero
	if(ImpactMag > KINDA_SMALL_NUMBER)
	{
		FVector ImpactNorm = RigidCollisionData.TotalNormalForceVector/ImpactMag;
		FLOAT ForwardImpactMag = Abs(Mesh->LocalToWorld.GetAxis(0) | ImpactNorm);
		if(ForwardImpactMag > 0.7f)
		{
			bFrontalCollision = TRUE;

			if(OtherActor && OtherActor->Physics != PHYS_RigidBody)
			{
				bFrontalCollisionWithFixed = TRUE;
			}
		}
	}

	if(GWorld->GetNetMode() != NM_DedicatedServer && Health <= 0 && LastDeathImpactTime + 0.6 < GWorld->GetTimeSeconds() && Info0.Actor != NULL && Info1.Actor != NULL) // impact sounds on clients for dead vehicles
	{
		LastDeathImpactTime = GWorld->GetTimeSeconds();
		FVector ContactLoc = RigidCollisionData.ContactInfos(0).ContactPosition;
		// Notes to self: using consistent self destruct: Speedbike numbers: 1000-7000, Goliath numbers: all over 40k
		if(ImpactMag >= 20000.0f && LargeChunkImpactSound != NULL) // large chunk
		{
			PlaySound(LargeChunkImpactSound,true,true,true,&ContactLoc);
		}
		else if(ImpactMag >= 4000.0f && MediumChunkImpactSound != NULL) // medium chunk
		{
			PlaySound(MediumChunkImpactSound,true,true,true,&ContactLoc);
		}
		else if(ImpactMag >= 1000.0f && SmallChunkImpactSound != NULL) // small chunk
		{
			PlaySound(SmallChunkImpactSound,true,true,true,&ContactLoc);
		}
	}
}

void AavaVehicle::PostEditChange(UProperty* PropertyThatChanged)
{
	if (!GIsEditor && !IsTemplate())
	{
		eventOnPropertyChange( *PropertyThatChanged->GetName() );
	}

	Super::PostEditChange(PropertyThatChanged);
}

void AavaVehicle::ApplyWeaponRotation(INT SeatIndex, FRotator NewRotation)
{
	if (Seats.IsValidIndex(SeatIndex) && Seats(SeatIndex).SeatPawn)
	{
		// @HACK - We don't want to have to replicate the entire seats array, so when we see that the 
		// vehicle has a gun, steal it if we are seat 0.
		if (SeatIndex == 0 && Weapon && !Seats(SeatIndex).Gun)
		{
			Seats(SeatIndex).Gun = Cast<AavaVehicleWeapon>(Weapon);
		}

		AController* C = Seats(SeatIndex).SeatPawn->Controller;

		Seats(SeatIndex).AimTarget = NULL;

		if ( C )
		{
			APlayerController* PC = C->GetAPlayerController();
			FVector AimPoint;

			if ( PC )
			{
				FVector CamLoc;
				FRotator CamRot;
				PC->eventGetPlayerViewPoint(CamLoc, CamRot);

				FLOAT TraceRange;
				TArray<AActor*> IgnoredActors;
				if (Seats(SeatIndex).Gun != NULL)
				{
					TraceRange = Seats(SeatIndex).Gun->AimTraceRange;
					// turn off bProjTarget on Actors we should ignore for the aiming trace
					for (INT i = 0; i < Seats(SeatIndex).Gun->AimingTraceIgnoredActors.Num(); i++)
					{
						AActor* IgnoredActor = Seats(SeatIndex).Gun->AimingTraceIgnoredActors(i);
						if (IgnoredActor != NULL && IgnoredActor->bProjTarget)
						{
							IgnoredActor->bProjTarget = FALSE;
							IgnoredActors.AddItem(IgnoredActor);
						}
					}
				}
				else
				{
					TraceRange = 5000.0f;
				}

				AimPoint = CamLoc + CamRot.Vector() * TraceRange;

				FCheckResult Hit(1.0f);
				if (!GWorld->SingleLineCheck(Hit, this, AimPoint, CamLoc, TRACE_ProjTargets | TRACE_ComplexCollision))
				{
					AimPoint = Hit.Location;
				}

				// Cache who we are aiming at

				Seats(SeatIndex).AimPoint  = AimPoint;
				Seats(SeatIndex).AimTarget = Hit.Actor;

				// restore bProjTarget on Actors we turned it off for
				for (INT i = 0; i < IgnoredActors.Num(); i++)
				{
					IgnoredActors(i)->bProjTarget = TRUE;
				}
			}
			else 
			{
				AimPoint = C->FocalPoint;
				FVector Pivot = GetSeatPivotPoint(SeatIndex);
				NewRotation = (AimPoint - Pivot).Rotation();
			}

			// Set the value
			SeatWeaponRotation(SeatIndex, NewRotation, false);
		}

		for (INT i=0;i<Seats(SeatIndex).TurretControllers.Num(); i++)
		{
			Seats(SeatIndex).TurretControllers(i)->DesiredBoneRotation = NewRotation;
		}
	}
}

UBOOL AavaVehicle::CheckAutoDestruct(ATeamInfo* InstigatorTeam, FLOAT CheckRadius)
{
	// check if close enough to something to auto destruct
	FMemMark Mark(GMem);
	FCheckResult* ActorCheckResult = GWorld->Hash->ActorPointCheck( GMem, Location, FVector(CheckRadius,CheckRadius,CheckRadius), TRACE_ProjTargets );

	// if still going fast, only explode if have gone by the target
	UBOOL bOnlyIfPast = (Velocity.SizeSquared() > 1000000.f);
	for( FCheckResult* Hits=ActorCheckResult; Hits!=NULL; Hits=Hits->GetNext() )
	{
		if ( Hits->Actor && Hits->Actor->GetAPawn() )
		{
			// explode if pawn on different team
			if ( !InstigatorTeam || !Hits->Actor->GetAPawn()->PlayerReplicationInfo || (Hits->Actor->GetAPawn()->PlayerReplicationInfo->Team != InstigatorTeam) )
			{
				// blow up
				if ( !bOnlyIfPast || (((Hits->Actor->Location - Location) | Velocity) < 0.f) )
				{
					eventSelfDestruct(Hits->Actor);
					Mark.Pop();
					return true;
				}
			}
		}
		else
		{
			AavaGameObjective* HitObjective = Cast<AavaGameObjective>(Hits->Actor);
			if ( HitObjective )
			{
				// explode if objective on different team
				if ( !InstigatorTeam || (HitObjective->DefenderTeamIndex != InstigatorTeam->TeamIndex) )
				{
					// blow up
					if ( !bOnlyIfPast || (((HitObjective->Location - Location) | Velocity) < 0.f) )
					{
						eventSelfDestruct(HitObjective);
						Mark.Pop();
						return true;
					}
				}
			}
		}
	}
	Mark.Pop();
	return false;
}

static FTakeHitInfo OldLastTakeHitInfo;
static FLOAT OldHealth;

void AavaVehicle::PreNetReceive()
{
	Super::PreNetReceive();

	OldLastTakeHitInfo = LastTakeHitInfo;
	OldHealth = Health;
}

void AavaVehicle::PostNetReceive()
{
	Super::PostNetReceive();

	//@note: the ordering of these two checks is important here because we'd like the client to try to figure out everything using the LastTakeHitInfo where possible,
	//		and only use the direct health change for corrections
//	if (OldLastTakeHitInfo != LastTakeHitInfo)
//	{
//		eventPlayTakeHitEffects();
//	}
	if (OldHealth != Health)
	{
		eventReceivedHealthChange();
	}
}

// native functions

void AavaVehicle::InitDamageSkel()
{
/*
	UAnimTree* AnimTree = Cast<UAnimTree>(Mesh->Animations);
	if(AnimTree)
	{
		TArray<USkelControlBase*>	Controls;
		AnimTree->GetSkelControls(Controls);
		INT j=0;
		for(INT i=0; i<Controls.Num(); ++i)
		{
			UUTSkelControl_Damage* DamageC = Cast<UUTSkelControl_Damage>(Controls(i));
			if(DamageC)
			{
				DamageSkelControls.Push(DamageC);
			}
		}
	}
*/
}

FRotator AavaVehicle::SeatWeaponRotation(INT SeatIndex,FRotator NewRot,UBOOL bReadValue)
{
	FRotator Result = FRotator(0,0,0);
	if ( SeatIndex>=0 && SeatIndex < Seats.Num() )
 	{
		if ( !Seats(SeatIndex).WeaponRotationProperty )
		{
			// Find the UProperty in question

			UProperty* Prop = FindField<UProperty>(GetClass(), Seats(SeatIndex).WeaponRotationName);
			if (Prop != NULL)
			{

				// check to make sure the property is an FRotator.  We do this by insuring it's a UStructProperty named
				// Rotator.

				if (Prop->GetClass() != UStructProperty::StaticClass() || ((UStructProperty*)Prop)->Struct->GetFName() != NAME_Rotator)
				{
					debugf( NAME_Warning, TEXT("WeaponRotation property type mismatch: %s is %s, expected Rotator"), *Seats(SeatIndex).WeaponRotationName.ToString(), 
							(Prop->GetClass() != UStructProperty::StaticClass()) ? *Prop->GetClass()->GetName() : *((UStructProperty*)Prop)->Struct->GetName() );
					Prop = NULL;
				}
			}

			// Short circut if we couldn't find the property

			if (Prop == NULL)
			{
				return Result;
			}
			
			Seats(SeatIndex).WeaponRotationProperty = Prop;
		}

		/*
			Process the value.  A property doesn't hold the value of the property, it describes where in its owner 
			struct's (or class's) _instance_ to find the value of the property. So, the code gets the offset of the 
			property that it found by name, adds that offset to the beginning of the memory used by the vehicle instance, 
			and then copies what that memory location is pointing to. 
		*/

		BYTE* PropLoc = (BYTE*) this + ((UProperty*) Seats(SeatIndex).WeaponRotationProperty)->Offset;

		if ( bReadValue )
		{
			((UProperty*) Seats(SeatIndex).WeaponRotationProperty)->CopySingleValue(&Result, PropLoc);
		}
		else
		{
//			debugf(TEXT("avaVehicle.SeatWeaponRotation( %d, %d, %d )"), NewRot.Pitch, NewRot.Yaw, NewRot.Roll);
			((UProperty*) Seats(SeatIndex).WeaponRotationProperty)->CopySingleValue(PropLoc, &NewRot);
			bNetDirty=true;
		}
	}

	return Result;
}

FVector AavaVehicle::SeatFlashLocation(INT SeatIndex,FVector NewLoc,UBOOL bReadValue)
{
	FVector Result = FVector(0,0,0);

	if ( SeatIndex>=0 && SeatIndex < Seats.Num() )
	{
		if ( !Seats(SeatIndex).FlashLocationProperty )
		{
			UProperty* Prop = FindField<UProperty>(GetClass(), Seats(SeatIndex).FlashLocationName);
			if (Prop != NULL)
			{
				if (Prop->GetClass() != UStructProperty::StaticClass() || ((UStructProperty*)Prop)->Struct->GetFName() != NAME_Vector)
				{
					debugf( NAME_Warning, TEXT("FlashLocation property type mismatch: %s is %s, expected Vector"), *Seats(SeatIndex).FlashLocationName.ToString(), 
							(Prop->GetClass() != UStructProperty::StaticClass()) ? *Prop->GetClass()->GetName() : *((UStructProperty*)Prop)->Struct->GetName() );
					Prop = NULL;
				}
			}

			if (Prop == NULL)
			{
				return Result;
			}

			Seats(SeatIndex).FlashLocationProperty = Prop;
		}

		BYTE* PropLoc = (BYTE*) this + ((UProperty*) Seats(SeatIndex).FlashLocationProperty)->Offset;
		if ( bReadValue )
		{
			((UProperty*) Seats(SeatIndex).FlashLocationProperty)->CopySingleValue(&Result, PropLoc);
		}
		else
		{
			((UProperty*) Seats(SeatIndex).FlashLocationProperty)->CopySingleValue(PropLoc, &NewLoc);
			bNetDirty=true;
		}
	}
	return Result;
}

BYTE AavaVehicle::SeatFlashCount(INT SeatIndex,BYTE NewCount,UBOOL bReadValue)
{
	BYTE Result = 0;

	if ( SeatIndex>=0 && SeatIndex < Seats.Num() )
	{
		if ( !Seats(SeatIndex).FlashCountProperty)
		{
			UProperty* Prop = FindField<UProperty>(GetClass(), Seats(SeatIndex).FlashCountName);
			if (Prop != NULL)
			{
				if (Prop->GetClass() != UByteProperty::StaticClass())
				{
					debugf(NAME_Warning, TEXT("FlashCount property type mismatch: %s is %s, expected ByteProperty"), *Seats(SeatIndex).FlashCountName.ToString(), *Prop->GetClass()->GetName());
					Prop = NULL;
				}
			}

			if (Prop == NULL)
			{
				return Result;
			}

			Seats(SeatIndex).FlashCountProperty = Prop;
		}
		
		BYTE* PropLoc = (BYTE*) this + ((UProperty*) Seats(SeatIndex).FlashCountProperty)->Offset;
		if ( bReadValue )
		{
			((UProperty*) Seats(SeatIndex).FlashCountProperty)->CopySingleValue(&Result, PropLoc);
		}
		else
		{
			((UProperty*) Seats(SeatIndex).FlashCountProperty)->CopySingleValue(PropLoc, &NewCount);
			bNetDirty=true;
		}
	}
	return Result;
}

BYTE AavaVehicle::SeatFiringMode(INT SeatIndex,BYTE NewFireMode,UBOOL bReadValue)
{
	BYTE Result = 0;

	if ( SeatIndex>=0 && SeatIndex < Seats.Num() )
	{
		if ( !Seats(SeatIndex).FiringModeProperty )
		{
			UProperty* Prop = FindField<UProperty>(GetClass(), Seats(SeatIndex).FiringModeName);
			if (Prop != NULL)
			{
				if (Prop->GetClass() != UByteProperty::StaticClass())
				{
					debugf(NAME_Warning, TEXT("FiringMode property type mismatch: %s is %s, expected ByteProperty"), *Seats(SeatIndex).FiringModeName.ToString(), *Prop->GetClass()->GetName());
					Prop = NULL;
				}
			}

			if (Prop == NULL)
			{
				return Result;
			}

			Seats(SeatIndex).FiringModeProperty = Prop;
		}

		BYTE* PropLoc = (BYTE*) this + ((UProperty*) Seats(SeatIndex).FiringModeProperty)->Offset;
		if ( bReadValue )
		{
			((UProperty*) Seats(SeatIndex).FiringModeProperty)->CopySingleValue(&Result, PropLoc);
		}
		else
		{
			((UProperty*) Seats(SeatIndex).FiringModeProperty)->CopySingleValue(PropLoc, &NewFireMode);
			bNetDirty=true;
		}
	}
	return Result;
}

void AavaVehicle::ForceWeaponRotation(INT SeatIndex,FRotator NewRotation)
{
	// 강제로 값을 쓴다.
	// Controller가 없으면 적용되지 않더라.(그럴꺼면 왜 Force가 붙은거여)
	SeatWeaponRotation(SeatIndex, NewRotation, false);

	// 해당 Rotation값을 보간없이 바로 적용 되도록 해준다.
	for (INT i=0;i<Seats(SeatIndex).TurretControllers.Num(); i++)
	{
		Seats(SeatIndex).TurretControllers(i)->bIgnoreOnceLagDegreesPerSecond = TRUE;

		// 만약 ApplyWeaponRotation()함수가 실행되지 않을 것 같다면 강제로 설정!!
		debugf(TEXT("avaVehicle::ForceWeaponRotation(%d, (%d,%d,%d))"), SeatIndex, NewRotation.Pitch, NewRotation.Yaw, NewRotation.Roll);
		Seats(SeatIndex).TurretControllers(i)->DesiredBoneRotation = NewRotation;
	}

	ApplyWeaponRotation(SeatIndex, NewRotation);
}

FVector AavaVehicle::GetSeatPivotPoint(INT SeatIndex)
{
	INT BarrelIndex = GetBarrelIndex(SeatIndex);
	INT ArrayLen = Seats(SeatIndex).GunPivotPoints.Num();

	if ( Seats(SeatIndex).Mesh && ArrayLen > 0 )
	{
		if ( BarrelIndex >= ArrayLen )
		{
			BarrelIndex = ArrayLen - 1;
		}

		FName	Pivot    = Seats(SeatIndex).GunPivotPoints(BarrelIndex);
		FVector	PivotLoc = Seats(SeatIndex).Mesh->GetBoneLocation(Pivot);

		//! @brief
		//!		거리가 300보다 더 멀리 있는 경우에는 엉뚱한 값이라고 판단한다.(2007/08/08 고광록)
		//! @note
		//!		HM처리 중에 Skeleton이 갱신되지 않고 GetBoneLocation이 호출되서 
		//!		엉뚱한 값이 나오는 경우에 대한 예외처리를 한다.
		FLOAT DistSq = CylinderComponent ? CylinderComponent->Bounds.BoxExtent.SizeSquared2D() : 300 * 300;

		if ( (PivotLoc - Location).SizeSquared2D() > DistSq )
		{
//			debugf(TEXT("(PivotLoc(%f,%f,%f) - Location(%f,%f,%f)).Distance > Vehicle.Distance(%f)"), 
//				PivotLoc.X, PivotLoc.Y, PivotLoc.Z, Location.X, Location.Y, Location.Z, appSqrt(DistSq));
			return Location;
		}

		return PivotLoc;
	}
	else
	{
		return Location;
	}
}

INT AavaVehicle::GetBarrelIndex(INT SeatIndex)
{
	if ( Seats(SeatIndex).GunSocket.Num() < 0 )
	{
		return 0;
	}
	else
	{
		return Seats(SeatIndex).GunSocket.Num() > 0 ? Seats(SeatIndex).BarrelIndex % Seats(SeatIndex).GunSocket.Num() : 0;
	}
}

void AavaVehicle::execIsSeatControllerReplicationViewer _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(SeatIndex);
	P_FINISH;

	UBOOL bResult = FALSE;
	if (SeatIndex < Seats.Num() && Seats(SeatIndex).SeatPawn != NULL)
	{
//		for (INT i = 0; i < WorldInfo->ReplicationViewers.Num(); i++)
//		{
//			if (WorldInfo->ReplicationViewers(i).InViewer == Seats(SeatIndex).SeatPawn->Controller)
//			{
//				bResult = TRUE;
//				break;
//			}
//		}
	}

	*(UBOOL*)Result = bResult;
}

/** 
* This function calculates the various damage parameters in the damage material overlay.  It scans the DamageMorphTargets
* list and generates a list of damage params and their weights.  It then blank assigns them.
*/
void AavaVehicle::UpdateDamageMaterial()
{
	if (DamageMaterialInstance != NULL)
	{
		TArray<FName> DamageNames;
		TArray<INT>	Healths;
		TArray<INT> MaxHealths;

		// Get a quick link to the default vehicle
		AavaVehicle* DefaultVehicle = GetArchetype<AavaVehicle>();

		for (INT i = 0; i < DamageMorphTargets.Num(); i++)
		{
			for (INT j = 0; j < DamageMorphTargets(i).DamagePropNames.Num(); j++)
			{
				INT ItemIndex;
				if (DamageNames.Num() == 0 || !DamageNames.FindItem(DamageMorphTargets(i).DamagePropNames(j), ItemIndex))
				{
					DamageNames.AddItem(DamageMorphTargets(i).DamagePropNames(j));
					Healths.AddItem(DamageMorphTargets(i).Health);
					MaxHealths.AddItem(DefaultVehicle->DamageMorphTargets(i).Health);
				}
				else
				{
					Healths(ItemIndex) += DamageMorphTargets(i).Health;
					MaxHealths(ItemIndex) += DefaultVehicle->DamageMorphTargets(i).Health;
				}
			}
		}

		for (INT i = 0; i < DamageNames.Num(); i++)
		{
			DamageMaterialInstance->SetScalarParameterValue(DamageNames(i), 1.0 - (FLOAT(Healths(i)) / FLOAT(MaxHealths(i))));
		}
	}
}

void AavaVehicle::ApplyMorphDamage(FVector HitLocation,INT Damage, FVector Momentum)
{
	FLOAT Dist = -1.f;
	FLOAT BestDist = 0.f;
	FVector BoneLocation;
	INT MorphIndex = -1;
	FName CurBone;

	// Quick exit if this vehicle doesn't have morph targets

	if ( DamageMorphTargets.Num() <= 0 )
	{
		return;
	}

	// Find the Influence bone that is closest to the hit

	for (INT i=0;i<Mesh->SkeletalMesh->RefSkeleton.Num();i++)
	{
		CurBone = Mesh->SkeletalMesh->RefSkeleton(i).Name;

		INT InfluenceBoneIndex = -1;
		for (INT j=0;j<DamageMorphTargets.Num();j++)
		{
			if (CurBone == DamageMorphTargets(j).InfluenceBone)
			{
				InfluenceBoneIndex = j;
				break;
			}
		}

		if ( InfluenceBoneIndex >= 0 )
		{
			BoneLocation = Mesh->GetBoneLocation(CurBone);

			Dist = (HitLocation - BoneLocation).Size();
			if (MorphIndex < 0 || Dist < BestDist)
			{
				BestDist = Dist;
				MorphIndex = InfluenceBoneIndex;
			}
		}
	}


	if ( MorphIndex >= 0 )	// We have the best
	{
		// Traverse the morph chain dealing out damage as needed

		while ( Damage > 0 )
		{
			// Deal some damage
			if ( DamageMorphTargets(MorphIndex).Health > 0 )
			{
				if ( DamageMorphTargets(MorphIndex).Health <= Damage )
				{
					Damage -= DamageMorphTargets(MorphIndex).Health;
					//debugf(TEXT("1. Adjusting Node %s %i"),*DamageMorphTargets(MorphIndex).MorphNodeName, DamageMorphTargets(MorphIndex).Health);
					DamageMorphTargets(MorphIndex).Health = 0;
		
					// This node is dead, so reset to the remaining damage and force this node's health to 0.  This
					// will allow the next node to get the proper damage amount
				}
				else
				{
					//debugf(TEXT("2. Adjusting Node %s %i"),*DamageMorphTargets(MorphIndex).MorphNodeName, DamageMorphTargets(MorphIndex).Health);
					DamageMorphTargets(MorphIndex).Health -= Damage;
					Damage = 0;
				}

				if ( DamageMorphTargets(MorphIndex).Health <= 0 )
				{
					eventMorphTargetDestroyed(MorphIndex);
				}
			}
				
			// Calculate the new Weight for the MorphTarget influenced by this node and set it.

			AavaVehicle* DefaultVehicle = (AavaVehicle*)( GetClass()->GetDefaultActor() );

			FLOAT Weight = 1.0 - ( FLOAT(DamageMorphTargets(MorphIndex).Health) / FLOAT(DefaultVehicle->DamageMorphTargets(MorphIndex).Health) );
			UMorphNodeWeight* MorphNode = DamageMorphTargets(MorphIndex).MorphNode;
			if (MorphNode != NULL)
			{
				MorphNode->SetNodeWeight(Weight);
			}
			else
			{
				debugf(NAME_Warning, TEXT("Failed to find MorphNode for DamageMorphTarget %i '%s' to apply damage"), MorphIndex, *DamageMorphTargets(MorphIndex).MorphNodeName.ToString());
			}

			// Contine the chain if we can.
			if ( DamageMorphTargets(MorphIndex).LinkedMorphNodeName != NAME_None && DamageMorphTargets(MorphIndex).LinkedMorphNodeIndex != MorphIndex )
			{
				MorphIndex = DamageMorphTargets(MorphIndex).LinkedMorphNodeIndex;
			}
			else
			{
				Damage = 0;
			}
		}
	}
	UpdateDamageMaterial();
}


//-----------------------------------------------------------------------------
//	class AavaVehicleFactory
//-----------------------------------------------------------------------------
void AavaVehicleFactory::TickSpecial( FLOAT DeltaSeconds )
{
	Super::TickSpecial(DeltaSeconds);

	if (RespawnProgress > 0.f)
	{
		RespawnProgress -= DeltaSeconds * RespawnRateModifier;
		if (RespawnProgress <= 0.f)
		{
			eventSpawnVehicle();
		}
	}
}

void AavaVehicleFactory::CheckForErrors()
{
	Super::CheckForErrors();

	// throw an error if multiple enabled vehicle factories overlap
	UBOOL bFoundThis = FALSE;
	for (FActorIterator It; It; ++It)
	{
		// don't start testing until we've found ourselves - avoids getting two errors for each pair of factories that overlap
		if (*It == this)
		{
			bFoundThis = TRUE;
		}
		else if (bFoundThis)
		{
			/*
			AavaVehicleFactory* OtherFactory = Cast<AavaVehicleFactory>(*It);
			// if they overlap and they'll both spawn vehicles for the same team
			if ( OtherFactory != NULL && (OtherFactory->Location - Location).SizeSquared() < Square(CylinderComponent->CollisionRadius + OtherFactory->CylinderComponent->CollisionRadius) &&
				(TeamSpawningControl == TS_All || OtherFactory->TeamSpawningControl == TS_All || TeamSpawningControl == OtherFactory->TeamSpawningControl) )
			{
				// check if they can both be active simultaneously
				UBOOL bActiveSimultaneously = (!bDisabled && !OtherFactory->bDisabled);
				FName ActiveLinkSetup = NAME_None;
				UUTOnslaughtMapInfo* ONSInfo = Cast<UUTOnslaughtMapInfo>(WorldInfo->GetMapInfo());
				if (ONSInfo != NULL)
				{
					bActiveSimultaneously = FALSE;
					for (INT i = 0; i < ONSInfo->LinkSetups.Num(); i++)
					{
						const FLinkSetup& Setup = ONSInfo->LinkSetups(i);

						UBOOL bNewThisDisabled = bDisabled;
						UBOOL bNewOtherDisabled = OtherFactory->bDisabled;

						if (Setup.ActivatedActors.ContainsItem(this) || Setup.ActivatedGroups.ContainsItem(Group))
						{
							bNewThisDisabled = FALSE;
						}
						else if (Setup.DeactivatedActors.ContainsItem(this) || Setup.DeactivatedGroups.ContainsItem(Group))
						{
							bNewThisDisabled = TRUE;
						}
						if (Setup.ActivatedActors.ContainsItem(OtherFactory) || Setup.ActivatedGroups.ContainsItem(OtherFactory->Group))
						{
							bNewOtherDisabled = FALSE;
						}
						else if (Setup.DeactivatedActors.ContainsItem(OtherFactory) || Setup.DeactivatedGroups.ContainsItem(OtherFactory->Group))
						{
							bNewOtherDisabled = TRUE;
						}

						if (!bNewThisDisabled && !bNewOtherDisabled)
						{
							bActiveSimultaneously = TRUE;
							ActiveLinkSetup = Setup.SetupName;
							break;
						}
					}
				}

				if (bActiveSimultaneously)
				{
					if (ActiveLinkSetup != NAME_None)
					{
						GWarn->MapCheck_Add(MCTYPE_ERROR, this, *FString::Printf(TEXT("Vehicle factory is too close to other factory '%s' that can be active simultaneously in link setup '%s'"), *OtherFactory->GetName(), *ActiveLinkSetup.ToString()));
					}
					else
					{
						GWarn->MapCheck_Add(MCTYPE_ERROR, this, *FString::Printf(TEXT("Vehicle factory is too close to other factory '%s' that can be active simultaneously"), *OtherFactory->GetName()));
					}
				}
			}
			*/
		}
	}
}

//-----------------------------------------------------------------------------
//	class AavaDestroyedVehicleHitProxy
//-----------------------------------------------------------------------------
UBOOL AavaDestroyedVehicleHitProxy::IgnoreBlockingBy(const AActor* Other) const
{
	return (Owner != NULL) ? Owner->IgnoreBlockingBy(Other) : Super::IgnoreBlockingBy(Other);
}


//-----------------------------------------------------------------------------
//	class UavaVehicleSimCar
//-----------------------------------------------------------------------------
float UavaVehicleSimCar::GetEngineOutput(ASVehicle* Vehicle)
{
	return EngineRPMCurve.Eval(Vehicle->ForwardVel, 0.0f);
}

void UavaVehicleSimCar::ProcessCarInput(ASVehicle* Vehicle)
{
	Super::ProcessCarInput(Vehicle);

	if ( Vehicle->IsHumanControlled() )
	{			
		Vehicle->DriverViewPitch = Vehicle->Controller->Rotation.Pitch;
		Vehicle->DriverViewYaw = Vehicle->Controller->Rotation.Yaw;
	}
	else
	{
		Vehicle->DriverViewPitch = Vehicle->Rotation.Pitch;
		Vehicle->DriverViewYaw = Vehicle->Rotation.Yaw;
	}
}

void UavaVehicleSimCar::UpdateVehicle(ASVehicle* Vehicle, FLOAT DeltaTime)
{
	if( Vehicle->bDriving )
	{
		FLOAT Speed = Abs(Vehicle->ForwardVel);

		/////////// STEERING ///////////
		FLOAT maxSteerAngle = MaxSteerAngleCurve.Eval(Vehicle->Velocity.Size(), 0.f);

		// change steering if not all wheels on ground
		INT NumWheelsOnGround = 0;
		for ( INT i=0; i<Vehicle->Wheels.Num(); i++ )
		{
			if ( Vehicle->Wheels(i)->bWheelOnGround )
			{
				NumWheelsOnGround++;
			}
		}
		UBOOL bForceHandbrake = FALSE;
		if ( (NumWheelsOnGround < NumWheelsForFullSteering) && (Speed > StopThreshold) )
		{
			FLOAT RampupRateMultiplier = 1.f;
			Vehicle->OutputGas *= 0.3f;
			if ( NumWheelsOnGround < NumWheelsForFullSteering - 1 ) 
			{
				Vehicle->OutputGas *= 0.2f;
				RampupRateMultiplier = 3.f;

				if ( bAutoHandbrake && (NumWheelsOnGround == 2) && (Vehicle->Wheels.Num() == 4) 
					&& (Vehicle->Wheels(0)->bWheelOnGround == Vehicle->Wheels(2)->bWheelOnGround) )
				{
					// if both on same side, apply handbrake
					bForceHandbrake = TRUE;
					Vehicle->bOutputHandbrake = TRUE; // @TODO FIXMESTEVE - turn off to let Phil tweak tire friction
				}
			}
			CurrentSteeringReduction = ::Max(SteeringReductionFactor, CurrentSteeringReduction - RampupRateMultiplier*SteeringReductionRampUpRate*DeltaTime);
			if ( Speed > SteeringReductionMinSpeed )
			{
				if ( Speed > SteeringReductionSpeed )
					maxSteerAngle *= CurrentSteeringReduction; 
				else
					maxSteerAngle *= (1.f - (Speed-SteeringReductionMinSpeed)/(SteeringReductionSpeed-SteeringReductionMinSpeed) * (1.f - CurrentSteeringReduction)); 
			}
			else
			{
				CurrentSteeringReduction = ::Min(1.f,CurrentSteeringReduction + 0.5f*SteeringReductionRampUpRate*DeltaTime);
			}
		}
		else
		{
			CurrentSteeringReduction = ::Min(1.f,CurrentSteeringReduction + 0.5f*SteeringReductionRampUpRate*DeltaTime);
		}

		FLOAT maxSteer = DeltaTime * SteerSpeed;
		FLOAT deltaSteer;
		AavaVehicle* avaV = Cast<AavaVehicle>(Vehicle);
		if(avaV && !avaV->ForceMovementDirection.IsZero() )
		{
			FRotationMatrix R(Vehicle->Rotation);
			FVector Forwards = R.GetAxis(0);
			// Steering:
			FLOAT DesiredSteer = FindDeltaAngle(HeadingAngle(Forwards),HeadingAngle(avaV->ForceMovementDirection))*RAD_TO_DEG;
			ActualSteering = Clamp<FLOAT>(DesiredSteer,-maxSteer,maxSteer);
			if(Vehicle->OutputGas < 0.f) // wheeled vehicles need to steer the other way when going in reverse.
			{
				ActualSteering *= -1.0f;
			}
			FLOAT DotProduct = Forwards | avaV->ForceMovementDirection;
			// don't allow gas in opposite direction of force movement.
			if((!avaV->bForceDirectionAllowedNegative && ((DotProduct < (1.0-KINDA_SMALL_NUMBER)) && (DotProduct < 0.f && Vehicle->OutputGas > 0.f) || (DotProduct > 0.f && Vehicle->OutputGas < 0.f))))
			{
				Vehicle->OutputGas = 0.0f; // GOOD IDEA: We have to move to steer, but this would take us farther 'out', go the other way player! :)
				Vehicle->OutputBrake = 1.0f;
			}
			//Vehicle->OutputGas *= Abs(DotProduct); // BAD IDEA: We have to move to steer, so cutting gas while we correct isn't good.
		}
		else
		{
			deltaSteer = (-Vehicle->OutputSteering * maxSteerAngle) - ActualSteering; // Amount we want to move (target - current)
			deltaSteer = Clamp<FLOAT>(deltaSteer, -maxSteer, maxSteer);
			ActualSteering += deltaSteer;
		}

		/////////// THROTTLE ///////////
		// scale throttle speed used based on current speed
		FRotationMatrix R(Vehicle->Rotation);
		FLOAT CurrentThrottleSpeed = ThrottleSpeed * Square(Square((Vehicle->AirSpeed - Speed)/Vehicle->AirSpeed));
		FLOAT OldActualThrottle = ActualThrottle;
		if ( (Vehicle->OutputGas == 0.f) || ((Vehicle->OutputGas > 0.f) != (ActualThrottle > 0.f)) )
			bHasForcedThrottle = FALSE;

		if ( Vehicle->OutputGas >= 0.f )
		{
			ActualThrottle = ::Max(ActualThrottle, 0.f);
			if ( Vehicle->ForwardVel <= 0.f )
			{
				bForceThrottle = TRUE;
				ActualThrottle = ::Min<FLOAT>(ActualThrottle + DeltaTime, Vehicle->OutputGas);	
			}
			else
			{
				if ( bForceThrottle && !bHasForcedThrottle )
				{
					ActualThrottle = ::Min<FLOAT>(CurrentThrottleSpeed*DeltaTime, Vehicle->OutputGas);	
					bHasForcedThrottle = TRUE;
				}
				else if ( Vehicle->OutputGas <= ActualThrottle )
				{
					ActualThrottle = Vehicle->OutputGas;
				}
				else
				{
					ActualThrottle = ::Min<FLOAT>(ActualThrottle + CurrentThrottleSpeed * DeltaTime, Vehicle->OutputGas);	
				}
				bForceThrottle = FALSE;
			}
		}
		else
		{
			if ( Vehicle->ForwardVel >= 0.f )
			{
				bForceThrottle = TRUE;
				ActualThrottle = ::Max<FLOAT>(ActualThrottle - DeltaTime, Vehicle->OutputGas);	
			}
			else
			{
				if ( bForceThrottle && !bHasForcedThrottle )
				{
					ActualThrottle = ::Max<FLOAT>(-1.f * CurrentThrottleSpeed*DeltaTime, Vehicle->OutputGas);	
					bHasForcedThrottle = TRUE;
				}
				else if ( Vehicle->OutputGas >= ActualThrottle )
				{
					ActualThrottle = Vehicle->OutputGas;
				}
				else
				{
					ActualThrottle = ::Max<FLOAT>(ActualThrottle - CurrentThrottleSpeed * DeltaTime, Vehicle->OutputGas);	
				}
				bForceThrottle = FALSE;
			}
		}

		/////////// TORQUE CURVE APPROXIMATION //////////////

		// Braking
		FLOAT BrakeTorque = Vehicle->OutputBrake * MaxBrakeTorque;

		// Torque
		FLOAT TorqueEval = TorqueVSpeedCurve.Eval(Vehicle->ForwardVel, 0.0f);;
		FLOAT MotorTorque = ActualThrottle * TorqueEval;
		if ( (Vehicle->ForwardVel > Vehicle->AirSpeed) && ((Vehicle->Velocity | Vehicle->Rotation.Vector()) > 0.f) )
		{
			// force vehicle to slow down if above airspeed limit
			MotorTorque = 0.f;
			BrakeTorque = MaxBrakeTorque;
		}
		else if ( ActualThrottle == 0.0f )
		{
			if ( Speed > StopThreshold )
			{
				MotorTorque -= EngineBrakeFactor * Vehicle->ForwardVel;				
			}
		}
		else if ( (abs(Vehicle->OutputSteering) > 0.6f) && (abs(Vehicle->ForwardVel) > MinHardTurnSpeed) 
			&& ((Vehicle->OutputGas > 0.f) == (Vehicle->ForwardVel > 0.f))
			&& (!Vehicle->bOutputHandbrake || bForceHandbrake) )
		{
			// reduce torque and throttle if turning hard
			FLOAT HardPct = 2.5f * (abs(Vehicle->OutputSteering) - 0.6f);
			MotorTorque = HardTurnMotorTorque * HardPct + MotorTorque * (1.f - HardPct);
			ActualThrottle = OldActualThrottle;
		}

		// Lose torque when climbing too steep
		if ( R.GetAxis(2).Z < Vehicle->WalkableFloorZ )
		{
			MotorTorque = 0.f;
		}

		FLOAT TotalSpinVel = 0.0f;
		if (LSDFactor > 0.0f)
		{
			for(INT i=0; i<Vehicle->Wheels.Num(); i++)
			{
				USVehicleWheel* vw = Vehicle->Wheels(i);

				// Accumulate wheel spin speeds to use for LSD
				TotalSpinVel += vw->SpinVel;
			}
		}

		FLOAT TotalMotorTorque = MotorTorque * Vehicle->NumPoweredWheels;
		FLOAT TotalBrakeTorque = BrakeTorque * Vehicle->NumPoweredWheels;
		FLOAT EvenSplit = 1.f/(FLOAT)Vehicle->NumPoweredWheels;

		// Do model for each wheel.
		for(INT i=0; i<Vehicle->Wheels.Num(); i++)
		{
			USVehicleWheel* vw = Vehicle->Wheels(i);

			if (vw->bPoweredWheel)
			{
				/////////// LIMITED SLIP DIFFERENTIAL ///////////

				// Heuristic to divide torque up so that the wheels that are spinning slower get more of it.
				// Sum of LSDFactor across all wheels should be 1.
				FLOAT LSDSplit, UseSplit;

				if (LSDFactor > 0.0f)
				{	
					// If no wheels are spinning, just do an even split.
					if(TotalSpinVel > 0.1f)
						LSDSplit = (TotalSpinVel - vw->SpinVel)/(((FLOAT)Vehicle->NumPoweredWheels-1.f) * TotalSpinVel);
					else
						LSDSplit = EvenSplit;

					UseSplit = ((1-LSDFactor) * EvenSplit) + (LSDFactor * LSDSplit);
				}
				else
					UseSplit = EvenSplit;

				vw->BrakeTorque = UseSplit * TotalBrakeTorque;
				vw->MotorTorque = UseSplit * TotalMotorTorque;

				// Calculate torque applied back to chassis if wheel is on the ground
				if (vw->bWheelOnGround)
					vw->ChassisTorque = -1.0f * vw->MotorTorque * ChassisTorqueScale;
				else
					vw->ChassisTorque = 0.0f;

#if WITH_NOVODEX // Update WheelShape in case it changed due to handbrake
				NxWheelShape* WheelShape = vw->GetNxWheelShape();
				check(WheelShape);	

				SetNxWheelShapeParams(WheelShape, vw);
#endif // WITH_NOVODEX
			}

			/////////// STEERING  ///////////

			// Pass on steering to wheels that want it.
			vw->Steer = ActualSteering * vw->SteerFactor;
		}
	}
	else
	{
		// no driver - just brake to a stop
		FLOAT TotalMotorTorque = 0.f;
		FLOAT TotalBrakeTorque = 0.f;
		FLOAT EvenSplit = 1.f/(FLOAT)Vehicle->NumPoweredWheels;

		if ( bDriverlessBraking )
		{
			if ( Abs(Vehicle->ForwardVel) > StopThreshold )
			{
				TotalMotorTorque -= EngineBrakeFactor * Vehicle->ForwardVel;
				TotalMotorTorque *= Vehicle->NumPoweredWheels;
			}
			else
			{
				TotalBrakeTorque = MaxBrakeTorque * Vehicle->NumPoweredWheels;
			}
		}

		// Do model for each wheel.
		for(INT i=0; i<Vehicle->Wheels.Num(); i++)
		{
			USVehicleWheel* vw = Vehicle->Wheels(i);

			if (vw->bPoweredWheel)
			{
				vw->BrakeTorque = EvenSplit * TotalBrakeTorque;
				vw->MotorTorque = EvenSplit * TotalMotorTorque;

				// Calculate torque applied back to chassis if wheel is on the ground
				if (vw->bWheelOnGround)
					vw->ChassisTorque = -1.0f * vw->MotorTorque * ChassisTorqueScale;
				else
					vw->ChassisTorque = 0.0f;
			}
			if ( Vehicle->bUpdateWheelShapes )
			{
#if WITH_NOVODEX 
				NxWheelShape* WheelShape = vw->GetNxWheelShape();
				check(WheelShape);	
				SetNxWheelShapeParams(WheelShape, vw);
#endif // WITH_NOVODEX 
			}
			vw->Steer = 0.f;
		}
		Vehicle->bUpdateWheelShapes = FALSE;
	}
}

/** Simulate the engine model of the treaded vehicle */

void UavaVehicleSimTank::UpdateVehicle(ASVehicle* Vehicle, FLOAT DeltaTime)
{
#if WITH_NOVODEX

	AavaVehicle* avaV=Cast<AavaVehicle>(Vehicle);
	// Lose torque when climbing too steep
	FRotationMatrix R(Vehicle->Rotation);
	FLOAT EngineTorque;
	UBOOL bInvertTorque;
	if(avaV && avaV->ForceMovementDirection != FVector(0,0,0)) // we need to recalculate the steering numbers to force us in the 'right' direction:
	{
		Vehicle->OutputSteering = Clamp<FLOAT>((FindDeltaAngle(HeadingAngle(avaV->ForceMovementDirection),HeadingAngle(R.GetAxis(0))))*10.0f,-1.0,1.0);
		FLOAT Threshold = bForceOnTarget?0.25f:0.1f;
		if(Abs(Vehicle->OutputSteering)>Threshold*2.0 || (Vehicle->OutputGas < 0.f && !avaV->bForceDirectionAllowedNegative))
		{
			Vehicle->OutputGas = 0.0;
		}
		else
		{
			if(Abs(Vehicle->OutputSteering)>Threshold)
			{
				Vehicle->OutputSteering = 0.0f;
				bForceOnTarget = true;
			}
		}
		EngineTorque = Clamp<FLOAT>(Abs(Vehicle->OutputGas) + Abs(TurnInPlaceThrottle * Vehicle->OutputSteering), -1.0, 1.0) * MaxEngineTorque;
		bInvertTorque = (Vehicle->OutputGas < 0.f);
	}
	else
	{
		// Determine how much torque we are getting from the engine
		if(bTurnInPlaceOnSteer || (Vehicle->OutputRise > 0.f))
		{
			EngineTorque = Clamp<FLOAT>(Abs(Vehicle->OutputGas) + Abs(TurnInPlaceThrottle * Vehicle->OutputSteering), -1.0, 1.0) * MaxEngineTorque;
		}
		else
		{
			EngineTorque = Clamp<FLOAT>(Abs(Vehicle->OutputGas), -1.0, 1.0) * MaxEngineTorque;
		}

		bInvertTorque = (Vehicle->OutputGas < 0.f);
		bForceOnTarget = false;
	}

	if ( R.GetAxis(2).Z < Vehicle->WalkableFloorZ )
	{
		if ( (Vehicle->OutputGas > 0.f) == (R.GetAxis(0).Z > 0.f) )
		{
			// Kill torque if trying to go up
			EngineTorque = 0.f;
		}
	}


	if (Vehicle->OutputSteering != 0.f)
	{
		FLOAT InsideTrackFactor;
		if( Abs(Vehicle->OutputGas) > 0.f )
		{
			// Smoothly modify inside track speed as we steer
			InsideTrackFactor = Lerp(0.5f, InsideTrackTorqueFactor, Abs(Vehicle->OutputSteering));
		}
		else
		{
			InsideTrackFactor = -0.5f;
		}

		//FLOAT InsideTrackFactor = Clamp(InsideTrackTorqueCurve.Eval(Vehicle->ForwardVel, 0.0f), -1.0f, 1.0f);

		// 회전시 너무 느려서 조절하게 한다.
//		if ( Abs(Vehicle->OutputSteering) > 0.9f )
//			EngineTorque *= TurnEngineTorqueFactor;

		// Determine how to split up the torque based on the InsideTrackTorqueCurve
		FLOAT InsideTrackTorque = EngineTorque * InsideTrackFactor;
		FLOAT OutsideTrackTorque = EngineTorque * (1.0f - Abs(InsideTrackFactor)); 

		if (Vehicle->OutputSteering < 0.f) // Turn Right
		{
			LeftTrackTorque = OutsideTrackTorque; 
			RightTrackTorque = InsideTrackTorque;
		}
		else // Turn Left
		{	
			LeftTrackTorque = InsideTrackTorque;
			RightTrackTorque = OutsideTrackTorque;
		}
	}
	else
	{
		// If not steering just split up the torque equally between the two tracks
		LeftTrackTorque = EngineTorque * 0.5f;
		RightTrackTorque = EngineTorque * 0.5f;
	}


	// Invert torques when you want to drive backwards.
	if(bInvertTorque)
	{
		LeftTrackTorque *= -1.f;
		RightTrackTorque *= -1.f;
	}

	// 속도와 토크의 뱡향이 다를 경우,
	if ( LeftTrackVel * LeftTrackTorque < 0 )
		LeftTrackVel = 0.0f;
	if ( RightTrackVel * RightTrackTorque < 0 )
		RightTrackVel = 0.0f;

	LeftTrackVel += (LeftTrackTorque - (LeftTrackVel * EngineDamping)) * DeltaTime;
	RightTrackVel += (RightTrackTorque - (RightTrackVel * EngineDamping)) * DeltaTime;


	UavaVehicleSimTank* DefaultTank = (UavaVehicleSimTank*)GetClass()->GetDefaultObject();
	if(avaV->bFrontalCollision)
	{
		WheelLongExtremumValue = DefaultTank->WheelLongExtremumValue * FrontalCollisionGripFactor;
		WheelLongAsymptoteValue = DefaultTank->WheelLongAsymptoteValue * FrontalCollisionGripFactor;
	}
	else
	{
		WheelLongExtremumValue = DefaultTank->WheelLongExtremumValue;
		WheelLongAsymptoteValue = DefaultTank->WheelLongAsymptoteValue;
	}

	// Do the simulation for each wheel.
	ApplyWheels(LeftTrackVel,RightTrackVel,Vehicle);
#endif // WITH_NOVODEX
}

void AavaVehicle_Leopard::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	// client side effects follow - return if server
	//@todo steve - skip if far away
	if ( LastRenderTime < WorldInfo->TimeSeconds - 0.2f || bDeadVehicle )
		return;

	FLOAT WheelRadius = Wheels(0)->WheelRadius;

	FLOAT LeftTrackSpeed = ( Cast<USVehicleSimTank>(SimObj)->LeftTrackVel * WheelRadius );
	FLOAT RightTrackSpeed = ( Cast<USVehicleSimTank>(SimObj)->RightTrackVel * WheelRadius );

	// PHYS_RigidBody가 아닌 경우에는 바퀴가 돌지 않도록 한다.
	if ( Physics == PHYS_None )
	{
		LeftTrackSpeed = 0;
		RightTrackSpeed = 0;
	}

	LeftTreadSpeed += DeltaSeconds * (LeftTrackSpeed * 0.001f);
	RightTreadSpeed += DeltaSeconds * (RightTrackSpeed * 0.001f);

	// pan tread materials
	if ( LeftTreadMaterialInstance )
	{
		LeftTreadMaterialInstance->SetScalarParameterValue(TreadSpeedParameterName, LeftTreadSpeed);
	}
	if ( RightTreadMaterialInstance )
	{
		RightTreadMaterialInstance->SetScalarParameterValue(TreadSpeedParameterName, RightTreadSpeed); //RightTrackSpeed * TexPan2UnrealScale);
	}

	// rotate other wheels
	if ( Velocity.SizeSquared() > 0.01 )
	{											
		FLOAT LeftRotation = Wheels(7)->CurrentRotation;
		USkelControlWheel* SkelControl = Cast<USkelControlWheel>( Mesh->FindSkelControl(LeftBigWheel) );
		if(SkelControl)
		{
			SkelControl->UpdateWheelControl( 0.f, LeftRotation, 0.f); //LeftBigWheelRot, 0.f );
		}

		for ( int i=0; i<7; i++ )
		{
			SkelControl = Cast<USkelControlWheel>( Mesh->FindSkelControl(LeftSmallWheels[i]) );
			if(SkelControl)
			{
				SkelControl->UpdateWheelControl( 0.f, LeftRotation, 0.f );
			}
		}

		FLOAT RightRotation = Wheels(16)->CurrentRotation;

		SkelControl = Cast<USkelControlWheel>( Mesh->FindSkelControl(RightBigWheel) );
		if(SkelControl)
		{
			SkelControl->UpdateWheelControl( 0.f, RightRotation, 0.f); 
		}

		for ( int i=0; i<7; i++ )
		{
			SkelControl = Cast<USkelControlWheel>( Mesh->FindSkelControl(RightSmallWheels[i]) );
			if(SkelControl)
			{
				SkelControl->UpdateWheelControl( 0.f, RightRotation, 0.f );
			}
		}
	}

	// --Dave-TODO
	//if ( Driver )
	//{		
	//	FLOAT EngineRPM = Cast<UUTVehicleSimCar>(SimObj)->EngineRPMCurve.Eval(ForwardVel, 0.0f);
	//	FName EffectName(TEXT("goliathexhaust"));
	//	VehicleEffects(2).EffectRef->SetFloatParameter(EffectName,EngineRPM);
	//	VehicleEffects(3).EffectRef->SetFloatParameter(EffectName,EngineRPM);
	//}
}