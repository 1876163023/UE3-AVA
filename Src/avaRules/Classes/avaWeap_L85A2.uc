class avaWeap_L85A2 extends avaWeap_BaseRifle;

defaultproperties
{
	BulletType=class'avaBullet_556NATO'

	BaseSpeed		= 255	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.32	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage=36

	FireInterval(0)=0.09625

	ClipCnt=30
	AmmoCount=30
	MaxAmmoCount=90

	Penetration = 2
	RangeModifier=0.83

	SpreadDecayTime = 1.2

	Kickback_WhenMoving = (UpBase=0.88,LateralBase=0,UpModifier=0.11,LateralModifier=0.418,UpMax=19.8,LateralMax=11,DirectionChange=8)
	Kickback_WhenFalling = (UpBase=1.6,LateralBase=0,UpModifier=0.2,LateralModifier=0.76,UpMax=36,LateralMax=20,DirectionChange=8)
	Kickback_WhenDucking = (UpBase=0.56,LateralBase=0,UpModifier=0.07,LateralModifier=0.266,UpMax=12.6,LateralMax=7,DirectionChange=8)
	Kickback_WhenSteady = (UpBase=0.8,LateralBase=0,UpModifier=0.1,LateralModifier=0.38,UpMax=18,LateralMax=10,DirectionChange=8)
	
	Spread_WhenFalling = (param1=0,param2=0.182)
	Spread_WhenMoving = (param1=0,param2=0.06825)
	Spread_WhenDucking = (param1=0,param2=0.03185)
	Spread_WhenSteady = (param1=0,param2=0.0455)
	
	AccuracyDivisor  =  800
	AccuracyOffset  =  0.25
	MaxInaccuracy  =  0.6
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 3


	Kickback_WhenMovingA = (UpBase=1.14,LateralBase=0,UpModifier=0.143,LateralModifier=0.5434,UpMax=25.7,LateralMax=14.3,DirectionChange=8)
	Kickback_WhenFallingA = (UpBase=2.08,LateralBase=0,UpModifier=0.26,LateralModifier=0.988,UpMax=46.8,LateralMax=26,DirectionChange=8)
	Kickback_WhenDuckingA = (UpBase=0.56,LateralBase=0,UpModifier=0.07,LateralModifier=0.266,UpMax=12.6,LateralMax=7,DirectionChange=8)
	Kickback_WhenSteadyA = (UpBase=0.8,LateralBase=0,UpModifier=0.1,LateralModifier=0.38,UpMax=18,LateralMax=10,DirectionChange=8)
	
	Spread_WhenFallingA = (param1=0,param2=0.364)
	Spread_WhenMovingA = (param1=0,param2=0.1365)
	Spread_WhenDuckingA = (param1=0,param2=0.028665)
	Spread_WhenSteadyA = (param1=0,param2=0.04095)
	
	AccuracyDivisorA  =  2400
	AccuracyOffsetA  =  0.225
	MaxInaccuracyA  =  0.72
	
	DirectionHold = 2
	FireIntervalMultiplierA = 1.3

	KickbackLimiter(0) = (Min=0.8,Max=1)
	KickbackLimiter(1) = (Min=1,Max=1)


	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.2
	PutDownTime			=	0.333
	ReloadTime			=	2.533

	SightInfos(0) = (FOV=90,ChangeTime=0.2)
	SightInfos(1) = (FOV=75,ChangeTime=0.15)

	bHideWeaponInSightMode = true
	bHideCursorInSightMode = true
	bReleaseZoomAfterFire = false
	
	ScopeMeshName = "Wp_Scope.Com_Scope.MS_UIG3601"
	BaseSkelMeshName	=	"Wp_Rif_G36.Wp_Rif_G36_Scope.MS_G36_Scope"
	BaseAnimSetName		=	"Wp_Rif_G36.Ani_G36"

	FireAnimInfos(0)		=	(AnimName=Fire1,FirstShotRate=1.0,OtherShotRate=0.0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(2)		=	(AnimName=Fire3,FirstShotRate=0.0,OtherShotRate=1.0)
	FireAnimInfos(3)		=	(AnimName=Fire4,FirstShotRate=0.0,OtherShotRate=1.0)
	WeaponFireAnim(0)		=	Fire1
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire3
	WeaponFireAnim(3)		=	Fire4
	AttachmentClass=class'avaAttachment_G36'
	WeaponFireSnd=SoundCue'avaWeaponSounds.AR_G36.AR_G36_Fire'
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
