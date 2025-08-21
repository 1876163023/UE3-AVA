class avaWeap_DesertEagle extends avaWeap_BasePistol;

defaultproperties
{
	BulletType=class'avaBullet_50AE'

	BaseSpeed		= 260	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage = 52

	FireInterval(0)=0.25
	ClipCnt=7
	AmmoCount=7
	MaxAmmoCount=21

	Penetration = 2
	RangeModifier=0.68

	SpreadDecayTime = 0.8

	Kickback_WhenMoving = (UpBase=14.4,LateralBase=2.64,UpModifier=0.024,LateralModifier=0.6,UpMax=72,LateralMax=9.6,DirectionChange=1)
	Kickback_WhenFalling = (UpBase=18,LateralBase=3.3,UpModifier=0.03,LateralModifier=0.75,UpMax=90,LateralMax=12,DirectionChange=1)
	Kickback_WhenDucking = (UpBase=9.6,LateralBase=1.76,UpModifier=0.016,LateralModifier=0.4,UpMax=48,LateralMax=6.4,DirectionChange=1)
	Kickback_WhenSteady = (UpBase=12,LateralBase=2.2,UpModifier=0.02,LateralModifier=0.5,UpMax=60,LateralMax=8,DirectionChange=1)
	
	Spread_WhenFalling = (param1=0.02,param2=0.072)
	Spread_WhenMoving = (param1=0.011,param2=0.066)
	Spread_WhenDucking = (param1=0.009,param2=0.054)
	Spread_WhenSteady = (param1=0.01,param2=0.06)
	
	AccuracyDivisor  =  350
	AccuracyOffset  =  0.3
	MaxInaccuracy  =  1.75
	
	Kickback_UpLimit = 20
	Kickback_LateralLimit = 10



	Kickback_WhenMovingA = (UpBase=14.4,LateralBase=2.64,UpModifier=0.024,LateralModifier=0.6,UpMax=26.4,LateralMax=6.6,DirectionChange=2)
	Kickback_WhenFallingA = (UpBase=18,LateralBase=3.3,UpModifier=0.03,LateralModifier=0.75,UpMax=33,LateralMax=8.25,DirectionChange=2)
	Kickback_WhenDuckingA = (UpBase=9.6,LateralBase=1.76,UpModifier=0.016,LateralModifier=0.4,UpMax=17.6,LateralMax=4.4,DirectionChange=2)
	Kickback_WhenSteadyA = (UpBase=12,LateralBase=2.2,UpModifier=0.02,LateralModifier=0.5,UpMax=22,LateralMax=5.5,DirectionChange=2)
	
	Spread_WhenFallingA = (param1=0.02,param2=0.072)
	Spread_WhenMovingA = (param1=0.011,param2=0.066)
	Spread_WhenDuckingA = (param1=0.009,param2=0.054)
	Spread_WhenSteadyA = (param1=0.01,param2=0.06)
	
	AccuracyDivisorA  =  350
	AccuracyOffsetA  =  0.3
	MaxInaccuracyA  =  1.75
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1.2



	KickbackLimiter(0) = (Min=0.8,Max=1.2)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.6
	BobDampingInDash	=	0.4

	EquipTime			=0.8333
	PutDownTime			=0.3333
	ReloadTime			=2.6667

	AttachmentClass=class'avaAttachment_DesertEagle'

	BaseSkelMeshName	=	"Wp_Pis_DesertEG.MS_DesertEG"
	BaseAnimSetName		=	"Wp_Pis_DesertEG.Ani_DesertEG"

	WeaponFireSnd=SoundCue'avaWeaponSounds.SR_GalilSniper.SR_GalilSniper_Fire'	

	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'
	InstantHitDamageTypes(0)	=	class'avaDmgType_Pistol'

// 발사시 연기 삽입 테스트
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_Sniper_Muzzlesmoke_1P'
//스나이퍼 머즐 붙이기
	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_SniperRifle_MuzzleFlash_1P'	
	MuzzleFlashColor=(R=255,G=120,B=255,A=255)
	MuzzleFlashDuration=0.33
	MuzzleFlashLightClass=class'avaGame.avaGunMuzzleFlashLight'
}
