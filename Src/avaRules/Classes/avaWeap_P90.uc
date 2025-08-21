class avaWeap_P90 extends avaWeap_BaseSMG;

defaultproperties
{
	BulletType		= class'avaBullet_57MM'

	BaseSpeed		= 260	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.25	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	HitDamage		= 30
	HitDamageS		= 30
	FireInterval(0)		= 0.084

	ClipCnt		= 50
	AmmoCount		= 50
	MaxAmmoCount		= 100

	Penetration		= 2
	PenetrationS		= 1

	RangeModifier		= 0.69
	RangeModifierS		= 0.64

	SpreadDecayTime = 0.5

	Kickback_WhenMoving = (UpBase=0.99,LateralBase=0.72,UpModifier=0.06,LateralModifier=0.1125,UpMax=60,LateralMax=6,DirectionChange=4)
	Kickback_WhenFalling = (UpBase=1.1,LateralBase=0.8,UpModifier=0.08,LateralModifier=0.15,UpMax=80,LateralMax=8,DirectionChange=4)
	Kickback_WhenDucking = (UpBase=0.44,LateralBase=0.32,UpModifier=0.032,LateralModifier=0.06,UpMax=48,LateralMax=3.2,DirectionChange=4)
	Kickback_WhenSteady = (UpBase=0.55,LateralBase=0.4,UpModifier=0.04,LateralModifier=0.075,UpMax=60,LateralMax=4,DirectionChange=4)
	
	Spread_WhenFalling = (param1=0.03,param2=0.084)
	Spread_WhenMoving = (param1=0.015,param2=0.084)
	Spread_WhenDucking = (param1=0.012,param2=0.0126)
	Spread_WhenSteady = (param1=0.015,param2=0.014)
	
	AccuracyDivisor  =  1500
	AccuracyOffset  =  0.08
	MaxInaccuracy  =  1.5
	
	Kickback_UpLimit = 7
	Kickback_LateralLimit = 2.5


	Kickback_WhenMovingA = (UpBase=1.287,LateralBase=0.936,UpModifier=0.078,LateralModifier=0.14625,UpMax=78,LateralMax=7.8,DirectionChange=4)
	Kickback_WhenFallingA = (UpBase=1.43,LateralBase=1.04,UpModifier=0.104,LateralModifier=0.195,UpMax=80,LateralMax=10.4,DirectionChange=4)
	Kickback_WhenDuckingA = (UpBase=0.44,LateralBase=0.32,UpModifier=0.032,LateralModifier=0.06,UpMax=48,LateralMax=3.2,DirectionChange=4)
	Kickback_WhenSteadyA = (UpBase=0.55,LateralBase=0.4,UpModifier=0.04,LateralModifier=0.075,UpMax=60,LateralMax=4,DirectionChange=4)
	
	Spread_WhenFallingA = (param1=0.03,param2=0.168)
	Spread_WhenMovingA = (param1=0.015,param2=0.168)
	Spread_WhenDuckingA = (param1=0.0108,param2=0.01134)
	Spread_WhenSteadyA = (param1=0.0135,param2=0.0126)
	
	AccuracyDivisorA  =  4500
	AccuracyOffsetA  =  0.072
	MaxInaccuracyA  =  1.8
	
	DirectionHold = 1
	FireIntervalMultiplierA = 1.3


	KickbackLimiter(0) = (Min=0.6,Max=1.0)
	KickbackLimiter(1) = (Min=0.8,Max=1.1)

	SightInfos(0) = (FOV=90,ChangeTime=0.2)
	SightInfos(1) = (FOV=75,ChangeTime=0.15)

	bHideWeaponInSightMode = true
	bHideCursorInSightMode = true
	bReleaseZoomAfterFire = false
	
	ScopeMeshName = "Wp_Scope.Com_Scope.MS_UIP90"

	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=1.0,OtherShotRate=0.0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.0,OtherShotRate=1.0)
	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime		= 0.833
	PutDownTime		= 0.233
	ReloadTime		= 2.9667

	AttachmentClass		= class'avaAttachment_P90'
	BaseSkelMeshName	= "Wp_Smg_P90.MS_Smg_P90"
	BaseAnimSetName		= "Wp_Smg_P90.Ani_P90_1P"

	WeaponFireSnd			=	SoundCue'avaWeaponSounds.SMG_P90.SMG_P90_fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	bEnableSilencer		= TRUE

	SilencerMeshName		= "Wp_Silencer.MS_SMG_MP5K_Silencer"
	SilencerBoneName		= Bone05


	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'
}