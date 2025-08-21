class avaWeap_Beretta92FS extends avaWeap_BasePistol;

defaultproperties
{
	BulletType=class'avaBullet_9MM'

	BaseSpeed		= 265	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage = 34
	HitDamageS= 32

	FireInterval(0)=0.11

	ClipCnt=15
	AmmoCount=15
	MaxAmmoCount=30

	Penetration = 2
	PenetrationS = 1

	RangeModifier=0.66
	RangeModifierS=0.61

	SpreadDecayTime = 0.6

	Kickback_WhenMoving = (UpBase=2.94,LateralBase=0.7,UpModifier=0.42,LateralModifier=0.028,UpMax=18.2,LateralMax=6.3,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=3.15,LateralBase=0.75,UpModifier=0.45,LateralModifier=0.03,UpMax=19.5,LateralMax=6.75,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=1.68,LateralBase=0.4,UpModifier=0.24,LateralModifier=0.016,UpMax=10.4,LateralMax=3.6,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=2.1,LateralBase=0.5,UpModifier=0.3,LateralModifier=0.02,UpMax=13,LateralMax=4.5,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.0024,param2=0.0384)
	Spread_WhenMoving = (param1=0.00132,param2=0.0352)
	Spread_WhenDucking = (param1=0.00108,param2=0.0288)
	Spread_WhenSteady = (param1=0.0012,param2=0.032)
	
	AccuracyDivisor  =  500
	AccuracyOffset  =  0.4
	MaxInaccuracy  =  1.8
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 10


	Kickback_WhenMovingA = (UpBase=2.94,LateralBase=0.7,UpModifier=0.42,LateralModifier=0.028,UpMax=18.2,LateralMax=6.3,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=3.15,LateralBase=0.75,UpModifier=0.45,LateralModifier=0.03,UpMax=19.5,LateralMax=6.75,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=1.68,LateralBase=0.4,UpModifier=0.24,LateralModifier=0.016,UpMax=10.4,LateralMax=3.6,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=2.1,LateralBase=0.5,UpModifier=0.3,LateralModifier=0.02,UpMax=13,LateralMax=4.5,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.0024,param2=0.0768)
	Spread_WhenMovingA = (param1=0.00132,param2=0.0704)
	Spread_WhenDuckingA = (param1=0.000972,param2=0.02592)
	Spread_WhenSteadyA = (param1=0.00108,param2=0.0288)
	
	AccuracyDivisorA  =  2500
	AccuracyOffsetA  =  0.32
	MaxInaccuracyA  =  1.8
	
	DirectionHold = 1
	FireIntervalMultiplierA = 1.2

	KickbackLimiter(0) = (Min=0.8,Max=1.2)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.6
	BobDampingInDash	=	0.4

	EquipTime			=0.6333
	PutDownTime			=0.3333
	ReloadTime			=2.3333

	SightInfos(0) =	(FOV=90,ChangeTime=0.2)
	SightInfos(1) = (FOV=75,ChangeTime=0.15)

	bHideWeaponInSightMode = false
	bHideCursorInSightMode = false
	ScopeMeshName          = ""
	SightInAnim            = ""
	SightOutAnim           = ""
	bReleaseZoomAfterFire = false

	AttachmentClass=class'avaAttachment_Beretta92FS'

	bEnableSilencer		= True
	SilencerMeshName		= "Wp_Silencer.MS_P226_Silencer"
	SilencerBoneName		= Bone01
	MountSilencerAnim		= Sil_In	// 소음기 장착 Animation 이름
	UnMountSilencerAnim	= Sil_Out	// 소음기 탈착 Animation 이름
	MountSilencerTime		= 2.000	// 소음기 장착시 걸리는 시간
	UnMountSilencerTime	= 2.000	// 소음기 탈착시 걸리는 시간


	BaseSkelMeshName	=	"Wp_Pis_Beretta92Fs.MS_Pis_Bereta92Fs"
	BaseAnimSetName		=	"Wp_Pis_Beretta92Fs.Ani_Beretta"
	WeaponFireSnd=SoundCue'avaWeaponSounds.Pistol_Bereta92f.Pistol_Bereta92f_fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Pistol'
	WeaponSilencerFireSnd	= SoundCue'avaWeaponSounds.Common.Silencer.Silencer_Fire_SMG'
}