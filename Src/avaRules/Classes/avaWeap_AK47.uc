class avaWeap_AK47 extends avaWeap_BaseRifle;

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

	HitDamage=42
	FireInterval(0)=0.121

	ClipCnt=30
	AmmoCount=30
	MaxAmmoCount=90

	Penetration = 2
	RangeModifier=0.82

	SpreadDecayTime = 0.4

	Kickback_WhenMoving = (UpBase=2.475,LateralBase=1.045,UpModifier=0.165,LateralModifier=0.275,UpMax=35.2,LateralMax=4.73,DirectionChange=7)
	Kickback_WhenFalling = (UpBase=4.5,LateralBase=1.9,UpModifier=0.3,LateralModifier=0.5,UpMax=64,LateralMax=8.6,DirectionChange=7)
	Kickback_WhenDucking = (UpBase=2.025,LateralBase=0.665,UpModifier=0.105,LateralModifier=0.175,UpMax=22.4,LateralMax=3.01,DirectionChange=7)
	Kickback_WhenSteady = (UpBase=2.25,LateralBase=0.95,UpModifier=0.15,LateralModifier=0.25,UpMax=32,LateralMax=4.3,DirectionChange=7)
	
	Spread_WhenFalling = (param1=-0.006,param2=0.228)
	Spread_WhenMoving = (param1=-0.005,param2=0.0912)
	Spread_WhenDucking = (param1=-0.0008,param2=0.0228)
	Spread_WhenSteady = (param1=-0.002,param2=0.038)
	
	AccuracyDivisor  =  700
	AccuracyOffset  =  0.31
	MaxInaccuracy  =  1.1
	
	Kickback_UpLimit = 8
	Kickback_LateralLimit = 3.2




	Kickback_WhenMovingA = (UpBase=3.22,LateralBase=1.3585,UpModifier=0.2145,LateralModifier=0.3575,UpMax=45.8,LateralMax=6.1,DirectionChange=7)
	Kickback_WhenFallingA = (UpBase=5.85,LateralBase=2.47,UpModifier=0.39,LateralModifier=0.65,UpMax=83.2,LateralMax=11.18,DirectionChange=7)
	Kickback_WhenDuckingA = (UpBase=2.025,LateralBase=0.665,UpModifier=0.105,LateralModifier=0.175,UpMax=22.4,LateralMax=3.01,DirectionChange=7)
	Kickback_WhenSteadyA = (UpBase=2.25,LateralBase=0.95,UpModifier=0.15,LateralModifier=0.25,UpMax=32,LateralMax=4.3,DirectionChange=7)
	
	Spread_WhenFallingA = (param1=-0.006,param2=0.456)
	Spread_WhenMovingA = (param1=-0.005,param2=0.1824)
	Spread_WhenDuckingA = (param1=-0.00072,param2=0.02052)
	Spread_WhenSteadyA = (param1=-0.0018,param2=0.0342)
	
	AccuracyDivisorA  =  2100
	AccuracyOffsetA  =  0.279
	MaxInaccuracyA  =  1.32
	
	DirectionHold = 1
	FireIntervalMultiplierA = 1.5

	KickbackLimiter(0) = (Min=0.6,Max=1.1)
	KickbackLimiter(1) = (Min=0.7,Max=1)
	KickbackLimiter(2) = (Min=0.85,Max=1.1)
	
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
