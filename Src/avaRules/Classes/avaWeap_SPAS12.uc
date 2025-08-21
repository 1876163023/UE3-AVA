class avaWeap_SPAS12 extends avaWeap_BaseShotGun;

defaultproperties
{
	BulletType=class'avaBullet_ShotgunShell'

	BaseSpeed		= 230	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	HitDamage=39

	FireInterval(0)=0.40

	ClipCnt=5
	AmmoCount=5
	MaxAmmoCount=30

	Penetration = 1
	RangeModifier=0.43

	SpreadDecayTime = 0.6

	Kickback_WhenMoving = (UpBase=21.6,LateralBase=4.8,UpModifier=0.6,LateralModifier=1.2,UpMax=60,LateralMax=8.4,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=27,LateralBase=6,UpModifier=0.75,LateralModifier=1.5,UpMax=75,LateralMax=10.5,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=14.4,LateralBase=3.2,UpModifier=0.4,LateralModifier=0.8,UpMax=40,LateralMax=5.6,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=18,LateralBase=4,UpModifier=0.5,LateralModifier=1,UpMax=50,LateralMax=7,DirectionChange=3)
	
	Spread_WhenFalling = (param1=-0.03,param2=0.17)
	Spread_WhenMoving = (param1=-0.03,param2=0.17)
	Spread_WhenDucking = (param1=-0.03,param2=0.17)
	Spread_WhenSteady = (param1=-0.03,param2=0.17)
	
	AccuracyDivisor  =  200
	AccuracyOffset  =  0.6
	MaxInaccuracy  =  4
	
	Kickback_UpLimit = 40
	Kickback_LateralLimit = 15


	Kickback_WhenMovingA = (UpBase=26.4,LateralBase=3.6,UpModifier=0.6,LateralModifier=1.2,UpMax=60,LateralMax=8.4,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=33,LateralBase=4.5,UpModifier=0.75,LateralModifier=1.5,UpMax=75,LateralMax=10.5,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=17.6,LateralBase=2.4,UpModifier=0.4,LateralModifier=0.8,UpMax=40,LateralMax=5.6,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=22,LateralBase=3,UpModifier=0.5,LateralModifier=1,UpMax=50,LateralMax=7,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=-0.042,param2=0.17)
	Spread_WhenMovingA = (param1=-0.042,param2=0.17)
	Spread_WhenDuckingA = (param1=-0.042,param2=0.17)
	Spread_WhenSteadyA = (param1=-0.042,param2=0.17)
	
	AccuracyDivisorA  =  200
	AccuracyOffsetA  =  0.6
	MaxInaccuracyA  =  5
	
	DirectionHold = 0
	FireIntervalMultiplierA = 2


	KickbackLimiter(0) = (Min=0.8,Max=1.2)

	bAutofire	= True

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.40
	PutDownTime			=	0.333
	ReloadTime			=	0.7

	SightInfos(0) = (FOV=90,ChangeTime=0.3)
	SightInfos(1) = (FOV=75,ChangeTime=0.15)

	bHideWeaponInSightMode = false
	bHideCursorInSightMode = false
	ScopeMeshName          = ""
	SightInAnim            = ""
	SightOutAnim           = ""
	bReleaseZoomAfterFire = false

//머즐 플래쉬 셋팅
	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_ShotGun_MuzzleFlash_1P'
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_ShotGun_Muzzlesmoke_1P'
	MuzzleFlashDuration=0.33	

	Begin Object Name=BulletTrailComponent0
		HalfLife	=	0.01
		Intensity	=	41.40
		Size		=	1.45
		Speed		=	6500
	End Object
	TrailInterval	=	2


	BaseSkelMeshName	=	"WP_Shot_Spas12.MS_Spas12"
	BaseAnimSetName		=	"WP_Shot_Spas12.Ani_SPAS12"

	AttachmentClass=class'avaAttachment_SPAS12'
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'

	WeaponFireSnd=SoundCue'avaWeaponSounds.SG_Spas12.SG_Spas12_Fire'	

	bReloadClip = false
	NumFiresPerShot = 8
	WeaponType	= WEAPON_SHOTGUN
	InventoryGroup	= 1

	bEjectBulletWhenFire = False

	bEnableFireWhenReload	=	true
	WeaponPreReloadAnim		=	PreReload
	PreReloadTime			=	0.5
	WeaponReloadAnim		=	Reload
	WeaponPostReloadAnim		=	PostReload
	PostReloadTime			=	0.8667

	FireAnimInfos.Empty
	FireAnimInfos(0)		=	(AnimName=Fire,FirstShotRate=1.0,OtherShotRate=1.0)

}
