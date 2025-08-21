class avaWeap_KnightsSR25 extends avaWeap_BaseSniperRifle;

defaultproperties
{
	BulletType=class'avaBullet_762NATO'

	BaseSpeed		= 232	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage=65

	FireInterval(0)=0.4
	FireInterval(1)=0.4
	FireInterval(2)=0.4

	ClipCnt=20
	AmmoCount=20
	MaxAmmoCount=60

	Penetration = 2
	RangeModifier=0.9

	SpreadDecayTime = 0.5

	Kickback_WhenMoving = (UpBase=7.5,LateralBase=1.8,UpModifier=1.5,LateralModifier=0.9,UpMax=30,LateralMax=22.5,DirectionChange=1)
	Kickback_WhenFalling = (UpBase=7.5,LateralBase=1.8,UpModifier=1.5,LateralModifier=0.9,UpMax=30,LateralMax=22.5,DirectionChange=1)
	Kickback_WhenDucking = (UpBase=4,LateralBase=0.96,UpModifier=0.8,LateralModifier=0.48,UpMax=16,LateralMax=12,DirectionChange=1)
	Kickback_WhenSteady = (UpBase=5,LateralBase=1.2,UpModifier=1,LateralModifier=0.6,UpMax=20,LateralMax=15,DirectionChange=1)
	
	Spread_WhenFalling = (param1=-0.03,param2=0.8)
	Spread_WhenMoving = (param1=0,param2=0.24)
	Spread_WhenDucking = (param1=-0.03,param2=0.072)
	Spread_WhenSteady = (param1=-0.03,param2=0.08)
	
	AccuracyDivisor  =  800
	AccuracyOffset  =  0.5
	MaxInaccuracy  =  3
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 3

	Kickback_WhenMovingA = (UpBase=7.5,LateralBase=1.8,UpModifier=1.5,LateralModifier=0.9,UpMax=30,LateralMax=22.5,DirectionChange=1)
	Kickback_WhenFallingA = (UpBase=7.5,LateralBase=1.8,UpModifier=1.5,LateralModifier=0.9,UpMax=30,LateralMax=22.5,DirectionChange=1)
	Kickback_WhenDuckingA = (UpBase=4,LateralBase=0.96,UpModifier=0.8,LateralModifier=0.48,UpMax=16,LateralMax=12,DirectionChange=1)
	Kickback_WhenSteadyA = (UpBase=5,LateralBase=1.2,UpModifier=1,LateralModifier=0.6,UpMax=20,LateralMax=15,DirectionChange=1)
	
	Spread_WhenFallingA = (param1=-0.03,param2=0.8)
	Spread_WhenMovingA = (param1=0,param2=0.24)
	Spread_WhenDuckingA = (param1=-0.03,param2=0.072)
	Spread_WhenSteadyA = (param1=-0.03,param2=0.08)
	
	AccuracyDivisorA  =  800
	AccuracyOffsetA  =  0.5
	MaxInaccuracyA  =  3
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1
	
	KickbackLimiter(0) = (Min=0.5,Max=1.5)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.6
	BobDamping		=	0.7
	BobDampingInDash	=	0.5

	EquipTime			=1.333
	PutDownTime			=0.233
	ReloadTime			=3.8333

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

	AttachmentClass=class'avaAttachment_MSG90A1'
	BaseSkelMeshName	=	"WP_Sn_MSG90A1.MS_MSG90A1"
	BaseAnimSetName		=	"WP_Sn_MSG90A1.Ani_MSG90A1"

	WeaponFireSnd			=	SoundCue'avaWeaponSounds.SR_MSG90A1.SR_MSG90A1_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
//스코프 추가
	DefaultModifiers(0)			=	class'avaRules.avaMod_MSG90A1_M_ACOG'
	InstantHitDamageTypes(0)	=	class'avaDmgType_Sniper'
	ScopeMeshName = "avaScopeUI.Distortion.MS_GalilSniper_Scope_Mesh"
}
