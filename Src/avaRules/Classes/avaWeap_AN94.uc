class avaWeap_AN94 extends avaWeap_BaseRifle;

defaultproperties
{
	BulletType=class'avaBullet_545M4'

	BaseSpeed		= 245	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.32	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage=39

	FireInterval(0)=0.1155

	ClipCnt=30
	AmmoCount=30
	MaxAmmoCount=90

	Penetration = 2
	RangeModifier=0.84

	SpreadDecayTime = 1.2

	Kickback_WhenMoving = (UpBase=1.98,LateralBase=1.44,UpModifier=0.132,LateralModifier=0.121,UpMax=38.5,LateralMax=4.95,DirectionChange=6)
	Kickback_WhenFalling = (UpBase=3.6,LateralBase=2.4,UpModifier=0.24,LateralModifier=0.22,UpMax=70,LateralMax=9,DirectionChange=6)
	Kickback_WhenDucking = (UpBase=1.26,LateralBase=0.84,UpModifier=0.12,LateralModifier=0.11,UpMax=24.5,LateralMax=3.15,DirectionChange=6)
	Kickback_WhenSteady = (UpBase=1.8,LateralBase=1.2,UpModifier=0.12,LateralModifier=0.11,UpMax=35,LateralMax=4.5,DirectionChange=6)
	
	Spread_WhenFalling = (param1=-0.012,param2=0.246)
	Spread_WhenMoving = (param1=-0.002,param2=0.0902)
	Spread_WhenDucking = (param1=-0.001,param2=0.0205)
	Spread_WhenSteady = (param1=-0.002,param2=0.041)
	
	AccuracyDivisor  =  800
	AccuracyOffset  =  0.2
	MaxInaccuracy  =  1.2
	
	Kickback_UpLimit = 12
	Kickback_LateralLimit = 4.5


	Kickback_WhenMovingA = (UpBase=2.57,LateralBase=1.872,UpModifier=0.1716,LateralModifier=0.1573,UpMax=50.1,LateralMax=6.4,DirectionChange=6)
	Kickback_WhenFallingA = (UpBase=4.68,LateralBase=3.12,UpModifier=0.312,LateralModifier=0.286,UpMax=91,LateralMax=11.7,DirectionChange=6)
	Kickback_WhenDuckingA = (UpBase=1.26,LateralBase=0.84,UpModifier=0.12,LateralModifier=0.11,UpMax=24.5,LateralMax=3.15,DirectionChange=6)
	Kickback_WhenSteadyA = (UpBase=1.8,LateralBase=1.2,UpModifier=0.12,LateralModifier=0.11,UpMax=35,LateralMax=4.5,DirectionChange=6)
	
	Spread_WhenFallingA = (param1=-0.012,param2=0.492)
	Spread_WhenMovingA = (param1=-0.002,param2=0.1804)
	Spread_WhenDuckingA = (param1=-0.0009,param2=0.01845)
	Spread_WhenSteadyA = (param1=-0.0018,param2=0.0369)
	
	AccuracyDivisorA  =  2400
	AccuracyOffsetA  =  0.18
	MaxInaccuracyA  =  1.44
	
	DirectionHold = 2
	FireIntervalMultiplierA = 1


	KickbackLimiter(0) = (Min=0.1,Max=0.1)
	KickbackLimiter(1) = (Min=0.1,Max=0.1)
	KickbackLimiter(2) = (Min=1.0,Max=1.5)
	
	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.367
	PutDownTime			=	0.1
	ReloadTime			=	2.367

	SightInfos(0) = (FOV=90,ChangeTime=0.2)
	SightInfos(1) = (FOV=75,ChangeTime=0.15)

	bHideWeaponInSightMode = false
	bHideCursorInSightMode = false
	ScopeMeshName          = ""
	SightInAnim            = ""
	SightOutAnim           = ""
	bReleaseZoomAfterFire = false

	BaseSkelMeshName	=	"Wp_Rif_AN94.MS_AN94"
	BaseAnimSetName		=	"Wp_Rif_AN94.Ani_An94"

	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=1.0,OtherShotRate=0.0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.0,OtherShotRate=1.0)
	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4

	AttachmentClass=class'avaAttachment_AN94'
	WeaponFireSnd=SoundCue'avaWeaponSounds.AR_AN94.AR_AN94_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
