class avaWeap_M82A1 extends avaWeap_BaseSniperRifle;

defaultproperties
{
	BulletType=class'avaBullet_50BMG'

	BaseSpeed		= 210	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치


	HitDamage=250

	FireInterval(0)=0.715
	FireInterval(1)=0.715
	FireInterval(2)=0.715

	ClipCnt=5
	AmmoCount=5
	MaxAmmoCount=15

	Penetration = 3
	RangeModifier=0.995

	SpreadDecayTime = 1.5

	Kickback_WhenMoving = (UpBase=21.75,LateralBase=3.75,UpModifier=0.75,LateralModifier=0.45,UpMax=120,LateralMax=9,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=21.75,LateralBase=3.75,UpModifier=0.75,LateralModifier=0.45,UpMax=120,LateralMax=9,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=11.6,LateralBase=2,UpModifier=0.4,LateralModifier=0.24,UpMax=64,LateralMax=4.8,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=14.5,LateralBase=2.5,UpModifier=0.5,LateralModifier=0.3,UpMax=80,LateralMax=6,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.45,param2=0.6)
	Spread_WhenMoving = (param1=0.45,param2=0.48)
	Spread_WhenDucking = (param1=0.0045,param2=0.024)
	Spread_WhenSteady = (param1=0.045,param2=0.06)
	
	AccuracyDivisor  =  500
	AccuracyOffset  =  0.4
	MaxInaccuracy  =  4
	
	Kickback_UpLimit = 40
	Kickback_LateralLimit = 10



	Kickback_WhenMovingA = (UpBase=21.75,LateralBase=3.75,UpModifier=0.75,LateralModifier=0.45,UpMax=45,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=21.75,LateralBase=3.75,UpModifier=0.75,LateralModifier=0.45,UpMax=45,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=11.6,LateralBase=2,UpModifier=0.4,LateralModifier=0.24,UpMax=24,LateralMax=4,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=14.5,LateralBase=2.5,UpModifier=0.5,LateralModifier=0.3,UpMax=30,LateralMax=5,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.3,param2=0.6)
	Spread_WhenMovingA = (param1=0.3,param2=0.48)
	Spread_WhenDuckingA = (param1=0.003,param2=0.024)
	Spread_WhenSteadyA = (param1=0.03,param2=0.06)
	
	AccuracyDivisorA  =  500
	AccuracyOffsetA  =  0.2
	MaxInaccuracyA  =  2
	
	DirectionHold = 1
	FireIntervalMultiplierA = 1


	KickbackLimiter(0) = (Min=0.8,Max=1.2)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.6
	BobDamping		=	0.7
	BobDampingInDash	=	0.5

	EquipTime			=1.667
	PutDownTime			=0.2333
	ReloadTime			=3.833

	SightInfos(0) = (FOV=90,ChangeTime=0.2)

	bHideWeaponInSightMode = true
	bHideCursorInSightMode = true
	bReleaseZoomAfterFire = false

	AttachmentClass=class'avaAttachment_M82A1'

	BaseSkelMeshName	=	"Wp_sn_TPG1.Wp_Sn_TPG1_Rail.MS_TPG1"
	BaseAnimSetName		=	"Wp_sn_TPG1.Ani_TPG_1"

	WeaponFireSnd			=	SoundCue'avaWeaponSounds.SR_M24.SR_M24_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	DefaultModifiers(0)			=	class'avaRules.avaMod_TPG1_M_Scope'
	InstantHitDamageTypes(0)	=	class'avaDmgType_Sniper'
	
	ScopeMeshName = "avaScopeUI.Distortion.MS_TPGSniper_Scope_Mesh"
}
