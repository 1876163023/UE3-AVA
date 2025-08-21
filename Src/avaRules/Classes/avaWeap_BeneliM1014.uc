class avaWeap_BeneliM1014 extends avaWeap_BaseShotGun;

defaultproperties
{
	BulletType=class'avaBullet_ShotgunShell'

	BaseSpeed		= 235	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	HitDamage=37

	FireInterval(0)=0.55

	ClipCnt=8
	AmmoCount=8
	MaxAmmoCount=32

	Penetration = 1
	RangeModifier=0.42

	SpreadDecayTime = 0.35

	Kickback_WhenMoving = (UpBase=9.84,LateralBase=0.6,UpModifier=0.6,LateralModifier=1.8,UpMax=26.4,LateralMax=7.2,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=12.3,LateralBase=0.75,UpModifier=0.75,LateralModifier=2.25,UpMax=33,LateralMax=9,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=6.56,LateralBase=0.4,UpModifier=0.4,LateralModifier=1.2,UpMax=17.6,LateralMax=4.8,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=8.2,LateralBase=0.5,UpModifier=0.5,LateralModifier=1.5,UpMax=22,LateralMax=6,DirectionChange=3)
	
	Spread_WhenFalling = (param1=-0.0305,param2=0.17)
	Spread_WhenMoving = (param1=-0.0305,param2=0.17)
	Spread_WhenDucking = (param1=-0.0305,param2=0.17)
	Spread_WhenSteady = (param1=-0.0305,param2=0.17)
	
	AccuracyDivisor  =  200
	AccuracyOffset  =  0.6
	MaxInaccuracy  =  5
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 10


	Kickback_WhenMovingA = (UpBase=9.84,LateralBase=1.44,UpModifier=0.6,LateralModifier=1.8,UpMax=26.4,LateralMax=7.2,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=12.3,LateralBase=1.8,UpModifier=0.75,LateralModifier=2.25,UpMax=33,LateralMax=9,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=6.56,LateralBase=0.96,UpModifier=0.4,LateralModifier=1.2,UpMax=17.6,LateralMax=4.8,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=8.2,LateralBase=1.2,UpModifier=0.5,LateralModifier=1.5,UpMax=22,LateralMax=6,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=-0.0555,param2=0.17)
	Spread_WhenMovingA = (param1=-0.0555,param2=0.17)
	Spread_WhenDuckingA = (param1=-0.0555,param2=0.17)
	Spread_WhenSteadyA = (param1=-0.0555,param2=0.17)
	
	AccuracyDivisorA  =  200
	AccuracyOffsetA  =  0.6
	MaxInaccuracyA  =  5
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1


	KickbackLimiter(0) = (Min=0.7,Max=1)

	bAutofire	= true

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.40
	PutDownTime			=	0.333
	ReloadTime			=	3

	SightInfos(0) = (FOV=90,ChangeTime=0.3)
	SightInfos(1) = (FOV=70,ChangeTime=0.15)

	bHideWeaponInSightMode = false
	bHideCursorInSightMode = false
	ScopeMeshName          = ""
	SightInAnim            = ""
	SightOutAnim           = ""
	bReleaseZoomAfterFire = false

	BaseSkelMeshName	=	"Wp_Rif_G36.Wp_Rif_G36_Scope.MS_G36_Scope"
	BaseAnimSetName		=	"Wp_Rif_G36.Ani_G36"

	AttachmentClass=class'avaAttachment_SPAS12'
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'
	
	bReloadClip = TRUE
	NumFiresPerShot = 8
	WeaponType	= WEAPON_SHOTGUN
	InventoryGroup	=	1
}
