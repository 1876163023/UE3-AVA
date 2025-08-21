class avaWeap_MP5 extends avaWeap_BaseSMG;

//MP5A3
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

	HitDamage			= 31
	HitDamageS			= 31
	FireInterval(0)		= 0.088

	ClipCnt			= 25
	AmmoCount			= 25
	MaxAmmoCount		= 75

	Penetration			= 2
	PenetrationS		= 1

	RangeModifier		= 0.7
	RangeModifierS		= 0.65

	SpreadDecayTime = 0.8

	Kickback_WhenMoving = (UpBase=1.44,LateralBase=0.6,UpModifier=0.03,LateralModifier=0.084,UpMax=12,LateralMax=5.4,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=1.8,LateralBase=0.75,UpModifier=0.0375,LateralModifier=0.105,UpMax=15,LateralMax=6.75,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=0.96,LateralBase=0.4,UpModifier=0.02,LateralModifier=0.056,UpMax=8,LateralMax=3.6,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=1.2,LateralBase=0.5,UpModifier=0.025,LateralModifier=0.07,UpMax=10,LateralMax=4.5,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.024656775,param2=0.02630056)
	Spread_WhenMoving = (param1=0.0227601,param2=0.0202312)
	Spread_WhenDucking = (param1=0.013276725,param2=0.01416184)
	Spread_WhenSteady = (param1=0.01896675,param2=0.0202312)
	
	AccuracyDivisor  =  300
	AccuracyOffset  =  0.4
	MaxInaccuracy  =  1.5
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 2.5


	Kickback_WhenMovingA = (UpBase=1.872,LateralBase=0.78,UpModifier=0.039,LateralModifier=0.1092,UpMax=15.6,LateralMax=7.02,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=2.34,LateralBase=0.975,UpModifier=0.04875,LateralModifier=0.1365,UpMax=19.5,LateralMax=8.775,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=0.96,LateralBase=0.4,UpModifier=0.02,LateralModifier=0.056,UpMax=8,LateralMax=3.6,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=1.2,LateralBase=0.5,UpModifier=0.025,LateralModifier=0.07,UpMax=10,LateralMax=4.5,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.024656775,param2=0.05260112)
	Spread_WhenMovingA = (param1=0.0227601,param2=0.0404624)
	Spread_WhenDuckingA = (param1=0.0119490525,param2=0.012745656)
	Spread_WhenSteadyA = (param1=0.017070075,param2=0.01820808)
	
	AccuracyDivisorA  =  900
	AccuracyOffsetA  =  0.36
	MaxInaccuracyA  =  1.8
	
	DirectionHold = 2
	FireIntervalMultiplierA = 1.5




	KickbackLimiter(0) = (Min=0.6,Max=1)
	KickbackLimiter(1) = (Min=0.6,Max=1)
	KickbackLimiter(2) = (Min=0.8,Max=1)
	KickbackLimiter(3) = (Min=1,Max=1)



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
	ReloadTime		= 2.667
	SightInfos(0)		= (FOV=90,ChangeTime=0.1)

	AttachmentClass		= class'avaAttachment_MP5'
	BaseSkelMeshName	= "WP_smg_MP5A3.MS_smg_MP5A3"
	BaseAnimSetName		= "WP_smg_MP5A3.Ani_MP5A3"

	WeaponFireSnd			=	SoundCue'avaWeaponSounds.SMG_MP5A3.SMG_MP5A3_Fire'	
//	WeaponSilencerFireSnd	= SoundCue'WeaponSounds.usp_cue''
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	bEnableSilencer		= true

	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'
	SilencerMeshName		= "Wp_Silencer.MS_SMG_MP5A3_Silencer"
	SilencerBoneName		= MP5A_Bone11
}
