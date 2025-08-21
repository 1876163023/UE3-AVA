//=============================================================================
//  avaWeap_GalilSniper
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/04/05 by OZ
//=============================================================================

class avaWeap_GalilSniper extends avaWeap_BaseSniperRifle;

defaultproperties
{
	BulletType=class'avaBullet_762NATO'

	BaseSpeed		= 232	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage=70

	FireInterval(0)=0.418
	FireInterval(1)=0.418
	FireInterval(2)=0.418

	ClipCnt=20
	AmmoCount=20
	MaxAmmoCount=40

	Penetration = 2
	RangeModifier=0.9

	SpreadDecayTime = 0.9

	Kickback_WhenMoving = (UpBase=4.5,LateralBase=1.8,UpModifier=1.5,LateralModifier=0.9,UpMax=30,LateralMax=22.5,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=4.5,LateralBase=1.8,UpModifier=1.5,LateralModifier=0.9,UpMax=30,LateralMax=22.5,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=2.4,LateralBase=0.96,UpModifier=0.8,LateralModifier=0.48,UpMax=16,LateralMax=12,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=3,LateralBase=1.2,UpModifier=1,LateralModifier=0.6,UpMax=20,LateralMax=15,DirectionChange=3)
	
	Spread_WhenFalling = (param1=-0.031,param2=0.8)
	Spread_WhenMoving = (param1=-0.0775,param2=0.24)
	Spread_WhenDucking = (param1=-0.031,param2=0.072)
	Spread_WhenSteady = (param1=-0.031,param2=0.08)
	
	AccuracyDivisor  =  500
	AccuracyOffset  =  1.5
	MaxInaccuracy  =  5
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 3


	Kickback_WhenMovingA = (UpBase=4.5,LateralBase=1.8,UpModifier=1.5,LateralModifier=0.9,UpMax=30,LateralMax=22.5,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=4.5,LateralBase=1.8,UpModifier=1.5,LateralModifier=0.9,UpMax=30,LateralMax=22.5,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=2.4,LateralBase=0.96,UpModifier=0.8,LateralModifier=0.48,UpMax=16,LateralMax=12,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=3,LateralBase=1.2,UpModifier=1,LateralModifier=0.6,UpMax=20,LateralMax=15,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=-0.031,param2=0.8)
	Spread_WhenMovingA = (param1=-0.0775,param2=0.24)
	Spread_WhenDuckingA = (param1=-0.031,param2=0.072)
	Spread_WhenSteadyA = (param1=-0.031,param2=0.08)
	
	AccuracyDivisorA  =  500
	AccuracyOffsetA  =  0.55
	MaxInaccuracyA  =  2.5
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1


	KickbackLimiter(0) = (Min=0.8,Max=1.6)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.6
	BobDamping		=	0.7
	BobDampingInDash	=	0.5

	EquipTime			=1.4333
	PutDownTime			=0.2333
	ReloadTime			=3.6333

	SightInfos(0) = (FOV=90,ChangeTime=0.15)

	bHideWeaponInSightMode = true
	bHideCursorInSightMode = true
	bReleaseZoomAfterFire = false

	bAutoFire	=	True

	WeaponFireAnim(0)	=Fire
	WeaponPutDownAnim	=Down
	WeaponEquipAnim		=BringUp
	WeaponReloadAnim	=Reload
	WeaponIdleAnims(0)	=Idle

	AttachmentClass=class'avaAttachment_GalilSniper'
	BaseSkelMeshName	=	"Wp_Sn_Galil.Wp_Sn_Galil_Scope.MS_Gali"
	BaseAnimSetName		=	"WP_sn_Galil.Ani_Galil_1P"

	WeaponFireSnd=SoundCue'avaWeaponSounds.SR_GalilSniper.SR_GalilSniper_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
//스코프 추가
	DefaultModifiers(0)			=	class'avaRules.avaMod_GalilSniper_M_QSScope'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Sniper'

	ScopeMeshName = "avaScopeUI.Distortion.MS_GalilSniper_Scope_Mesh"
// 발사시 연기 삽입 테스트
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_Sniper_Muzzlesmoke_1P'
}
