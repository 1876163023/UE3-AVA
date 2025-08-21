class avaWeap_CZ75 extends avaWeap_BasePistol;

defaultproperties
{
	BulletType=class'avaBullet_9MM'

	BaseSpeed		= 268	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage = 30

	FireInterval(0)=0.121

	ClipCnt=16
	AmmoCount=16
	MaxAmmoCount=48

	Penetration = 2

	RangeModifier=0.66

	SpreadDecayTime = 0.3

	Kickback_WhenMoving = (UpBase=3.5,LateralBase=0.42,UpModifier=0.42,LateralModifier=0.028,UpMax=18.2,LateralMax=6.3,DirectionChange=1)
	Kickback_WhenFalling = (UpBase=3.75,LateralBase=0.45,UpModifier=0.45,LateralModifier=0.03,UpMax=19.5,LateralMax=6.75,DirectionChange=1)
	Kickback_WhenDucking = (UpBase=2,LateralBase=0.24,UpModifier=0.24,LateralModifier=0.016,UpMax=10.4,LateralMax=3.6,DirectionChange=1)
	Kickback_WhenSteady = (UpBase=2.5,LateralBase=0.3,UpModifier=0.3,LateralModifier=0.02,UpMax=13,LateralMax=4.5,DirectionChange=1)
	
	Spread_WhenFalling = (param1=0.014,param2=0.0384)
	Spread_WhenMoving = (param1=0.007,param2=0.0352)
	Spread_WhenDucking = (param1=0.0056,param2=0.0288)
	Spread_WhenSteady = (param1=0.007,param2=0.032)
	
	AccuracyDivisor  =  400
	AccuracyOffset  =  0.35
	MaxInaccuracy  =  2
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 10

	Kickback_WhenMovingA = (UpBase=3.5,LateralBase=0.42,UpModifier=0.42,LateralModifier=0.028,UpMax=18.2,LateralMax=6.3,DirectionChange=1)
	Kickback_WhenFallingA = (UpBase=3.75,LateralBase=0.45,UpModifier=0.45,LateralModifier=0.03,UpMax=19.5,LateralMax=6.75,DirectionChange=1)
	Kickback_WhenDuckingA = (UpBase=2,LateralBase=0.24,UpModifier=0.24,LateralModifier=0.016,UpMax=10.4,LateralMax=3.6,DirectionChange=1)
	Kickback_WhenSteadyA = (UpBase=2.5,LateralBase=0.3,UpModifier=0.3,LateralModifier=0.02,UpMax=13,LateralMax=4.5,DirectionChange=1)
	
	Spread_WhenFallingA = (param1=0.014,param2=0.0768)
	Spread_WhenMovingA = (param1=0.007,param2=0.0704)
	Spread_WhenDuckingA = (param1=0.00504,param2=0.02592)
	Spread_WhenSteadyA = (param1=0.0063,param2=0.0288)
	
	AccuracyDivisorA  =  2000
	AccuracyOffsetA  =  0.28
	MaxInaccuracyA  =  2
	
	DirectionHold = 1
	FireIntervalMultiplierA = 1.2


	KickbackLimiter(0) = (Min=0.8,Max=1.2)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.6
	BobDampingInDash	=	0.4

	EquipTime			=0.83
	PutDownTime			=0.3333
	ReloadTime			=2.23

	AttachmentClass=class'avaAttachment_P226'

	bEnableSilencer		= False

	BaseSkelMeshName	=	"Wp_Pis_P226.Wp_Pis_P226_Basic.MS_P226_Basic_1p"
	BaseAnimSetName		=	"Wp_Pis_P226.Ani_P226"
	WeaponFireSnd=SoundCue'avaWeaponSounds.Pistol_P226.Pistol_P226_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Pistol'
}