class avaWeap_M16A2 extends avaWeap_BaseRifle;

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

	HitDamage			=	34
	FireInterval(0)			=	0.1012
	
	ClipCnt			=	30
	AmmoCount			=	30
	MaxAmmoCount			=	90

	Penetration			=	2
	RangeModifier			=	0.83

	SpreadDecayTime = 1.1

	Kickback_WhenMoving = (UpBase=1.265,LateralBase=0.7475,UpModifier=0.1045,LateralModifier=0.09775,UpMax=19.8,LateralMax=6.6,DirectionChange=7)
	Kickback_WhenFalling = (UpBase=2.3,LateralBase=1.3,UpModifier=0.19,LateralModifier=0.17,UpMax=36,LateralMax=12,DirectionChange=7)
	Kickback_WhenDucking = (UpBase=1.0925,LateralBase=0.6175,UpModifier=0.0665,LateralModifier=0.0595,UpMax=16.2,LateralMax=5.4,DirectionChange=7)
	Kickback_WhenSteady = (UpBase=1.15,LateralBase=0.65,UpModifier=0.095,LateralModifier=0.085,UpMax=18,LateralMax=6,DirectionChange=7)
	
	Spread_WhenFalling = (param1=0.04,param2=0.11)
	Spread_WhenMoving = (param1=0.0176,param2=0.0506)
	Spread_WhenDucking = (param1=0.0076,param2=0.0187)
	Spread_WhenSteady = (param1=0.008,param2=0.022)
	
	AccuracyDivisor  =  900
	AccuracyOffset  =  0.22
	MaxInaccuracy  =  3.2
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 4


	Kickback_WhenMovingA = (UpBase=1.64,LateralBase=0.97175,UpModifier=0.13585,LateralModifier=0.127075,UpMax=25.7,LateralMax=8.6,DirectionChange=7)
	Kickback_WhenFallingA = (UpBase=2.99,LateralBase=1.69,UpModifier=0.247,LateralModifier=0.221,UpMax=46.8,LateralMax=15.6,DirectionChange=7)
	Kickback_WhenDuckingA = (UpBase=1.0925,LateralBase=0.6175,UpModifier=0.0665,LateralModifier=0.0595,UpMax=16.2,LateralMax=5.4,DirectionChange=7)
	Kickback_WhenSteadyA = (UpBase=1.15,LateralBase=0.65,UpModifier=0.095,LateralModifier=0.085,UpMax=18,LateralMax=6,DirectionChange=7)
	
	Spread_WhenFallingA = (param1=0.04,param2=0.22)
	Spread_WhenMovingA = (param1=0.0176,param2=0.1012)
	Spread_WhenDuckingA = (param1=0.00684,param2=0.01683)
	Spread_WhenSteadyA = (param1=0.0072,param2=0.0198)
	
	AccuracyDivisorA  =  2700
	AccuracyOffsetA  =  0.198
	MaxInaccuracyA  =  3.84
	
	DirectionHold = 3
	FireIntervalMultiplierA = 1.5

	KickbackLimiter(0) = (Min=0.7,Max=1)
	KickbackLimiter(1) = (Min=0.7,Max=1.1)
	KickbackLimiter(2) = (Min=0.8,Max=1.15)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.167
	PutDownTime			=	0.233
	ReloadTime			=	2.8333
	SightInfos(0)			=	(FOV=90,ChangeTime=0.2)
	SightInfos(1) = (FOV=75,ChangeTime=0.15)

	bHideWeaponInSightMode = false
	bHideCursorInSightMode = false
	ScopeMeshName          = ""
	SightInAnim            = ""
	SightOutAnim           = ""
	bReleaseZoomAfterFire = false

	AttachmentClass			=	class'avaAttachment_M16A2'

	BaseSkelMeshName		=	"Wp_Rif_M16A2.MS_M16A2"
	BaseAnimSetName			=	"Wp_Rif_M16A2.Ani_M16A2"


	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=1.0,OtherShotRate=0.0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.0,OtherShotRate=1.0)
	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4


 	WeaponPutDownAnim		=	Down
	WeaponEquipAnim			=	BringUp
	WeaponReloadAnim		=	Reload

	WeaponIdleAnims(0)		=	Idle
	WeaponFireSnd			=	SoundCue'avaWeaponSounds.AR_M16A2.AR_M16A2_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
