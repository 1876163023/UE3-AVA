class avaWeap_SSG550 extends avaWeap_BaseRifle;

defaultproperties
{

	BulletType				=	class'avaBullet_556NATO'	
	BaseSpeed		= 250	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.32	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage			=	38
	FireInterval(0)			=	0.1045
	
	ClipCnt			=	30
	AmmoCount			=	30
	MaxAmmoCount			=	90

	Penetration			=	2
	RangeModifier			=	0.84

	SpreadDecayTime = 0.35

	Kickback_WhenMoving = (UpBase=0,LateralBase=1.12,UpModifier=0.77,LateralModifier=0.042,UpMax=35,LateralMax=3.5,DirectionChange=8)
	Kickback_WhenFalling = (UpBase=0,LateralBase=1.6,UpModifier=1.1,LateralModifier=0.06,UpMax=50,LateralMax=5,DirectionChange=8)
	Kickback_WhenDucking = (UpBase=0,LateralBase=0.56,UpModifier=0.385,LateralModifier=0.021,UpMax=17.5,LateralMax=1.75,DirectionChange=8)
	Kickback_WhenSteady = (UpBase=0,LateralBase=0.8,UpModifier=0.55,LateralModifier=0.03,UpMax=25,LateralMax=2.5,DirectionChange=8)
	
	Spread_WhenFalling = (param1=0,param2=0.12)
	Spread_WhenMoving = (param1=-0.016,param2=0.06)
	Spread_WhenDucking = (param1=-0.005,param2=0.012)
	Spread_WhenSteady = (param1=-0.01,param2=0.024)
	
	AccuracyDivisor  =  100
	AccuracyOffset  =  0.5
	MaxInaccuracy  =  2.5
	
	Kickback_UpLimit = 8
	Kickback_LateralLimit = 3


	Kickback_WhenMovingA = (UpBase=0,LateralBase=1.456,UpModifier=1.001,LateralModifier=0.0546,UpMax=45.5,LateralMax=4.6,DirectionChange=8)
	Kickback_WhenFallingA = (UpBase=0,LateralBase=2.08,UpModifier=1.43,LateralModifier=0.078,UpMax=65,LateralMax=6.5,DirectionChange=8)
	Kickback_WhenDuckingA = (UpBase=0,LateralBase=0.56,UpModifier=0.385,LateralModifier=0.021,UpMax=17.5,LateralMax=1.75,DirectionChange=8)
	Kickback_WhenSteadyA = (UpBase=0,LateralBase=0.8,UpModifier=0.55,LateralModifier=0.03,UpMax=25,LateralMax=2.5,DirectionChange=8)
	
	Spread_WhenFallingA = (param1=0,param2=0.24)
	Spread_WhenMovingA = (param1=-0.016,param2=0.12)
	Spread_WhenDuckingA = (param1=-0.004,param2=0.0108)
	Spread_WhenSteadyA = (param1=-0.008,param2=0.0216)
	
	AccuracyDivisorA  =  300
	AccuracyOffsetA  =  0.45
	MaxInaccuracyA  =  3
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1.1

	KickbackLimiter(0) = (Min=0.6,Max=0.8)
	KickbackLimiter(1) = (Min=0.9,Max=1.25)


	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.2667
	PutDownTime			=	0.233
	ReloadTime			=	3.00
	SightInfos(0)			=	(FOV=90,ChangeTime=0.2)
	SightInfos(1)			=	(FOV=75,ChangeTime=0.15)

	AttachmentClass			=	class'avaAttachment_SSG550'

	BaseSkelMeshName		=	"Wp_Rif_SG550.MS_Rif_SSG550"
	BaseAnimSetName			=	"Wp_Rif_SG550.Ani_SG550"

	FireAnimInfos(0)		=	(AnimName=Fire2,FirstShotRate=1.0,OtherShotRate=0)
	FireAnimInfos(1)		=	(AnimName=Fire1,FirstShotRate=0.0,OtherShotRate=1)
	WeaponFireAnim(0)		=	Fire2
	WeaponFireAnim(1)		=	Fire1

 	WeaponPutDownAnim		=	Down
	WeaponEquipAnim			=	BringUp
	WeaponReloadAnim		=	Reload

	WeaponIdleAnims(0)		=	Idle
	HudMaterial			=	Texture2D'avaDotSightUI.Textures.M4_dotsight'
	WeaponFireSnd=SoundCue'avaWeaponSounds.SR_SV98.SR_SV98_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
