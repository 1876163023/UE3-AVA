class avaWeap_VSS extends avaWeap_BaseSniperRifle;

defaultproperties
{
	BulletType=class'avaBullet_9x39MM'

	BaseSpeed		= 238	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage=80
	HitDamageS=80

	FireInterval(0)=0.3
	FireInterval(1)=0.3
	FireInterval(2)=0.3

	ClipCnt=10
	AmmoCount=10
	MaxAmmoCount=30

	Penetration = 2
	PenetrationS = 2

	RangeModifier=0.87

	SpreadDecayTime = 0.65

	Kickback_WhenMoving = (UpBase=4,LateralBase=3,UpModifier=0.75,LateralModifier=0.45,UpMax=52.5,LateralMax=18,DirectionChange=2)
	Kickback_WhenFalling = (UpBase=6,LateralBase=3.75,UpModifier=0.75,LateralModifier=0.45,UpMax=52.5,LateralMax=18,DirectionChange=2)
	Kickback_WhenDucking = (UpBase=2,LateralBase=2.5,UpModifier=0.4,LateralModifier=0.24,UpMax=28,LateralMax=9.6,DirectionChange=2)
	Kickback_WhenSteady = (UpBase=4,LateralBase=2.5,UpModifier=0.5,LateralModifier=0.3,UpMax=35,LateralMax=12,DirectionChange=2)
	
	Spread_WhenFalling = (param1=0.012,param2=0.8)
	Spread_WhenMoving = (param1=0,param2=0.18)
	Spread_WhenDucking = (param1=0.0042,param2=0.072)
	Spread_WhenSteady = (param1=0.006,param2=0.08)
	
	AccuracyDivisor  =  1500
	AccuracyOffset  =  0.3
	MaxInaccuracy  =  4
	
	Kickback_UpLimit = 20
	Kickback_LateralLimit = 4.5



	Kickback_WhenMovingA = (UpBase=4,LateralBase=3,UpModifier=0.75,LateralModifier=0.45,UpMax=52.5,LateralMax=18,DirectionChange=2)
	Kickback_WhenFallingA = (UpBase=6,LateralBase=3.75,UpModifier=0.75,LateralModifier=0.45,UpMax=52.5,LateralMax=18,DirectionChange=2)
	Kickback_WhenDuckingA = (UpBase=2,LateralBase=2.5,UpModifier=0.4,LateralModifier=0.24,UpMax=28,LateralMax=9.6,DirectionChange=2)
	Kickback_WhenSteadyA = (UpBase=4,LateralBase=2.5,UpModifier=0.5,LateralModifier=0.3,UpMax=35,LateralMax=12,DirectionChange=2)
	
	Spread_WhenFallingA = (param1=0.008,param2=0.8)
	Spread_WhenMovingA = (param1=0,param2=0.18)
	Spread_WhenDuckingA = (param1=0.0028,param2=0.072)
	Spread_WhenSteadyA = (param1=0.004,param2=0.08)
	
	AccuracyDivisorA  =  1500
	AccuracyOffsetA  =  0.1
	MaxInaccuracyA  =  2
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1

	KickbackLimiter(0) = (Min=0.7,Max=1.3)

	//기본적으로 소음기 장비
	bEnableSilencer = false
	bMountSilencer = true

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.6
	BobDamping		=	0.7
	BobDampingInDash	=	0.5

	EquipTime			=0.9333
	PutDownTime			=0.2333
	ReloadTime			=2.6667

	SightInfos(0) = (FOV=90,ChangeTime=0.15)

	bHideWeaponInSightMode = true
	bHideCursorInSightMode = true
	bReleaseZoomAfterFire = false

	bAutoFire	=	True

	WeaponFireAnim(0)	=Fire
	WeaponPutDownAnim	=Down
	WeaponEquipAnim		=BringUp
	WeaponReloadAnim	=Reload
	WeaponIdleAnims(0)	=Idle

	AttachmentClass=class'avaAttachment_GalilSniper'
	BaseSkelMeshName	=	"WP_sn_vss.MS_sn_vss"
	BaseAnimSetName		=	"WP_sn_vss.Ani_VSS"

	WeaponFireSnd=SoundCue'avaWeaponSounds.SR_GalilSniper.SR_GalilSniper_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
//스코프 추가
	DefaultModifiers(0)			=	class'avaRules.avaMod_GalilSniper_M_QSScope'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Sniper'

	ScopeMeshName = "avaScopeUI.Distortion.MS_GalilSniper_Scope_Mesh"
}
