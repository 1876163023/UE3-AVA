class avaWeap_SakoRk95 extends avaWeap_BaseRifle;

defaultproperties
{
	BulletType=class'avaBullet_762X39R'

	BaseSpeed		= 248	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.32	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage=43

	FireInterval(0)=0.101

	ClipCnt=30
	AmmoCount=30
	MaxAmmoCount=90

	Penetration = 2
	RangeModifier=0.82

	SpreadDecayTime = 0.35

	Kickback_WhenMoving = (UpBase=2.2,LateralBase=0.88,UpModifier=0.165,LateralModifier=0.22,UpMax=38.5,LateralMax=5.5,DirectionChange=6)
	Kickback_WhenFalling = (UpBase=4,LateralBase=1.6,UpModifier=0.3,LateralModifier=0.4,UpMax=70,LateralMax=10,DirectionChange=6)
	Kickback_WhenDucking = (UpBase=1.8,LateralBase=0.56,UpModifier=0.105,LateralModifier=0.14,UpMax=24.5,LateralMax=3.5,DirectionChange=6)
	Kickback_WhenSteady = (UpBase=2,LateralBase=0.8,UpModifier=0.15,LateralModifier=0.2,UpMax=35,LateralMax=5,DirectionChange=6)
	
	Spread_WhenFalling = (param1=0.06,param2=0.1512)
	Spread_WhenMoving = (param1=0.018,param2=0.04725)
	Spread_WhenDucking = (param1=0.008,param2=0.017955)
	Spread_WhenSteady = (param1=0.01,param2=0.0189)
	
	AccuracyDivisor  =  700
	AccuracyOffset  =  0.2
	MaxInaccuracy  =  1
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 3


	Kickback_WhenMovingA = (UpBase=2.86,LateralBase=1.144,UpModifier=0.2145,LateralModifier=0.286,UpMax=50.1,LateralMax=7.2,DirectionChange=6)
	Kickback_WhenFallingA = (UpBase=5.2,LateralBase=2.08,UpModifier=0.39,LateralModifier=0.52,UpMax=91,LateralMax=13,DirectionChange=6)
	Kickback_WhenDuckingA = (UpBase=1.8,LateralBase=0.56,UpModifier=0.105,LateralModifier=0.14,UpMax=24.5,LateralMax=3.5,DirectionChange=6)
	Kickback_WhenSteadyA = (UpBase=2,LateralBase=0.8,UpModifier=0.15,LateralModifier=0.2,UpMax=35,LateralMax=5,DirectionChange=6)
	
	Spread_WhenFallingA = (param1=0.06,param2=0.3024)
	Spread_WhenMovingA = (param1=0.018,param2=0.0945)
	Spread_WhenDuckingA = (param1=0.0072,param2=0.0161595)
	Spread_WhenSteadyA = (param1=0.009,param2=0.01701)
	
	AccuracyDivisorA  =  2100
	AccuracyOffsetA  =  0.18
	MaxInaccuracyA  =  1.2
	
	DirectionHold = 3
	FireIntervalMultiplierA = 1.5

	KickbackLimiter(0) = (Min=0.7,Max=1.2)
	
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

	BaseSkelMeshName	=	"Wp_Rif_AK47.MS_AK47"
	BaseAnimSetName		=	"Wp_Rif_AK47.Ani_AK47"

	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=1.0,OtherShotRate=0.0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.0,OtherShotRate=1.0)
	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4

	AttachmentClass=class'avaAttachment_AK47'
	WeaponFireSnd=SoundCue'avaWeaponSounds.AR_AK47.AR_AK47_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
