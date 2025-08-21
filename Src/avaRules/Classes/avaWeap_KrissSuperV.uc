class avaWeap_KrissSuperV extends avaWeap_BaseSMG;

defaultproperties
{
	BulletType		= class'avaBullet_45ACP'

	BaseSpeed		= 255	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.25	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	HitDamage		= 32
	HitDamageS		= 32

	FireInterval(0)		= 0.075

	ClipCnt			= 28
	AmmoCount		= 28
	MaxAmmoCount		= 84

	Penetration		= 2
	PenetrationS		= 1

	RangeModifier		= 0.68
	RangeModifierS		= 0.63

	SpreadDecayTime = 1.1

	Kickback_WhenMoving = (UpBase=1.2,LateralBase=0.96,UpModifier=0.018,LateralModifier=0.0744,UpMax=10.2,LateralMax=9.6,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=1.2,LateralBase=1.2,UpModifier=0.0225,LateralModifier=0.093,UpMax=12.75,LateralMax=12,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=0.64,LateralBase=0.64,UpModifier=0.012,LateralModifier=0.0496,UpMax=6.8,LateralMax=6.4,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=0.8,LateralBase=0.8,UpModifier=0.015,LateralModifier=0.062,UpMax=8.5,LateralMax=8,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.0286,param2=0.00665)
	Spread_WhenMoving = (param1=0.0264,param2=0.00665)
	Spread_WhenDucking = (param1=0.0176,param2=0.00133)
	Spread_WhenSteady = (param1=0.022,param2=0.00133)
	
	AccuracyDivisor  =  400
	AccuracyOffset  =  0.3
	MaxInaccuracy  =  2.5
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 2.5



	Kickback_WhenMovingA = (UpBase=1.56,LateralBase=1.248,UpModifier=0.0234,LateralModifier=0.09672,UpMax=13.26,LateralMax=12.48,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=1.56,LateralBase=1.56,UpModifier=0.02925,LateralModifier=0.1209,UpMax=16.575,LateralMax=15.6,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=0.64,LateralBase=0.64,UpModifier=0.012,LateralModifier=0.0496,UpMax=6.8,LateralMax=6.4,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=0.8,LateralBase=0.8,UpModifier=0.015,LateralModifier=0.062,UpMax=8.5,LateralMax=8,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.0286,param2=0.0133)
	Spread_WhenMovingA = (param1=0.0264,param2=0.0133)
	Spread_WhenDuckingA = (param1=0.01584,param2=0.001197)
	Spread_WhenSteadyA = (param1=0.0198,param2=0.001197)
	
	AccuracyDivisorA  =  1200
	AccuracyOffsetA  =  0.27
	MaxInaccuracyA  =  3
	
	DirectionHold = 5
	FireIntervalMultiplierA = 1.5

	KickbackLimiter(0) = (Min=0.8,Max=1.25)

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