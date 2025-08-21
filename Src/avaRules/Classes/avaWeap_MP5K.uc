class avaWeap_MP5K extends avaWeap_BaseSMG;

defaultproperties
{
	BulletType				= class'avaBullet_9MM'

	BaseSpeed		= 265	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	HitDamage		= 31
	HitDamageS		= 31
	FireInterval(0)		= 0.0825

	ClipCnt		= 30
	AmmoCount		= 30
	MaxAmmoCount		= 90

	Penetration		= 2
	PenetrationS		= 1

	RangeModifier		= 0.68
	RangeModifierS		= 0.64

	SpreadDecayTime = 0.75

	Kickback_WhenMoving = (UpBase=1.2,LateralBase=0.72,UpModifier=0.018,LateralModifier=0.0624,UpMax=9.84,LateralMax=4.56,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=1.2,LateralBase=0.9,UpModifier=0.0225,LateralModifier=0.078,UpMax=12.3,LateralMax=5.7,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=0.64,LateralBase=0.48,UpModifier=0.012,LateralModifier=0.0416,UpMax=6.56,LateralMax=3.04,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=0.8,LateralBase=0.6,UpModifier=0.015,LateralModifier=0.052,UpMax=8.2,LateralMax=3.8,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.02587,param2=0.00665)
	Spread_WhenMoving = (param1=0.02388,param2=0.00665)
	Spread_WhenDucking = (param1=0.01592,param2=0.00133)
	Spread_WhenSteady = (param1=0.0199,param2=0.00133)
	
	AccuracyDivisor  =  200
	AccuracyOffset  =  0.35
	MaxInaccuracy  =  2.5
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 2


	Kickback_WhenMovingA = (UpBase=1.56,LateralBase=0.936,UpModifier=0.0234,LateralModifier=0.08112,UpMax=12.792,LateralMax=5.928,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=1.56,LateralBase=1.17,UpModifier=0.02925,LateralModifier=0.1014,UpMax=15.99,LateralMax=7.41,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=0.64,LateralBase=0.48,UpModifier=0.012,LateralModifier=0.0416,UpMax=6.56,LateralMax=3.04,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=0.8,LateralBase=0.6,UpModifier=0.015,LateralModifier=0.052,UpMax=8.2,LateralMax=3.8,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.02587,param2=0.0133)
	Spread_WhenMovingA = (param1=0.02388,param2=0.0133)
	Spread_WhenDuckingA = (param1=0.014328,param2=0.001197)
	Spread_WhenSteadyA = (param1=0.01791,param2=0.001197)
	
	AccuracyDivisorA  =  600
	AccuracyOffsetA  =  0.315
	MaxInaccuracyA  =  3
	
	DirectionHold = 4
	FireIntervalMultiplierA = 1.5


	KickbackLimiter(0) = (Min=0.5,Max=0.7)
	KickbackLimiter(1) = (Min=0.5,Max=0.8)
	KickbackLimiter(2) = (Min=0.65,Max=1)


	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime		= 0.833
	PutDownTime		= 0.233
	ReloadTime		= 2.633
	SightInfos(0)			= (FOV=90,ChangeTime=0.1)

	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=1.0,OtherShotRate=0.0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.0,OtherShotRate=1.0)
	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4

	AttachmentClass			= class'avaAttachment_MP5K'
	BaseSkelMeshName		= "Wp_Smg_MP5K.Wp_Smg_MP5K_Basic.MS_MP5K"
	BaseAnimSetName			= "Wp_Smg_MP5K.Ani_MP5K_PDW_1P"

	WeaponFireSnd			=	SoundCue'avaWeaponSounds.SMG_MP5K.SMG_MP5K_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	SilencerMeshName		= "Wp_Silencer.MS_SMG_MP5K_Silencer"
	SilencerBoneName		= MP5_Bone11
	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'
}
