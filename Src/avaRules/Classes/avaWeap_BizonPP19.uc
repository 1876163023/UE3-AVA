class avaWeap_BizonPP19 extends avaWeap_BaseSMG;

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
	FireInterval(0)		= 0.11

	ClipCnt		= 64
	AmmoCount		= 64
	MaxAmmoCount		= 128

	Penetration		= 2
	PenetrationS		= 1

	RangeModifier		= 0.68
	RangeModifierS		= 0.62

	SpreadDecayTime = 1.1

	Kickback_WhenMoving = (UpBase=1.2,LateralBase=0.96,UpModifier=0.03,LateralModifier=0.042,UpMax=10.2,LateralMax=9,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=1.5,LateralBase=1.2,UpModifier=0.0375,LateralModifier=0.0525,UpMax=12.75,LateralMax=11.25,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=0.8,LateralBase=0.64,UpModifier=0.02,LateralModifier=0.028,UpMax=6.8,LateralMax=6,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=1,LateralBase=0.8,UpModifier=0.025,LateralModifier=0.035,UpMax=8.5,LateralMax=7.5,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.033033,param2=0.00847)
	Spread_WhenMoving = (param1=0.02949375,param2=0.0075625)
	Spread_WhenDucking = (param1=0.0165165,param2=0.004235)
	Spread_WhenSteady = (param1=0.023595,param2=0.00605)
	
	AccuracyDivisor  =  250
	AccuracyOffset  =  0.1
	MaxInaccuracy  =  3.8
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 4


	Kickback_WhenMovingA = (UpBase=1.56,LateralBase=1.248,UpModifier=0.039,LateralModifier=0.0546,UpMax=13.26,LateralMax=11.7,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=1.95,LateralBase=1.56,UpModifier=0.04875,LateralModifier=0.06825,UpMax=16.575,LateralMax=14.625,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=0.8,LateralBase=0.64,UpModifier=0.02,LateralModifier=0.028,UpMax=6.8,LateralMax=6,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=1,LateralBase=0.8,UpModifier=0.025,LateralModifier=0.035,UpMax=8.5,LateralMax=7.5,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.033033,param2=0.01694)
	Spread_WhenMovingA = (param1=0.02949375,param2=0.015125)
	Spread_WhenDuckingA = (param1=0.01486485,param2=0.0038115)
	Spread_WhenSteadyA = (param1=0.0212355,param2=0.005445)
	
	AccuracyDivisorA  =  750
	AccuracyOffsetA  =  0.09
	MaxInaccuracyA  =  4.56
	
	DirectionHold = 2
	FireIntervalMultiplierA = 1.5

	KickbackLimiter(0) = (Min=0.7,Max=1)
	KickbackLimiter(1) = (Min=0.8,Max=1)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime		= 1.0333
	PutDownTime		= 0.5333
	ReloadTime		= 2.5333
	SightInfos(0)		= (FOV=90,ChangeTime=0.1)

	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=1.0,OtherShotRate=0.0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.0,OtherShotRate=1.0)
	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4

	AttachmentClass		= class'avaAttachment_BizonPP19'
	BaseSkelMeshName	= "WP_smg_bizon.MS_bizon"
	BaseAnimSetName		= "WP_smg_bizon.Ani_Bizon"

	WeaponFireSnd=SoundCue'avaWeaponSounds.SMG_BizonPP19.SMG_BizonPP19_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	bEnableSilencer		= True

	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'
	SilencerMeshName		= "Wp_Silencer.MS_SMG_Silencer"
	SilencerBoneName		= Bizon_Bone05
}
