//=============================================================================
// This is an ava player-specific camera
// Cameras define the Point of View of a player in world space.
// Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class avaPlayerCamera extends Camera
	config(Game)
	native(Camera);

var avaSoundScape			CurrentSoundScape;

/** resets camera interpolation. Set on first frame and teleports to prevent long distance or wrong camera interpolations. */
var					bool	bResetCameraInterpolation;

/**
 * Struct defining a feeler ray used for camera penetration avoidance.
 */
struct native PenetrationAvoidanceFeeler
{
	/** rotator describing deviance from main ray */
	var() Rotator	AdjustmentRot;
	/** how much this feeler affects the final position if it hits the world */
	var() float		WorldWeight;
	/** how much this feeler affects the final position if it hits a Pawn (setting to 0 will not attempt to collide with pawns at all) */
	var() float		PawnWeight;
	/** extent to use for collision when firing this ray */
	var() vector	Extent;
};
var() array<PenetrationAvoidanceFeeler> PenetrationAvoidanceFeelers;

var() float					WorstLocInterpSpeed;
var transient vector		LastWorstLocationLocal;
var() float					LazyCamSpeed;
var() float					BlendTime;
var() float					SpectatorCameraRotInterpSpeed;

var transient vector	LastActualCameraOrigin,LastViewOffset;
var transient float LastCamFOV;

/** True to turn do predictive camera avoidance, false otherwise */
var() bool		bDoPredictiveAvoidance;

/** offset from viewtarget, to calculate worst camera location. Is also mirrored like player. */
var() vector	WorstLocOffset;

/** TRUE to do a raytrace from camera base loc to worst loc, just to be sure it's cool.  False to skip it */
var() bool		bValidateWorstLoc;

/** obstruction pct from origin to worstloc origin */
var		float	WorstLocBlockedPct;
/** camera extent scale to use when calculating penetration for this segment */
var()	float	WorstLocPenetrationExtentScale;

/** Time to transition from blocked location to ideal position, after camera collision with geometry. */
var()	float	PenetrationBlendOutTime;
/** Time to transition from ideal location to blocked position, after camera collision with geometry. (used only by predictive feelers) */
var()	float	PenetrationBlendInTime;
/** Percentage of distance blocked by collision. From worst location, to desired location. */
var private float	PenetrationBlockedPct;
/** camera extent scale to use when calculating penetration for this segment */
var()	float	PenetrationExtentScale;

/**
 * Last pawn relative offset, for slow offsets interpolation.
 * This is because this offset is relative to the Pawn's rotation, which can change abruptly (when snapping to cover).
 * Used to adjust the camera origin (evade, lean, pop up, blind fire, reload..)
 */
var		transient	vector	LastActualOriginOffset;
var		transient	rotator	LastActualCameraOriginRot;
var		transient	Actor	LastViewTarget;

var(FOVRemapping) float FOV_SrcMin, FOV_SrcMax, FOV_DestMin, FOV_DestMax;

// reset the camera to a good state
function Reset()
{
	bResetCameraInterpolation = true;	
}

simulated event vector GetViewOffset(Pawn TargetPawn, float DeltaTime, rotator ViewRotation )
{
	return avaCharacter(TargetPawn).GetViewOffset( DeltaTime, ViewRotation );
}

simulated event vector GetCameraOriginOffset(Pawn TargetPawn)
{
	return avaCharacter(TargetPawn).GetCameraOriginOffset();
}

simulated event float GetDesiredFOV( Pawn P )
{
	local float NeutralFOV;
	NeutralFOV = 90.0;
	if ( avaPawn(P).CurrentWeapon != None )
	{
		NeutralFOV = avaPawn(P).CurrentWeapon.AdjustFOVAngle( NeutralFOV );
	}
	return FClamp( ( NeutralFOV - FOV_SrcMin ) / ( FOV_SrcMax - FOV_SrcMin), 0, 1 ) * (FOV_DestMax - FOV_DestMin) + FOV_DestMin;
}

simulated event vector GetCameraWorstCaseLoc(Pawn TargetPawn)
{
	local avaPawn AP;	

	AP = avaPawn(TargetPawn);

	if (AP != None)
	{
		return AP.GetCameraWorstCaseLoc();
	}
	else
	{
		return TargetPawn.Location;
	}
}

function UpdateViewTarget(out TViewTarget OutVT, float DeltaTime)
{
	local vector		Loc, Pos, HitLocation, HitNormal;
	local rotator		Rot;
	local Actor			HitActor;
	local CameraActor	CamActor;
	local bool			bDoNotApplyModifiers;
	local TPOV			OrigPOV;

	local bool			bUseShoulderView;
	

	if ( !PCOwner.IsLocalPlayerController() )	return;

	// store previous POV, in case we need it later
	OrigPOV = OutVT.POV;

	// Default FOV on viewtarget
	OutVT.POV.FOV = DefaultFOV;

	avaPlayerController( PCOwner ).UpdatePawnOffset();

	bUseShoulderView	=	avaPlayerController( PCOwner ).UseShoulderView();
	

	if ( bUseShoulderView == true )
	{
		if (Pawn(OutVT.Target) != None)
		{
			if (OutVT.Target != LastViewTarget)
			{
				bResetCameraInterpolation = true;
				LastViewTarget = OutVT.Target;
			}
			
			PlayerUpdateCameraNative( Pawn(OutVT.Target), DeltaTime, OutVT );
		}

		// if we had to reset camera interpolation, then turn off flag once it's been processed.
		if( bResetCameraInterpolation )
		{
			bResetCameraInterpolation = FALSE;
		}			
	}
	else
	{
		// Viewing through a camera actor.
		CamActor = CameraActor(OutVT.Target);
		if( CamActor != None )
		{
			CamActor.GetCameraView(DeltaTime, OutVT.POV);

			// Grab aspect ratio from the CameraActor.
			bConstrainAspectRatio	= bConstrainAspectRatio || CamActor.bConstrainAspectRatio;
			OutVT.AspectRatio		= CamActor.AspectRatio;
		}
		else
		{			
			// Give Pawn Viewtarget a chance to dictate the camera position.
			// If Pawn doesn't override the camera view, then we proceed with our own defaults
			if( Pawn(OutVT.Target) == None || 
				!Pawn(OutVT.Target).CalcCamera(DeltaTime, OutVT.POV.Location, OutVT.POV.Rotation, OutVT.POV.FOV) )
			{
				// don't apply modifiers when using these debug camera modes.
				bDoNotApplyModifiers = TRUE;

				switch( CameraStyle )
				{
					case 'Fixed'		:	// do not update, keep previous camera position by restoring
											// saved POV, in case CalcCamera changes it but still returns false
											OutVT.POV = OrigPOV;
											break;

					case 'ThirdPerson'	: // Simple third person view implementation
					case 'FreeCam'		: 	
											Loc = OutVT.Target.Location;
											Rot = OutVT.Target.Rotation;

											//OutVT.Target.GetActorEyesViewPoint(Loc, Rot);
											if( CameraStyle == 'FreeCam' )
											{
												Rot = PCOwner.Rotation;
											}
											Loc += FreeCamOffset >> Rot;

											Pos = Loc - Vector(Rot) * FreeCamDistance;
											HitActor = Trace(HitLocation, HitNormal, Pos, Loc, FALSE, vect(0,0,0));
											if( HitActor != None )
											{
												Loc = HitLocation + HitNormal*2;
											}
											else
											{
												Loc = Pos;
											}
											OutVT.POV.Location = Loc;
											OutVT.POV.Rotation = Rot;
											break;

					case 'FirstPerson'	: // Simple first person, view through viewtarget's 'eyes'
					default				:	/// 2006/3/13 ; deif
											/// ÀÌ°Å ¾ÈÇÏ¸é ´ú´ú ¶³¸± ¼ö ÀÖÀ½ -_-!!!
											//if (avaPawn(OutVT.Target) != none)
											//	avaPawn(OutVT.Target).UpdateEyeHeight2(DeltaTime);
											
											OutVT.Target.GetActorEyesViewPoint(OutVT.POV.Location, OutVT.POV.Rotation);
											break;

				}
			}
		}	

		if( !bDoNotApplyModifiers )
		{
			// Apply camera modifiers at the end (view shakes for example)
			ApplyCameraModifiers(DeltaTime, OutVT.POV);
		}	
	}
}

simulated native function PlayerUpdateCameraNative( Pawn P, float DeltaTime, out TViewTarget OutVT );

simulated event UpdateCamera(float DeltaTime)
{
	Super.UpdateCamera( DeltaTime );

	UpdateNightvision();
}

simulated function UpdateNightvision()
{
	local avaPawn target;
	local float x;
	local avaGameViewportClient gvc;

	if ( !PCOwner.IsLocalPlayerController() )							return;
	if ( avaPlayerController( PCOwner ).bDisableCameraEffect == true )	return;
	if ( PCOwner.Player == None )										return;

	target = avaPawn(ViewTarget.Target);	

	gvc = avaGameViewportClient(Engine(PCOwner.Player.Outer).GameViewport);	

	if (target != none && target.NightvisionActivated)
	{			
		gvc.NightVisionEffect.bShowInGame = true;

		x = (1 - Exp( -4*(WorldInfo.TimeSeconds - target.NightvisionActivatedTime) ));		

		gvc.NightVisionMaterialInstance.SetScalarParameterValue( 'Amount', x );		

		bEnableColorScaling = true;

		ColorScale.X = 1/16.0;
	}
	else
	{
		gvc.NightVisionEffect.bShowInGame = false;		

		bEnableColorScaling = false;
	}	
}

simulated function UpdateFlashEffect( float NewAlpha )
{
	local avaGameViewportClient gvc;	

	gvc = avaGameViewportClient(Engine(PCOwner.Player.Outer).GameViewport);

	if (NewAlpha > 0)
	{
		gvc.FlashEffect.bShowInGame = true;

		gvc.FlashMaterialInstance.SetScalarParameterValue( 'Amount', NewAlpha );				
	}
	else
	{
		gvc.FlashEffect.bShowInGame = false;
	}
}

/**
 * Handles traces to make sure camera does not penetrate geometry and tries to find
 * the best location for the camera.
 * Also handles interpolating back smoothly to ideal/desired position.
 *
 * @param	WorstLocation		Worst location (Start Trace)
 * @param	DesiredLocation		Desired / Ideal position for camera (End Trace)
 * @param	DeltaTime			Time passed since last frame.
 * @param	DistBlockedPct		percentage of distance blocked last frame, between WorstLocation and DesiredLocation. To interpolate out smoothly
 * @param	CameraExtentScale	Scale camera extent. (box used for collision)
 * @param	bSingleRayOnly		Only fire a single ray.  Do not send out extra predictive feelers.
 */
final native function PreventCameraPenetration(Pawn P, const out vector WorstLocation, out vector DesiredLocation, float DeltaTime, out float DistBlockedPct, float CameraExtentScale, optional bool bSingleRayOnly);

defaultproperties
{
	PenetrationBlendOutTime=0.15f
	PenetrationBlendInTime=0.1f
	PenetrationBlockedPct=1.f
	PenetrationExtentScale=1.f

	WorstLocPenetrationExtentScale=1.f
	WorstLocInterpSpeed=8

	WorstLocOffset=(X=-25,Y=15,Z=90)	

	// ray 0 is the main ray
	PenetrationAvoidanceFeelers(0)=(AdjustmentRot=(Pitch=0,Yaw=0,Roll=0),WorldWeight=1.f,PawnWeight=1.f,Extent=(X=14,Y=14,Z=14))

	// horizontally offset
	PenetrationAvoidanceFeelers(1)=(AdjustmentRot=(Pitch=0,Yaw=3072,Roll=0),WorldWeight=0.75f,PawnWeight=0.75f,Extent=(X=0,Y=0,Z=0))
	PenetrationAvoidanceFeelers(2)=(AdjustmentRot=(Pitch=0,Yaw=-3072,Roll=0),WorldWeight=0.75f,PawnWeight=0.75f,Extent=(X=0,Y=0,Z=0))
	PenetrationAvoidanceFeelers(3)=(AdjustmentRot=(Pitch=0,Yaw=6144,Roll=0),WorldWeight=0.5f,PawnWeight=0.5f,Extent=(X=0,Y=0,Z=0))
	PenetrationAvoidanceFeelers(4)=(AdjustmentRot=(Pitch=0,Yaw=-6144,Roll=0),WorldWeight=0.5f,PawnWeight=0.5f,Extent=(X=0,Y=0,Z=0))

	// vertically offset
	PenetrationAvoidanceFeelers(5)=(AdjustmentRot=(Pitch=3640,Yaw=0,Roll=0),WorldWeight=1.f,PawnWeight=1.f,Extent=(X=0,Y=0,Z=0))
	PenetrationAvoidanceFeelers(6)=(AdjustmentRot=(Pitch=-3640,Yaw=0,Roll=0),WorldWeight=0.5f,PawnWeight=0.5f,Extent=(X=0,Y=0,Z=0))

	
	LazyCamSpeed=12
	SpectatorCameraRotInterpSpeed=14

	bResetCameraInterpolation=TRUE	// set to true by default, so first frame is never interpolated
	BlendTime=0.1
	bDoPredictiveAvoidance=TRUE
	bValidateWorstLoc=TRUE

	FOV_SrcMax = 90.0
	FOV_SrcMin = 10.0
	FOV_DestMax = 80.0
	FOV_DestMin = 30.0
}