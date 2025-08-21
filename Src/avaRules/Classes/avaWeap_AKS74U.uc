class avaWeap_AKS74U extends avaWeap_BaseSMG;

defaultproperties
{
	BulletType		= class'avaBullet_545M4'

	BaseSpeed		= 255	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	HitDamage		= 38
	HitDamageS		= 38

	FireInterval(0)		= 0.094

	ClipCnt			= 30
	AmmoCount		= 30
	MaxAmmoCount		= 90

	Penetration		= 2
	PenetrationS		= 1

	RangeModifier		= 0.72
	RangeModifierS		= 0.62

	SpreadDecayTime = 0.35

	Kickback_WhenMoving = (UpBase=2.16,LateralBase=0,UpModifier=0.108,LateralModifier=0.132,UpMax=30,LateralMax=6,DirectionChange=2)
	Kickback_WhenFalling = (UpBase=2.7,LateralBase=0,UpModifier=0.135,LateralModifier=0.165,UpMax=37.5,LateralMax=7.5,DirectionChange=2)
	Kickback_WhenDucking = (UpBase=1.26,LateralBase=0,UpModifier=0.072,LateralModifier=0.088,UpMax=20,LateralMax=4,DirectionChange=2)
	Kickback_WhenSteady = (UpBase=1.8,LateralBase=0,UpModifier=0.09,LateralModifier=0.11,UpMax=25,LateralMax=5,DirectionChange=2)
	
	Spread_WhenFalling = (param1=-0.006,param2=0.152)
	Spread_WhenMoving = (param1=-0.005,param2=0.095)
	Spread_WhenDucking = (param1=-0.0028,param2=0.0722)
	Spread_WhenSteady = (param1=-0.0008,param2=0.0722)
	
	AccuracyDivisor  =  700
	AccuracyOffset  =  0.31
	MaxInaccuracy  =  1.2
	
	Kickback_UpLimit = 15
	Kickback_LateralLimit = 3.5


	Kickback_WhenMovingA = (UpBase=2.808,LateralBase=0,UpModifier=0.1404,LateralModifier=0.1716,UpMax=39,LateralMax=7.8,DirectionChange=2)
	Kickback_WhenFallingA = (UpBase=3.51,LateralBase=0,UpModifier=0.1755,LateralModifier=0.2145,UpMax=48.75,LateralMax=9.75,DirectionChange=2)
	Kickback_WhenDuckingA = (UpBase=1.26,LateralBase=0,UpModifier=0.072,LateralModifier=0.088,UpMax=20,LateralMax=4,DirectionChange=2)
	Kickback_WhenSteadyA = (UpBase=1.8,LateralBase=0,UpModifier=0.09,LateralModifier=0.11,UpMax=25,LateralMax=5,DirectionChange=2)
	
	Spread_WhenFallingA = (param1=-0.006,param2=0.304)
	Spread_WhenMovingA = (param1=-0.005,param2=0.19)
	Spread_WhenDuckingA = (param1=-0.00252,param2=0.06498)
	Spread_WhenSteadyA = (param1=-0.00072,param2=0.06498)
	
	AccuracyDivisorA  =  2100
	AccuracyOffsetA  =  0.279
	MaxInaccuracyA  =  1.44
	
	DirectionHold = 2
	FireIntervalMultiplierA = 1.5

	KickbackLimiter(0) = (Min=0.5,Max=0.5)
	KickbackLimiter(1) = (Min=0.5,Max=0.6)
	KickbackLimiter(2) = (Min=0.8,Max=1.4)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime		= 1.2667
	PutDownTime		= 0.5333
	ReloadTime		= 2.6667
	SightInfos(0)		= (FOV=90,ChangeTime=0.1)

	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=1.0,OtherShotRate=0.0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.0,OtherShotRate=1.0)
	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4

	AttachmentClass		= class'avaAttachment_AKS74U'
	BaseSkelMeshName	= "Wp_smg_AK74U.MS_AKS_74U"
	BaseAnimSetName		= "Wp_smg_AK74U.Ani_AKS74U_1P"

	WeaponFireSnd=SoundCue'avaWeaponSounds.SMG_AKS74U.SMG_AKS74U_fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	bEnableSilencer		= False

	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'
//	SilencerMeshName		= "Wp_Silencer.MS_SMG_Silencer"
//	SilencerBoneName		= Bizon_Bone05
}
