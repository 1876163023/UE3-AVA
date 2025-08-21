class avaWeap_AUGA1 extends avaWeap_BaseRifle;

defaultproperties
{

	BulletType				=	class'avaBullet_556NATO'	

	BaseSpeed		= 248	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.32	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage			=	35
	FireInterval(0)			=	0.100
	
	ClipCnt			=	30
	AmmoCount			=	30
	MaxAmmoCount			=	90

	Penetration			=	2
	RangeModifier			=	0.86

	SpreadDecayTime = 0.65

	Kickback_WhenMoving = (UpBase=0.77,LateralBase=0.85,UpModifier=0.39,LateralModifier=0.05,UpMax=16.5,LateralMax=6.6,DirectionChange=5)
	Kickback_WhenFalling = (UpBase=1.4,LateralBase=1.7,UpModifier=0.6,LateralModifier=0.1,UpMax=30,LateralMax=12,DirectionChange=5)
	Kickback_WhenDucking = (UpBase=0.49,LateralBase=0.68,UpModifier=0.21,LateralModifier=0.035,UpMax=10.5,LateralMax=4.2,DirectionChange=5)
	Kickback_WhenSteady = (UpBase=0.7,LateralBase=0.85,UpModifier=0.3,LateralModifier=0.05,UpMax=15,LateralMax=6,DirectionChange=5)
	
	Spread_WhenFalling = (param1=0,param2=0.252)
	Spread_WhenMoving = (param1=0.003,param2=0.084)
	Spread_WhenDucking = (param1=0.003,param2=0.021)
	Spread_WhenSteady = (param1=-0.001,param2=0.042)
	
	AccuracyDivisor  =  850
	AccuracyOffset  =  0.212
	MaxInaccuracy  =  1.43
	
	Kickback_UpLimit = 9
	Kickback_LateralLimit = 3.2



	Kickback_WhenMovingA = (UpBase=1,LateralBase=1.105,UpModifier=0.507,LateralModifier=0.065,UpMax=21.5,LateralMax=8.6,DirectionChange=5)
	Kickback_WhenFallingA = (UpBase=1.82,LateralBase=2.21,UpModifier=0.78,LateralModifier=0.13,UpMax=39,LateralMax=15.6,DirectionChange=5)
	Kickback_WhenDuckingA = (UpBase=0.49,LateralBase=0.68,UpModifier=0.21,LateralModifier=0.035,UpMax=10.5,LateralMax=4.2,DirectionChange=5)
	Kickback_WhenSteadyA = (UpBase=0.7,LateralBase=0.85,UpModifier=0.3,LateralModifier=0.05,UpMax=15,LateralMax=6,DirectionChange=5)
	
	Spread_WhenFallingA = (param1=0,param2=0.504)
	Spread_WhenMovingA = (param1=0.003,param2=0.168)
	Spread_WhenDuckingA = (param1=0.0027,param2=0.0189)
	Spread_WhenSteadyA = (param1=-0.0009,param2=0.0378)
	
	AccuracyDivisorA  =  2550
	AccuracyOffsetA  =  0.1908
	MaxInaccuracyA  =  1.716
	
	DirectionHold = 1
	FireIntervalMultiplierA = 1


	KickbackLimiter(0) = (Min=0.5,Max=0.8)
	KickbackLimiter(1) = (Min=0.6,Max=1)
	KickbackLimiter(2) = (Min=0.8,Max=1.1)
	KickbackLimiter(3) = (Min=1,Max=1.4)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.13667
	PutDownTime			=	0.1
	ReloadTime			=	2.8333
	SightInfos(0)			=	(FOV=90,ChangeTime=0.2)
	SightInfos(1) 			= (FOV=75,ChangeTime=0.15)

	bHideWeaponInSightMode = true
	bHideCursorInSightMode = true
	bReleaseZoomAfterFire = false

	AttachmentClass			=	class'avaAttachment_AUGA1'

	ScopeMeshName = "Wp_Scope.Com_Scope.MS_UIAUGa1"
	BaseSkelMeshName		=	"Wp_Rif_AUG_a1.MS_AUGa1"
	BaseAnimSetName			=	"Wp_Rif_AUG_a1.Ani_AUGA1"

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


	WeaponFireSnd=SoundCue'avaWeaponSounds.AR_AUG_A1.AR_AUG_A1_fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
