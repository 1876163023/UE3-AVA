class avaWeap_Stechkin extends avaWeap_BasePistol;

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

	HitDamage = 27

	FireInterval(0)=0.104

	ClipCnt=20
	AmmoCount=20
	MaxAmmoCount=40

	Penetration = 2
	RangeModifier=0.63

	SpreadDecayTime = 1

	Kickback_WhenMoving = (UpBase=2.58,LateralBase=0,UpModifier=0.036,LateralModifier=0.216,UpMax=18,LateralMax=9.6,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=3.225,LateralBase=0,UpModifier=0.045,LateralModifier=0.27,UpMax=22.5,LateralMax=12,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=1.72,LateralBase=0,UpModifier=0.024,LateralModifier=0.144,UpMax=12,LateralMax=6.4,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=2.15,LateralBase=0,UpModifier=0.03,LateralModifier=0.18,UpMax=15,LateralMax=8,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.0255,param2=0.0375)
	Spread_WhenMoving = (param1=0.0221,param2=0.0325)
	Spread_WhenDucking = (param1=0.0153,param2=0.0225)
	Spread_WhenSteady = (param1=0.017,param2=0.025)
	
	AccuracyDivisor  =  750
	AccuracyOffset  =  0.5
	MaxInaccuracy  =  2
	
	Kickback_UpLimit = 5
	Kickback_LateralLimit = 3.5

	
	Kickback_WhenMovingA = (UpBase=1.8,LateralBase=0.48,UpModifier=0.036,LateralModifier=0.48,UpMax=14.4,LateralMax=9.6,DirectionChange=2)
	Kickback_WhenFallingA = (UpBase=2.25,LateralBase=0.6,UpModifier=0.045,LateralModifier=0.6,UpMax=18,LateralMax=12,DirectionChange=2)
	Kickback_WhenDuckingA = (UpBase=1.2,LateralBase=0.32,UpModifier=0.024,LateralModifier=0.32,UpMax=9.6,LateralMax=6.4,DirectionChange=2)
	Kickback_WhenSteadyA = (UpBase=1.5,LateralBase=0.4,UpModifier=0.03,LateralModifier=0.4,UpMax=12,LateralMax=8,DirectionChange=2)
	
	Spread_WhenFallingA = (param1=0.0259545,param2=0.037752)
	Spread_WhenMovingA = (param1=0.0224939,param2=0.0327184)
	Spread_WhenDuckingA = (param1=0.0155727,param2=0.0226512)
	Spread_WhenSteadyA = (param1=0.017303,param2=0.025168)
	
	AccuracyDivisorA  =  750
	AccuracyOffsetA  =  0.5
	MaxInaccuracyA  =  2.2
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1.2


	KickbackLimiter(0) = (Min=0.8,Max=1)
	KickbackLimiter(1) = (Min=0.75,Max=1.25)


//	FireAnimInfos(0)		=	(AnimName=Fire2,FirstShotRate=1.0,OtherShotRate=0)
//	FireAnimInfos(1)		=	(AnimName=Fire1,FirstShotRate=0.0,OtherShotRate=1)
//	WeaponFireAnim(0)		=	Fire2
//	WeaponFireAnim(1)		=	Fire1

	bAutoFire	=	True

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=1.2333
	PutDownTime			=0.2333
	ReloadTime			=2.7000

	AttachmentClass=class'avaAttachment_Stechkin'

	BaseSkelMeshName	=	"Wp_Pis_Stechkin.MS_Pis_StechkinStock"
	BaseAnimSetName		=	"Wp_Pis_Stechkin.Ani_Stechkin"
	WeaponFireSnd			=	SoundCue'avaWeaponSounds.Pistol_Stechkin.Pistol_Stechkin_fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	InstantHitDamageTypes(0)	=	class'avaDmgType_Pistol'
}
