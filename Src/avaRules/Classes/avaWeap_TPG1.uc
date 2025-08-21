//=============================================================================
//  avaWeap_TPG1
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/04/05 by OZ
//		AWP 는 현재 TPG1 으로 사용되고 있음
//=============================================================================

class avaWeap_TPG1 extends avaWeap_BaseSniperRifle;

defaultproperties
{
	BulletType=class'avaBullet_338LUPUA'

	BaseSpeed		= 228	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage=110

	FireInterval(0)=1.4
	FireInterval(1)=1.4
	FireInterval(2)=1.4

	ClipCnt=5
	AmmoCount=5
	MaxAmmoCount=20

	Penetration = 2
	RangeModifier=0.99

	SpreadDecayTime = 1.4

	Kickback_WhenMoving = (UpBase=19.5,LateralBase=0.525,UpModifier=0.27,LateralModifier=0.15,UpMax=11.25,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=19.5,LateralBase=0.525,UpModifier=0.27,LateralModifier=0.15,UpMax=11.25,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=10.4,LateralBase=0.28,UpModifier=0.144,LateralModifier=0.08,UpMax=6,LateralMax=4,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=13,LateralBase=0.35,UpModifier=0.18,LateralModifier=0.1,UpMax=7.5,LateralMax=5,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.99,param2=0.024)
	Spread_WhenMoving = (param1=0.495,param2=0.01)
	Spread_WhenDucking = (param1=0.0132,param2=0.0016)
	Spread_WhenSteady = (param1=0.0165,param2=0.002)
	
	AccuracyDivisor  =  600
	AccuracyOffset  =  0.3
	MaxInaccuracy  =  2
	
	Kickback_UpLimit = 15
	Kickback_LateralLimit = 5


	Kickback_WhenMovingA = (UpBase=19.5,LateralBase=0.525,UpModifier=0.27,LateralModifier=0.15,UpMax=11.25,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=19.5,LateralBase=0.525,UpModifier=0.27,LateralModifier=0.15,UpMax=11.25,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=10.4,LateralBase=0.28,UpModifier=0.144,LateralModifier=0.08,UpMax=6,LateralMax=4,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=13,LateralBase=0.35,UpModifier=0.18,LateralModifier=0.1,UpMax=7.5,LateralMax=5,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.66,param2=0.024)
	Spread_WhenMovingA = (param1=0.33,param2=0.01)
	Spread_WhenDuckingA = (param1=0.0088,param2=0.0016)
	Spread_WhenSteadyA = (param1=0.011,param2=0.002)
	
	AccuracyDivisorA  =  600
	AccuracyOffsetA  =  0.15
	MaxInaccuracyA  =  1
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1

	KickbackLimiter(0) = (Min=0.8,Max=1.2)

	bEjectBulletWhenFire = False

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.6
	BobDamping		=	0.7
	BobDampingInDash	=	0.5

	EquipTime			=1.20
	PutDownTime			=0.2333
	ReloadTime			=3.033

	LastFireAnim	 =	Fire_Last
	LastFireInterval =	0.3667

	SightInfos(0) = (FOV=90,ChangeTime=0.12)

	bHideWeaponInSightMode = true
	bHideCursorInSightMode = true
	bReleaseZoomAfterFire  = true
	fReleaseZoomAfterFireInterval = 0.08
	bRecoverZoomAfterFire  = true
	RecoverZoomTime        = 0.12

	AttachmentClass=class'avaAttachment_TPG1'
	BaseSkelMeshName	=	"Wp_sn_TPG1.Wp_Sn_TPG1_Rail.MS_TPG1"
	BaseAnimSetName		=	"Wp_sn_TPG1.Ani_TPG_1"

	WeaponFireSnd=SoundCue'avaWeaponSounds.SN_TPG1.SN_TPG1_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	DefaultModifiers(0)			=	class'avaRules.avaMod_TPG1_M_Scope'
	InstantHitDamageTypes(0)	=	class'avaDmgType_Sniper'
	
	ScopeMeshName = "avaScopeUI.Distortion.MS_TPGSniper_Scope_Mesh"
// 발사시 연기 삽입 테스트
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_Sniper_Muzzlesmoke_1P'
}
