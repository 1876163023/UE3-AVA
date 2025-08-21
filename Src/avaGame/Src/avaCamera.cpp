#include "avaGame.h"

IMPLEMENT_CLASS(AavaPlayerCamera)

/**
* Smoothing function for interpolants.  Accelerates and decelerates along an
* exponential curve.  Should this go someplace more general, like with the
* other math functions in Object.uc?
* @param	f		The float you want the ramp function applied to.
* @param	exp		Exponent of the ramp function.  Higher = sharper accel/decel.
* @return			The float after the ramp has been applied.
*/
static FLOAT FInOutRamp(FLOAT f, FLOAT exp)
{
	if (f < 0.5f)
	{
		return ( 0.5f * (appPow((2.f * f), exp)) );
	}
	else
	{
		return ( ( -0.5f * ( appPow((2.f * (f-1.f)), exp) ) ) + 1 );
	}
}

static FVector WorldToLocal(FVector const& WorldVect, FRotator const& SystemRot)
{
	return FRotationMatrix(SystemRot).Transpose().TransformNormal( WorldVect );
}

static FVector LocalToWorld(FVector const& LocalVect, FRotator const& SystemRot)
{
	return FRotationMatrix(SystemRot).TransformNormal( LocalVect );
}

static UBOOL PawnIsAliveAndWell(const AavaPawn* WP)
{
	return ( (WP->Health > 0) && !WP->bHidden && !WP->bDeleteMe );
}

void AavaPlayerCamera::PreventCameraPenetration(APawn* P, const FVector& WorstLocation, FVector& DesiredLocation, FLOAT DeltaTime, FLOAT& DistBlockedPct, FLOAT CameraExtentScale, UBOOL bSingleRayOnly)
{	
	FLOAT HardBlockedPct = DistBlockedPct;
	FLOAT SoftBlockedPct = DistBlockedPct;

	FVector BaseRay = DesiredLocation - WorstLocation;
	FRotationMatrix BaseRayMatrix(BaseRay.Rotation());
	FVector BaseRayLocalUp, BaseRayLocalFwd, BaseRayLocalRight;
	BaseRayMatrix.GetAxes(BaseRayLocalFwd, BaseRayLocalRight, BaseRayLocalUp);

	FLOAT CheckDist = BaseRay.Size();

	FLOAT DistBlockedPctThisFrame = 1.f;

	FVector RayTarget;

	//FlushPersistentDebugLines();
	INT NumRaysToShoot = (bSingleRayOnly) ? Min(1, PenetrationAvoidanceFeelers.Num()) : PenetrationAvoidanceFeelers.Num();

	for (INT RayIdx=0; RayIdx<NumRaysToShoot; ++RayIdx)
	{
		FMemMark Mark(GMem);

		FPenetrationAvoidanceFeeler& Feeler = PenetrationAvoidanceFeelers(RayIdx);

		// calc ray target
		FVector RayTarget;
		{
			FVector RotatedRay = BaseRay.RotateAngleAxis(Feeler.AdjustmentRot.Yaw, BaseRayLocalUp);
			RotatedRay = RotatedRay.RotateAngleAxis(Feeler.AdjustmentRot.Pitch, BaseRayLocalRight);
			RayTarget = WorstLocation + RotatedRay;
		}

		// cast for world and pawn hits separately.  this is so we can safely ignore the 
		// camera's target pawn
		DWORD const TraceFlags = (Feeler.PawnWeight > 0.f) ? TRACE_World | TRACE_Pawns : TRACE_World;
		FVector const CheckExtent = Feeler.Extent * CameraExtentScale;

		FCheckResult const* pHitList = NULL;
		FCheckResult SingleHit;
		if ( (Feeler.PawnWeight >= 1.f) || (Feeler.WorldWeight >= 1.f) )
		{
			// for weight 1.0 rays, do multiline check to make sure we hits we throw out aren't
			// masking real hits behind (these are important rays).
			pHitList = GWorld->MultiLineCheck(GMem, RayTarget, WorstLocation, CheckExtent, TraceFlags, P);
		}
		else
		{
			// P is passed in as SourceActor, not "this", so hits for it are ignored.
			// make zero-extent?
			GWorld->SingleLineCheck(SingleHit, P, RayTarget, WorstLocation, TraceFlags, CheckExtent);
			SingleHit.Next = NULL; // not sure if this is necessary, but sanity to make sure below loop ends when it should
			pHitList = &SingleHit;
		}

		//DrawDebugLine(WorstLocation, RayTarget, Feeler.Weight*255, 255, 0, TRUE);
		//DrawDebugCoordinateSystem(WorstLocation, BaseRay.Rotation(), 32.f, TRUE);
		//DrawDebugLine(WorstLocation, WorstLocation + BaseRay, Feeler.Weight*255, 255, 0, TRUE);

		for( FCheckResult const* Hit = pHitList; Hit != NULL; Hit = Hit->GetNext() )
		{
			if (Hit->Actor != NULL)
			{
				FLOAT Weight;

				// experimenting with not colliding with pawns
				APawn* ThePawn = Hit->Actor->GetAPawn();
				if( ThePawn )
				{
					if ( P && (P->DrivenVehicle == ThePawn) )
					{
						// ignore hits on the vehicle we're driving
						continue;
					}

					// * ignore ragdoll hits (can still his the collision cylinder, which is bad)
					// * also ignore zero-time pawn hits, which can happen since the raycast extent
					// box can poke out of the player's collision cylinder and register hits
					// even if the raycast goes away from the pawn
					// * also ignore avapawns that don't block the camera
					AavaPawn const* const AP = Cast<AavaPawn>(ThePawn);
					if ( AP && ( /*!AP->bBlockCamera || */(AP->Physics == PHYS_RigidBody) || (Hit->Time == 0.f) ) )
					{
						continue;
					}
					else
					{
						/*// ignore hits that hit the vehicle for the avaweaponpawn we're controlling
						AavaWeaponPawn const* const WAP = Cast<AavaWeaponPawn>(P);
						if ( WAP && (WAP->MyVehicle == ThePawn) )
						{
							continue;
						}*/
					}

					Weight = Feeler.PawnWeight;
				}
				else
				{
					// ignore KActorSpawnables, since they're only used for small, inconsequential things (for now anyway)
					if (Cast<AKActorSpawnable>(Hit->Actor))
					{
						continue;
					}

					Weight = Feeler.WorldWeight;
				}

				FLOAT NewBlockPct	= Hit->Time;
				NewBlockPct += (1.f - NewBlockPct) * (1.f - Weight);
				DistBlockedPctThisFrame = Min(NewBlockPct, DistBlockedPctThisFrame);
			}
		}


		// collide with any extra collision planes the camera mode may want to use
		// @fixme, if this gets uncommented, move the eventGetAuxCollisionPlane() call out of the for loop
		//FVector PlaneNormal, PlaneOrigin;
		//if (CurrentCamMode->eventGetAuxCollisionPlane(P, PlaneOrigin, PlaneNormal))
		//{
		//	FVector PlaneIntersect = FLinePlaneIntersection(WorstLocation, DesiredLocation, PlaneOrigin, PlaneNormal);
		//	FLOAT NewBlockPct = (PlaneIntersect - WorstLocation).Size() / CheckDist;
		//	NewBlockPct += (1.f - NewBlockPct) * (1.f - Feeler.WorldWeight);
		//	FLOAT NewBlockPct2 = (PlaneIntersect - DesiredLocation).Size() / CheckDist;
		//	if( (NewBlockPct < DistBlockedPctThisFrame) && (NewBlockPct2 < 1.f) )
		//	{
		//		DistBlockedPctThisFrame = NewBlockPct;
		//	}
		//}

		if (RayIdx == 0)
		{
			// don't interpolate toavad this one, snap to it
			HardBlockedPct = DistBlockedPctThisFrame;
		}
		else
		{
			SoftBlockedPct = DistBlockedPctThisFrame;
		}

		Mark.Pop();
	}

	if (DistBlockedPct < DistBlockedPctThisFrame)
	{
		// interpolate smoothly out
		DistBlockedPct = DistBlockedPct + DeltaTime / PenetrationBlendOutTime * (DistBlockedPctThisFrame - DistBlockedPct);
	}
	else
	{
		if (DistBlockedPct > HardBlockedPct)
		{
			DistBlockedPct = HardBlockedPct;
		}
		else if (DistBlockedPct > SoftBlockedPct)
		{
			// interpolate smoothly in
			DistBlockedPct = DistBlockedPct - DeltaTime / PenetrationBlendInTime * (DistBlockedPct - SoftBlockedPct);
		}
	}

	if( DistBlockedPct < 1.f ) 
	{
		DesiredLocation	= WorstLocation + (DesiredLocation - WorstLocation) * DistBlockedPct;
	}
}

void AavaPlayerCamera::PlayerUpdateCameraNative(APawn* Pawn, FLOAT DeltaTime, FTViewTarget& OutVT)
{
	static FLOAT LastTime = 0;

	if (GWorld->GetTimeSeconds() <= LastTime)
	{
		DeltaTime = 0.0f;
	}

	LastTime = GWorld->GetTimeSeconds();

	// this can legitimately be NULL if the target isn't a avaPawn (such as a vehicle or turret)
	AavaPawn* AP = Cast<AavaPawn>(Pawn);

	if ( AP == NULL )
	{
		AVehicle* VP = Cast<AVehicle>(Pawn);
		if ( VP )
		{
			AP = Cast<AavaPawn>(VP->Driver);
		}
	}

	// RisingDust에 난입하고 나면 Vehicle.Driver가 NULL일 수 있나 보다. (2007/08/02 고광록)
	if ( AP == NULL )
	{
		debugf(TEXT("avaPlayerCamera::PlayerUpdateCameraNative - %s is NOT avaPawn or avaVehicle(Driver)!"), *Pawn->GetName());
		return ;
	}

	// "effective" AP is the avaPawn that's connected to the camera, be it 
	// in a vehicle or turret or whatever.
	//AavaPawn* EffectiveAP = AP;
	//if (!EffectiveAP)
	//{
	//	AVehicle* VP = Cast<AVehicle>(P);
	//	if (VP)
	//	{
	//		EffectiveAP = Cast<AavaPawn>(VP->Driver);
	//	}
	//}	

	FRotator	IdealCameraOriginRot;
	FVector		IdealCameraOrigin;
	FVector		HeadBoneLocation;
	static FName FNameHead( TEXT("Bip01_Head" ) );

	if ( AP->Mesh != NULL )	
	{
		HeadBoneLocation =  AP->Mesh->GetBoneLocation( FNameHead );
	}
	else
	{
		HeadBoneLocation =	AP->Location;
		HeadBoneLocation.Z += AP->BaseEyeHeight;
	}
	{			
		AP->eventGetActorEyesViewPoint(IdealCameraOrigin, IdealCameraOriginRot);

		if (AP)
		{
			//@fixme, this is a bit of a hack, but WarPawn::GetPawnViewLocation does some
			// stuff with the neck bone, which screws up some offsets we've already figured out.
			// ideally, we'll adjust those values and just call GetPawnViewLocation() here.

			
			IdealCameraOrigin = HeadBoneLocation;
			//IdealCameraOrigin = P->Location;
			//IdealCameraOrigin.Z += P->BaseEyeHeight;
			IdealCameraOrigin += ::LocalToWorld(eventGetCameraOriginOffset(AP), AP->Rotation);
		}
	}

	// First, update the camera origin.
	// This is the point in world space where camera offsets are applied.
	// We apply lazy cam on this location, so we can have a smooth / slow interpolation speed there,
	// And a different speed for offsets.
	FVector ActualCameraOrigin;
	{
		if( bResetCameraInterpolation )
		{
			// if this is the first time we update, then do not interpolate.
			ActualCameraOrigin = IdealCameraOrigin;
		}
		else
		{
			// Apply lazy cam effect to the camera origin point
			ActualCameraOrigin	= VInterpTo(LastActualCameraOrigin, IdealCameraOrigin, DeltaTime, LazyCamSpeed);
		}
		LastActualCameraOrigin = ActualCameraOrigin;
	}

	// smooth out CameraOriginRot if necessary
	FRotator ActualCameraOriginRot = IdealCameraOriginRot;
	{
		AavaPlayerController* const APC = Cast<AavaPlayerController>(PCOwner);
		if (APC && !bResetCameraInterpolation)
		{
			FLOAT SpectatorCameraRotInterpSpeed = 14.0f;
			ActualCameraOriginRot = RInterpTo(LastActualCameraOriginRot, IdealCameraOriginRot, DeltaTime, SpectatorCameraRotInterpSpeed);
		}
		else
		{
			ActualCameraOriginRot = IdealCameraOriginRot;
		}
		LastActualCameraOriginRot = ActualCameraOriginRot;
	}

	// do any pre-viewoffset focus point adjustment
	//eventUpdateFocusPoint(P);

	// doing adjustment before view offset application in order to rotate around target
	// also doing before origin offset application to avoid pops in cover
	// using last viewoffset here, since we have a circular dependency with the data.  focus point adjustment
	// needs viewoffset, but viewoffset is dependent on results of adjustment.  this introduces a bit of error,
	// but I believe it won't be noticeable.
	//AdjustToFocusPointKeepingTargetInView(P, DeltaTime, ActualCameraOrigin, ActualCameraOriginRot, LastViewOffset);

	// Get the camera-space offset from the camera origin
	FVector IdealViewOffset = eventGetViewOffset(AP, DeltaTime, ActualCameraOriginRot);
	//`CamDLog("IdealViewOffset out of GetViewOffset is "@IdealViewOffset);

	// get the desired FOV
	OutVT.POV.FOV = eventGetDesiredFOV(AP);

	OutVT.POV.Rotation = ActualCameraOriginRot;	

	//
	// View relative offset
	//

	// Interpolate FOV
	if( !bResetCameraInterpolation && BlendTime > 0.f )
	{
		FLOAT InterpSpeed = 1.f / BlendTime;
		OutVT.POV.FOV = FInterpTo(LastCamFOV, OutVT.POV.FOV, DeltaTime, InterpSpeed);
	}
	LastCamFOV = OutVT.POV.FOV;

	// View relative offset.
	FVector ActualViewOffset;
	{
		if( !bResetCameraInterpolation && BlendTime > 0.f )
		{
			FLOAT InterpSpeed = 1.f / BlendTime;
			ActualViewOffset = VInterpTo(LastViewOffset, IdealViewOffset, DeltaTime, InterpSpeed);
		}
		else
		{
			ActualViewOffset = IdealViewOffset;
		}
		LastViewOffset = ActualViewOffset;
		//`CamDLog("ActualViewOffset post interp is "@ActualViewOffset);
	}	
	
	// apply viewoffset (in camera space)
	FVector DesiredCamLoc = ActualCameraOrigin + ::LocalToWorld(ActualViewOffset, OutVT.POV.Rotation);

	// try to have a focus point in view
	// AdjustToFocusPoint(P, DeltaTime, DesiredCamLoc, OutVT.POV.Rotation);

	// Set new camera position
	OutVT.POV.Location = DesiredCamLoc;

	// cache this up, for potential later use
	// LastPreModifierCameraRot = OutVT.POV.Rotation;

	// apply post processing modifiers
	eventApplyCameraModifiers(DeltaTime, OutVT.POV);

	//
	// find "worst" location, or location we will shoot the penetration tests from
	//
	FVector WorstLocation = eventGetCameraWorstCaseLoc(AP);

	// conv to local space for interpolation
	WorstLocation = ::WorldToLocal((WorstLocation - HeadBoneLocation), AP->Rotation);

	if (!bResetCameraInterpolation)
	{
		WorstLocation = VInterpTo(LastWorstLocationLocal, WorstLocation, DeltaTime, WorstLocInterpSpeed);
	}
	LastWorstLocationLocal = WorstLocation;

	// rotate back to world space
	WorstLocation = ::LocalToWorld(WorstLocation, AP->Rotation) + HeadBoneLocation;

	//clock(Time);

	//
	// test for penetration
	//
	//DrawDebugSphere(WorstLocation, 4, 16, 255, 255, 0, FALSE);

	// adjust worst location origin to prevent any penetration
	if (bValidateWorstLoc)
	{
		PreventCameraPenetration(AP, IdealCameraOrigin, WorstLocation, DeltaTime, WorstLocBlockedPct, WorstLocPenetrationExtentScale, TRUE);
	}
	else
	{
		WorstLocBlockedPct = 0.f;
	}

	//DrawDebugSphere(WorstLocation, 16, 10, 255, 255, 0, FALSE);

	// adjust final desired camera location, to again, prevent any penetration
	UBOOL bSingleRayPenetrationCheck = ( !bDoPredictiveAvoidance ) ? TRUE : FALSE;
	PreventCameraPenetration(AP, WorstLocation, OutVT.POV.Location, DeltaTime, PenetrationBlockedPct, PenetrationExtentScale, bSingleRayPenetrationCheck);	
}


//<@ 2007. 5. 22 changmin

FLOAT AavaCameraActor::ComputeFOV()
{
	const FLOAT RefResolution = Max( ReferenceResolution, 1024.0f );
	const FLOAT RefFOV = Max( FOVAngle, 10.0f );
	const FLOAT HalfFOVRadians = (RefFOV / 2.0f) * PI / 180.0f;

	FLOAT CurResolution = 0;
	if( GEngine && GEngine->GameViewport )
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize( ViewportSize );
		CurResolution = ViewportSize.X;
	}

	if( CurResolution > 0 )
	{
		FLOAT NewFOV = appAtan( appTan(HalfFOVRadians) * ( CurResolution / RefResolution ) ) * 2.0f;

		return NewFOV * 180.0f / PI;
	}
	else
	{
		return RefFOV;
	}
}
//>@ changmin
