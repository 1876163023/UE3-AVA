class avaWeap_MP7 extends avaWeap_BaseSMG;

defaultproperties
{
	BulletType		= class'avaBullet_46MM'

	BaseSpeed		= 270	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.25	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	HitDamage		= 28
	HitDamageS		= 28
	FireInterval(0)		= 0.075

	ClipCnt			= 20
	AmmoCount		= 20
	MaxAmmoCount		= 60

	Penetration		= 2
	PenetrationS		= 1

	RangeModifier		= 0.66
	RangeModifierS		= 0.62

	SpreadDecayTime = 0.5

	Kickback_WhenMoving = (UpBase=0.96,LateralBase=0.9,UpModifier=0.03,LateralModifier=0.036,UpMax=10.2,LateralMax=9,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=1.2,LateralBase=1.125,UpModifier=0.0375,LateralModifier=0.045,UpMax=12.75,LateralMax=11.25,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=0.64,LateralBase=0.6,UpModifier=0.02,LateralModifier=0.024,UpMax=6.8,LateralMax=6,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=0.8,LateralBase=0.75,UpModifier=0.025,LateralModifier=0.03,UpMax=8.5,LateralMax=7.5,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.024656775,param2=0.02630056)
	Spread_WhenMoving = (param1=0.0227601,param2=0.0202312)
	Spread_WhenDucking = (param1=0.013276725,param2=0.01416184)
	Spread_WhenSteady = (param1=0.01896675,param2=0.0202312)
	
	AccuracyDivisor  =  500
	AccuracyOffset  =  0.4
	MaxInaccuracy  =  1.5
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 2



	Kickback_WhenMovingA = (UpBase=1.248,LateralBase=1.17,UpModifier=0.039,LateralModifier=0.0468,UpMax=13.26,LateralMax=11.7,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=1.56,LateralBase=1.4625,UpModifier=0.04875,LateralModifier=0.0585,UpMax=16.575,LateralMax=14.625,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=0.64,LateralBase=0.6,UpModifier=0.02,LateralModifier=0.024,UpMax=6.8,LateralMax=6,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=0.8,LateralBase=0.75,UpModifier=0.025,LateralModifier=0.03,UpMax=8.5,LateralMax=7.5,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.024656775,param2=0.05260112)
	Spread_WhenMovingA = (param1=0.0227601,param2=0.0404624)
	Spread_WhenDuckingA = (param1=0.0119490525,param2=0.012745656)
	Spread_WhenSteadyA = (param1=0.017070075,param2=0.01820808)
	
	AccuracyDivisorA  =  1500
	AccuracyOffsetA  =  0.36
	MaxInaccuracyA  =  1.8
	
	DirectionHold = 2
	FireIntervalMultiplierA = 1.5


	KickbackLimiter(0) = (Min=0.7,Max=1)
	KickbackLimiter(1) = (Min=0.8,Max=1.3)


	FireAnimInfos(0)		=	(AnimName=Fire2,FirstShotRate=1,OtherShotRate=0.4)
	FireAnimInfos(1)		=	(AnimName=Fire1,FirstShotRate=0,OtherShotRate=0.6)
	WeaponFireAnim(0)		=	Fire2
	WeaponFireAnim(1)		=	Fire1

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime		= 0.833
	PutDownTime		= 0.233
	ReloadTime		= 2.33
	SightInfos(0)		= (FOV=90,ChangeTime=0.1)

	AttachmentClass		= class'avaAttachment_P90'
	BaseSkelMeshName	= "WP_smg_bizon.MS_bizon"
	BaseAnimSetName		= "WP_smg_bizon.Ani_Bizon"

	WeaponFireSnd			=	SoundCue'avaWeaponSounds.SMG_MP5K.SMG_MP5K_Fire'	
//	WeaponSilencerFireSnd	= SoundCue'WeaponSounds.usp_cue''
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	bEnableSilencer		= false

	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'
}