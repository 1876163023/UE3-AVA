class avaWeap_PP2000 extends avaWeap_BaseSMG;

defaultproperties
{
	BulletType		= class'avaBullet_9MM'

	BaseSpeed		= 270	// 기본속도
	AimSpeedPct		= 0.85	// 조준시 보정치
	WalkSpeedPct		= 0.45	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.25	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1.1	// 앉아서 스프린트시 보정치

	HitDamage		= 32
	HitDamageS		= 32

	FireInterval(0)		= 0.095

	ClipCnt			= 44
	AmmoCount		= 44
	MaxAmmoCount		= 88

	Penetration		= 2
	PenetrationS		= 1

	RangeModifier		= 0.68
	RangeModifierS		= 0.63

	SpreadDecayTime = 0.4

	Kickback_WhenMoving = (UpBase=1.2,LateralBase=0.96,UpModifier=0.108,LateralModifier=0.132,UpMax=42,LateralMax=18,DirectionChange=6)
	Kickback_WhenFalling = (UpBase=1.5,LateralBase=1.2,UpModifier=0.135,LateralModifier=0.165,UpMax=52.5,LateralMax=22.5,DirectionChange=6)
	Kickback_WhenDucking = (UpBase=0.8,LateralBase=0.64,UpModifier=0.072,LateralModifier=0.088,UpMax=28,LateralMax=12,DirectionChange=6)
	Kickback_WhenSteady = (UpBase=1,LateralBase=0.8,UpModifier=0.09,LateralModifier=0.11,UpMax=35,LateralMax=15,DirectionChange=6)
	
	Spread_WhenFalling = (param1=0.024,param2=0.026)
	Spread_WhenMoving = (param1=0.024,param2=0.024)
	Spread_WhenDucking = (param1=0.017,param2=0.017)
	Spread_WhenSteady = (param1=0.02,param2=0.02)
	
	AccuracyDivisor  =  800
	AccuracyOffset  =  0.08
	MaxInaccuracy  =  2.2
	
	Kickback_UpLimit = 3
	Kickback_LateralLimit = 2



	Kickback_WhenMovingA = (UpBase=1.56,LateralBase=1.248,UpModifier=0.1404,LateralModifier=0.1716,UpMax=54.6,LateralMax=23.4,DirectionChange=6)
	Kickback_WhenFallingA = (UpBase=1.95,LateralBase=1.56,UpModifier=0.1755,LateralModifier=0.2145,UpMax=68.25,LateralMax=29.25,DirectionChange=6)
	Kickback_WhenDuckingA = (UpBase=0.8,LateralBase=0.64,UpModifier=0.072,LateralModifier=0.088,UpMax=28,LateralMax=12,DirectionChange=6)
	Kickback_WhenSteadyA = (UpBase=1,LateralBase=0.8,UpModifier=0.09,LateralModifier=0.11,UpMax=35,LateralMax=15,DirectionChange=6)
	
	Spread_WhenFallingA = (param1=0.024,param2=0.052)
	Spread_WhenMovingA = (param1=0.024,param2=0.048)
	Spread_WhenDuckingA = (param1=0.0153,param2=0.0153)
	Spread_WhenSteadyA = (param1=0.018,param2=0.018)
	
	AccuracyDivisorA  =  2400
	AccuracyOffsetA  =  0.072
	MaxInaccuracyA  =  2.64
	
	DirectionHold = 1
	FireIntervalMultiplierA = 1.5



	KickbackLimiter(0) = (Min=0.8,Max=1)
	KickbackLimiter(1) = (Min=0.8,Max=1.1)



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
	ReloadTime		= 2.33
	SightInfos(0)		= (FOV=90,ChangeTime=0.1)

	AttachmentClass		= class'avaAttachment_PP2000'
	BaseSkelMeshName	= "Wp_Smg_KBP2000.MS_Smg_KBP2000"
	BaseAnimSetName		= "Wp_Smg_KBP2000.Ani_KBP2000_1P"

	WeaponFireSnd			=	SoundCue'avaWeaponSounds.SMG_PP2000.SMG_PP2000_Fire'	
//	WeaponSilencerFireSnd	= SoundCue'WeaponSounds.usp_cue''
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	bEnableSilencer		= TRUE
	SilencerMeshName		= "Wp_Silencer.MS_SMG_MP5K_Silencer"
	SilencerBoneName		= Bone06

	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'
}