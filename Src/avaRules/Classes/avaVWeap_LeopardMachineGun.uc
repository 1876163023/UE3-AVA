class avaVWeap_LeopardMachineGun extends avaVehicleWeapon
	HideDropDown;

simulated state WeaponReloading
{
	simulated function StartReload( optional bool bFirstReload = true )
	{
		local float			RealReloadTime;
		ReleasedSightMode	= 0;
		bForceReload		= false;
		RealReloadTime		= ReloadTime;
		//PlayWeaponAnimation( WeaponReloadAnim, RealReloadTime );
		SetTimer( RealReloadTime, false, 'ReloadDone');
		// Play Reload Animation...
		if ( Role == ROLE_Authority )
		{
			avaPawn( avaWeaponPawn( Instigator ).Driver ).PlayReloadAnimation(EBT_Reload);
			if ( avaVehicle_Leopard(MyVehicle) != None )
				avaVehicle_Leopard(MyVehicle).PlayReloadAnimation( );
		}
	}
}

DefaultProperties
{
	BulletType		= class'avaBullet_762NATO'

	BaseSpeed		= 248	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.32	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage		=	55
	FireInterval(0)		=	0.095

	ClipCnt			= 100
	AmmoCount		= 100
	MaxAmmoCount		= 200

	Penetration		=	2
	RangeModifier		=	0.95

	SpreadDecayTime = 0.3

	Kickback_WhenMoving = (UpBase=0,LateralBase=1.4,UpModifier=0,LateralModifier=0,UpMax=6,LateralMax=1.5,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=0,LateralBase=1.4,UpModifier=0,LateralModifier=0,UpMax=6,LateralMax=1.5,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=0,LateralBase=0.35,UpModifier=0,LateralModifier=0,UpMax=1.5,LateralMax=1.5,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=0,LateralBase=0.7,UpModifier=0,LateralModifier=0,UpMax=3,LateralMax=1.5,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.025,param2=0.02)
	Spread_WhenMoving = (param1=0.025,param2=0.02)
	Spread_WhenDucking = (param1=0.025,param2=0.02)
	Spread_WhenSteady = (param1=0.025,param2=0.02)
	
	AccuracyDivisor  =  2100
	AccuracyOffset  =  0.255
	MaxInaccuracy  =  0
	
	Kickback_UpLimit = 5
	Kickback_LateralLimit = 2


	Kickback_WhenMovingA = (UpBase=0,LateralBase=1.82,UpModifier=0,LateralModifier=0,UpMax=7.8,LateralMax=1.5,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=0,LateralBase=1.82,UpModifier=0,LateralModifier=0,UpMax=7.8,LateralMax=1.5,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=0,LateralBase=0.35,UpModifier=0,LateralModifier=0,UpMax=1.5,LateralMax=1.5,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=0,LateralBase=0.7,UpModifier=0,LateralModifier=0,UpMax=3,LateralMax=1.5,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.025,param2=0.02)
	Spread_WhenMovingA = (param1=0.025,param2=0.02)
	Spread_WhenDuckingA = (param1=0.025,param2=0.02)
	Spread_WhenSteadyA = (param1=0.025,param2=0.02)
	
	AccuracyDivisorA  =  2100
	AccuracyOffsetA  =  0.255
	MaxInaccuracyA  =  0
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1

	KickbackLimiter(0) = (Min=0.5,Max=1.5)
	
	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash	=	0.45

	EquipTime			=	1.367
	PutDownTime			=	0.1
	ReloadTime			=	2.367

	SightInfos(0) = (FOV=90,ChangeTime=0.2)

	bHideWeaponInSightMode = false
	bHideCursorInSightMode = false
	ScopeMeshName          = ""
	SightInAnim            = ""
	SightOutAnim           = ""
	bReleaseZoomAfterFire = false

	AttachmentClass			=	class'avaAttachment_VLeopardMachineGun'
	WeaponFireSnd			=	SoundCue'avaWeaponSounds.Leopard_MG3.Leopard_MG3_Fire'	
	//PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_MachineGun'

	bInstantHit=true

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_Heavy_MuzzleFlash_1P_Tank'	
	MuzzleFlashDuration=0.33	

	DamageCode=Gun

	bAutoFire			=	true
	bInfinityAmmo		=	true
}