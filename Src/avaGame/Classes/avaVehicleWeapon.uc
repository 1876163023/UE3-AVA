/**
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */

class avaVehicleWeapon extends avaWeap_BaseGun
	abstract
	native
	nativereplication
	dependson(avaPhysicalMaterialProperty);

/** Holds a link in to the Seats array in MyVehicle that represents this weapon */
var int SeatIndex;

/** Holds a link to the parent vehicle */
var RepNotify avaVehicle	MyVehicle;

/** Triggers that should be activated when a weapon fires */
var array<name>	FireTriggerTags, AltFireTriggerTags;

/** impact effects by material type */
var name DamageCode;

/** default impact effect to use if a material specific one isn't found */
var PhysicalMaterial DefaultPhysicalMaterial;

/** impact effects by material type */
var array<MaterialImpactEffect> ImpactEffects, AltImpactEffects;

/** default impact effect to use if a material specific one isn't found */
var MaterialImpactEffect DefaultImpactEffect, DefaultAltImpactEffect;

/** sound that is played when the bullets go whizzing past your head */
var SoundCue BulletWhip;

struct native ColorOverTime
{
	/** the color to lerp to */
	var color TargetColor;
	/** time at which the color is exactly TargetColor */
	var float Time;
};
/** color flash interpolation when aim becomes correct - assumed to be in order of increasing time */
var array<ColorOverTime> GoodAimColors;
/** last time aim was incorrect, used for looking up GoodAimColor */
var float LastIncorrectAimTime;

var color BadAimColor;

/** cached max range of the weapon used for aiming traces */
var float AimTraceRange;

/** actors that the aiming trace should ignore */
var array<Actor> AimingTraceIgnoredActors;

/** This value is used to cap the maximum amount of "automatic" adjustment that will be made to a shot
    so that it will travel at the crosshair.  If the angle between the barrel aim and the player aim is
    less than this angle, it will be adjusted to fire at the crosshair.  The value is in radians */
var float MaxFinalAimAdjustment;

var bool bPlaySoundFromSocket;

/** used for client to tell server when zooming, as that is clientside
 * but on console it affects the controls so the server needs to know
 */
var bool bCurrentlyZoomed;

/**
 * If the weapon is attached to a socket that doesn't pitch with
 * player view, and should fire at the aimed pitch, then this should be enabled.
 */
var bool bIgnoreSocketPitchRotation;
/**
 * Same as above, but only allows for downward direction, for vehicles with 'bomber' like behavior.
 */
var bool bIgnoreDownwardPitch;
/** Vehicle class used for drawing kill icons */
var class<avaVehicle> VehicleClass;

//!
var bool bInitialized;

cpptext
{
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

replication
{
	if (Role == ROLE_Authority && bNetInitial)
		SeatIndex, MyVehicle;
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	`log("avaVehicleWeapon.PostBeginPlay" @self @MyVehicle @bNetOwner);

	AimTraceRange = MaxRange();
}

simulated function OnInitialize()
{
	if ( !bInitialized )
	{
		// 초기화.
		AttachMuzzleFlash();

		bInitialized = true;
	}
}

simulated event ReplicatedEvent(name VarName)
{
//	local UTVehicle_Deployable DeployableVehicle;

	if (VarName == 'MyVehicle')
	{
//! @comment UTVehicle_Deployable는 일단 제외해 둠.
//!		(주로 UTVehicle_Deployable(Bot.Pawn)형태인 것으로 보아 AI쪽에서 쓰이나?)
/*
		DeployableVehicle = UTVehicle_Deployable(MyVehicle);
		if (DeployableVehicle != None && DeployableVehicle.IsDeployed())
		{
			NotifyVehicleDeployed();
		}
*/

		OnInitialize();

		`log("avaVehicleWeapon.ReplicatedEvent" @VarName @MyVehicle);
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/** checks if the weapon is actually capable of hitting the desired target, including trace test (used by crosshair)
 * if false because trace failed, RealAimPoint is set to what the trace hit
 */
simulated function bool CanHitDesiredTarget(vector SocketLocation, rotator SocketRotation, vector DesiredAimPoint, Actor TargetActor, out vector RealAimPoint)
{
	local int i;
	local array<Actor> IgnoredActors;
	local Actor HitActor;
	local vector HitLocation, HitNormal;
	local bool bResult;

	if ((Normal(DesiredAimPoint - SocketLocation) dot Normal(RealAimPoint - SocketLocation)) >= GetMaxFinalAimAdjustment())
	{
		// turn off bProjTarget on Actors we should ignore for the aiming trace
		for (i = 0; i < AimingTraceIgnoredActors.length; i++)
		{
			if (AimingTraceIgnoredActors[i] != None && AimingTraceIgnoredActors[i].bProjTarget)
			{
				AimingTraceIgnoredActors[i].bProjTarget = false;
				IgnoredActors[IgnoredActors.length] = AimingTraceIgnoredActors[i];
			}
		}
		// perform the trace
		HitActor = GetTraceOwner().Trace(HitLocation, HitNormal, DesiredAimPoint, SocketLocation, true,,, TRACEFLAG_Bullet);
		if (HitActor == None || HitActor == TargetActor)
		{
			bResult = true;
		}
		else
		{
			RealAimPoint = HitLocation;
		}
		// restore bProjTarget on Actors we turned it off for
		for (i = 0; i < IgnoredActors.length; i++)
		{
			IgnoredActors[i].bProjTarget = true;
		}
	}

	return bResult;
}


simulated static function DrawKillIcon(Canvas Canvas, float ScreenX, float ScreenY, float HUDScaleX, float HUDScaleY)
{
	if ( default.VehicleClass != None )
	{
		default.VehicleClass.static.DrawKillIcon(Canvas, ScreenX, ScreenY, HUDScaleX, HUDScaleY);
	}
}

simulated function DrawWeaponCrosshair( Hud HUD )
{
/*
	local vector SocketLocation, DesiredAimPoint, RealAimPoint;
	local rotator SocketRotation;
	local Actor TargetActor;
	local bool bAimIsCorrect;
	local int i;
	local float TimeSinceIncorrectAim, Pct;
	local color LastColor;

	DesiredAimPoint = GetDesiredAimPoint(TargetActor);
	GetFireStartLocationAndRotation(SocketLocation, SocketRotation);
	RealAimPoint = SocketLocation + Vector(SocketRotation) * GetTraceRange();

	bAimIsCorrect = CanHitDesiredTarget(SocketLocation, SocketRotation, DesiredAimPoint, TargetActor, RealAimPoint);

	// draw the center crosshair, then figure out where to draw the crosshair that shows the vehicle's actual aim
	if (bAimIsCorrect)
	{
		// if recently aim became correct, play color flash
		TimeSinceIncorrectAim = WorldInfo.TimeSeconds - LastIncorrectAimTime;
		if (TimeSinceIncorrectAim < GoodAimColors[GoodAimColors.length - 1].Time)
		{
			for (i = 0; i < GoodAimColors.length; i++)
			{
				if (TimeSinceIncorrectAim <= GoodAimColors[i].Time)
				{
					if (i == 0)
					{
						Pct = TimeSinceIncorrectAim / GoodAimColors[i].Time;
						LastColor = BadAimColor;
					}
					else
					{
						Pct = (TimeSinceIncorrectAim - GoodAimColors[i - 1].Time) / (GoodAimColors[i].Time - GoodAimColors[i - 1].Time);
						LastColor = GoodAimColors[i - 1].TargetColor;
					}
//					CrosshairColor = (LastColor * (1.0 - Pct)) + (GoodAimColors[i].TargetColor * Pct);
					break;
				}
			}
		}
		else
		{
//			CrosshairColor = GoodAimColors[GoodAimColors.length - 1].TargetColor;
		}
//		CrosshairColor.A = 255;
//		Super.DrawWeaponCrosshair(HUD);

		Hud.Canvas.SetPos(HUD.Canvas.ClipX * 0.5 - 10.0, HUD.Canvas.ClipY * 0.5 - 10.0);
	}
	else
	{
		LastIncorrectAimTime = WorldInfo.TimeSeconds;
//		CrosshairColor = BadAimColor;
//		Super.DrawWeaponCrosshair(HUD);

		RealAimPoint = Hud.Canvas.Project(RealAimPoint);
		if (RealAimPoint.X < 12 || RealAimPoint.X > Hud.Canvas.ClipX-12)
		{
			RealAimPoint.X = Clamp(RealAimPoint.X,12,Hud.Canvas.ClipX-12);
		}
		if (RealAimPoint.Y < 12 || RealAimPoint.Y > Hud.Canvas.ClipY-12)
		{
			RealAimPoint.Y = Clamp(RealAimPoint.Y,12,Hud.Canvas.ClipY-12);
		}
		Hud.Canvas.SetPos(RealAimPoint.X - 10.0, RealAimPoint.Y - 10.0);
	}
	Hud.Canvas.DrawTile(Texture2D'UI_HUD.HUD.UTCrossHairs', 21, 21, 63, 257, 20, 20);
*/
}

/** GetDesiredAimPoint - Returns the desired aim given the current controller
 * @param TargetActor (out) - if specified, set to the actor being aimed at
 * @return The location the controller is aiming at
 */
simulated event vector GetDesiredAimPoint(optional out Actor TargetActor)
{
	local vector CameraLocation, HitLocation, HitNormal, DesiredAimPoint;
	local rotator CameraRotation;
	local Controller C;
	local PlayerController PC;

	C = MyVehicle.Seats[SeatIndex].SeatPawn.Controller;

	PC = PlayerController(C);
	if (PC != None)
	{
		PC.GetPlayerViewPoint(CameraLocation, CameraRotation);
		DesiredAimPoint = CameraLocation + Vector(CameraRotation) * GetTraceRange();
		TargetActor = GetTraceOwner().Trace(HitLocation, HitNormal, DesiredAimPoint, CameraLocation);
		if (TargetActor != None)
		{
			DesiredAimPoint = HitLocation;
		}
	}
	else if ( C != None )
	{
		DesiredAimPoint = C.FocalPoint;
		TargetActor = C.Focus;
	}
	return DesiredAimPoint;
}

/** returns the location and rotation that the weapon's fire starts at */
simulated function GetFireStartLocationAndRotation(out vector StartLocation, out rotator StartRotation)
{
	if ( MyVehicle.Seats[SeatIndex].GunSocket.Length>0 )
	{
		MyVehicle.GetBarrelLocationAndRotation(SeatIndex, StartLocation, StartRotation);
	}
	else
	{
		StartLocation = MyVehicle.Location;
		StartRotation = MyVehicle.Rotation;
	}
}

/**
 * IsAimCorrect - Returns true if the turret associated with a given seat is aiming correctly
 *
 * @return TRUE if we can hit where the controller is aiming
 */
simulated function bool IsAimCorrect()
{
	local vector SocketLocation, DesiredAimPoint, RealAimPoint;
	local rotator SocketRotation;

	DesiredAimPoint = GetDesiredAimPoint();

	GetFireStartLocationAndRotation(SocketLocation, SocketRotation);

	RealAimPoint = SocketLocation + Vector(SocketRotation) * GetTraceRange();
	return ((Normal(DesiredAimPoint - SocketLocation) dot Normal(RealAimPoint - SocketLocation)) >= GetMaxFinalAimAdjustment());
}


simulated static function Name GetFireTriggerTag(int BarrelIndex, int FireMode)
{
	if (FireMode==0)
	{
		if (default.FireTriggerTags.Length > BarrelIndex)
		{
			return default.FireTriggerTags[BarrelIndex];
		}
	}
	else
	{
		if (default.AltFireTriggerTags.Length > BarrelIndex)
		{
			return default.AltFireTriggerTags[BarrelIndex];
		}
	}
	return '';
}

/**
 * Returns interval in seconds between each shot, for the firing state of FireModeNum firing mode.
 *
 * @param	FireModeNum	fire mode
 * @return	Period in seconds of firing mode
 */
simulated function float GetFireInterval( byte FireModeNum )
{
	local Vehicle V;
	local avaPawn avaP;

	V = Vehicle(Instigator);
	if (V != None)
	{
		avaP = avaPawn(V.Driver);
		if (avaP != None)
		{
			return (FireInterval[FireModeNum] * avaP.FireRateMultiplier);
		}
	}

	return Super.GetFireInterval(FireModeNum);
}

/** returns the impact effect that should be used for hits on the given actor and physical material */
simulated function MaterialImpactEffect GetImpactEffect(Actor HitActor, PhysicalMaterial HitMaterial, byte FireModeNum)
{
	local int i;
	local avaPhysicalMaterialProperty PhysicalProperty;
	local ImpactDecalData IDD;

	//! @comment avaWeaponAttachment.GetImpactEffect함수를 그대로 옮겨옴.(2007/07/31 고광록)
	if (HitMaterial == None)
		HitMaterial = DefaultPhysicalMaterial;

	PhysicalProperty = avaPhysicalMaterialProperty(HitMaterial.GetPhysicalMaterialProperty(class'avaPhysicalMaterialProperty'));

	if (PhysicalProperty != None)
	{
		i = PhysicalProperty.ImpactEffects.Find('DamageCode', DamageCode);
		if ( i != -1 )
		{
			`log("PhysicalProperty.ImpactEffects.Find('DamageCode', " @DamageCode @")" @i);
			return PhysicalProperty.ImpactEffects[i];
		}

		if (IsA( 'avaAttachment_BaseGun' ))
		{
			i = PhysicalProperty.ImpactEffects.Find('DamageCode', 'Gun');
			if ( i != -1 )
			{
				`log("PhysicalProperty.ImpactEffects.Find('DamageCode', 'Gun')" @i);
				return PhysicalProperty.ImpactEffects[i];
			}
		}
	}	

	`log( "Error invalid damage code"@default.DamageCode );

	if (DefaultImpactEffect.ImpactDecals.Length == 0)
	{		
		DefaultImpactEffect.ImpactDecals[0] = IDD;
		DefaultImpactEffect.ImpactDecals[0].DecalMaterial = DefaultImpactEffect.DecalMaterial;
		DefaultImpactEffect.ImpactDecals[0].DecalWidth = DefaultImpactEffect.DecalWidth;
		DefaultImpactEffect.ImpactDecals[0].DecalHeight = DefaultImpactEffect.DecalHeight;		
	}

	return DefaultImpactEffect;
}

simulated function SetHand(avaPawn.EWeaponHand NewWeaponHand);
simulated function avaPawn.EWeaponHand GetHand();
simulated function AttachWeaponTo( SkeletalMeshComponent MeshCpnt, optional Name SocketName );
simulated function DetachWeapon();

simulated function Activate()
{
	// don't reactivate if already firing
	if (!IsFiring())
	{
		GotoState('Active');
	}
}

simulated function PutDownWeapon()
{
	GotoState('Inactive');
}

simulated function Vector GetPhysicalFireStartLoc(optional vector AimDir)
{
	if ( MyVehicle != none )
		return MyVehicle.GetPhysicalFireStartLoc(self);
	else
		return Location;
}

simulated function BeginFire(byte FireModeNum)
{
	local avaVehicle V;

	`log("avaVehicleWeapon.BeginFire" @FireModeNum @Instigator);

	// allow the vehicle to override the call
	V = avaVehicle(Instigator);
	if (V == None || (!V.bIsDisabled && !V.OverrideBeginFire(FireModeNum)))
	{
		Super.BeginFire(FireModeNum);
	}
}

simulated function EndFire(byte FireModeNum)
{
	local avaVehicle V;

	`log("avaVehicleWeapon.EndFire" @FireModeNum);

	// allow the vehicle to override the call
	V = avaVehicle(Instigator);
	if (V == None || !V.OverrideEndFire(FireModeNum))
	{
		Super.EndFire(FireModeNum);
	}
}

simulated function Rotator GetAdjustedAim( vector StartFireLoc )
{
	// Start the chain, see Pawn.GetAdjustedAimFor()
	// @note we don't add in spread here because avaVehicle::GetWeaponAim() assumes
	// that a return value of Instigator.Rotation or Instigator.Controller.Rotation means 'no adjustment', which spread interferes with
	return Instigator.GetAdjustedAimFor( Self, StartFireLoc );
}

/**
 * Create the projectile, but also increment the flash count for remote client effects.
 */
simulated function Projectile ProjectileFire()
{
	local Projectile SpawnedProjectile;

	IncrementFlashCount();

	if (Role==ROLE_Authority)
	{
		SpawnedProjectile = Spawn(GetProjectileClass(),,, MyVehicle.GetPhysicalFireStartLoc(self));

//		`log("avaVehicleWeapon.ProjectileFire" @GetProjectileClass());

		if ( SpawnedProjectile != None )
		{
			SpawnedProjectile.Init( vector(AddSpread(MyVehicle.GetWeaponAim(self))) );

			// return it up the line
			avaProjectile(SpawnedProjectile).InitStats(self);
		}
	}
	return SpawnedProjectile;

//	return Super.ProjectileFire();
}

/**
* Overriden to use vehicle starttrace/endtrace locations
* @returns position of trace start for instantfire()
*/
simulated function vector InstantFireStartTrace()
{
	return MyVehicle.GetPhysicalFireStartLoc(self);
}

/**
* @returns end trace position for instantfire()
*/
simulated function vector InstantFireEndTrace(vector StartTrace)
{
	return StartTrace + vector(AddSpread(MyVehicle.GetWeaponAim(self))) * GetTraceRange();
}

simulated function Actor GetTraceOwner()
{
	return ( Instigator != None ) ? avaVehicleBase( Instigator ).Driver : Super.GetTraceOwner();
	//return (MyVehicle != None) ? MyVehicle : Super.GetTraceOwner();
}

simulated function float GetMaxFinalAimAdjustment()
{
	return MaxFinalAimAdjustment;
}

/** notification that MyVehicle has been deployed/undeployed, since that often changes how its weapon works */
simulated function NotifyVehicleDeployed();
simulated function NotifyVehicleUndeployed();

simulated function WeaponPlaySound( SoundCue Sound, optional float NoiseLoudness, optional bool bNotReplicated, optional bool bRepToOwner, optional bool bNoDup )
{
	local int Barrel;
	local name Pivot;
	local vector Loc;
	local rotator Rot;

	if(bPlaySoundFromSocket && MyVehicle != none && MyVehicle.Mesh != none)
	{
		if( Sound == None || Instigator == None )
		{
			return;
		}
		Barrel = MyVehicle.GetBarrelIndex(SeatIndex);
		Pivot = MyVehicle.Seats[SeatIndex].GunSocket[Barrel];
		MyVehicle.Mesh.GetSocketWorldLocationAndRotation(Pivot, Loc, Rot);
		Instigator.PlaySound(Sound, false, true,false,Loc);
	}
	else
		super.WeaponPlaySound(Sound,NoiseLoudness);
}

/**
 * If you want to be able to hide a crosshair for this weapon, override this function and return false;
 */
simulated function bool ShowCrosshair()
{
	return true;
}

//! @comment 이 함수는 사용하지 않는다.
/*
simulated function EZoomState GetZoomedState()
{
	// override on server to use what the client told us
	if (Role == ROLE_Authority && Instigator != None && !Instigator.IsLocallyControlled())
	{
		return bCurrentlyZoomed ? ZST_Zoomed : ZST_NotZoomed;
	}
	else
	{
		return Super.GetZoomedState();
	}
}


reliable server function ServerSetZoom(bool bNowZoomed)
{
	bCurrentlyZoomed = bNowZoomed;
}

simulated function StartZoom(avaPlayerController PC)
{
	Super.StartZoom(PC);

	ServerSetZoom(true);
}

simulated function EndZoom(avaPlayerController PC)
{
	Super.EndZoom(PC);

	ServerSetZoom(false);
}
*/

//-----------------------------------------------------------------------------
//	avaPawn을 참조하는 함수를 재코딩 한다.
//-----------------------------------------------------------------------------

//! (from avaWeap_BaseGun)
simulated function float GetHitDamage()
{
	if ( avaVehicleBase(Instigator) != None )
		return HitDamage;
	else
		return Super.GetHitDamage();
}

//! (from avaWeap_BaseGun)
simulated function float GetTraceRange()
{
	// Weapon.GetTraceRange()함수와 동일하다.
	return WeaponRange;
}

simulated function GetFireLocAndRot( out vector loc, out rotator rot )
{
	Instigator.Controller.GetPlayerViewPoint( loc, rot );
	`log( "avaVehicleWeapon.GetFireLocAndRot" @loc @rot );
}

simulated function rotator AddSpread(rotator BaseAim)
{
	return Super.AddSpread( BaseAim );
	//return BaseAim;
}

simulated function ApplyPunchAngleToWeapon( out rotator rot )
{
	avaPawn( avaVehicleBase( Instigator ).Driver ).ApplyPunchAngleToWeapon( rot );	
}

//! (from avaWeap_BaseGun)
//simulated event function RifleFire( int ShotNum )
//{
//	local vector						StartTrace, EndTrace;
//	local Array<ImpactInfo>				ImpactList;	
//	local ImpactInfo					RealImpact;
//	local vector						dir;			
//	local int							iPenetration;
//	local int							iRound, iImpactRound;
//	local vector						org;
//	local int							iPenetrationPower;
//	local float							fPenetrationDistance;
//	local float							CurrentDamage;
//	local float							fDamageModifier;
//	local avaPhysicalMaterialProperty	PhysicalProperty;
//	local float							fCurrentDistance, fDistance;	
//	local vector						NextStart, PrevHit, PrevNormal;
//	local actor							HitActor;
//	local vector						HitNormal, HitLocation;
//	local TraceHitInfo					HitInfo;
//	local bool							blood, glass;
//	local float							ArmorParam;
//	local float							PenetrationMinDistance;
//	local float							CheckDistance;
//	local rotator						TmpRot;
//	local vector						RealHitLocation;
//
//	blood = false;
//
//	iPenetration = GetPenetration();
//
//	CurrentDamage = GetHitDamage();
//
//	GetBulletTypeParameters( iPenetrationPower, fPenetrationDistance, CurrentDamage, ArmorParam );
//	//`log("RifleFire "$BulletType);
//
//	// define range to use for CalcWeaponFire() 
//	//StartTrace = Instigator.GetWeaponStartTraceLocation();
//
//	// 해당 무기의 발사되는 소켓의 좌표를 얻어오게 될 것이다.
//
//	//GetFireStartLocationAndRotation(StartTrace, TmpRot);
//
//	EndTrace = StartTrace + Vector(TmpRot) * GetTraceRange();
//
//	org = StartTrace;
//	
//	fDamageModifier = 0.5;	
//
//	while (iPenetration != 0 || blood)
//	{        
//		// Perform shot
//		RealImpact = CalcWeaponFire(StartTrace, EndTrace, ImpactList);
//
//		fCurrentDistance = VSize(RealImpact.HitLocation - StartTrace);
//
//		if ( fCurrentDistance == 0.0 )	break;
//		
//		/// 뒷면 뚫고 나간 것 체크
//		if (iRound > 0)
//		{	
//			CheckDistance = VSize( StartTrace - EndTrace );
//			PenetrationMinDistance = fCurrentDistance > CheckDistance ? CheckDistance : fCurrentDistance;
//
//			GetTraceOwner().Trace( HitLocation, HitNormal, EndTrace, StartTrace, true, vect(0,0,0), HitInfo, TRACEFLAG_Bullet );
//
//			if ( PenetrationMinDistance > VSize(HitLocation - StartTrace) )	break;
//
//			if (RealImpact.HitActor != none)
//			{
//				HitActor = GetTraceOwner().Trace(HitLocation, HitNormal, PrevHit + dir * 8, RealImpact.HitLocation - dir * 8, true, vect(0,0,0), HitInfo, TRACEFLAG_Bullet );			
//			}
//			else
//			{
//				HitActor = GetTraceOwner().Trace(HitLocation, HitNormal, PrevHit + dir * 8, EndTrace, true, vect(0,0,0), HitInfo, TRACEFLAG_Bullet );			
//			}
//
//			if (HitActor != None)
//			{				
//				if ((PrevNormal dot HitNormal) < 0)
//				{
//					SetImpactLocation(HitLocation,true);
//				}
//			}
//		}
//
//		/// 시작점이 invalid한 경우!
//		if (iRound > 0 && VSize(StartTrace - RealImpact.HitLocation) < 2)
//			break;
//
//		glass = RealImpact.HitActor != none && RealImpact.HitActor.IsA( 'avaShatterGlassActor' );		
//
//		if ( iImpactRound == 0 )
//		{
//			RealHitLocation = RealImpact.HitLocation;
//			
//		}
//
//		if ( !glass )
//		{
//			if (iImpactRound == 0 && ShotNum == 0 )						
//			{
//				SetFlashLocation(RealImpact.HitLocation);
//			}
//			else
//			{
//				SetImpactLocation(RealImpact.HitLocation,,blood);
//			}
//						
//			iImpactRound++;
//		}
//
//		iRound++;
//
//		if (RealImpact.HitActor == None) break;								
//		
//		if (RealImpact.HitInfo.PhysMaterial != None)
//			PhysicalProperty = avaPhysicalMaterialProperty(RealImpact.HitInfo.PhysMaterial.GetPhysicalMaterialProperty(class'avaPhysicalMaterialProperty'));
//		else
//			PhysicalProperty = None;
//
//		/// 유리창 통과 시 그냐아앙!
//		if (glass)
//		{
//			fDamageModifier = 1.0;
//			iPenetration++;
//		}
//		else
//		{
//			if (PhysicalProperty != None)
//			{
//				iPenetrationPower = int( iPenetrationPower * PhysicalProperty.PenetrationDamper );
//				fDamageModifier = PhysicalProperty.DamageModifier;
//			}
//			else
//			{
//				iPenetrationPower = iPenetrationPower / 2;
//				fDamageModifier = 0.6;
//			}
//		}
//
//		/* 피 안나오는 것 고침 */
//		if (iPenetration == 0 && blood)
//			break;
//
//		iPenetration--;		
//
//		CurrentDamage = CurrentDamage * Exp( Loge(GetRangeModifier()) * VSize(org-RealImpact.HitLocation) / 500 );
//
//		if (fCurrentDistance > fPenetrationDistance)
//			iPenetration = 0;				
//
//		ProcessInstantHit2( RealImpact, CurrentDamage, ArmorParam );
//
//		if (Pawn(RealImpact.HitActor) != None)
//		{		
//			if ( Instigator.GetTeamNum() != Pawn(RealImpact.HitActor ).GetTeamNum() && class'avaNetHandler'.static.GetAvaNetHandler().IsPlayerAdult() )
//			{
//				blood = true;
//			}
//			NextStart = RealImpact.HitLocation + dir * 42;
//			fDistance = (fDistance - fCurrentDistance) * 0.75;
//		}
//		else
//		{
//			NextStart = RealImpact.HitLocation + dir * iPenetrationPower;
//			fDistance = (fDistance - fCurrentDistance) * 0.5;
//			blood = false;
//		}		
//
//		StartTrace = NextStart;
//		
//		EndTrace = StartTrace + dir * fDistance;
//		CurrentDamage *= fDamageModifier;
//
//		PrevHit = RealImpact.HitLocation;
//		PrevNormal = RealImpact.HitNormal;
//	}	
//}

//! (from avaWeap_BaseGun)
simulated function PlayTrailEffect( vector HitLocation )
{
	local vector	l;
	local rotator	r;

	`log( "avaVehicleWeapon.PlayTrailEffect" );

	GetFireLocAndRot( l, r );
	if ( BulletTrailComponent != None )
		BulletTrailComponent.Fire( l, HitLocation, Vect(0,0,0) );
}

//! (from avaWeap_BaseGun)
exec simulated function EjectBullet()
{
	local Emitter	Bullet;			// The Effects for the explosion 
	local vector	l;
	local rotator	r;
	local vector	v;

	// Have pawn play firing anim
	avaPawn( avaVehicleBase( Instigator ).Driver ).PlayAnimByEvent( EBT_Fire );
	avaVehicle_Leopard( MyVehicle ).PlayAnimByEvent( EBT_Fire );

	if ( !bEjectBulletWhenFire )
		return ;

	if ( BulletTemplete != None )//&& MyVehicle.GetSeatMesh(SeatIndex).HiddenGame == false )
	{
		v = MyVehicle.Velocity;
		v.z = 0.0;
		
		if ( MyVehicle.GetSeatMesh(SeatIndex).GetSocketWorldLocationAndRotation( 'GunEjectorSocket', l, r ) )
		{
			r.Pitch = 0;
			r.Roll	= 0;
			Bullet = avaGameReplicationInfo(WorldInfo.GRI).ObjectPool.GetBulletEmitter(BulletTemplete,l, r);						
			Bullet.ParticleSystemComponent.PartSysVelocity = v;					
			Bullet.ParticleSystemComponent.ActivateSystem();
			Bullet.SetTimer( 3, FALSE, 'HideSelf' );
		}
	}
}

//! (from avaWeapon)
simulated event CauseMuzzleFlash()
{
	`log("avaVehicleWeapon.CauseMuzzleFlash");

//	IsCameraWithinRadius(Instigator.Location, MuzzleFlashRadius)함수에서 false를 리턴해서
//	조건이 만족하지 않아 아래 함수에 실행이 안되는데,
//	avaVehicle.EffectIsRelevant함수가 실행되니까 무시한다?
//
//	IsCameraWithinRadius 함수가 1인칭용이라 계산에 맞지 않는다고 함.(from 정백이형)

	//if (IsMuzzleFlashRelevant())
	//{
		if ( MuzzleFlashLight == None )
		{
			if ( MuzzleFlashLightClass != None )
				CauseMuzzleFlashLight();
		}
		else
		{
			MuzzleFlashLight.ResetLight();
			MuzzleFlashLight.CastDynamicShadows = bDynamicMuzzleFlashes;
		}

		if (MuzzleFlashPSC != none)
		{
			if ( !bMuzzleFlashPSCLoops || MuzzleFlashPSC.bWasDeactivated )
			{
				if (Instigator != None && Instigator.FiringMode == 1 && MuzzleFlashAltPSCTemplate != None)
				{
					MuzzleFlashPSC.SetTemplate(MuzzleFlashAltPSCTemplate);
				}
				else
				{
					MuzzleFlashPSC.SetTemplate(MuzzleFlashPSCTemplate);
				}
				MuzzleFlashPSC.SetVectorParameter('MFlashScale',Vect(0.5,0.5,0.5));
				MuzzleFlashPSC.ActivateSystem();
			}
		}

		if (MuzzleFlashMesh != none)
			MuzzleFlashMesh.SetHidden(false);

		// Set when to turn it off.
		SetTimer(MuzzleFlashDuration,false,'MuzzleFlashTimer');
	//}
}

//! (from avaWeapon)
simulated function CauseMuzzleFlashLight()
{
	local SkeletalMeshComponent OwnerMesh;

	`log("avaVehicleWeapon.CauseMuzzleFlashLight()");

	OwnerMesh = MyVehicle.GetSeatMesh(SeatIndex);
	if ( MuzzleFlashLight == None )
	{
		MuzzleFlashLight = new(self) MuzzleFlashLightClass;

		if ( OwnerMesh.GetSocketByName(MuzzleFlashSocket) != None )
			OwnerMesh.AttachComponentToSocket(MuzzleFlashLight, MuzzleFlashSocket);
	}
}

//! (from avaWeapon)
simulated function AttachMuzzleFlash()
{
	local SkeletalMeshComponent OwnerMesh;

	`log("avaVehicleWeapon.AttachMuzzleFlash()" @MuzzleFlashSocket @MuzzleFlashMesh @MuzzleFlashPSCTemplate @MuzzleFlashLightClass);

	// Attach the Muzzle Flash
	OwnerMesh = MyVehicle.GetSeatMesh(SeatIndex);

	if ( OwnerMesh.GetSocketByName( MuzzleFlashSocket ) == None )
		return;

	if ( OwnerMesh != none )
	{
		// Muzzle Flash mesh
		if ( MuzzleFlashMesh != None )
			OwnerMesh.AttachComponentToSocket(MuzzleFlashMesh, MuzzleFlashSocket);

		if ( MuzzleFlashPSC == none && MuzzleFlashPSCTemplate != none )
		{
			MuzzleFlashPSC  = new(self) class'avaParticleSystemComponent';
			OwnerMesh.AttachComponentToSocket(MuzzleFlashPSC, MuzzleFlashSocket);
			// MuzzleFlashPSC.SetDepthPriorityGroup(SDPG_Foreground);를 설정하면 이상한 좌표에 나온다.
			MuzzleFlashPSC.DeactivateSystem();
			MuzzleFlashPSC.SetColorParameter('MuzzleFlashColor', MuzzleFlashColor);
		}

		// Muzzle Flash dynamic light
		if ( MuzzleFlashLight == None )
		{
			MuzzleFlashLight = new(self) MuzzleFlashLightClass;
			OwnerMesh.AttachComponentToSocket(MuzzleFlashLight, MuzzleFlashSocket);
		}
	}
}

//! (from avaWeapon)
simulated function DetachMuzzleFlash()
{
	local SkeletalMeshComponent OwnerMesh;

	`log("avaVehicleWeapon.DetachMuzzleFlash()");

	OwnerMesh = MyVehicle.GetSeatMesh(SeatIndex);
	if ( OwnerMesh != none )
	{
		// Muzzle Flash Mesh
		if ( MuzzleFlashMesh != None )
			OwnerMesh.DetachComponent( MuzzleFlashMesh );

		if (MuzzleFlashPSC != none)
			OwnerMesh.DetachComponent( MuzzleFlashPSC );

		// Muzzle Flash dynamic light
		if ( MuzzleFlashLight != None )
			OwnerMesh.DetachComponent( MuzzleFlashLight );
	}
}

simulated function PlayFireEffects( byte FireModeNum, optional vector HitLocation )
{
	//Super.PlayFireEffects(FireModeNum, HitLocation);
}

function DriverEnter()
{

}

function DriverLeave()
{
	ClientDriverLeave();
	ForceDriverLeave();
}

reliable client function ClientDriverLeave()
{
	ForceDriverLeave();
}

simulated function ForceDriverLeave()
{
	GotoState( 'Active' );
}

defaultproperties
{
//	Components.Remove(FirstPersonMesh)

//	Components.Add(BulletTrailComponent0);

	// 일단 이렇게 해야 Client에서 탑승 후 총을 발사했을 때 제대로 ImpactEffects까지 보여진다.
	bOnlyRelevantToOwner=true
//	bAlwaysRelevant=true

	TickGroup=TG_PostAsyncWork
	InventoryGroup=100
	GroupWeight=0.5
	GoodAimColors[0]=(Time=0.25,TargetColor=(R=255,G=255,B=64,A=255))
	GoodAimColors[1]=(Time=0.70,TargetColor=(R=0,G=255,B=64,A=255))
	BadAimColor=(R=255,G=64,B=0,A=255)
//	bExportMenuData=false

//	ShotCost[0]=0
//	ShotCost[1]=0

	// ~ 5 Degrees
	MaxFinalAimAdjustment=0.995;

	DefaultPhysicalMaterial	= PhysicalMaterial'avaPhyMats.Default'
	DefaultImpactEffect		= (Sound=SoundCue'avaPhySounds.Impact.ConcreteImpact',ParticleTemplate=ParticleSystem'avaEffect.Particles.P_WP_Enforcer_Impact',DecalMaterial=DecalMaterial'avaDecal.Metal.M_Metal_Hole01',DecalWidth=4.0,DecalHeight=4.0)
}
