class avaWeap_MiniUzi extends avaWeap_BaseSMG;
//모델링이 UZI라 UZI로 명명하지만 스크립트는 그대로 유지합니다.

defaultproperties
{

	BulletType		= class'avaBullet_9MM'

	BaseSpeed		= 260	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	HitDamage		= 31
	HitDamageS		= 31
	FireInterval(0)		= 0.0748

	ClipCnt		= 25
	AmmoCount		= 25
	MaxAmmoCount		= 75

	Penetration		= 2
	PenetrationS		= 1

	RangeModifier		= 0.68
	RangeModifierS		= 0.6

	SpreadDecayTime = 0.7

	Kickback_WhenMoving = (UpBase=1.44,LateralBase=0.72,UpModifier=0.108,LateralModifier=0.036,UpMax=24,LateralMax=7.2,DirectionChange=4)
	Kickback_WhenFalling = (UpBase=1.8,LateralBase=0.9,UpModifier=0.135,LateralModifier=0.045,UpMax=30,LateralMax=9,DirectionChange=4)
	Kickback_WhenDucking = (UpBase=1.2,LateralBase=0.6,UpModifier=0.09,LateralModifier=0.03,UpMax=20,LateralMax=6,DirectionChange=2)
	Kickback_WhenSteady = (UpBase=1.2,LateralBase=0.6,UpModifier=0.09,LateralModifier=0.03,UpMax=20,LateralMax=6,DirectionChange=2)
	
	Spread_WhenFalling = (param1=0.0208,param2=0.036)
	Spread_WhenMoving = (param1=0.0192,param2=0.03)
	Spread_WhenDucking = (param1=0.016,param2=0.018)
	Spread_WhenSteady = (param1=0.016,param2=0.02)
	
	AccuracyDivisor  =  200
	AccuracyOffset  =  0.32
	MaxInaccuracy  =  2
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 4.5


	Kickback_WhenMovingA = (UpBase=1.872,LateralBase=0.936,UpModifier=0.1404,LateralModifier=0.0468,UpMax=31.2,LateralMax=9.36,DirectionChange=4)
	Kickback_WhenFallingA = (UpBase=2.34,LateralBase=1.17,UpModifier=0.1755,LateralModifier=0.0585,UpMax=39,LateralMax=11.7,DirectionChange=4)
	Kickback_WhenDuckingA = (UpBase=1.2,LateralBase=0.6,UpModifier=0.09,LateralModifier=0.03,UpMax=20,LateralMax=6,DirectionChange=2)
	Kickback_WhenSteadyA = (UpBase=1.2,LateralBase=0.6,UpModifier=0.09,LateralModifier=0.03,UpMax=20,LateralMax=6,DirectionChange=2)
	
	Spread_WhenFallingA = (param1=0.0208,param2=0.072)
	Spread_WhenMovingA = (param1=0.0192,param2=0.06)
	Spread_WhenDuckingA = (param1=0.0144,param2=0.0162)
	Spread_WhenSteadyA = (param1=0.0144,param2=0.018)
	
	AccuracyDivisorA  =  600
	AccuracyOffsetA  =  0.288
	MaxInaccuracyA  =  2.4
	
	DirectionHold = 2
	FireIntervalMultiplierA = 1.5

	KickbackLimiter(0) = (Min=0.8,Max=1)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=1.0,OtherShotRate=0.0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.0,OtherShotRate=1.0)
	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4


	EquipTime		= 0.833
	PutDownTime		= 0.2333
	ReloadTime		= 2.5
	SightInfos(0)		= (FOV=90,ChangeTime=0.1)

	AttachmentClass		= class'avaAttachment_MiniUzi'
	BaseSkelMeshName	= "WP_smg_miniUZI.WP_Smg_miniUZI_Basic.MS_miniUZI_Basic"
	BaseAnimSetName		= "WP_smg_miniUZI.Ani_MiniUzi_1P"

	WeaponFireSnd		= SoundCue'avaWeaponSounds.SMG_MiniUzi.SMG_MiniUzi_Fire'	
	PickupSound		= SoundCue'avaItemSounds.Item_Get_Cue'
	bEnableSilencer		= True

	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'

//	WeaponSilencerFireSnd	= SoundCue'WeaponSounds.usp_cue'
	SilencerMeshName		= "Wp_Silencer.MS_SMG_Uzi_Silencer"
	SilencerBoneName		= Uzi_Bone06

	DefaultModifiers(0)			=	class'avaRules.avaMod_MiniUzi_T_Catridge'
}
