class avaWeap_L96A1 extends avaWeap_BaseSniperRifle;

defaultproperties
{
	BulletType=class'avaBullet_762NATO'

	BaseSpeed		= 238	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치


	HitDamage=110

	FireInterval(0)=1.20
	FireInterval(1)=1.20
	FireInterval(2)=1.20

	ClipCnt=5
	AmmoCount=5
	MaxAmmoCount=30

	Penetration = 2

	RangeModifier=0.96

	SpreadDecayTime = 1

	Kickback_WhenMoving = (UpBase=15,LateralBase=0.525,UpModifier=0.27,LateralModifier=0.15,UpMax=11.25,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=15,LateralBase=0.525,UpModifier=0.27,LateralModifier=0.15,UpMax=11.25,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=8,LateralBase=0.28,UpModifier=0.144,LateralModifier=0.08,UpMax=6,LateralMax=4,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=10,LateralBase=0.35,UpModifier=0.18,LateralModifier=0.1,UpMax=7.5,LateralMax=5,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.225,param2=0.045)
	Spread_WhenMoving = (param1=0.03,param2=0.015)
	Spread_WhenDucking = (param1=0.01425,param2=0.0024)
	Spread_WhenSteady = (param1=0.015,param2=0.003)
	
	AccuracyDivisor  =  500
	AccuracyOffset  =  0
	MaxInaccuracy  =  2
	
	Kickback_UpLimit = 15
	Kickback_LateralLimit = 5



	Kickback_WhenMovingA = (UpBase=15,LateralBase=0.525,UpModifier=0.27,LateralModifier=0.15,UpMax=11.25,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=15,LateralBase=0.525,UpModifier=0.27,LateralModifier=0.15,UpMax=11.25,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=8,LateralBase=0.28,UpModifier=0.144,LateralModifier=0.08,UpMax=6,LateralMax=4,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=10,LateralBase=0.35,UpModifier=0.18,LateralModifier=0.1,UpMax=7.5,LateralMax=5,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.15,param2=0.045)
	Spread_WhenMovingA = (param1=0.02,param2=0.015)
	Spread_WhenDuckingA = (param1=0.0095,param2=0.0024)
	Spread_WhenSteadyA = (param1=0.01,param2=0.003)
	
	AccuracyDivisorA  =  500
	AccuracyOffsetA  =  0
	MaxInaccuracyA  =  1
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1



	KickbackLimiter(0) = (Min=0.8,Max=1.2)

	bEjectBulletWhenFire = False

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.6
	BobDamping		=	0.7
	BobDampingInDash	=	0.5

	EquipTime			=0.767
	PutDownTime			=0.2333
	ReloadTime			=2.133

	LastFireAnim		= Fire_Last
	LastFireInterval =	0.3667

	SightInfos(0) = (FOV=90,ChangeTime=0.12)

	bHideWeaponInSightMode = true
	bHideCursorInSightMode = true
	bReleaseZoomAfterFire  = true
	fReleaseZoomAfterFireInterval = 0.08
	bRecoverZoomAfterFire  = true
	RecoverZoomTime        = 0.12

	AttachmentClass=class'avaAttachment_M24'
	BaseSkelMeshName	=	"Wp_sn_M24_Scope.MS_M24_Scope"
	BaseAnimSetName		=	"Wp_sn_M24_Scope.Ani_M24"

	WeaponFireSnd			=	SoundCue'avaWeaponSounds.SR_M24.SR_M24_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	DefaultModifiers(0)			=	class'avaRules.avaMod_M24_M_BaseScope'
	InstantHitDamageTypes(0)	=	class'avaDmgType_Sniper'
	
	ScopeMeshName = "avaScopeUI.Distortion.MS_TPGSniper_Scope_Mesh"
}
