class avaWeap_UMP45 extends avaWeap_BaseSMG;

defaultproperties
{
	BulletType		= class'avaBullet_45ACP'

	BaseSpeed		= 265	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	HitDamage		= 34
	HitDamageS		= 34
	FireInterval(0)		= 0.095

	ClipCnt			= 25
	AmmoCount		= 25
	MaxAmmoCount		= 75

	Penetration		= 2
	PenetrationS		= 1

	RangeModifier		= 0.72
	RangeModifierS		= 0.65

	SpreadDecayTime = 1.2

	Kickback_WhenMoving = (UpBase=1.92,LateralBase=0.96,UpModifier=0.072,LateralModifier=0.024,UpMax=19.2,LateralMax=4.8,DirectionChange=5)
	Kickback_WhenFalling = (UpBase=2.4,LateralBase=1.2,UpModifier=0.09,LateralModifier=0.03,UpMax=24,LateralMax=6,DirectionChange=5)
	Kickback_WhenDucking = (UpBase=1.28,LateralBase=0.64,UpModifier=0.048,LateralModifier=0.016,UpMax=12.8,LateralMax=3.2,DirectionChange=5)
	Kickback_WhenSteady = (UpBase=1.6,LateralBase=0.8,UpModifier=0.06,LateralModifier=0.02,UpMax=16,LateralMax=4,DirectionChange=5)
	
	Spread_WhenFalling = (param1=0.02388,param2=0.028314)
	Spread_WhenMoving = (param1=0.02388,param2=0.026136)
	Spread_WhenDucking = (param1=0.016915,param2=0.018513)
	Spread_WhenSteady = (param1=0.0199,param2=0.02178)
	
	AccuracyDivisor  =  1100
	AccuracyOffset  =  0.15
	MaxInaccuracy  =  1.5
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 1.5


	Kickback_WhenMovingA = (UpBase=2.496,LateralBase=1.248,UpModifier=0.0936,LateralModifier=0.0312,UpMax=24.96,LateralMax=6.24,DirectionChange=5)
	Kickback_WhenFallingA = (UpBase=3.12,LateralBase=1.56,UpModifier=0.117,LateralModifier=0.039,UpMax=31.2,LateralMax=7.8,DirectionChange=5)
	Kickback_WhenDuckingA = (UpBase=1.28,LateralBase=0.64,UpModifier=0.048,LateralModifier=0.016,UpMax=12.8,LateralMax=3.2,DirectionChange=5)
	Kickback_WhenSteadyA = (UpBase=1.6,LateralBase=0.8,UpModifier=0.06,LateralModifier=0.02,UpMax=16,LateralMax=4,DirectionChange=5)
	
	Spread_WhenFallingA = (param1=0.02388,param2=0.056628)
	Spread_WhenMovingA = (param1=0.02388,param2=0.052272)
	Spread_WhenDuckingA = (param1=0.0152235,param2=0.0166617)
	Spread_WhenSteadyA = (param1=0.01791,param2=0.019602)
	
	AccuracyDivisorA  =  3300
	AccuracyOffsetA  =  0.135
	MaxInaccuracyA  =  1.8
	
	DirectionHold = 5
	FireIntervalMultiplierA = 1.5

	KickbackLimiter(0) = (Min=0.5,Max=1)
	KickbackLimiter(1) = (Min=0.8,Max=1)

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

	EquipTime		= 1.0333
	PutDownTime		= 0.2333
	ReloadTime		= 2.6333
	SightInfos(0)		= (FOV=90,ChangeTime=0.1)

	AttachmentClass		= class'avaAttachment_UMP45'
	BaseSkelMeshName	= "Wp_Smg_UMP45.MS_UMP45"
	BaseAnimSetName		= "Wp_Smg_UMP45.Ani_UMP45"

	WeaponFireSnd			=	SoundCue'avaWeaponSounds.SMG_UMP45.SMG_UMP45_Fire'	
//	WeaponSilencerFireSnd	= SoundCue'WeaponSounds.usp_cue''
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	bEnableSilencer		= True
	SilencerBoneName		= UMP45_Bone3
	SilencerMeshName		= "Wp_Silencer.MS_SMG_UMP45_Silencer"

	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'
	DefaultModifiers(0)			=	class'avaRules.avaMod_UMP45_M_BaseDot'
}
