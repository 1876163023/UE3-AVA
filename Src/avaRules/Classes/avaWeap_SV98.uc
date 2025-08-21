//060824 TPG1복사하여 SV98제작

class avaWeap_SV98 extends avaWeap_BaseSniperRifle;

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

	FireInterval(0)=1.2
	FireInterval(1)=1.2
	FireInterval(2)=1.2

	ClipCnt=10
	AmmoCount=10
	MaxAmmoCount=30

	Penetration = 2
	RangeModifier=0.95

	SpreadDecayTime = 0.7

	Kickback_WhenMoving = (UpBase=18,LateralBase=0.525,UpModifier=0.27,LateralModifier=0.15,UpMax=11.25,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=18,LateralBase=0.525,UpModifier=0.27,LateralModifier=0.15,UpMax=11.25,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=9.6,LateralBase=0.28,UpModifier=0.144,LateralModifier=0.08,UpMax=6,LateralMax=4,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=12,LateralBase=0.35,UpModifier=0.18,LateralModifier=0.1,UpMax=7.5,LateralMax=5,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.165,param2=0.03)
	Spread_WhenMoving = (param1=0.033,param2=0.015)
	Spread_WhenDucking = (param1=0.0132,param2=0.0024)
	Spread_WhenSteady = (param1=0.0165,param2=0.003)
	
	AccuracyDivisor  =  500
	AccuracyOffset  =  0.3
	MaxInaccuracy  =  2



	Kickback_WhenMovingA = (UpBase=18,LateralBase=0.525,UpModifier=0.27,LateralModifier=0.15,UpMax=11.25,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=18,LateralBase=0.525,UpModifier=0.27,LateralModifier=0.15,UpMax=11.25,LateralMax=7.5,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=9.6,LateralBase=0.28,UpModifier=0.144,LateralModifier=0.08,UpMax=6,LateralMax=4,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=12,LateralBase=0.35,UpModifier=0.18,LateralModifier=0.1,UpMax=7.5,LateralMax=5,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.11,param2=0.03)
	Spread_WhenMovingA = (param1=0.022,param2=0.015)
	Spread_WhenDuckingA = (param1=0.0088,param2=0.0024)
	Spread_WhenSteadyA = (param1=0.011,param2=0.003)
	
	AccuracyDivisorA  =  500
	AccuracyOffsetA  =  0.15
	MaxInaccuracyA  =  1
	
	DirectionHold = 1
	FireIntervalMultiplierA = 1


	KickbackLimiter(0) = (Min=0.8,Max=1.2)

	bEjectBulletWhenFire = False

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.6
	BobDamping		=	0.7
	BobDampingInDash	=	0.5

	EquipTime			=0.967
	PutDownTime			=0.2333
	ReloadTime			=2.533

	LastFireAnim		= Fire_Last
	LastFireInterval =	0.3667

	SightInfos(0) = (FOV=90,ChangeTime=0.12)

	bHideWeaponInSightMode = true
	bHideCursorInSightMode = true
	bReleaseZoomAfterFire  = true
	fReleaseZoomAfterFireInterval = 0.08
	bRecoverZoomAfterFire  = true
	RecoverZoomTime        = 0.12

	AttachmentClass=class'avaAttachment_SV98'
	BaseSkelMeshName	=	"Wp_Sn_SV98.MS_Sn_SV98"
	BaseAnimSetName		=	"Wp_Sn_SV98.Ani_SV98"

	WeaponFireSnd=SoundCue'avaWeaponSounds.SR_SV98.SR_SV98_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	DefaultModifiers(0)			=	class'avaRules.avaMod_SV98_M_BaseScope'
//	DefaultModifiers(0)			=	class'avaRules.avaMod_SV98_M_HPScope'
	InstantHitDamageTypes(0)	=	class'avaDmgType_Sniper'
	
	ScopeMeshName = "avaScopeUI.Distortion.MS_TPGSniper_Scope_Mesh"
// 발사시 연기 삽입 테스트
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_Sniper_Muzzlesmoke_1P'
}
