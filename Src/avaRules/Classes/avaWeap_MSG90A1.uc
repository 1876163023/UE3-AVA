class avaWeap_MSG90A1 extends avaWeap_BaseSniperRifle;

defaultproperties
{
	BulletType=class'avaBullet_762NATO'

	BaseSpeed		= 228	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage=82

	FireInterval(0)=0.528
	FireInterval(1)=0.528
	FireInterval(2)=0.528

	ClipCnt=5
	AmmoCount=5
	MaxAmmoCount=30

	Penetration = 2
	RangeModifier=0.96

	SpreadDecayTime = 1.2

	Kickback_WhenMoving = (UpBase=9.75,LateralBase=1.8,UpModifier=0.75,LateralModifier=0.45,UpMax=45,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=9.75,LateralBase=1.8,UpModifier=0.75,LateralModifier=0.45,UpMax=45,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=5.2,LateralBase=0.96,UpModifier=0.4,LateralModifier=0.24,UpMax=24,LateralMax=4,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=6.5,LateralBase=1.2,UpModifier=0.5,LateralModifier=0.3,UpMax=30,LateralMax=5,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.48,param2=0.2)
	Spread_WhenMoving = (param1=0.048,param2=0.3)
	Spread_WhenDucking = (param1=0.0096,param2=0.016)
	Spread_WhenSteady = (param1=0.012,param2=0.02)
	
	AccuracyDivisor  =  150
	AccuracyOffset  =  0.6
	MaxInaccuracy  =  6
	
	Kickback_UpLimit = 15
	Kickback_LateralLimit = 6


	Kickback_WhenMovingA = (UpBase=9.75,LateralBase=1.8,UpModifier=0.75,LateralModifier=0.45,UpMax=45,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=9.75,LateralBase=1.8,UpModifier=0.75,LateralModifier=0.45,UpMax=45,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=5.2,LateralBase=0.96,UpModifier=0.4,LateralModifier=0.24,UpMax=24,LateralMax=4,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=6.5,LateralBase=1.2,UpModifier=0.5,LateralModifier=0.3,UpMax=30,LateralMax=5,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.32,param2=0.2)
	Spread_WhenMovingA = (param1=0.032,param2=0.3)
	Spread_WhenDuckingA = (param1=0.0064,param2=0.016)
	Spread_WhenSteadyA = (param1=0.008,param2=0.02)
	
	AccuracyDivisorA  =  150
	AccuracyOffsetA  =  0.2
	MaxInaccuracyA  =  3
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1


	KickbackLimiter(0) = (Min=0.8,Max=1.5)


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

// 발사시 연기 삽입 테스트
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_Sniper_Muzzlesmoke_1P'
}
