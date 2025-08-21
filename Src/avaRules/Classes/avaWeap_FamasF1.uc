class avaWeap_FamasF1 extends avaWeap_BaseRifle;

defaultproperties
{

	BulletType				=	class'avaBullet_556NATO'	

	BaseSpeed		= 250	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.32	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage			=	32
	FireInterval(0)			=	0.075
	
	ClipCnt			=	25
	AmmoCount			=	25
	MaxAmmoCount			=	100

	Penetration			=	2
	RangeModifier			=	0.79

	SpreadDecayTime = 1.8

	Kickback_WhenMoving = (UpBase=1.62,LateralBase=0.75,UpModifier=0.144,LateralModifier=0.12,UpMax=60,LateralMax=9.6,DirectionChange=6)
	Kickback_WhenFalling = (UpBase=2.025,LateralBase=0.75,UpModifier=0.18,LateralModifier=0.15,UpMax=75,LateralMax=12,DirectionChange=4)
	Kickback_WhenDucking = (UpBase=1.35,LateralBase=0.5,UpModifier=0.12,LateralModifier=0.1,UpMax=50,LateralMax=8,DirectionChange=2)
	Kickback_WhenSteady = (UpBase=1.35,LateralBase=0.5,UpModifier=0.12,LateralModifier=0.1,UpMax=50,LateralMax=8,DirectionChange=2)
	
	Spread_WhenFalling = (param1=0.036,param2=0.112)
	Spread_WhenMoving = (param1=0.008,param2=0.042)
	Spread_WhenDucking = (param1=0.009,param2=0.0224)
	Spread_WhenSteady = (param1=0.009,param2=0.028)
	
	AccuracyDivisor  =  1800
	AccuracyOffset  =  0.31
	MaxInaccuracy  =  2.5
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 5

	Kickback_WhenMovingA = (UpBase=2.11,LateralBase=0.975,UpModifier=0.1872,LateralModifier=0.156,UpMax=78,LateralMax=12.5,DirectionChange=6)
	Kickback_WhenFallingA = (UpBase=2.6325,LateralBase=0.975,UpModifier=0.234,LateralModifier=0.195,UpMax=97.5,LateralMax=15.6,DirectionChange=4)
	Kickback_WhenDuckingA = (UpBase=0.81,LateralBase=0.4,UpModifier=0.096,LateralModifier=0.1,UpMax=50,LateralMax=8,DirectionChange=2)
	Kickback_WhenSteadyA = (UpBase=1.08,LateralBase=0.4,UpModifier=0.096,LateralModifier=0.1,UpMax=50,LateralMax=8,DirectionChange=2)
	
	Spread_WhenFallingA = (param1=0.036,param2=0.224)
	Spread_WhenMovingA = (param1=0.008,param2=0.084)
	Spread_WhenDuckingA = (param1=0.0081,param2=0.02016)
	Spread_WhenSteadyA = (param1=0.0081,param2=0.0252)
	
	AccuracyDivisorA  =  5760
	AccuracyOffsetA  =  0.279
	MaxInaccuracyA  =  3
	
	DirectionHold = 1
	FireIntervalMultiplierA = 1.3

	KickbackLimiter(0) = (Min=1,Max=1.5)
	KickbackLimiter(1) = (Min=1,Max=1.5)
	KickbackLimiter(2) = (Min=0.8,Max=1.3)


	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.3333
	PutDownTime			=	0.1
	ReloadTime			=	3.000
	SightInfos(0)			=	(FOV=90,ChangeTime=0.2)
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

	AttachmentClass			=	class'avaAttachment_FamasF1'

	BaseSkelMeshName		=	"Wp_Rif_Famas.Wp_Rif_Famas_F1.MS_Rif_Famas"
	BaseAnimSetName			=	"Wp_Rif_Famas.Ani_Famas_1P"

 	WeaponPutDownAnim		=	Down
	WeaponEquipAnim			=	BringUp
	WeaponReloadAnim		=	Reload

	WeaponIdleAnims(0)		=	Idle
	WeaponFireSnd=SoundCue'avaWeaponSounds.AR_FamasF1.AR_FamasF1_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
