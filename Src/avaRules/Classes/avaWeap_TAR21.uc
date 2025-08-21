class avaWeap_TAR21 extends avaWeap_BaseRifle;

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
	FireInterval(0)			=	0.095
	
	ClipCnt			=	30
	AmmoCount			=	30
	MaxAmmoCount			=	90

	Penetration			=	2
	RangeModifier			=	0.81


	SpreadDecayTime = 0.4

	Kickback_WhenMoving = (UpBase=1.012,LateralBase=0.715,UpModifier=0.132,LateralModifier=0.022,UpMax=88,LateralMax=11,DirectionChange=5)
	Kickback_WhenFalling = (UpBase=1.84,LateralBase=1.3,UpModifier=0.24,LateralModifier=0.04,UpMax=160,LateralMax=20,DirectionChange=5)
	Kickback_WhenDucking = (UpBase=0.644,LateralBase=0.455,UpModifier=0.084,LateralModifier=0.014,UpMax=56,LateralMax=7,DirectionChange=5)
	Kickback_WhenSteady = (UpBase=0.92,LateralBase=0.65,UpModifier=0.12,LateralModifier=0.02,UpMax=80,LateralMax=10,DirectionChange=5)
	
	Spread_WhenFalling = (param1=0,param2=0.182)
	Spread_WhenMoving = (param1=-0.002,param2=0.06825)
	Spread_WhenDucking = (param1=0,param2=0.03185)
	Spread_WhenSteady = (param1=0,param2=0.0455)
	
	AccuracyDivisor  =  800
	AccuracyOffset  =  0.2
	MaxInaccuracy  =  1.6
	
	Kickback_UpLimit = 15
	Kickback_LateralLimit = 2




	Kickback_WhenMovingA = (UpBase=1.32,LateralBase=0.9295,UpModifier=0.1716,LateralModifier=0.0286,UpMax=114.4,LateralMax=14.3,DirectionChange=5)
	Kickback_WhenFallingA = (UpBase=2.392,LateralBase=1.69,UpModifier=0.312,LateralModifier=0.052,UpMax=208,LateralMax=26,DirectionChange=5)
	Kickback_WhenDuckingA = (UpBase=0.644,LateralBase=0.455,UpModifier=0.084,LateralModifier=0.014,UpMax=56,LateralMax=7,DirectionChange=5)
	Kickback_WhenSteadyA = (UpBase=0.92,LateralBase=0.65,UpModifier=0.12,LateralModifier=0.02,UpMax=80,LateralMax=10,DirectionChange=5)
	
	Spread_WhenFallingA = (param1=0,param2=0.364)
	Spread_WhenMovingA = (param1=-0.002,param2=0.1365)
	Spread_WhenDuckingA = (param1=0,param2=0.028665)
	Spread_WhenSteadyA = (param1=0,param2=0.04095)
	
	AccuracyDivisorA  =  2400
	AccuracyOffsetA  =  0.18
	MaxInaccuracyA  =  1.92
	
	DirectionHold = 2
	FireIntervalMultiplierA = 1.4



	KickbackLimiter(0) = (Min=0.7,Max=1)
	KickbackLimiter(1) = (Min=0.8,Max=1)
	KickbackLimiter(2) = (Min=0.8,Max=1)
	KickbackLimiter(3) = (Min=0.6,Max=1.2)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.1667
	PutDownTime			=	0.333
	ReloadTime			=	2.8333
	SightInfos(0)			=	(FOV=90,ChangeTime=0.2)

	AttachmentClass			=	class'avaAttachment_TAR21'

	BaseSkelMeshName		=	"Wp_Rif_TAR21.MS_TAR21"
	BaseAnimSetName			=	"Wp_Rif_TAR21.Ani_TAR21"


 	WeaponPutDownAnim		=	Down
	WeaponEquipAnim			=	BringUp
	WeaponReloadAnim		=	Reload

	WeaponIdleAnims(0)		=	Idle
	HudMaterial			=	Texture2D'avaDotSightUI.Textures.M4_dotsight'
	WeaponFireSnd=SoundCue'avaWeaponSounds.Pistol_P226.Pistol_P226_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

//기본적으로 도트를 붙인다.
	DefaultModifiers(0)			=	class'avaRules.avaMod_M4A1_M_Dot'
	DefaultModifiers(1)			=	class'avaRules.avaMod_M4A1_F_Flash'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
