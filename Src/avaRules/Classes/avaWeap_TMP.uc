class avaWeap_TMP extends avaWeap_BasePistol;

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

	HitDamage = 29

	FireInterval(0)=0.095

	ClipCnt=15
	AmmoCount=15
	MaxAmmoCount=45

	Penetration = 2
	RangeModifier=0.67

	SpreadDecayTime = 0.8

	Kickback_WhenMoving = (UpBase=0.84,LateralBase=0.6,UpModifier=0.12,LateralModifier=0.12,UpMax=7.2,LateralMax=9.6,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=1.05,LateralBase=0.75,UpModifier=0.15,LateralModifier=0.15,UpMax=9,LateralMax=12,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=0.56,LateralBase=0.4,UpModifier=0.08,LateralModifier=0.08,UpMax=4.8,LateralMax=6.4,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=0.7,LateralBase=0.5,UpModifier=0.1,LateralModifier=0.1,UpMax=6,LateralMax=8,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.0259545,param2=0.037752)
	Spread_WhenMoving = (param1=0.0224939,param2=0.0327184)
	Spread_WhenDucking = (param1=0.0155727,param2=0.0226512)
	Spread_WhenSteady = (param1=0.017303,param2=0.025168)
	
	AccuracyDivisor  =  1000
	AccuracyOffset  =  0.5
	MaxInaccuracy  =  4
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 4.5



	Kickback_WhenMovingA = (UpBase=1.8,LateralBase=0.48,UpModifier=0.036,LateralModifier=0.48,UpMax=14.4,LateralMax=9.6,DirectionChange=2)
	Kickback_WhenFallingA = (UpBase=2.25,LateralBase=0.6,UpModifier=0.045,LateralModifier=0.6,UpMax=18,LateralMax=12,DirectionChange=2)
	Kickback_WhenDuckingA = (UpBase=1.2,LateralBase=0.32,UpModifier=0.024,LateralModifier=0.32,UpMax=9.6,LateralMax=6.4,DirectionChange=2)
	Kickback_WhenSteadyA = (UpBase=1.5,LateralBase=0.4,UpModifier=0.03,LateralModifier=0.4,UpMax=12,LateralMax=8,DirectionChange=2)
	
	Spread_WhenFallingA = (param1=0.0259545,param2=0.037752)
	Spread_WhenMovingA = (param1=0.0224939,param2=0.0327184)
	Spread_WhenDuckingA = (param1=0.0155727,param2=0.0226512)
	Spread_WhenSteadyA = (param1=0.017303,param2=0.025168)
	
	AccuracyDivisorA  =  450
	AccuracyOffsetA  =  1.5
	MaxInaccuracyA  =  4
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1.2

	KickbackLimiter(0) = (Min=0.5,Max=1)
	KickbackLimiter(1) = (Min=0.8,Max=1.2)

	bAutoFire	=	True

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=1.033
	PutDownTime			=0.3333
	ReloadTime			=2.333


	AttachmentClass=class'avaAttachment_TMP'

	BaseSkelMeshName	=	"Wp_Pis_P226.Wp_Pis_P226_Basic.MS_P226_Basic_1p"
	BaseAnimSetName		=	"Wp_Pis_P226.Wp_Pis_P226_Basic.Ani_P226_Dup"
	WeaponFireSnd			=	SoundCue'avaWeaponSounds.SMG_MP5K.SMG_MP5K_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	InstantHitDamageTypes(0)	=	class'avaDmgType_Pistol'
}
