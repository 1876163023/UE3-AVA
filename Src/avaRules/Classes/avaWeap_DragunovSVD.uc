class avaWeap_DragunovSVD extends avaWeap_BaseSniperRifle;

defaultproperties
{
	BulletType=class'avaBullet_762x54R'

	BaseSpeed		= 232	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage=105

	FireInterval(0)=0.48
	FireInterval(1)=0.48
	FireInterval(2)=0.48

	ClipCnt=10
	AmmoCount=10
	MaxAmmoCount=20

	Penetration = 2

	RangeModifier=0.91

	SpreadDecayTime = 0.6

	Kickback_WhenMoving = (UpBase=7,LateralBase=3.6,UpModifier=0.75,LateralModifier=0.45,UpMax=52.5,LateralMax=18,DirectionChange=1)
	Kickback_WhenFalling = (UpBase=10.5,LateralBase=4.5,UpModifier=0.75,LateralModifier=0.45,UpMax=52.5,LateralMax=18,DirectionChange=1)
	Kickback_WhenDucking = (UpBase=3.5,LateralBase=3,UpModifier=0.4,LateralModifier=0.24,UpMax=28,LateralMax=9.6,DirectionChange=1)
	Kickback_WhenSteady = (UpBase=7,LateralBase=3,UpModifier=0.5,LateralModifier=0.3,UpMax=35,LateralMax=12,DirectionChange=1)
	
	Spread_WhenFalling = (param1=0.0375,param2=0.75)
	Spread_WhenMoving = (param1=0,param2=0.2)
	Spread_WhenDucking = (param1=0.0075,param2=0.075)
	Spread_WhenSteady = (param1=0.0075,param2=0.075)
	
	AccuracyDivisor  =  1500
	AccuracyOffset  =  0.3
	MaxInaccuracy  =  2
	
	Kickback_UpLimit = 20
	Kickback_LateralLimit = 5


	Kickback_WhenMovingA = (UpBase=7,LateralBase=3.6,UpModifier=0.75,LateralModifier=0.45,UpMax=52.5,LateralMax=18,DirectionChange=1)
	Kickback_WhenFallingA = (UpBase=10.5,LateralBase=4.5,UpModifier=0.75,LateralModifier=0.45,UpMax=52.5,LateralMax=18,DirectionChange=1)
	Kickback_WhenDuckingA = (UpBase=3.5,LateralBase=3,UpModifier=0.4,LateralModifier=0.24,UpMax=28,LateralMax=9.6,DirectionChange=1)
	Kickback_WhenSteadyA = (UpBase=7,LateralBase=3,UpModifier=0.5,LateralModifier=0.3,UpMax=35,LateralMax=12,DirectionChange=1)
	
	Spread_WhenFallingA = (param1=0.025,param2=0.75)
	Spread_WhenMovingA = (param1=0,param2=0.2)
	Spread_WhenDuckingA = (param1=0.005,param2=0.075)
	Spread_WhenSteadyA = (param1=0.005,param2=0.075)
	
	AccuracyDivisorA  =  1500
	AccuracyOffsetA  =  0.1
	MaxInaccuracyA  =  2
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1


	KickbackLimiter(0) = (Min=0.5,Max=1.65)


	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.6
	BobDamping		=	0.7
	BobDampingInDash	=	0.5

	EquipTime			=1.0333
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

	AttachmentClass=class'avaAttachment_DragunovSVD'
	BaseSkelMeshName	=	"Wp_sn_SVD.MS_SVD"
	BaseAnimSetName		=	"Wp_sn_SVD.Ani_SVD"

	WeaponFireSnd=SoundCue'avaWeaponSounds.SR_SVD.SR_SVD_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
//스코프 추가
	DefaultModifiers(0)			=	class'avaRules.avaMod_DragunovSVD_M_Scope'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Sniper'

	ScopeMeshName = "avaScopeUI.Distortion.MS_GalilSniper_Scope_Mesh"

// 발사시 연기 삽입 테스트
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_Sniper_Muzzlesmoke_1P'
}
