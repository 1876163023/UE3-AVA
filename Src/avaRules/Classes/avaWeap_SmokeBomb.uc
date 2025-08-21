//=============================================================================
//  avaWeap_SmokeBomb.
// 
//  Copyright 2006 Ntix Soft, Inc. All Rights Reserved.
//  
//	2006/02/09 by OZ
//		1.	avaWeap_Grenade 를 기준으로 제작되었음.
//=============================================================================

class avaWeap_SmokeBomb extends avaThrowableWeapon;

defaultproperties
{
	BaseSkelMeshName	=	"Wp_M83.MS_M83"
	BaseAnimSetName		=	"Wp_M83.Ani_M83"

	AmmoCount=2
	MaxAmmoCount=2
	WeaponRange=300

	InventoryGroup=4
	GroupWeight=0.5

	WeaponColor=(R=255,G=0,B=0,A=255)

	AttachmentClass=class'avaAttachment_SmokeBomb'
	
	/*WeaponLoadSnd=SoundCue'A_Weapon.RocketLauncher.Cue.A_Weapon_RL_Load_Cue'
	WeaponLoadedSnd=SoundCue'A_Pickups.Ammo.Cue.A_Pickup_Ammo_Sniper_Cue'*/
	WeaponFireSnd=None

	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	FireShake=(OffsetMag=(X=-5.0,Y=-5.00,Z=-5.00),OffsetRate=(X=-1000.0,Y=-1000.0,Z=-1000.0),OffsetTime=2,RotMag=(X=0.0,Y=0.0,Z=0.0),RotRate=(X=0.0,Y=0.0,Z=0.0),RotTime=2)	

	//MuzzleFlashSocket=MuzzleFlashSocketA
	//MuzzleFlashSocketList(0)=MuzzleFlashSocketA
	//MuzzleFlashSocketList(1)=MuzzleFlashSocketB
	//MuzzleFlashSocketList(2)=MuzzleFlashSocketC

	//MuzzleFlashPSCTemplate=WP_RocketLauncher.Particles.P_WP_RockerLauncher_Muzzle_Flash
	//MuzzleFlashDuration=0.33
	//WeaponKProjectiles(0)=class'avaKProj_SmokeGrenadeANM8'
	//WeaponKProjectiles(1)=class'avaKProj_SmokeGrenadeANM8'
	WeaponProjectiles(0)=class'avaProj_SmokeGrenadeANM8'
	WeaponProjectiles(1)=class'avaProj_SmokeGrenadeANM8'

	WeaponFireAnim(0)	=	None
	WeaponFireAnim(1)	=	None
 	WeaponPutDownAnim	=	None
	WeaponEquipAnim		=	BringUp
	EquipTime			=	0.8333
	PutDownTime			=	0.0
	WeaponIdleAnims(0)	=	Idle
	WeaponIdleAnims(1)	=	M83_Gesture
	MinIdleContinuousCnt=	3
	
	// 수류탄의 기본값들임
	PrepareAnim(0)		=	PinUp
	PrepareTime(0)		=	0.5667
	PrepareSnd(0)		=	None

	PrepareAnim(1)		=	PinUp
	PrepareTime(1)		=	0.5667
	PrepareSnd(1)		=	None

	//CancelAnim		=	WeaponAltFireLaunch1End
	//CancelTime=   0.3
	//CancelSnd=

	ThrowAnim(0)		=	Fire1
	ThrowTime(0)		=	0.60

	ThrowAnim(1)		=	Fire2
	ThrowTime(1)		=	0.60
	//ThrowSnd=
	//ThrowEndAnim		=	BringUp
	ThrowEndTime(0)		=	0
	ThrowEndTime(1)		=	0
	//ThrowEndSnd=
	FireTime(0)			=	0.08		//  던지는 Animation 으로 전환후 몇초 있다 Projectile 을 생성할 것인가?
	FireTime(1)			=	0.14		//  던지는 Animation 으로 전환후 몇초 있다 Projectile 을 생성할 것인가?

	FiringStatesArray(0)=	WeaponPreparing
	FiringStatesArray(1)=	WeaponPreparing

	WeaponFireTypes(0)	=	EWFT_Projectile
	WeaponFireTypes(1)	=	EWFT_Projectile

	FireOffsetEx(0)		=	(X=20,Y=10,Z=10)
	FireOffsetEx(1)		=	(X=17,Y=12,Z=-5)

	
	ProjZAng(0)				=	0.28
	ProjZAng(1)				=	0.05

	ThrowEndRadioSoundIndex(0)	=	-1
	ThrowEndRadioSoundIndex(1)	=	-1

	/// cass 는 누르는 동안에 취소할 수 없다.
	bCancelPrepare(0)	=false
	bCancelPrepare(1)	=false
	/// 준비가 다 되면 들고 있는다.
	bSkipReady(0)		=false
	bSkipReady(1)		=false

	ProjStartVel(0)			=	900
	ProjStartVel(1)			=	400

	// Projectile 의 윗방향으로의 Speed
	ProjTossZ(0)			=	150
	ProjTossZ(1)			=	30

	DropOnlyOneAmmo		=	true
	bAlwaysDrawIcon		=	true

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.6
	BobDampingInDash	=	0.4
}


