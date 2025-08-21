class avaWeap_SSG552 extends avaWeap_BaseSMG;

defaultproperties
{
	BulletType		= class'avaBullet_556NATO'

	BaseSpeed		= 253	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	HitDamage		= 33
	HitDamageS		= 33

	FireInterval(0)		= 0.09

	ClipCnt			= 30
	AmmoCount		= 30
	MaxAmmoCount		= 90

	Penetration		= 2
	PenetrationS		= 1

	RangeModifier		= 0.75
	RangeModifierS		= 0.66

	SpreadDecayTime = 0.9

	Kickback_WhenMoving = (UpBase=1.2,LateralBase=0.72,UpModifier=0.036,LateralModifier=0.06,UpMax=12,LateralMax=4.8,DirectionChange=5)
	Kickback_WhenFalling = (UpBase=1.2,LateralBase=0.9,UpModifier=0.045,LateralModifier=0.075,UpMax=15,LateralMax=6,DirectionChange=5)
	Kickback_WhenDucking = (UpBase=0.64,LateralBase=0.48,UpModifier=0.024,LateralModifier=0.04,UpMax=8,LateralMax=3.2,DirectionChange=5)
	Kickback_WhenSteady = (UpBase=0.8,LateralBase=0.6,UpModifier=0.03,LateralModifier=0.05,UpMax=10,LateralMax=4,DirectionChange=5)
	
	Spread_WhenFalling = (param1=0.03993,param2=0.028314)
	Spread_WhenMoving = (param1=0.023958,param2=0.026136)
	Spread_WhenDucking = (param1=0.01697025,param2=0.018513)
	Spread_WhenSteady = (param1=0.019965,param2=0.02178)
	
	AccuracyDivisor  =  800
	AccuracyOffset  =  0.15
	MaxInaccuracy  =  1.5
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 2.5


	Kickback_WhenMovingA = (UpBase=1.56,LateralBase=0.936,UpModifier=0.0468,LateralModifier=0.078,UpMax=15.6,LateralMax=6.24,DirectionChange=5)
	Kickback_WhenFallingA = (UpBase=1.56,LateralBase=1.17,UpModifier=0.0585,LateralModifier=0.0975,UpMax=19.5,LateralMax=7.8,DirectionChange=5)
	Kickback_WhenDuckingA = (UpBase=0.64,LateralBase=0.48,UpModifier=0.024,LateralModifier=0.04,UpMax=8,LateralMax=3.2,DirectionChange=5)
	Kickback_WhenSteadyA = (UpBase=0.8,LateralBase=0.6,UpModifier=0.03,LateralModifier=0.05,UpMax=10,LateralMax=4,DirectionChange=5)
	
	Spread_WhenFallingA = (param1=0.03993,param2=0.056628)
	Spread_WhenMovingA = (param1=0.023958,param2=0.052272)
	Spread_WhenDuckingA = (param1=0.015273225,param2=0.0166617)
	Spread_WhenSteadyA = (param1=0.0179685,param2=0.019602)
	
	AccuracyDivisorA  =  2400
	AccuracyOffsetA  =  0.135
	MaxInaccuracyA  =  1.8
	
	DirectionHold = 5
	FireIntervalMultiplierA = 1.5


	KickbackLimiter(0) = (Min=0.5,Max=1)
	KickbackLimiter(1) = (Min=0.8,Max=1)
	KickbackLimiter(2) = (Min=0.8,Max=1.3)


	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime		= 1.1000
	PutDownTime		= 0.8333
	ReloadTime		= 2.5000

	SightInfos(0)		= (FOV=90,ChangeTime=0.1)

	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=1.0,OtherShotRate=0.0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.0,OtherShotRate=1.0)
	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4

	AttachmentClass		= class'avaAttachment_SSG552'
	BaseSkelMeshName	= "Wp_Smg_SG552.Wp_Smg_SG552_Basic.MS_Smg_Sig552"
	BaseAnimSetName		= "Wp_Smg_SG552.Ani_SG552"

	WeaponFireSnd=SoundCue'avaWeaponSounds.SMG_BizonPP19.SMG_BizonPP19_Fire'	

	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	bEnableSilencer		= True

	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'
	SilencerMeshName		= "Wp_Silencer.MS_SMG_Silencer"
	SilencerBoneName		= Bone11
}
