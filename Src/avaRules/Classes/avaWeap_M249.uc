class avaWeap_M249 extends avaWeap_BaseMachineGun;

//고정형이 아니라 들고 다닐수 있는 머신건
//이미지 적인 부분을 설정할 필요 있음.

defaultproperties
{
	BulletType		= class'avaBullet_556NATO'

	BaseSpeed		= 230	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치


	HitDamage		=	70
	FireInterval(0)		=	0.1

	ClipCnt			= 100
	AmmoCount		= 100
	MaxAmmoCount		= 200

	Penetration		=	2
	RangeModifier		=	0.95


	SpreadDecayTime = 1.2

	Kickback_WhenMoving = (UpBase=6,LateralBase=4,UpModifier=0.2,LateralModifier=0.1,UpMax=60,LateralMax=8,DirectionChange=8)
	Kickback_WhenFalling = (UpBase=6,LateralBase=4,UpModifier=0.2,LateralModifier=0.1,UpMax=60,LateralMax=16,DirectionChange=8)
	Kickback_WhenDucking = (UpBase=1.5,LateralBase=1,UpModifier=0.05,LateralModifier=0.025,UpMax=15,LateralMax=4,DirectionChange=8)
	Kickback_WhenSteady = (UpBase=3,LateralBase=2,UpModifier=0.1,LateralModifier=0.05,UpMax=30,LateralMax=8,DirectionChange=8)
	
	Spread_WhenFalling = (param1=0.12,param2=0.15)
	Spread_WhenMoving = (param1=0.06,param2=0.075)
	Spread_WhenDucking = (param1=0.02,param2=0.025)
	Spread_WhenSteady = (param1=0.04,param2=0.05)
	
	AccuracyDivisor  =  600
	AccuracyOffset  =  0.3
	MaxInaccuracy  =  3
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 4

	Kickback_WhenMovingA = (UpBase=7.8,LateralBase=5.2,UpModifier=0.26,LateralModifier=0.13,UpMax=78,LateralMax=10.4,DirectionChange=8)
	Kickback_WhenFallingA = (UpBase=7.8,LateralBase=5.2,UpModifier=0.26,LateralModifier=0.13,UpMax=78,LateralMax=20.8,DirectionChange=8)
	Kickback_WhenDuckingA = (UpBase=1.5,LateralBase=1,UpModifier=0.05,LateralModifier=0.025,UpMax=15,LateralMax=4,DirectionChange=8)
	Kickback_WhenSteadyA = (UpBase=1.7,LateralBase=1,UpModifier=0.05,LateralModifier=0.025,UpMax=15,LateralMax=4,DirectionChange=8)
	
	Spread_WhenFallingA = (param1=0.12,param2=0.3)
	Spread_WhenMovingA = (param1=0.06,param2=0.15)
	Spread_WhenDuckingA = (param1=0.016,param2=0.0225)
	Spread_WhenSteadyA = (param1=0.025,param2=0.025)
	
	AccuracyDivisorA  =  2100
	AccuracyOffsetA  =  0.255
	MaxInaccuracyA  =  2.5
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1

	KickbackLimiter(0) = (Min=0.4,Max=0.6)
	KickbackLimiter(1) = (Min=0.5,Max=0.8)
	KickbackLimiter(2) = (Min=0.6,Max=1)
	KickbackLimiter(3) = (Min=0.7,Max=1)
	KickbackLimiter(4) = (Min=0.8,Max=1.1)

	EquipTime		= 1.5
	PutDownTime		= 1.5333
	ReloadTime		= 5

	WeaponFireSnd			=	SoundCue'avaWeaponSounds.Heavy_M249.Heavy_M249_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	AttachmentClass		= class'avaAttachment_M249'
	BaseSkelMeshName	= "WP_Heavy_M249.WP_M249"
	BaseAnimSetName		= "WP_Heavy_M249.M249_Ani_1P"

	MuzzleFlashPSCTemplate  = ParticleSystem'avaEffect.Particles.P_WP_Heavy_MuzzleFlash_1P'
	MuzzleFlashDuration		= 0.065

	FireAnimInfos(0)		=	(AnimName=Fire2,FirstShotRate=1,OtherShotRate=0.4)
	FireAnimInfos(1)		=	(AnimName=Fire1,FirstShotRate=0,OtherShotRate=0.6)
	WeaponFireAnim(0)		=	Fire2
	WeaponFireAnim(1)		=	Fire1

	InstantHitDamageTypes(0)	=	class'avaDmgType_MachineGun'

	SightInfos(0) = (FOV=90,ChangeTime=0.2)
	SightInfos(1) = (FOV=65,ChangeTime=0.2)

	bHideWeaponInSightMode = false
	bHideCursorInSightMode = false
	ScopeMeshName          = ""
	SightInAnim            = ""
	SightOutAnim           = ""
	bReleaseZoomAfterFire = false

	Begin Object Name=BulletTrailComponent0
		HalfLife	=	0.03
		Intensity	=	40.1
		Size		=	3.57
		Speed		=	6294.77
	End Object
	TrailInterval	=	1

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.6
	BobDamping		=	0.7
	BobDampingInDash	=	0.5
}