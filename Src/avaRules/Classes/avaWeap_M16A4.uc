class avaWeap_M16A4 extends avaWeap_BaseRifle;

defaultproperties
{

	BulletType				=	class'avaBullet_556NATO'	

	BaseSpeed		= 250	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.32	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage			=	35
	FireInterval(0)			=	0.1012
	
	ClipCnt			=	30
	AmmoCount			=	30
	MaxAmmoCount			=	90

	Penetration			=	2
	RangeModifier			=	0.945

	AccuracyDivisor			=	1200
	AccuracyOffset			=	0.3
	MaxInaccuracy			=	1.1

	SpreadDecayTime = 0.5


	Kickback_WhenMoving = (UpBase=1.1,LateralBase=0.66,UpModifier=0.088,LateralModifier=0.099,UpMax=18.7,LateralMax=6.6,DirectionChange=7)
	Kickback_WhenFalling = (UpBase=2,LateralBase=1.2,UpModifier=0.16,LateralModifier=0.18,UpMax=34,LateralMax=12,DirectionChange=7)
	Kickback_WhenDucking = (UpBase=0.7,LateralBase=0.42,UpModifier=0.056,LateralModifier=0.063,UpMax=11.9,LateralMax=4.2,DirectionChange=7)
	Kickback_WhenSteady = (UpBase=1,LateralBase=0.6,UpModifier=0.08,LateralModifier=0.09,UpMax=17,LateralMax=6,DirectionChange=7)
	
	Spread_WhenFalling = (param1=0.025,param2=0.11)
	Spread_WhenMoving = (param1=0.008,param2=0.033)
	Spread_WhenDucking = (param1=0.0035,param2=0.0154)
	Spread_WhenSteady = (param1=0.005,param2=0.022)


	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.167
	PutDownTime			=	0.233
	ReloadTime			=	2.8333
	SightInfos(0)			=	(FOV=90,ChangeTime=0.2)
	SightInfos(1) = (FOV=75,ChangeTime=0.15)

	bHideWeaponInSightMode = false
	bHideCursorInSightMode = false
	ScopeMeshName          = ""
	SightInAnim            = ""
	SightOutAnim           = ""
	bReleaseZoomAfterFire = false

	AttachmentClass			=	class'avaAttachment_M16A4'

	BaseSkelMeshName		=	"Wp_Rif_M4.Wp_Rif_M4_Rail.MS_M4_Rail"
	BaseAnimSetName			=	"Wp_Rif_M4.Ani_M4A1_1P"

	FireAnimInfos(0)		=	(AnimName=Fire3,FirstShotRate=0.2,OtherShotRate=0)
	FireAnimInfos(1)		=	(AnimName=Fire2,FirstShotRate=0.8,OtherShotRate=0.3)
	FireAnimInfos(2)		=	(AnimName=Fire1,FirstShotRate=0.0,OtherShotRate=0.7)
	WeaponFireAnim(0)		=	Fire3
	WeaponFireAnim(1)		=	Fire2
	WeaponFireAnim(2)		=	Fire1
 	WeaponPutDownAnim		=	Down
	WeaponEquipAnim			=	BringUp
	WeaponReloadAnim		=	Reload

	WeaponIdleAnims(0)		=	Idle
	HudMaterial			=	Texture2D'avaDotSightUI.Textures.M4_dotsight'
	WeaponFireSnd			=	SoundCue'avaWeaponSounds.AR_M4A1.AR_M4A1_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
