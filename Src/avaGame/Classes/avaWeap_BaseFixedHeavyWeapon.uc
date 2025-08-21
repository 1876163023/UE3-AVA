/*=============================================================================
  avaWeap_BaseFixedHeavyWeapon
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/06/23 by OZ

	고정중화기 Weapon class

		Logical 한 내용만을 담고 있다.
		Visual 적인 내용은 avaFixedHeavyWeapon.uc 에서 담당한다.

	ToDo.

		Ammo 소비나 Reload 는 어떤식으로 처리할 것인가???? Cool Time 이 존재하는가???
	
=============================================================================*/
class avaWeap_BaseFixedHeavyWeapon extends avaWeap_BaseGun;

`include(avaGame/avaGame.uci)

var avaFixedHeavyWeapon		BaseWeap;

replication
{
	if (bNetDirty && Role == ROLE_Authority)
		BaseWeap;
}

function SetInactive()
{
	// Detach weapon components from instigator
	DetachWeapon();

	ClientWeaponThrown();

	// Become inactive
	GotoState('Inactive');

	if( Instigator != None && Instigator.InvManager != None )
	{
//		`log( "RemoveFromInventoryEx" );
		avaInventoryManager(Instigator.InvManager).RemoveFromInventoryEx(Self,true);
	}
	Instigator = None;
}

simulated state Inactive
{
	// inactive 상태에서는 무기 교체가 가능해야 한다.
	simulated function bool DenyClientWeaponSet()
	{
		return false;
	}
}

// 거치 기관총을 잡고 있는 중에는 무기 교체를 할 수 없다...
simulated function bool DenyClientWeaponSet()
{	
	return true;	
}

simulated function AttachMuzzleFlash()
{
	local SkeletalMeshComponent SKMesh;
	// Attach the Muzzle Flash
	SKMesh = SkeletalMeshComponent(BaseWeap.WeaponMesh);
	if (  SKMesh != none )
	{
		// Muzzle Flash mesh
		if ( MuzzleFlashMesh != None )		SKMesh.AttachComponentToSocket(MuzzleFlashMesh, MuzzleFlashSocket);

		if (MuzzleFlashPSCTemplate != none)
		{
			MuzzleFlashPSC  = new(self) class'avaParticleSystemComponent';
			SKMesh.AttachComponentToSocket(MuzzleFlashPSC, MuzzleFlashSocket);
			MuzzleFlashPSC.DeactivateSystem();
			MuzzleFlashPSC.SetColorParameter('MuzzleFlashColor', MuzzleFlashColor);
		}

		// Muzzle Flash dynamic light
		if ( MuzzleFlashLight != None )
			SKMesh.AttachComponentToSocket(MuzzleFlashLight, MuzzleFlashSocket);
	}
}

simulated function DetachMuzzleFlash()
{
	local SkeletalMeshComponent SKMesh;
	SKMesh = SkeletalMeshComponent(BaseWeap.WeaponMesh);
	if (  SKMesh != none )
	{
		// Muzzle Flash Mesh
		if ( MuzzleFlashMesh != None )
			SKMesh.DetachComponent( MuzzleFlashMesh );

		if (MuzzleFlashPSC != none)
			SKMesh.DetachComponent( MuzzleFlashPSC );

		// Muzzle Flash dynamic light
		if ( MuzzleFlashLight != None )
			SKMesh.DetachComponent( MuzzleFlashLight );
	}
}

simulated function PlayTrailEffect( vector HitLocation );
//{
//	local rotator	r;
//	local vector	StartTrace, EndTrace;
//	local ImpactInfo		RealImpact;
//	local Array<ImpactInfo>	ImpactList;	
//	if (SkeletalMeshComponent(BaseWeap.WeaponMesh).GetSocketWorldLocationAndRotation( 'MuzzleFlashSocket', StartTrace, r ) )
//	{				
//		StartTrace  = StartTrace - Vector(r) * 30;
//		EndTrace 	= StartTrace + Vector(r) * GetTraceRange();
//		RealImpact = CalcWeaponFire(StartTrace, EndTrace, ImpactList);
//
//		if (RealImpact.HitActor != none)
//		{
//			EndTrace = RealImpact.HitLocation;
//		}
//
//		avaPawn(Instigator).BulletTrailComponent.Fire( StartTrace, EndTrace, /*bullet_class.Default.BulletSpeedForTrailEffect*/300.0 * 100.0 / 30.0 * 16.0 );			
//	}
//}

`devexec function DebugHW(string cmd)
{
	local string c,v;

	c = left(Cmd,InStr(Cmd,"="));
	v = mid(Cmd,InStr(Cmd,"=")+1);

	if (c~="cx")  BaseWeap.CamOffset.X += float(v);
	if (c~="cax") BaseWeap.CamOffset.X =  float(v);
	if (c~="cy")  BaseWeap.CamOffset.Y += float(v);
	if (c~="cay") BaseWeap.CamOffset.Y =  float(v);
	if (c~="cz")  BaseWeap.CamOffset.Z += float(v);
	if (c~="caz") BaseWeap.CamOffset.Z =  float(v);

	`log("#### DebugHW ####");
	`log("####    CamOffset :"@BaseWeap.CamOffset);
}

simulated function AnimNodeSequence PlayWeaponAnimation( Name Sequence, float fDesiredDuration, optional bool bLoop, optional SkeletalMeshComponent SkelMesh)
{	
//	Super.PlayWeaponAnimation( Sequence, fDesiredDuration, bLoop, SkelMesh );
	BaseWeap.PlayAnimatin( Sequence, fDesiredDuration, bLoop );
	return None;
}

// 일단은 Ammo 를 소비하지 않도록..
simulated function FireAmmunition()
{
	Super( avaWeapon ).FireAmmunition();
}

function ConsumeAmmo( byte FireModeNum )
{

}

simulated function SetPosition(avaPawn Holder)
{
	local vector DrawOffset, ViewOffset;
	local avaPawn.EWeaponHand CurrentHand;
	local rotator NewRotation;

	if ( !Instigator.IsFirstPerson() )
		return;

	// Hide the weapon if hidden
	CurrentHand = GetHand();
	if ( CurrentHand == HAND_Hidden)
	{
		SetHidden( true );
		return;
	}
	SetHidden( false );

	// Adjust for the current hand
	ViewOffset = PlayerViewOffset;
	switch ( CurrentHand )
	{
		case HAND_Left:
			ViewOffset.Y *= -1;
			break;

		case HAND_Centered:
			ViewOffset.Y = 0;
			break;
	}

	// Calculate the draw offset
	if ( Holder.Controller == None )
		DrawOffset = (ViewOffset >> Rotation) + Holder.GetEyeHeight() * vect(0,0,1);
	else
	{
		//DrawOffset.Z = Holder.GetEyeHeight();
		if ( Holder.bWeaponBob )
		{
			DrawOffset += Holder.WeaponBob(BobDamping,JumpDamping);
		}

		if ( avaPlayerController(Holder.Controller) != None )
		{
			DrawOffset += avaPlayerController(Holder.Controller).ShakeOffset >> Holder.Controller.Rotation;
		}

		DrawOffset = DrawOffset + ( ViewOffset >> Holder.Controller.Rotation );
	}

	// Adjust it in the world
	SetLocation( BaseWeap.GetViewPoint() + DrawOffset );

	NewRotation = (Holder.Controller == None ) ? Holder.Rotation : Holder.Controller.Rotation;
	// 아래 Code 를 적용하면 무기가 늦게 따라오기 때문에 답답해 보일 수 있어서 뺐음...
	//if ( Holder.bWeaponBob )
	//{
	//	// if bWeaponBob, then add some rotation lag
	//	NewRotation.Yaw = LagRot(NewRotation.Yaw & 65535, Rotation.Yaw & 65535, MaxYawLag);
	//	NewRotation.Pitch = LagRot(NewRotation.Pitch & 65535, Rotation.Pitch & 65535, MaxPitchLag);
	//}
	SetRotation(NewRotation);
}

simulated function Actor GetTraceOwner()
{
	return (BaseWeap != None) ? BaseWeap : ( Instigator != None ) ? Instigator : self;
}

defaultproperties
{
	WeaponFireTypes(0)		=	EWFT_InstantHit
	bAutoFire				=	true
	FireInterval(0)			=	0.0875

	InstantHitDamageTypes(0)=class'avaDmgType_Gun'

	WeaponFireAnim(0)		=	H_Gun_Fire


	HitDamage = 50

	//WeaponFireAnim(0)	=		Fire
 //	WeaponPutDownAnim		=	Down
	//WeaponEquipAnim			=	Idle
	//WeaponReloadAnim		=	Reload
	//WeaponIdleAnims(0)		=	Idle

	//EquipTime				=	1.1333
	//PutDownTime				=	0.0333
	//ReloadTime				=	2.8333



	// KickBack_WhenSteady 만 유효하다....
	Kickback_WhenMoving		=	(UpBase=1.0,LateralBase=0.45,UpModifier=0.28,LateralModifier=0.045,UpMax=4.75,LateralMax=3,DirectionChange=7)
	Kickback_WhenFalling	=	(UpBase=1.2,LateralBase=0.5,UpModifier=0.23,LateralModifier=0.15,UpMax=5.5,LateralMax=3.5,DirectionChange=6)
	Kickback_WhenDucking	=	(UpBase=0.6,LateralBase=0.3,UpModifier=0.2,LateralModifier=0.0125,UpMax=4.25,LateralMax=2,DirectionChange=7)
	Kickback_WhenSteady		=	(UpBase=0.65,LateralBase=0.35,UpModifier=0.25,LateralModifier=0.015,UpMax=4.5,LateralMax=2.25,DirectionChange=7)

	// Spread_WhenSteady 만 유효하다....
	Spread_WhenFalling		=	(param1=0.035,param2=0.4)
	Spread_WhenMoving		=	(param1=0.035,param2=0.07)
	Spread_WhenDucking		=	(param1=0,param2=0.02)
	Spread_WhenSteady		=	(param1=0,param2=0.02)	

	AttachmentClass			=	class'avaAttachment_BaseFixedWeapon'

	MuzzleFlashPSCTemplate	=	ParticleSystem'avaEffect.Particles.P_WP_Rifle_MuzzleFlash_1P'
	MuzzleFlashDuration		=	0.33

	Penetration				=	1
	bHideWeaponMenu			=	true	
}