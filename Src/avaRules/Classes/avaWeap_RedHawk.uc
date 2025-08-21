class avaWeap_RedHawk extends avaWeap_BasePistol;

defaultproperties
{
	BulletType=class'avaBullet_44MAG'

	BaseSpeed		= 262	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage = 45

	FireInterval(0)=0.2035

	ClipCnt=6
	AmmoCount=6
	MaxAmmoCount=18

	Penetration = 2
	RangeModifier=0.72

	SpreadDecayTime = 0.5

	Kickback_WhenMoving = (UpBase=7.8,LateralBase=0.6,UpModifier=0.6,LateralModifier=0.36,UpMax=26.4,LateralMax=8.4,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=9.75,LateralBase=0.75,UpModifier=0.75,LateralModifier=0.45,UpMax=33,LateralMax=10.5,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=5.2,LateralBase=0.4,UpModifier=0.4,LateralModifier=0.24,UpMax=17.6,LateralMax=5.6,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=6.5,LateralBase=0.5,UpModifier=0.5,LateralModifier=0.3,UpMax=22,LateralMax=7,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.02,param2=0.06)
	Spread_WhenMoving = (param1=-0.0033,param2=0.055)
	Spread_WhenDucking = (param1=-0.0027,param2=0.045)
	Spread_WhenSteady = (param1=-0.003,param2=0.05)
	
	AccuracyDivisor  =  300
	AccuracyOffset  =  0.3
	MaxInaccuracy  =  0.8
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 10


	Kickback_WhenMovingA = (UpBase=7.8,LateralBase=0.6,UpModifier=0.6,LateralModifier=0.36,UpMax=26.4,LateralMax=8.4,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=9.75,LateralBase=0.75,UpModifier=0.75,LateralModifier=0.45,UpMax=33,LateralMax=10.5,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=5.2,LateralBase=0.4,UpModifier=0.4,LateralModifier=0.24,UpMax=17.6,LateralMax=5.6,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=6.5,LateralBase=0.5,UpModifier=0.5,LateralModifier=0.3,UpMax=22,LateralMax=7,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=-0.0036,param2=0.06)
	Spread_WhenMovingA = (param1=-0.0033,param2=0.055)
	Spread_WhenDuckingA = (param1=-0.0027,param2=0.045)
	Spread_WhenSteadyA = (param1=-0.003,param2=0.05)
	
	AccuracyDivisorA  =  800
	AccuracyOffsetA  =  0.3
	MaxInaccuracyA  =  0.8
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1.2

	KickbackLimiter(0) = (Min=0.7,Max=1.7)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.6
	BobDampingInDash	=	0.4

	EquipTime			=0.9
	PutDownTime			=0.3333
	ReloadTime			=1.833

	SightInfos(0) = (FOV=90,ChangeTime=0.2)
	SightInfos(1) = (FOV=70,ChangeTime=0.15)

	bHideWeaponInSightMode = false
	bHideCursorInSightMode = false
	ScopeMeshName          = ""
	SightInAnim            = ""
	SightOutAnim           = ""
	bReleaseZoomAfterFire = false

	AttachmentClass=class'avaAttachment_Annaconda'

	BaseSkelMeshName	=	"Wp_Pis_P226.Wp_Pis_P226_Basic.MS_P226_Basic_1p"
	BaseAnimSetName		=	"Wp_Pis_P226.Wp_Pis_P226_Basic.Ani_P226_Dup"
	WeaponFireSnd=SoundCue'avaWeaponSounds.AR_AK47.AR_AK47_Fire'
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	InstantHitDamageTypes(0)	=	class'avaDmgType_Pistol'
}
