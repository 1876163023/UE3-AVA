class avaWeap_G3A3 extends avaWeap_BaseRifle;

defaultproperties
{

	BulletType			=	class'avaBullet_762NATO'	
	BaseSpeed		= 250	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.32	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage			=	32
	FireInterval(0)			=	0.1045
	
	ClipCnt			=	25
	AmmoCount			=	25
	MaxAmmoCount			=	100

	Penetration			=	2
	RangeModifier			=	0.82

	SpreadDecayTime = 0.7

	Kickback_WhenMoving = (UpBase=1.98,LateralBase=0.825,UpModifier=0.198,LateralModifier=0.165,UpMax=27.5,LateralMax=4.95,DirectionChange=8)
	Kickback_WhenFalling = (UpBase=3.6,LateralBase=1.5,UpModifier=0.36,LateralModifier=0.3,UpMax=50,LateralMax=9,DirectionChange=8)
	Kickback_WhenDucking = (UpBase=1.53,LateralBase=0.525,UpModifier=0.126,LateralModifier=0.105,UpMax=21.25,LateralMax=3.825,DirectionChange=8)
	Kickback_WhenSteady = (UpBase=1.8,LateralBase=0.75,UpModifier=0.18,LateralModifier=0.15,UpMax=25,LateralMax=4.5,DirectionChange=8)
	
	Spread_WhenFalling = (param1=-0.015,param2=0.27)
	Spread_WhenMoving = (param1=-0.005,param2=0.117)
	Spread_WhenDucking = (param1=-0.0005,param2=0.0315)
	Spread_WhenSteady = (param1=-0.0025,param2=0.045)
	
	AccuracyDivisor  =  1000
	AccuracyOffset  =  0.3
	MaxInaccuracy  =  1
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 2.9


	Kickback_WhenMovingA = (UpBase=2.57,LateralBase=1.0725,UpModifier=0.2574,LateralModifier=0.2145,UpMax=35.8,LateralMax=6.4,DirectionChange=8)
	Kickback_WhenFallingA = (UpBase=4.68,LateralBase=1.95,UpModifier=0.468,LateralModifier=0.39,UpMax=65,LateralMax=11.7,DirectionChange=8)
	Kickback_WhenDuckingA = (UpBase=1.53,LateralBase=0.525,UpModifier=0.126,LateralModifier=0.105,UpMax=21.25,LateralMax=3.825,DirectionChange=8)
	Kickback_WhenSteadyA = (UpBase=1.8,LateralBase=0.75,UpModifier=0.18,LateralModifier=0.15,UpMax=25,LateralMax=4.5,DirectionChange=8)
	
	Spread_WhenFallingA = (param1=-0.015,param2=0.54)
	Spread_WhenMovingA = (param1=-0.005,param2=0.234)
	Spread_WhenDuckingA = (param1=-0.00045,param2=0.02835)
	Spread_WhenSteadyA = (param1=-0.00225,param2=0.0405)
	
	AccuracyDivisorA  =  3000
	AccuracyOffsetA  =  0.27
	MaxInaccuracyA  =  1.2
	
	DirectionHold = 1
	FireIntervalMultiplierA = 1.5

	KickbackLimiter(0) = (Min=0.8,Max=1.1)
	KickbackLimiter(1) = (Min=0.8,Max=1.2)
	KickbackLimiter(2) = (Min=0.7,Max=1.2)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.167
	PutDownTime			=	0.333
	ReloadTime			=	2.667
	SightInfos(0)			=	(FOV=90,ChangeTime=0.2)
	SightInfos(1) = (FOV=75,ChangeTime=0.15)

	bHideWeaponInSightMode = false
	bHideCursorInSightMode = false
	ScopeMeshName          = ""
	SightInAnim            = ""
	SightOutAnim           = ""
	bReleaseZoomAfterFire = false

	AttachmentClass			=	class'avaAttachment_G3A3'

	BaseSkelMeshName		=	"Wp_Rif_G3A3.MS_G3A3"
	BaseAnimSetName			=	"Wp_Rif_G3A3.Ani_G3A3"

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
	HudMaterial			=	Texture2D'avaDotSightUI.Textures.M4_dotsight'
	WeaponFireSnd			=	SoundCue'avaWeaponSounds.AR_G3A3.AR_G3A3_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
