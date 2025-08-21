class avaWeap_HK416 extends avaWeap_BaseRifle;

defaultproperties
{

	BulletType				=	class'avaBullet_556NATO'	

	BaseSpeed		= 252	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.32	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage			=	33

	FireInterval(0)			=	0.095
	
	ClipCnt			=	30
	AmmoCount			=	30
	MaxAmmoCount			=	90

	Penetration			=	2
	RangeModifier			=	0.85

	SpreadDecayTime = 0.5

	Kickback_WhenMoving = (UpBase=1.1,LateralBase=0.9,UpModifier=0.22,LateralModifier=0.36,UpMax=22,LateralMax=8.8,DirectionChange=5)
	Kickback_WhenFalling = (UpBase=2,LateralBase=1.2,UpModifier=0.4,LateralModifier=0.6,UpMax=40,LateralMax=16,DirectionChange=5)
	Kickback_WhenDucking = (UpBase=1,LateralBase=0.6,UpModifier=0.2,LateralModifier=0.3,UpMax=20,LateralMax=8,DirectionChange=5)
	Kickback_WhenSteady = (UpBase=1,LateralBase=0.6,UpModifier=0.2,LateralModifier=0.3,UpMax=20,LateralMax=8,DirectionChange=5)
	
	Spread_WhenFalling = (param1=0.042,param2=0.112)
	Spread_WhenMoving = (param1=0.0091,param2=0.0448)
	Spread_WhenDucking = (param1=0.00595,param2=0.0133)
	Spread_WhenSteady = (param1=0.007,param2=0.014)
	
	AccuracyDivisor  =  1200
	AccuracyOffset  =  0.15
	MaxInaccuracy  =  4
	
	Kickback_UpLimit = 8
	Kickback_LateralLimit = 3


	Kickback_WhenMovingA = (UpBase=1.43,LateralBase=1.17,UpModifier=0.286,LateralModifier=0.468,UpMax=28.6,LateralMax=11.4,DirectionChange=5)
	Kickback_WhenFallingA = (UpBase=2.6,LateralBase=1.56,UpModifier=0.52,LateralModifier=0.78,UpMax=52,LateralMax=20.8,DirectionChange=5)
	Kickback_WhenDuckingA = (UpBase=1,LateralBase=0.6,UpModifier=0.2,LateralModifier=0.3,UpMax=20,LateralMax=8,DirectionChange=5)
	Kickback_WhenSteadyA = (UpBase=1,LateralBase=0.6,UpModifier=0.2,LateralModifier=0.3,UpMax=20,LateralMax=8,DirectionChange=5)
	
	Spread_WhenFallingA = (param1=0.042,param2=0.224)
	Spread_WhenMovingA = (param1=0.0091,param2=0.0896)
	Spread_WhenDuckingA = (param1=0.005355,param2=0.01197)
	Spread_WhenSteadyA = (param1=0.0063,param2=0.0126)
	
	AccuracyDivisorA  =  3600
	AccuracyOffsetA  =  0.135
	MaxInaccuracyA  =  4.8
	
	DirectionHold = 2
	FireIntervalMultiplierA = 1.5


	KickbackLimiter(0) = (Min=0.7,Max=1)
	KickbackLimiter(1) = (Min=0.7,Max=1)
	KickbackLimiter(2) = (Min=0.8,Max=1.2)


	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.0333
	PutDownTime			=	0.1
	ReloadTime			=	2.967

	SightInfos(0)			=	(FOV=90,ChangeTime=0.2)
	ScopeMeshName = "Wp_Scope.Com_Scope.MS_UIHolosight01"

	AttachmentClass			=	class'avaAttachment_M4A1'

	BaseSkelMeshName		=	"Wp_Rif_M4.Wp_Rif_M4_Rail.MS_M4_Rail"
	BaseAnimSetName			=	"Wp_Rif_M4.Ani_M4A1_1P"

	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=1.0,OtherShotRate=0.0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.0,OtherShotRate=1.0)
	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4

 	WeaponPutDownAnim		=	Down
	WeaponEquipAnim			=	BringUp
	WeaponReloadAnim		=	Reload

	WeaponIdleAnims(0)		=	Idle
	HudMaterial			=	Texture2D'avaDotSightUI.Textures.M4_dotsight'
	WeaponFireSnd			=	SoundCue'avaWeaponSounds.AR_M4A1.AR_M4A1_Fire'
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	DefaultModifiers(0)			=	class'avaRules.avaMod_M4A1_M_Dot'
	//DefaultModifiers(0)			=	class'avaRules.avaMod_M4A1_M_Dot_B'
	//DefaultModifiers(1)			=	class'avaRules.avaMod_M4A1_F_Flash'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
