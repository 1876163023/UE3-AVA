class avaWeap_G36 extends avaWeap_BaseRifle;

defaultproperties
{
	BulletType=class'avaBullet_556NATO'

	BaseSpeed		= 250	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.32	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage=36
	FireInterval(0)=0.0985

	ClipCnt=30
	AmmoCount=30
	MaxAmmoCount=90

	Penetration = 2
	RangeModifier=0.83

	SpreadDecayTime = 1.1

	Kickback_WhenMoving = (UpBase=1.92,LateralBase=0.385,UpModifier=0.088,LateralModifier=0.352,UpMax=31.2,LateralMax=6.05,DirectionChange=8)
	Kickback_WhenFalling = (UpBase=3.2,LateralBase=0.7,UpModifier=0.16,LateralModifier=0.64,UpMax=52,LateralMax=11,DirectionChange=8)
	Kickback_WhenDucking = (UpBase=1.44,LateralBase=0.28,UpModifier=0.072,LateralModifier=0.16,UpMax=20.8,LateralMax=4.4,DirectionChange=12)
	Kickback_WhenSteady = (UpBase=1.6,LateralBase=0.35,UpModifier=0.08,LateralModifier=0.32,UpMax=26,LateralMax=5.5,DirectionChange=8)
	
	Spread_WhenFalling = (param1=0.06,param2=0.1512)
	Spread_WhenMoving = (param1=0.018,param2=0.04725)
	Spread_WhenDucking = (param1=0.008,param2=0.017955)
	Spread_WhenSteady = (param1=0.01,param2=0.0189)
	
	AccuracyDivisor  =  650
	AccuracyOffset  =  0.2
	MaxInaccuracy  =  1
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 3.5

	Kickback_WhenMovingA = (UpBase=2.5,LateralBase=0.5005,UpModifier=0.1144,LateralModifier=0.4576,UpMax=40.6,LateralMax=7.9,DirectionChange=8)
	Kickback_WhenFallingA = (UpBase=4.16,LateralBase=0.91,UpModifier=0.208,LateralModifier=0.832,UpMax=67.6,LateralMax=14.3,DirectionChange=8)
	Kickback_WhenDuckingA = (UpBase=1.44,LateralBase=0.28,UpModifier=0.072,LateralModifier=0.16,UpMax=20.8,LateralMax=4.4,DirectionChange=12)
	Kickback_WhenSteadyA = (UpBase=1.6,LateralBase=0.35,UpModifier=0.08,LateralModifier=0.32,UpMax=26,LateralMax=5.5,DirectionChange=8)
	
	Spread_WhenFallingA = (param1=0.06,param2=0.3024)
	Spread_WhenMovingA = (param1=0.018,param2=0.0945)
	Spread_WhenDuckingA = (param1=0.0072,param2=0.0161595)
	Spread_WhenSteadyA = (param1=0.009,param2=0.01701)
	
	AccuracyDivisorA  =  1950
	AccuracyOffsetA  =  0.18
	MaxInaccuracyA  =  1.2
	
	DirectionHold = 1
	FireIntervalMultiplierA = 1.5

	KickbackLimiter(0) = (Min=0.7,Max=1)
	KickbackLimiter(1) = (Min=0.8,Max=1)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.2
	PutDownTime			=	0.333
	ReloadTime			=	2.533

	SightInfos(0) = (FOV=90,ChangeTime=0.2)
	SightInfos(1) = (FOV=75,ChangeTime=0.15)

	bHideWeaponInSightMode = true
	bHideCursorInSightMode = true
	bReleaseZoomAfterFire = false
	
	ScopeMeshName = "Wp_Scope.Com_Scope.MS_UIG3601"
	BaseSkelMeshName	=	"Wp_Rif_G36.Wp_Rif_G36_Scope.MS_G36_Scope"
	BaseAnimSetName		=	"Wp_Rif_G36.Ani_G36"

	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=1.0,OtherShotRate=0.0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.0,OtherShotRate=1.0)
	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4
	AttachmentClass=class'avaAttachment_G36'
	WeaponFireSnd=SoundCue'avaWeaponSounds.AR_G36.AR_G36_Fire'
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
