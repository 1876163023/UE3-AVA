class avaWeap_M4A1 extends avaWeap_BaseRifle;

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

	HitDamage			=	32
	FireInterval(0)			=	0.09
	
	ClipCnt			=	30
	AmmoCount			=	30
	MaxAmmoCount			=	90

	Penetration			=	2
	RangeModifier			=	0.81

	SpreadDecayTime = 0.8

	Kickback_WhenMoving = (UpBase=0.825,LateralBase=0.9,UpModifier=0.275,LateralModifier=0.3,UpMax=20.9,LateralMax=6.6,DirectionChange=5)
	Kickback_WhenFalling = (UpBase=1.5,LateralBase=1.2,UpModifier=0.5,LateralModifier=0.5,UpMax=38,LateralMax=12,DirectionChange=5)
	Kickback_WhenDucking = (UpBase=0.75,LateralBase=0.6,UpModifier=0.25,LateralModifier=0.25,UpMax=19,LateralMax=6,DirectionChange=5)
	Kickback_WhenSteady = (UpBase=0.75,LateralBase=0.6,UpModifier=0.25,LateralModifier=0.25,UpMax=19,LateralMax=6,DirectionChange=5)
	
	Spread_WhenFalling = (param1=0.054,param2=0.112)
	Spread_WhenMoving = (param1=0.0117,param2=0.049)
	Spread_WhenDucking = (param1=0.00765,param2=0.0133)
	Spread_WhenSteady = (param1=0.009,param2=0.014)
	
	AccuracyDivisor  =  1200
	AccuracyOffset  =  0.2
	MaxInaccuracy  =  3.6
	
	Kickback_UpLimit = 8
	Kickback_LateralLimit = 2.5

	Kickback_WhenMovingA = (UpBase=1.07,LateralBase=1.17,UpModifier=0.3575,LateralModifier=0.39,UpMax=27.2,LateralMax=8.6,DirectionChange=5)
	Kickback_WhenFallingA = (UpBase=1.95,LateralBase=1.56,UpModifier=0.65,LateralModifier=0.65,UpMax=49.4,LateralMax=15.6,DirectionChange=5)
	Kickback_WhenDuckingA = (UpBase=0.75,LateralBase=0.6,UpModifier=0.25,LateralModifier=0.25,UpMax=19,LateralMax=6,DirectionChange=5)
	Kickback_WhenSteadyA = (UpBase=0.75,LateralBase=0.6,UpModifier=0.25,LateralModifier=0.25,UpMax=19,LateralMax=6,DirectionChange=5)
	
	Spread_WhenFallingA = (param1=0.054,param2=0.224)
	Spread_WhenMovingA = (param1=0.0117,param2=0.098)
	Spread_WhenDuckingA = (param1=0.006885,param2=0.01197)
	Spread_WhenSteadyA = (param1=0.0081,param2=0.0126)
	
	AccuracyDivisorA  =  3600
	AccuracyOffsetA  =  0.18
	MaxInaccuracyA  =  4.32
	
	DirectionHold = 4
	FireIntervalMultiplierA = 1.2

	KickbackLimiter(0) = (Min=0.5,Max=1)
	KickbackLimiter(1) = (Min=0.6,Max=1)
	KickbackLimiter(2) = (Min=0.6,Max=1)
	KickbackLimiter(3) = (Min=0.7,Max=1.3)

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
