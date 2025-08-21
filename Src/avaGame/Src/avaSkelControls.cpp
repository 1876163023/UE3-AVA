#include "PrecompiledHeaders.h"
#include "avaGame.h"

IMPLEMENT_CLASS(UavaSkelControl_CantileverBeam);
IMPLEMENT_CLASS(UavaSkelControl_Twist);
IMPLEMENT_CLASS(UavaSkelControl_TankTread);
IMPLEMENT_CLASS(UavaSkelControl_TurretConstrained);

/** 
 * ClacDist - Takes 2 yaw values A & B and calculates the distance between them. 
 *
 *	@param	YawA	- First Yaw
 *  @param	YawB	- Second Yaw
 *  @param	Dist	- The distance between them is returned here
 *  
 *	@return			- Returns the sign needed to move from A to B
*/

static INT CalcDist(INT YawA, INT YawB, INT& Dist)
{
	INT Sign = 1;

	Dist = YawA - YawB;
	if ( Abs(Dist) > 32767 )
	{
		if (Dist > 0)
		{
			Sign = -1;
		}
		Dist = Abs( Abs(Dist) - 65536 );
	}
	else
	{
		if (Dist < 0)
		{
			Sign = -1;
		}
	}
	return Sign;
}


//-----------------------------------------------------------------------------
//	avaSkelControl_CantileverBeam
//-----------------------------------------------------------------------------

void UavaSkelControl_CantileverBeam::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	Super::TickSkelControl(DeltaSeconds, SkelComp);

	WorldSpaceGoal = (SkelComp->LocalToWorld.GetOrigin())+InitialWorldSpaceGoalOffset;
	FVector DistFromGoal = TargetLocation - WorldSpaceGoal;

	FVector Force = (DistFromGoal *-SpringStiffness);
	Force -= (SpringDamping * Velocity);
	// apply force to Velocity:
	Velocity += (Force*DeltaSeconds);
	TargetLocation += (Velocity*DeltaSeconds);

	// 너무 심하게 흔들리는 부분 수정.

	// 최대거리의 제곱.
	FLOAT MaxTargetDistSq = MaxTargetDistance * MaxTargetDistance;
	FLOAT MaxTargetSpeedSp = MaxTargetSpeed * MaxTargetSpeed;

	// 속도의 크기가 최대거리보다 크다면 줄여준다.
	if ( Velocity.SizeSquared() > MaxTargetSpeedSp )
	{
		Velocity.Normalize();
		Velocity *= MaxTargetSpeed;
	}

	DistFromGoal = TargetLocation - WorldSpaceGoal;
	// 최대 거리를 넘는다면 줄여준다.
	if ( DistFromGoal.SizeSquared() > MaxTargetDistSq )
	{
		DistFromGoal.Normalize();
		DistFromGoal *= MaxTargetDistance;
		
		// 대상위치를 수정해 준다.
		TargetLocation = WorldSpaceGoal - DistFromGoal;
	}
}

//-----------------------------------------------------------------------------
//	avaSkelControl_Twist
//-----------------------------------------------------------------------------

void UavaSkelControl_Twist::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	Super::TickSkelControl(	DeltaSeconds, SkelComp);

	if ( !SkelComp || !SkelComp->GetOwner() )	return;

	AavaPawn* PawnOwner = Cast<AavaPawn>(SkelComp->GetOwner());
	if (!PawnOwner)								return;

	// If we are being forced to look at something, calculate the needed rotation
	if (bForcedLookAt)
	{
		if ( !bRecentering )
		{
			FRotator NewRot;

			if (ForceFocalActor)
			{
				NewRot = (PawnOwner->Location - ForceFocalActor->Location).Rotation();
			}
			else
			{
				NewRot = (PawnOwner->Location - ForceFocalPoint ).Rotation();
			}
			HeadYaw = NewRot.Yaw & 65535;
		}
		else
		{
			INT LookYaw = PawnOwner->LookYaw & 65535;
			INT TotalTolerance = 0;
			for (INT i=0;i<TwistData.Num()-1;i++)
			{
				TotalTolerance += TwistData(i).BoneYawTolerance;
			}

			INT Dist = (HeadYaw - LookYaw) & 65535;

			if (Dist>32767)
			{
				Dist = Abs(Dist - 65535);
				if (Dist > TotalTolerance)
					HeadYaw += Dist - TotalTolerance;
			}
			else if (Dist>TotalTolerance)
			{
				HeadYaw -= Dist - TotalTolerance;
			}

			HeadYaw = SkelComp->GetOwner()->fixedTurn(HeadYaw, LookYaw, appTrunc(16384 * DeltaSeconds));
			if ( (HeadYaw & 65535) == (LookYaw & 65535) )
			{
				bRecentering = false;
				bForcedLookAt = false;
				for (INT i=0;i<TwistData.Num();i++)
				{
					TwistData(i).BoneYawOffset = PawnOwner->LookYaw;
					TwistData(i).BoneAdj = 0;
				}
			}
		}

	}
	else	// Just use the LookYaw
	{
		HeadYaw = PawnOwner->LookYaw;
	}

	HeadYaw &= 65535;

	// Initialize everything to the current value

	if (!bInitialized)
	{
		for (INT i=0;i<TwistData.Num();i++)
		{
			TwistData(i).BoneIndex = SkelComp->MatchRefBone(TwistData(i).BoneName);
			TwistData(i).BoneYawOffset = HeadYaw;
		}
		LastHeadYaw = HeadYaw;

		for (INT i=0;i<LeanData.Num();i++)
		{
			LeanData(i).BoneIndex = SkelComp->MatchRefBone(LeanData(i).BoneName);
		}

		bInitialized = true;
	}

	if (bForcedLookAt)
	{
		// Figure out how far we have to turn.

		INT Dist;
		INT Sign = CalcDist(PawnOwner->LookYaw, HeadYaw, Dist);

		// Adjust each bone to be twisted to it's max

		for (INT i=0;i<TwistData.Num()-1;i++)
		{

			if ( Dist <= TwistData(i).BoneYawTolerance )
			{
				TwistData(i).BoneYawOffset = (Dist * Sign) & 65535;
				Dist = 0;
			}
			else
			{
				TwistData(i).BoneYawOffset = (TwistData(i).BoneYawTolerance * Sign) & 65535;
				Dist -= TwistData(i).BoneYawTolerance;
			}

			INT Rate = appTrunc(16384 * DeltaSeconds);
			if ( Abs(TwistData(i).BoneYawOffset) == TwistData(i).BoneYawTolerance )
			{
				Rate *= 2;
			}

			if (TwistData(i).BoneAdj != TwistData(i).BoneYawOffset)
			{
				TwistData(i).BoneAdj = SkelComp->GetOwner()->fixedTurn(TwistData(i).BoneAdj,TwistData(i).BoneYawOffset, Rate);
			}
		}

		TwistData(TwistData.Num()-1).BoneYawOffset = PawnOwner->LookYaw;
		TwistData(TwistData.Num()-1).BoneAdj = 0;
	}
	else
	{
		// Head is always right on
		TwistData(0).BoneYawOffset = HeadYaw;

		// Figure out the rest
		INT TotalDist = 0;
		UBOOL bInMotion = false;

		for (INT i=1;i<TwistData.Num();i++)
		{

			INT Dist;
			INT Sign = CalcDist(TwistData(i-1).BoneYawOffset, TwistData(i).BoneYawOffset, Dist);

			if ( PawnOwner->Velocity.Size() > 0 )
			{
				TwistData(i).BoneYawOffset = HeadYaw;
			}
			// If we are greater than the tolerance, set it to the tolerance and flag the bone as in motion
			else if ( Abs(Dist) > TwistData(i).BoneYawTolerance )
			{
				TwistData(i).BoneYawOffset = ( TwistData(i-1).BoneYawOffset - ( TwistData(i).BoneYawTolerance * Sign) ) & 65535;

				if ( PawnOwner->Velocity.Size() > 0 )
				{
					if ( DELEGATE_IS_SET( OwnerNotification ) )
					{
						delegateOwnerNotification(TwistData(i).BoneName,TwistData(i).BoneYawOffset);
					}
					TwistData(i).bInMotion = true;
				}
			}

			// If we are in motion AND the distance is > 0 then move the offset
			else if ( Abs(Dist) > 0 && TwistData(i).bInMotion )
			{
				//TwistData(i).BoneYawOffset = SkelComp->GetOwner()->fixedTurn(TwistData(i).BoneYawOffset, HeadYaw, appTrunc(16384 * DeltaSeconds));
				TwistData(i).BoneYawOffset = SkelComp->GetOwner()->fixedTurn(TwistData(i).BoneYawOffset, HeadYaw, appTrunc(4096 * DeltaSeconds));
			}

			// If we are at rest, 
			if (TwistData(i).BoneYawOffset == HeadYaw)
			{
				TwistData(i).bInMotion = false;

				if ( DELEGATE_IS_SET( OwnerNotification ) )
				{
					delegateOwnerNotification(TwistData(i).BoneName,TwistData(i).BoneYawOffset);
				}
			}

			TotalDist += Dist;
			if ( TwistData(i).bInMotion )
			{
				bInMotion = true;
			}
		}

		INT Sec = TwistData.Num()-1;

		// Position the hips
		TwistData(Sec).BoneAdj = (TwistData(Sec).BoneYawOffset - HeadYaw) & 65535;

		if ( TwistData(Sec).BoneAdj != 0 )	TotalDist = 1;

		while(Sec>0)
		{
			// Now calculate the postion of each bone above the hips releative to the one below it
			Sec--;
			TwistData(Sec).BoneAdj = (TwistData(Sec).BoneYawOffset - TwistData(Sec+1).BoneYawOffset) & 65535;
			if ( TwistData(Sec).BoneAdj != 0 )	TotalDist = 1;
		}

		// Finally, handling auto-recentering
		if ( LastHeadYaw == HeadYaw && TotalDist != 0 && !bInMotion && GWorld->GetWorldInfo()->TimeSeconds - LastZeroed > RecenteringTime )
		{
			for (INT i=1;i<TwistData.Num();i++)
			{
				if ( DELEGATE_IS_SET( OwnerNotification ) )
				{
					delegateOwnerNotification(TwistData(i).BoneName,TwistData(i).BoneYawOffset);
				}
				TwistData(i).bInMotion = true;
			}

			LastZeroed = GWorld->GetWorldInfo()->TimeSeconds;

		}
		else if (TotalDist == 0 || LastHeadYaw!=HeadYaw)
		{
			LastZeroed = GWorld->GetWorldInfo()->TimeSeconds;
		}
	}

	LastHeadYaw = HeadYaw;
}

void UavaSkelControl_Twist::GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices)
{
	check(OutBoneIndices.Num() == 0);

	if (!bDormant)
		OutBoneIndices.AddItem(BoneIndex);
}

void UavaSkelControl_Twist::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	check(OutBoneTransforms.Num() == 0);

	// Find the Twist Index for this bone
	INT TwistIndex=-1;
	for (INT i=0;i<TwistData.Num();i++)
	{
		if (TwistData(i).BoneIndex == BoneIndex)
		{
			TwistIndex = i;
			break;
		}
	}

	// Find the Lean index for this bone
	INT LeanIndex=-1;
	for (INT i=0;i<LeanData.Num();i++)
	{
		if (LeanData(i).BoneIndex == BoneIndex)
		{
			LeanIndex = i;
			break;
		}
	}

	// Get the Bone's current matrix
	FMatrix NewBoneTM = SkelComp->SpaceBases(BoneIndex);
	AavaPawn* PawnOwner = Cast<AavaPawn>(SkelComp->GetOwner());

	if (PawnOwner)
	{
		// SpaceBases are in component space - so we need to calculate the BoneRotationSpace -> Component transform
		FMatrix ComponentToFrame = SkelComp->CalcComponentToFrameMatrix(BoneIndex, BCS_ActorSpace, NAME_None);
		ComponentToFrame.SetOrigin( FVector(0.f) );

		INT AdjPitch = 0;
		INT AdjYaw = 0;

		// Handling leaning

		if ( LeanIndex >= 0 )
		{

			// We attempt to use the Controller's pitch if it's available, otherwise
			// we rely on the RemoteViewPitch

			INT LookPitch = 0;

			if (PawnOwner && PawnOwner->Controller)
				LookPitch = PawnOwner->Controller->Rotation.Pitch;

			if (LookPitch==0)
			{
				LookPitch = PawnOwner->LastRotation.Roll;
			}

			if (LookPitch > 32767)
			{
				LookPitch -= 65535;
			}

			LookPitch = Clamp<INT>(LookPitch,-16384,16384);

			FLOAT Perc = FLOAT( Abs(LookPitch) / 16384.0f );

			if (LookPitch<0)
			{
				AdjPitch = INT ( FLOAT(LeanData(LeanIndex).BonePitchLowerTolerance) * Perc);
			}
			else if (LookPitch > 0)
			{
				AdjPitch = INT ( FLOAT(LeanData(LeanIndex).BonePitchUpperTolerance) * Perc);
			}
		}

		// Apply the twist

		if (TwistIndex >= 0)
		{
			AdjYaw = TwistData(TwistIndex).BoneAdj;
		}

		// Build the new rotation matrix
		const FRotator BoneRotation( AdjPitch,AdjYaw,0);

		// Add to existing rotation.
		FMatrix RotInComp = NewBoneTM * (ComponentToFrame * FRotationMatrix(BoneRotation) * ComponentToFrame.Inverse());
		RotInComp.SetOrigin( NewBoneTM.GetOrigin() );
		NewBoneTM = RotInComp;
	}

	OutBoneTransforms.AddItem(NewBoneTM);
}


/************************************************************************************
* avaSkelControl_TankTread
*
* Tank Treads
************************************************************************************/

FLOAT UavaSkelControl_TankTread::CalcAdjustment(FVector TraceStart, FVector TraceEnd, FVector Offsets, AActor* Owner)
{
	TraceStart = TraceStart + (FRotationMatrix(Owner->Rotation).TransformNormal(Offsets));
	TraceEnd = TraceEnd + (FRotationMatrix(Owner->Rotation).TransformNormal(Offsets));

	FCheckResult Hit(1.0f);

	//GWorld->LineBatcher->DrawLine(TraceStart, TraceEnd, FColor(255,255,255));
	//GWorld->LineBatcher->DrawLine(TraceStart, FVector(0,0,0), FColor(255,255,0));
	//GWorld->LineBatcher->DrawLine(TraceEnd,   FVector(0,0,0), FColor(255,255,0));


	if ( !GWorld->SingleLineCheck(Hit, Owner, TraceEnd , TraceStart, TRACE_AllBlocking ) )
	{
		//GWorld->LineBatcher->DrawLine(Hit.Location, FVector(0,0,0), FColor(0,255,0));

		return SpaceAbove - (TraceStart - Hit.Location).Size();
	}
	else
		return SpaceBelow * -1;
}

void UavaSkelControl_TankTread::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	check(SkelComp);

	if ( SkelComp && !bInitialized && TreadBone != NAME_None )
	{
		TreadIndex = SkelComp->MatchRefBone( TreadBone );
		Adjustment = 0.0f;
		TargetAdjustment = 0.0f;
		bInitialized = true;
	}

	Super::TickSkelControl(DeltaSeconds, SkelComp);

	if (bInitialized)
	{
		AActor* Owner = SkelComp->GetOwner();

		//FLOAT Dist = 0.0f;
		if (Owner != NULL && Owner->Velocity.SizeSquared() > 10.f)
		{
			const FVector TotalScale = Owner->DrawScale * Owner->DrawScale3D;
			const FVector BoneLoc = TotalScale * (SkelComp->SkeletalMesh->RefSkeleton(TreadIndex).BonePos.Position);	

			FMatrix ActorToWorld = Owner->LocalToWorld();
			ActorToWorld.RemoveScaling();

			FMatrix CompToWorld = SkelComp->LocalToWorld;
			CompToWorld.RemoveScaling();

			const FVector TraceStart = CompToWorld.TransformFVector( BoneLoc ) + ActorToWorld.TransformNormal( FVector(0,0,SpaceAbove));
			const FVector TraceEnd = CompToWorld.TransformFVector(BoneLoc)  - ActorToWorld.TransformNormal( FVector(0,0, SpaceBelow ));

			// + Debug

			TargetAdjustment = CalcAdjustment(TraceStart, TraceEnd, FVector(0,CenterOffset,0), Owner);

			bLastDirWasBackwards = ((Owner->Velocity | Owner->Rotation.Vector()) < 0.f);

			for (INT i = 0; i < AlternateScanOffsets.Num(); i++)
			{
				if ( bAlwaysScan || (bLastDirWasBackwards && AlternateScanOffsets(i) < 0) || (!bLastDirWasBackwards && AlternateScanOffsets(i)>0) )
				{
					const FLOAT SecondaryAdj = CalcAdjustment(TraceStart, TraceEnd, FVector(AlternateScanOffsets(i),CenterOffset,0), Owner);
					if (SecondaryAdj > TargetAdjustment)
					{
						TargetAdjustment = SecondaryAdj;
					}
				}
			}

			if (Adjustment > TargetAdjustment)
			{
				const FLOAT NewAdj = Adjustment - ( (Adjustment-TargetAdjustment) * DeltaSeconds * 4);
				Adjustment = Clamp<FLOAT>( NewAdj, TargetAdjustment, Adjustment);
			}
			else if (Adjustment < TargetAdjustment)
			{
				const FLOAT NewAdj = Adjustment + ( (TargetAdjustment - Adjustment) * DeltaSeconds * 4);
				Adjustment = Clamp<FLOAT>(NewAdj, Adjustment, TargetAdjustment);
			}
		}
	}

}

void UavaSkelControl_TankTread::CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms)
{
	checkSlow(OutBoneTransforms.Num() == 0);

	FMatrix NewBoneTM = SkelComp->SpaceBases(BoneIndex);

	if (bInitialized)
	{
		// Find the distance from the wheel to the ground

		FVector NewOrigin = NewBoneTM.GetOrigin() + FVector(0,0,Adjustment+2);
		NewBoneTM.SetOrigin(NewOrigin);
	}

	OutBoneTransforms.AddItem(NewBoneTM);
}

void UavaSkelControl_TankTread::GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices)
{
	checkSlow(OutBoneIndices.Num() == 0);
	if (!bDormant)
	{
		OutBoneIndices.AddItem(BoneIndex);
	}
}

/**********************************************************************************
* avaSkelControl_TurretConstrained
*
* Most of the ava Vehicles use this class to constrain their weapon turrets
**********************************************************************************/

/**
* This function will check a rotational value and make sure it's constrained within
* a given angle.  It returns the value
*/
static INT CheckConstrainValue(INT Rotational, INT MinAngle, INT MaxAngle)
{
	INT NormalizedRotational = Rotational & 65535;

	if (NormalizedRotational > 32767)
	{
		NormalizedRotational = NormalizedRotational - 65535;
	}

	// Convert from Degrees to Unreal Units

	MinAngle = appTrunc( FLOAT(MinAngle) * 182.0444);
	MaxAngle = appTrunc( FLOAT(MaxAngle) * 182.0444);

	if ( NormalizedRotational > MaxAngle )
	{
		return MaxAngle;
	}
	else if ( NormalizedRotational < MinAngle )
	{
		return MinAngle;
	}

	return NormalizedRotational;
}

/**
* This function performs the magic.  It will attempt to rotate the turret to face the rotational specified in DesiredBoneRotation.
* 
*/
void UavaSkelControl_TurretConstrained::TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp)
{
	AavaVehicle* OwnerVehicle = Cast<AavaVehicle>(SkelComp->GetOwner());
	if ( bFixedWhenFiring && OwnerVehicle )
	{
		if (OwnerVehicle->SeatFlashLocation(AssociatedSeatIndex, FVector(0,0,0),TRUE) != FVector(0,0,0) ||
			OwnerVehicle->SeatFlashCount(AssociatedSeatIndex,0,TRUE) > 0 )
		{
			return;
		}
	}

	if ( bResetWhenUnattended && OwnerVehicle &&
		OwnerVehicle->Seats.IsValidIndex(AssociatedSeatIndex) &&
		(OwnerVehicle->SeatMask & (1 << AssociatedSeatIndex)) == 0 )
	{
		StrengthTarget = 0.0;
		ControlStrength = 0.0;
		Super::TickSkelControl(DeltaSeconds, SkelComp);
		return;
	}
	else
	{
		StrengthTarget = 1.0;
		ControlStrength = 1.0;
	}

	FVector LocalDesiredVect; 
	FRotator LocalDesired;
	FMatrix LocalToWorld;

	// Convert the Desired to Local Space

	LocalToWorld = SkelComp->LocalToWorld;
	LocalDesiredVect = LocalToWorld.InverseTransformNormal( DesiredBoneRotation.Vector() );
	LocalDesired = LocalDesiredVect.Rotation();

	LocalDesired.Yaw	*= bInvertYaw   ? -1 : 1;
	LocalDesired.Pitch	*= bInvertPitch ? -1 : 1;
	LocalDesired.Roll	*= bInvertRoll  ? -1 : 1;

	// Constrain the Desired Location.


	// Look up the proper step givin the current yaw

	FTurretConstraintData FMinAngle = MinAngle;
	FTurretConstraintData FMaxAngle = MaxAngle;

	INT NormalizedYaw = LocalDesired.Yaw & 65535;

	for (INT i=0;i<Steps.Num(); i++)
	{
		if ( NormalizedYaw >= Steps(i).StepStartAngle && NormalizedYaw <= Steps(i).StepEndAngle )
		{
			FMinAngle = Steps(i).MinAngle;
			FMaxAngle = Steps(i).MaxAngle;
			break;
		}
	}

	// constrain the rotation
	if (bConstrainYaw)
	{
		LocalDesired.Yaw = CheckConstrainValue(LocalDesired.Yaw, FMinAngle.YawConstraint,FMaxAngle.YawConstraint);
	}
	if (bConstrainPitch)
	{
		LocalDesired.Pitch = CheckConstrainValue(LocalDesired.Pitch, FMinAngle.PitchConstraint,FMaxAngle.PitchConstraint);
	}
	if (bConstrainRoll)
	{
		LocalDesired.Roll = CheckConstrainValue(LocalDesired.Roll, FMinAngle.RollConstraint,FMaxAngle.RollConstraint);
	}

	// If we are not Pointing at the desired rotation, rotate towards it
	FRotator LocalConstrainedBoneRotation = LocalToWorld.InverseTransformNormal(ConstrainedBoneRotation.Vector()).Rotation().Denormalize();
	if (LocalConstrainedBoneRotation != LocalDesired)
	{
		if (LagDegreesPerSecond>0 && SkelComp->GetOwner() && !bIgnoreOnceLagDegreesPerSecond)
		{
			INT DeltaDegrees = appTrunc((LagDegreesPerSecond * 182.0444) * DeltaSeconds);

			if (LocalConstrainedBoneRotation.Yaw != LocalDesired.Yaw)
			{
				LocalConstrainedBoneRotation.Yaw = SkelComp->GetOwner()->fixedTurn(LocalConstrainedBoneRotation.Yaw, LocalDesired.Yaw, DeltaDegrees);
			}

			if (LocalConstrainedBoneRotation.Pitch != LocalDesired.Pitch)
			{
				LocalConstrainedBoneRotation.Pitch = SkelComp->GetOwner()->fixedTurn(LocalConstrainedBoneRotation.Pitch, LocalDesired.Pitch, DeltaDegrees);
			}

			if (LocalConstrainedBoneRotation.Roll != LocalDesired.Roll)
			{
				LocalConstrainedBoneRotation.Roll = SkelComp->GetOwner()->fixedTurn(LocalConstrainedBoneRotation.Roll, LocalDesired.Roll, DeltaDegrees);
			}
		}
		else
		{
			LocalConstrainedBoneRotation = LocalDesired;

//			debugf(TEXT("vaSkelControl_TurretConstrained::TickSkelControl [%d:%s] (%d,%d,%d)"), AssociatedSeatIndex, ControlName.GetName(), 
//				DesiredBoneRotation.Pitch, DesiredBoneRotation.Yaw, DesiredBoneRotation.Roll);

			// 1번만 바로 적용된다.
			bIgnoreOnceLagDegreesPerSecond = FALSE;
		}
	}
	// set the bone rotation to the final clamped value
	UBOOL bNewInMotion;
	if(BoneRotation == LocalConstrainedBoneRotation)
	{
		bNewInMotion = FALSE;
	}
	else
	{
		bNewInMotion = TRUE;
		BoneRotation = LocalConstrainedBoneRotation;
	}

	// also save the current world space rotation for future ticks
	// this is so that the movement of the actor/component won't affect the rotation rate
	ConstrainedBoneRotation = LocalToWorld.TransformNormal(LocalConstrainedBoneRotation.Vector()).Rotation();

	// find out if we're still in motion and call delegate if the status has changed

	if (bNewInMotion != bIsInMotion)
	{
		bIsInMotion = bNewInMotion;

		// Notify anyone listening

		if (DELEGATE_IS_SET(OnTurretStatusChange))
		{
			delegateOnTurretStatusChange(bIsInMotion);
		}
	}
	Super::TickSkelControl(DeltaSeconds, SkelComp);
}

