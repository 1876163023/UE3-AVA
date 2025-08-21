class avaWeap_MK23Socom extends avaWeap_BasePistol;

defaultproperties
{
	BulletType=class'avaBullet_45ACP'

	BaseSpeed		= 260	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage = 35
	HitDamageS=35

	FireInterval(0)=0.14

	ClipCnt=12
	AmmoCount=12
	MaxAmmoCount=36

	Penetration = 2
	PenetrationS = 1

	RangeModifier=0.7
	RangeModifierS=0.6

	SpreadDecayTime = 0.65

	Kickback_WhenMoving = (UpBase=3.6,LateralBase=0.6,UpModifier=0.54,LateralModifier=0.36,UpMax=14.4,LateralMax=4.8,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=4.5,LateralBase=0.75,UpModifier=0.675,LateralModifier=0.45,UpMax=18,LateralMax=6,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=2.4,LateralBase=0.4,UpModifier=0.36,LateralModifier=0.24,UpMax=9.6,LateralMax=3.2,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=3,LateralBase=0.5,UpModifier=0.45,LateralModifier=0.3,UpMax=12,LateralMax=4,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.0105,param2=0.048)
	Spread_WhenMoving = (param1=0.00875,param2=0.0416)
	Spread_WhenDucking = (param1=0.0063,param2=0.0288)
	Spread_WhenSteady = (param1=0.007,param2=0.032)
	
	AccuracyDivisor  =  120
	AccuracyOffset  =  0.4
	MaxInaccuracy  =  2
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 10


	Kickback_WhenMovingA = (UpBase=3,LateralBase=0.6,UpModifier=0.54,LateralModifier=0.36,UpMax=14.4,LateralMax=4.8,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=3.75,LateralBase=0.75,UpModifier=0.675,LateralModifier=0.45,UpMax=18,LateralMax=6,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=2,LateralBase=0.4,UpModifier=0.36,LateralModifier=0.24,UpMax=9.6,LateralMax=3.2,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=2.5,LateralBase=0.5,UpModifier=0.45,LateralModifier=0.3,UpMax=12,LateralMax=4,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.0105,param2=0.096)
	Spread_WhenMovingA = (param1=0.00875,param2=0.0832)
	Spread_WhenDuckingA = (param1=0.00567,param2=0.02592)
	Spread_WhenSteadyA = (param1=0.0056,param2=0.0256)
	
	AccuracyDivisorA  =  600
	AccuracyOffsetA  =  0.2
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