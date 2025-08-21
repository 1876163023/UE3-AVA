class avaWeap_AK74M extends avaWeap_BaseRifle;

defaultproperties
{
	BulletType=class'avaBullet_545M4'

	BaseSpeed		= 250	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.32	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage=39
	FireInterval(0)=0.112

	ClipCnt=30
	AmmoCount=30
	MaxAmmoCount=90

	Penetration = 2
	RangeModifier=0.82

	SpreadDecayTime = 0.55

	Kickback_WhenMoving = (UpBase=2.66,LateralBase=0.91,UpModifier=0.165,LateralModifier=0.132,UpMax=35.2,LateralMax=4.29,DirectionChange=6)
	Kickback_WhenFalling = (UpBase=3.8,LateralBase=1.3,UpModifier=0.22,LateralModifier=0.22,UpMax=64,LateralMax=7.8,DirectionChange=6)
	Kickback_WhenDucking = (UpBase=1.71,LateralBase=0.52,UpModifier=0.077,LateralModifier=0.077,UpMax=22.4,LateralMax=2.73,DirectionChange=6)
	Kickback_WhenSteady = (UpBase=1.9,LateralBase=0.65,UpModifier=0.11,LateralModifier=0.11,UpMax=32,LateralMax=3.9,DirectionChange=6)
	
	Spread_WhenFalling = (param1=-0.0027,param2=0.192)
	Spread_WhenMoving = (param1=-0.00225,param2=0.0832)
	Spread_WhenDucking = (param1=-0.00036,param2=0.0192)
	Spread_WhenSteady = (param1=-0.0009,param2=0.032)
	
	AccuracyDivisor  =  600
	AccuracyOffset  =  0.32
	MaxInaccuracy  =  1.5
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 4


	Kickback_WhenMovingA = (UpBase=3.46,LateralBase=1.183,UpModifier=0.2145,LateralModifier=0.1716,UpMax=45.8,LateralMax=5.6,DirectionChange=6)
	Kickback_WhenFallingA = (UpBase=4.94,LateralBase=1.69,UpModifier=0.286,LateralModifier=0.286,UpMax=83.2,LateralMax=10.14,DirectionChange=6)
	Kickback_WhenDuckingA = (UpBase=1.197,LateralBase=0.364,UpModifier=0.0539,LateralModifier=0.0539,UpMax=22.4,LateralMax=2.73,DirectionChange=6)
	Kickback_WhenSteadyA = (UpBase=1.52,LateralBase=0.52,UpModifier=0.088,LateralModifier=0.088,UpMax=32,LateralMax=3.9,DirectionChange=6)
	
	Spread_WhenFallingA = (param1=-0.0027,param2=0.384)
	Spread_WhenMovingA = (param1=-0.00225,param2=0.1664)
	Spread_WhenDuckingA = (param1=-0.000324,param2=0.01728)
	Spread_WhenSteadyA = (param1=-0.00081,param2=0.0288)
	
	AccuracyDivisorA  =  1800
	AccuracyOffsetA  =  0.256
	MaxInaccuracyA  =  1.8
	
	DirectionHold = 1
	FireIntervalMultiplierA = 1.1

	KickbackLimiter(0) = (Min=0.7,Max=1.0)
	KickbackLimiter(1) = (Min=0.8,Max=1.1)
	KickbackLimiter(2) = (Min=0.9,Max=1.3)
	
	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.3667
	PutDownTime			=	0.333
	ReloadTime			=	2.3667

	SightInfos(0) = (FOV=90,ChangeTime=0.2)
	SightInfos(1) = (FOV=75,ChangeTime=0.15)

	bHideWeaponInSightMode = false
	bHideCursorInSightMode = false
	ScopeMeshName          = ""
	SightInAnim            = ""
	SightOutAnim           = ""
	bReleaseZoomAfterFire = false

	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=1.0,OtherShotRate=0.0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.0,OtherShotRate=1.0)

	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4

	BaseSkelMeshName	=	"Wp_Rif_AK74.MS_AK74M_rail"
	BaseAnimSetName		=	"Wp_Rif_AK74.Ani_AK74"

	AttachmentClass=class'avaAttachment_AK74M'
	WeaponFireSnd=SoundCue'avaWeaponSounds.AR_AK74M.AR_AK74M_fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'

	DefaultModifiers(0)			=	class'avaRules.avaMod_AK74MRail_M_BaseDot'
}
