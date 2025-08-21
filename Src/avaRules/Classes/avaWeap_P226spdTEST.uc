class avaWeap_P226spdTEST extends avaWeap_BasePistol;

defaultproperties
{
	BulletType=class'avaBullet_9MM'

	BaseSpeed		= 330	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage = 30
	HitDamageS=30

	FireInterval(0)=0.142
	ClipCnt=15
	AmmoCount=15
	MaxAmmoCount=45

	Penetration = 2
	PenetrationS = 1

	RangeModifier=0.65
	RangeModifierS=0.6

	SpreadDecayTime = 0.35

	Kickback_WhenMoving = (UpBase=2.8,LateralBase=1.12,UpModifier=0.42,LateralModifier=0.028,UpMax=18.2,LateralMax=6.3,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=3,LateralBase=1.2,UpModifier=0.45,LateralModifier=0.03,UpMax=19.5,LateralMax=6.75,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=1.6,LateralBase=0.64,UpModifier=0.24,LateralModifier=0.016,UpMax=10.4,LateralMax=3.6,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=2,LateralBase=0.8,UpModifier=0.3,LateralModifier=0.02,UpMax=13,LateralMax=4.5,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.0105,param2=0.0384)
	Spread_WhenMoving = (param1=0.0077,param2=0.0352)
	Spread_WhenDucking = (param1=0.0063,param2=0.0288)
	Spread_WhenSteady = (param1=0.007,param2=0.032)
	
	AccuracyDivisor  =  100
	AccuracyOffset  =  0.3
	MaxInaccuracy  =  1.8


	Kickback_WhenMovingA = (UpBase=2.8,LateralBase=1.12,UpModifier=0.42,LateralModifier=0.028,UpMax=18.2,LateralMax=6.3,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=3,LateralBase=1.2,UpModifier=0.45,LateralModifier=0.03,UpMax=19.5,LateralMax=6.75,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=1.6,LateralBase=0.64,UpModifier=0.24,LateralModifier=0.016,UpMax=10.4,LateralMax=3.6,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=2,LateralBase=0.8,UpModifier=0.3,LateralModifier=0.02,UpMax=13,LateralMax=4.5,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0,param2=0.042)
	Spread_WhenMovingA = (param1=0,param2=0.0385)
	Spread_WhenDuckingA = (param1=0,param2=0.0315)
	Spread_WhenSteadyA = (param1=0,param2=0.035)
	
	AccuracyDivisorA  =  100
	AccuracyOffsetA  =  0.55
	MaxInaccuracyA  =  0.8
	
	DirectionHold = 2
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

	bEnableSilencer		= True
	SilencerMeshName		= "Wp_Silencer.MS_P226_Silencer"
	SilencerBoneName		= Bone01
	MountSilencerAnim		= Sil_In	// 소음기 장착 Animation 이름
	UnMountSilencerAnim	= Sil_Out	// 소음기 탈착 Animation 이름
	MountSilencerTime		= 2.000	// 소음기 장착시 걸리는 시간
	UnMountSilencerTime	= 2.000	// 소음기 탈착시 걸리는 시간

	WeaponSilencerFireSnd	= SoundCue'avaWeaponSounds.Common.Silencer.Silencer_Fire_SMG'	

	BaseSkelMeshName	=	"Wp_Pis_P226.Wp_Pis_P226_Basic.MS_P226_Basic_1p"
	BaseAnimSetName		=	"Wp_Pis_P226.Ani_P226"
	WeaponFireSnd=SoundCue'avaWeaponSounds.Pistol_P226.Pistol_P226_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	InstantHitDamageTypes(0)	=	class'avaDmgType_Pistol'
}