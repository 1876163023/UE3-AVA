class avaWeap_Skorpion extends avaWeap_BasePistol;

defaultproperties
{
	BulletType=class'avaBullet_9MM'

	BaseSpeed		= 262	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage = 30

	FireInterval(0)=0.1045
	ClipCnt=10
	AmmoCount=10
	MaxAmmoCount=30

	Penetration = 2
	RangeModifier=0.69

	AccuracyDivisor=300
	AccuracyOffset=0.1
	MaxInaccuracy=3

	SpreadDecayTime = 0.4

	Kickback_WhenMoving = (UpBase=0.96,LateralBase=0.6,UpModifier=0.42,LateralModifier=0.24,UpMax=16.8,LateralMax=8.4,DirectionChange=2)
	Kickback_WhenFalling = (UpBase=1.2,LateralBase=0.75,UpModifier=0.525,LateralModifier=0.3,UpMax=21,LateralMax=10.5,DirectionChange=2)
	Kickback_WhenDucking = (UpBase=0.64,LateralBase=0.4,UpModifier=0.28,LateralModifier=0.16,UpMax=11.2,LateralMax=5.6,DirectionChange=2)
	Kickback_WhenSteady = (UpBase=0.8,LateralBase=0.5,UpModifier=0.35,LateralModifier=0.2,UpMax=14,LateralMax=7,DirectionChange=2)
	
	Spread_WhenFalling = (param1=0.0675825,param2=0.0212355)
	Spread_WhenMoving = (param1=0.0585715,param2=0.0184041)
	Spread_WhenDucking = (param1=0.0405495,param2=0.0127413)
	Spread_WhenSteady = (param1=0.045055,param2=0.014157)


	Kickback_WhenMovingA = (UpBase=0.96,LateralBase=0.6,UpModifier=0.42,LateralModifier=0.24,UpMax=16.8,LateralMax=8.4,DirectionChange=2)
	Kickback_WhenFallingA = (UpBase=1.2,LateralBase=0.75,UpModifier=0.525,LateralModifier=0.3,UpMax=21,LateralMax=10.5,DirectionChange=2)
	Kickback_WhenDuckingA = (UpBase=0.64,LateralBase=0.4,UpModifier=0.28,LateralModifier=0.16,UpMax=11.2,LateralMax=5.6,DirectionChange=2)
	Kickback_WhenSteadyA = (UpBase=0.8,LateralBase=0.5,UpModifier=0.35,LateralModifier=0.2,UpMax=14,LateralMax=7,DirectionChange=2)
	
	Spread_WhenFallingA = (param1=0.0675825,param2=0.0212355)
	Spread_WhenMovingA = (param1=0.0585715,param2=0.0184041)
	Spread_WhenDuckingA = (param1=0.0405495,param2=0.0127413)
	Spread_WhenSteadyA = (param1=0.045055,param2=0.014157)
	
	AccuracyDivisorA  =  300
	AccuracyOffsetA  =  0.1
	MaxInaccuracyA  =  3
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1.2

	KickbackLimiter(0) = (Min=0.5,Max=0.8)
	KickbackLimiter(1) = (Min=0.5,Max=1)
	KickbackLimiter(2) = (Min=0.7,Max=1)

	FireAnimInfos(0)		=	(AnimName=Fire2,FirstShotRate=1.0,OtherShotRate=0)
	FireAnimInfos(1)		=	(AnimName=Fire1,FirstShotRate=0.0,OtherShotRate=1)
	WeaponFireAnim(0)		=	Fire2
	WeaponFireAnim(1)		=	Fire1

	bAutoFire	=	True

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=1.0667
	PutDownTime			=0.2333
	ReloadTime			=2.6333

	AttachmentClass=class'avaAttachment_Skorpion'

	BaseSkelMeshName	=	"Wp_Pis_SkorVZ61.MS_SkorVZ61"
	BaseAnimSetName		=	"Wp_Pis_SkorVZ61.Ani_SkorVZ61"
	WeaponFireSnd			=	SoundCue'avaWeaponSounds.Pistol_SkorVZ61.Pistol_SkorVZ61_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	InstantHitDamageTypes(0)	=	class'avaDmgType_Pistol'
}
