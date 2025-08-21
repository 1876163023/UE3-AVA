class avaWeap_Shipka extends avaWeap_BaseSMG;

defaultproperties
{
	BulletType		= class'avaBullet_9MM'

	BaseSpeed		= 270	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	HitDamage		= 30
	HitDamageS		= 30
	FireInterval(0)		= 0.088

	ClipCnt		= 32
	AmmoCount		= 32
	MaxAmmoCount		= 96

	Penetration		= 2
	PenetrationS		= 1

	RangeModifier		= 0.7
	RangeModifierS		= 0.65

	SpreadDecayTime = 0.5

	Kickback_WhenMoving = (UpBase=0.96,LateralBase=0.96,UpModifier=0.084,LateralModifier=0,UpMax=9.6,LateralMax=7.2,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=1.2,LateralBase=1.2,UpModifier=0.105,LateralModifier=0,UpMax=12,LateralMax=9,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=0.64,LateralBase=0.64,UpModifier=0.056,LateralModifier=0,UpMax=6.4,LateralMax=4.8,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=0.8,LateralBase=0.8,UpModifier=0.07,LateralModifier=0,UpMax=8,LateralMax=6,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.01497375,param2=0.02662)
	Spread_WhenMoving = (param1=0.01497375,param2=0.02662)
	Spread_WhenDucking = (param1=0.00952875,param2=0.01694)
	Spread_WhenSteady = (param1=0.0136125,param2=0.0242)
	
	AccuracyDivisor  =  1000
	AccuracyOffset  =  0.1
	MaxInaccuracy  =  0.8
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 3.8

	Kickback_WhenMovingA = (UpBase=1.248,LateralBase=1.248,UpModifier=0.1092,LateralModifier=0,UpMax=12.48,LateralMax=9.36,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=1.56,LateralBase=1.56,UpModifier=0.1365,LateralModifier=0,UpMax=15.6,LateralMax=11.7,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=0.64,LateralBase=0.64,UpModifier=0.056,LateralModifier=0,UpMax=6.4,LateralMax=4.8,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=0.8,LateralBase=0.8,UpModifier=0.07,LateralModifier=0,UpMax=8,LateralMax=6,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.01497375,param2=0.05324)
	Spread_WhenMovingA = (param1=0.01497375,param2=0.05324)
	Spread_WhenDuckingA = (param1=0.008575875,param2=0.015246)
	Spread_WhenSteadyA = (param1=0.01225125,param2=0.02178)
	
	AccuracyDivisorA  =  3000
	AccuracyOffsetA  =  0.09
	MaxInaccuracyA  =  0.96
	
	DirectionHold = 3
	FireIntervalMultiplierA = 1.5

	KickbackLimiter(0) = (Min=0.3,Max=1)
	KickbackLimiter(1) = (Min=0.6,Max=1)
	KickbackLimiter(2) = (Min=0.6,Max=1)
	KickbackLimiter(3) = (Min=0.4,Max=1)
	KickbackLimiter(4) = (Min=0.8,Max=1)
	KickbackLimiter(5) = (Min=1,Max=1)


	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime		= 0.833
	PutDownTime		= 0.2333
	ReloadTime		= 2.50
	SightInfos(0)		= (FOV=90,ChangeTime=0.1)

	AttachmentClass		= class'avaAttachment_Shipka'
	BaseSkelMeshName	= "WP_smg_bizon.MS_bizon"
	BaseAnimSetName		= "WP_smg_bizon.Ani_Bizon"

	WeaponFireSnd			=	SoundCue'avaWeaponSounds.SMG_MP5K.SMG_MP5K_Fire'	
//	WeaponSilencerFireSnd	= SoundCue'WeaponSounds.usp_cue''
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	bEnableSilencer		= false

	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'
}
