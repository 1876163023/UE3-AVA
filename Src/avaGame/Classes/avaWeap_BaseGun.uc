/**
 * Copyright 2006 AVA.

 Counter-strike의 판정 기준을 데리고 왔습니다.
 여러 firemode가 있다고 가정하였습니다. :)

 UT와 다른 점은; FireMode를 PC에서 준 대로 사용하지 않고 Weapon에 있다는 점입니다. 이것은 단발/점사/연사 설정 스위치가 총에 있기 때문에
 제대로 되었다고 판단됩니다. UT의 경우, 두 가지 모드의 '버튼'이 존재한다는 가정이 있는 듯합니다.

 또한, 조준 모드를 기본으로 지원합니다.

	2006/04/11 by OZ
		도트사이트 및 Lager Point 를 탈부착 하기 위한 Code 추가

 */
class avaWeap_BaseGun extends avaWeapon		
	abstract
	native;

`include(avaGame/avaGame.uci)

cpptext
{
	virtual VOID	TickSpecial(FLOAT DeltaTime);
	virtual VOID	RecalculateAccuracy( FLOAT DeltaTime );
	virtual VOID	ClampAccuracyEx( FLOAT& result );
	virtual FLOAT	CalcAccuracyEx( int ShotsFired, float DeltaTime );
	virtual FLOAT	DecayAccuracyEx( FLOAT Acc );
	virtual FLOAT	CalcSpreadEx(float fAccuracy);
	virtual FVector	GetWeaponBob(class AavaPawn* Holder);
	virtual VOID	ApplyKickback();
}

struct native RifleKickback
{
	var() float UpBase;
	var() float LateralBase;
	var() float UpModifier;
	var() float LateralModifier;
	var() float UpMax;
	var() float LateralMax;
	var() int DirectionChange;
};

// CalcSpread에서 param1 + param2 * Accuracy
struct native SCalcSpread
{
	var() float param1;
	var() float param2;
};

struct native KickbackRange
{
	var() float Min;
	var() float Max;
};

var byte									iShotsFired, iDirection;

var(RifleInfo)	float						TimeToIdleAfterFire,
											IdleInterval;			// 아직 적용되지 않은 수치....

var(RifleInfo) class<avaBullet_Base>		BulletType;
var(RifleInfo) bool							bAutoFire;
var(RifleInfo) int							NumFiresPerShot;

var float									Inaccuracy, AccuracyTime;
var float									LastFire, AccumulatedAccurayQueuedTime;

var float									CrossHairSpeed;

//***************************************************************************************************************************
// Fire 관련 Variable
var float									Accuracy, 
											AccumulatedAccuracy,
											CurrentSpread,
											LastSpread,
											SpreadDecayTime,
											HitDamage,
											DisplayedSpreadMax;

var	bool									bDisplaySpreadInfoInSightMode;

var(RifleInfo)	byte						Penetration;			// 총기특성
var(RifleInfo)	float						RangeModifier;
var(RifleInfo) editinline RifleKickback		Kickback_WhenMoving,
											Kickback_WhenFalling,
											Kickback_WhenDucking,
											Kickback_WhenSteady;
var(RifleInfo) editinline SCalcSpread		Spread_WhenMoving, 
											Spread_WhenFalling,
											Spread_WhenDucking,
											Spread_WhenSteady;
var(RifleInfo)	float						AccuracyDivisor, 
											AccuracyOffset, 
											MaxInaccuracy;
var(RifleInfo) float						Kickback_LateralLimit, Kickback_UpLimit;
var(AimShotInfo) editinline float			FireIntervalMultiplierA;
var(AimShotInfo) editinline RifleKickback	Kickback_WhenMovingA, 
											Kickback_WhenFallingA, 
											Kickback_WhenDuckingA, 
											Kickback_WhenSteadyA;
var(AimShotInfo) editinline SCalcSpread		Spread_WhenMovingA,
											Spread_WhenFallingA,
											Spread_WhenDuckingA,
											Spread_WhenSteadyA;
var(AimShotInfo)	float					AccuracyDivisorA, 
											AccuracyOffsetA, 
											MaxInaccuracyA;
var(RifleInfo) editinline array<KickbackRange> KickbackLimiter;
var(RifleInfo) editinline int				DirectionHold;
var transient int							CurrentDirectionHold;
//***************************************************************************************************************************
//***************************************************************************************************************************
// Silencer 관련 Variable
var(Silencer)	bool						bEnableSilencer;		// true 이면 소음기를 장착할 수 있다.
var hmserialize repnotify	bool			bMountSilencer;			// true 이면 소음기를 장착한 상태이다.

var(Silencer)	name						MountSilencerAnim;		// 소음기 장착 Animation 이름
var(Silencer)	name						UnMountSilencerAnim;	// 소음기 탈착 Animation 이름
var(Silencer)	float						MountSilencerTime;		// 소음기 장착시 걸리는 시간
var(Silencer)	float						UnMountSilencerTime;	// 소음기 탈착시 걸리는 시간
var(Silencer)	SoundCue					WeaponSilencerFireSnd;	// 소음기 장착시 Fire Sound

var(Silencer)	byte						PenetrationS;			// 소음기 장착시 총기특성
var(Silencer)	float						RangeModifierS;
var(Silencer)	float						HitDamageS;
var				MeshComponent				SilencerMesh;			// 소음기 Mesh Component
var(Silencer)	string						SilencerMeshName;		// 소음기 Mesh 이름
var(Silencer)	name						SilencerBoneName;		// 소음기가 붙을 Bone 이름
var(Silencer)	Array< class<DamageType> >	InstantHitDamageTypesS;	// 소음기가 붙었을 경우의 Damage Type
//***************************************************************************************************************************
//***************************************************************************************************************************
// Reload 관련 Variable
var() hmserialize cipher BYTE				ReloadCnt; // [!] 20070411 dEAthcURe|HM 'hmserialize'
var()			int							ClipCnt;
var() float									ReloadTime;
var bool									bForceReload;
var(Animations)	name						WeaponReloadAnim;
var bool									bReloadClip;			// true 이면 Reload 를 Clip 단위로 한다. default 는 true 임. shot gun 류의 총기의 경우 false
var(Animations) name						WeaponPreReloadAnim;
var()			float						PreReloadTime;
var(Animations) name						WeaponPostReloadAnim;
var()			float						PostReloadTime;
var()			bool						bEnableFireWhenReload;

//***************************************************************************************************************************
//***************************************************************************************************************************
// SightMode 관련 Variable
struct native SightInfo
{
	var float FOV;
	//var float MaxSpeed;
	var float ChangeTime;
};

var()	MeshComponent						ScopeComp;						// Scope 용 Component
var		string								ScopeMeshName;					// Scope Mesh 이름
var		MaterialInstanceConstant			ScopeMIC;

var()	array<SightInfo>					SightInfos;
var		byte								SightMode;						// 현재 SightMode 값
var		name								SightInAnim, 
											SightOutAnim;					// Animation Name For Sight In & Out
var		float								fov_start,
											fov_target,
											fov_current,
											fov_transitionTime,
											fov_transitionElapsedTime;
var		bool								bHideWeaponInSightMode;			// True 이면 SightMode 가 0 이 아닌경우에는 Weapon 을 그리지 않는다.
var		bool								bHideCursorInSightMode;			// True 이면 SightMode 에서 Cursor를 그리지 않는다.
var()	float								BobDampingInSight;				// Sight Mode 일때의 Bob Damping 값.
var()	ParticleSystemComponent				InSightMuzzleFlashPSC;			// Sight Mode 에서의 Muzzle Flash
var		ParticleSystem						InSightMuzzleFlashPSCTemplate;	// Sight Mode 에서의 Muzzle Flash Template
var		bool								bReleaseZoomAfterFire;			// True 이면 Fire 시 Zoom 이 풀린다.
var		float								fReleaseZoomAfterFireInterval;	// Interval 이후에 Zoom 이 풀린다. bReleaseZoomAfterFire 가 True 인 경우에만 의미가 있다.
var		bool								bRecoverZoomAfterFire;			// True 이면 Fire 시 Zoom 이 풀린 후 원상 복구 된다. bReleaseZoomAfterFire 이 True 인 경우에만 의미 있다. 
var		float								RecoverZoomTime;				//
var		int									ReleasedSightMode;				// Fire 시 Zoom 이 풀린 후 원래대로 돌아가야 한다.
//***************************************************************************************************************************
//***************************************************************************************************************************
// Fire Animation 관련 Variable
struct native FireAnimInfo{
	var name	AnimName;		// 사용할 Animation Sequence 이름
	var float	FirstShotRate;	// 초탄일 경우 시 Play 빈도
	var float	OtherShotRate;	// 초탄을 제외한 탄의 Play 빈도
};

var()	array<FireAnimInfo>					FireAnimInfos;
var()	array<FireAnimInfo>					AltFireAnimInfos;
var		float								FireAnimFirstShotTotalRate,		FireAnimOtherShotTotalRate;
var		float								AltFireAnimFirstShotTotalRate,	AltFireAnimOtherShotTotalRate;
var		name								LastFireAnim, LastAltFireAnim;					// Bolt Action 식의 총기의 경우 Clip 에 Ammo 가 한발만 남아 있는 경우 Animation 을 다르게 Play 할 수도 있다.
var		float								LastFireInterval;			// 
//***************************************************************************************************************************
//***************************************************************************************************************************
// Trail 관련 Variable
var(Trail)	avaBulletTrailComponent			BulletTrailComponent;
var int										TrailCount;
var(Trail)	int								TrailInterval;
//***************************************************************************************************************************
//***************************************************************************************************************************
// Flash Light 관련 Variable
//var()			bool						bEnableFlashLight;				//	true 이면 Flash 를 켤수 있다.
//var	repnotify	bool						bActivateFlashLight;			//	true 이면 Flash 를 켠 상태이다.
//var				avaFlashlightComponent		FlashlightComponent;
//var()			StaticMeshComponent			LightConeComponent;	

//***************************************************************************************************************************

/** 탄피 Particle  **/
var ParticleSystem							BulletTemplete;
var ParticleSystemComponent					BulletPSC;
var bool									TryAttachBulletPSC;

var avaEmitter								BulletEmitter;


var(Sounds)	SoundCue						WeaponNoAmmoSnd;
var	float									LastNoAmmoSoundPlayTime;	// 
var	float									NoAmmoSoundInterval;		// No Ammo Sound Interval
var	float									UnZoomPenalty;
var	bool									bEnableLaserSight;			// true 이면 Laser Sight 를 장착한 상태이다.
var bool									bEjectBulletWhenFire;		//

var float									CrossHairMultiflier;

var transient float							LastHitTime;
var transient Pawn							LastHitPawn;

var float									UnZoomSpreadAmp;
var float									ZoomSpreadAmp;

var	bool									bAvailableInstantZoomFire;
var int										ReloadableCount;

var	bool									bSyncSightMode;

replication
{
	if (Role == ROLE_Authority)
		ReloadCnt;
	if (Role == Role_Authority && bNetDirty)
		bMountSilencer;

	if (Role == Role_Authority && bNetDirty && bNetOwner)
		iShotsFired, iDirection;

	if (!bNetOwner && Role == ROLE_Authority)
		SightMode;
}

native simulated function vector TransformTest( vector loc, rotator rot, vector src );

simulated function UpdateReloadCnt()
{
	if ( AmmoCount < ClipCnt )	ReloadCnt = AmmoCount;
	else						ReloadCnt = ClipCnt;
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'bMountSilencer' )				UpdateSilencerState();
	else Super.ReplicatedEvent( VarName );
}

simulated function AttachItems()
{
	Super.AttachItems();
	AttachSilencer();
	//AttachFlashLight();
}

simulated function AttachSilencer()
{
	local StaticMesh	tempStaticMesh;
	if ( SilencerMeshName != "" )
	{
		tempStaticMesh = StaticMesh( DynamicLoadObject( SilencerMeshName, class'StaticMesh' ) );
		StaticMeshComponent( SilencerMesh ).SetStaticMesh( tempStaticMesh );
		SilencerMesh.SetShadowParent( Mesh );
		SilencerMesh.SetOcclusionGroup( Mesh );
		SilencerMesh.SetDepthPriorityGroup( SDPG_Foreground );
		SilencerMesh.SetOnlyOwnerSee( true );
		SkeletalMeshComponent(Mesh).AttachComponent(SilencerMesh, SilencerBoneName );
		ChangeSilencerVisibility( bMountSilencer );
	}
}

//simulated function AttachFlashLight()
//{
//	if ( bEnableFlashLight == true )
//	{
//		if ( SkeletalMeshComponent( Mesh ).GetSocketByName( 'Flash' ) != None )
//		{
//			SkeletalMeshComponent( Mesh ).AttachComponentToSocket( FlashlightComponent, 'Flash' );
//			SkeletalMeshComponent( Mesh ).AttachComponentToSocket( LightConeComponent,	'Front' );
//			UpdateFlashLight();
//		}
//	}
//}

//simulated function UpdateFlashLight()
//{
//	local bool bFirstPerson;
//	local avaAttachment_BaseGun avaWeapAttachment;
//
//	if ( WeaponAttachment == None )	return;
//	bFirstPerson = Instigator.IsFirstPerson();
//	FlashlightComponent.SetEnabled( bActivateFlashLight && bFirstPerson );
//	LightConeComponent.SetHidden( !bActivateFlashLight || !bFirstPerson );
//
//	//avaWeapAttachment = avaAttachment_BaseGun(WeaponAttachment);
//	//// None이여서 경고창 뜨는 부분 막음.(2007/04/24)
//	//if ( avaWeapAttachment != None )
//	//	avaWeapAttachment.UpdateFlashLight();
//
//	//SoundGroupClass.static.PlayFlashlightSound( Self );
//}

//simulated function ToggleFlashLight()
//{
//	if ( bEnableFlashLight == false )	return;
//	ServerToggleFlashLight();
//}
//
//reliable server function ServerToggleFlashLight()
//{
//	if ( bEnableFlashLight == false )	return;
//	bActivateFlashLight = !bActivateFlashLight;
//	UpdateFlashLight();
//}

simulated function DetachWeapon()
{
	SightMode	=	0;
	ChangeFOV( SightInfos[SightMode].FOV, 0.0 );							
	EndSightOut();

	ClearTimer( 'EndSightInState' );
	ClearTimer( 'EndSightOutState' );

	DetachComponent( BulletTrailComponent );
	//if ( Role == ROLE_Authority )
	//{
	//	bActivateFlashLight = false;
	//	UpdateFlashLight();
	//}
	DetachBulletEjector();
	Super.DetachWeapon();
}

simulated function SetLightEnvironment( LightEnvironmentComponent env )
{
	Super.SetLightEnvironment( env );
	if (SilencerMesh != None)	SilencerMesh.SetLightEnvironment( env );
}



simulated function UpdateSilencerState()
{
	avaAttachment_BaseGun(WeaponAttachment).ChangeSilencerVisibility( bMountSilencer );
	ChangeSilencerVisibility( bMountSilencer );
}

simulated function ToggleSilencer()
{
	// Client에서 Active 상태인지 미리 체크하자
	if ( bEnableSilencer && IsInState('Active') )
		ServerToggleSilencer();
}

// 소음기 탈부착 server Entry Point
reliable server function ServerToggleSilencer()
{
	local bool bToggle;
	if ( !IsInState('Active') )	return;	// 소음기 탈부착은 Active State 에서만 가능하다.
	bToggle = !bMountSilencer;
	ForceToggleSilencer( bToggle );
	ClientToggleSilencer( bToggle );
}

reliable client simulated function ClientToggleSilencer( bool bMount )
{
	ForceToggleSilencer( bMount );
}

// 소음기 탈부착 상태로 State 전이
simulated function ForceToggleSilencer( bool bMount )
{
	// 소음기를 탈부착 하기 위해서 SightMode 를 푼다...
	ClearSwitchSightMode();
	SwitchSightMode( 0, 0.0 );

	if ( bMount )	GotoState('MountSilencerState');
	else			GotoState('UnMountSilencerState');
}

simulated function ChangeSilencerVisibility( bool bVisible )
{
	if ( SilencerMesh == None )	return;
	if ( !bVisible )				SilencerMesh.SetHidden( true );
	else if ( !Mesh.HiddenGame )	SilencerMesh.SetHidden( false );
}

event simulated PostBeginPlay()
{
	local int	i;

	UpdateReloadCnt();

	fov_target	=	SightInfos[0].FOV;
	fov_current	=	SightInfos[0].FOV;

	FireAnimFirstShotTotalRate		=	0.0;
	FireAnimOtherShotTotalRate		=	0.0;
	AltFireAnimFirstShotTotalRate	=	0.0;
	AltFireAnimOtherShotTotalRate	=	0.0;

	for ( i = 0 ; i < FireAnimInfos.length ; ++ i )
	{
		FireAnimFirstShotTotalRate += FireAnimInfos[i].FirstShotRate;
		FireAnimOtherShotTotalRate += FireAnimInfos[i].OtherShotRate;
	}

	for ( i = 0 ; i < AltFireAnimInfos.length ; ++ i )
	{
		AltFireAnimFirstShotTotalRate += AltFireAnimInfos[i].FirstShotRate;
		AltFireAnimOtherShotTotalRate += AltFireAnimInfos[i].OtherShotRate;
	}

	Super.PostBeginPlay();
}

simulated function SetSightInfo( int nIdx, float fov, float fChangeTime )
{
	if ( SightInfos.length <= nIdx )	SightInfos.length = nIdx + 1;
	SightInfos[nIdx].FOV				= fov;
	SightInfos[nIdx].ChangeTime			= fChangeTime;
}

// SightMode 일 경우 HUD 그리기
simulated function DrawZoomedOverlay( HUD H );

simulated function AttachWeaponTo( SkeletalMeshComponent MeshCpnt, optional Name SocketName )
{
	Super.AttachWeaponTo( MeshCpnt, SocketName );

	AttachComponent( BulletTrailComponent );
}

simulated function float GetExposureCenterRegionScale()
{
	if (SightMode == 0)
		return 1.0;
	else
		return 0.4;
}

simulated function float AdjustForegroundFOVAngle( float FOV )
{
	if ( !bHideWeaponInSightMode )
		return fov_current * WeaponFOV / SightInfos[0].FOV;
	else
		return WeaponFOV;
}

simulated function float AdjustFOVAngle(float FOVAngle)
{
	if ( Role == Role_Authority || Instigator.IsLocallyControlled() )
		return fov_current;
	else
		return SightInfos[SightMode].FOV;
}

simulated function SendToFiringState(byte FireModeNum)
{
	GotoState( 'WeaponFiring' );
}

simulated state WeaponFiring
{
	simulated function BeginFire(Byte FireModeNum)
	{
		if (bAutoFire)	Super.BeginFire( FireModeNum );
	}

	simulated function RefireCheckTimer()
	{
		local int rsm;
		if ( bReleaseZoomAfterFire && ReleasedSightMode > 0 )
		{
			if ( bRecoverZoomAfterFire && ReloadCnt > 0 )
			{
				rsm = ReleasedSightMode;
				ReleasedSightMode = 0;
				SwitchSightMode( rsm, RecoverZoomTime );
				// 여기서 SightIn 으로 간 이후에 Active 상태로 갈것입니다.
				return;
			}
			else
			{
				ReleasedSightMode = 0;
			}
		}

		// if switching to another weapon, abort firing and put down right away
		if( bWeaponPutDown )
		{
			PutDownWeapon();
			return;
		}

		// If weapon should keep on firing, then do not leave state and fire again.
		if( ShouldRefire() )
		{
			FireAmmunition();
			return;
		}
		
		GotoState('Active');

		// if out of ammo, then call weapon empty notification
		if( !HasAnyAmmo() )
		{
			WeaponEmpty();
		}
	}

	event Timer()
	{
		GotoState('Active');
	}

	simulated function EndState( Name NextStateName )
	{
		// Set weapon as not firing
		ClearFlashCount();
		ClearFlashLocation();
		ClearTimer('RefireCheckTimer');

		if (Instigator != none && AIController(Instigator.Controller) != None)
		{
			AIController(Instigator.Controller).NotifyWeaponFinishedFiring(self,CurrentFireMode);
		}
	}
}

simulated function PlayNoAmmoSound()
{
	// play weapon fire soundd
	if ( WeaponNoAmmoSnd != None && WorldInfo.TimeSeconds - LastNoAmmoSoundPlayTime > NoAmmoSoundInterval )
	{
		WeaponPlaySound( WeaponNoAmmoSnd );
		LastNoAmmoSoundPlayTime = WorldInfo.TimeSeconds;
	}
}

simulated function FireAmmunition()
{
	if( ReloadCnt > 0 )
	{
		Super.FireAmmunition();
		if ( Role==ROLE_Authority )
			ReloadCnt--;
	}
	else
	{
		PlayNoAmmoSound();
	}

	//if (ReloadCnt<=0 && GetAmmoCount() > 0 )
	//{
	//	DoReload();
	//}

	HandleAutofire();
}

simulated function HandleAutofire()
{
	if (!bAutoFire)
		ClearPendingFire( 0 );
}


function DoReload()
{
	if ( IsReloadable() && IsInState('Active') )
	{
		ForceReload();
		ClientReload();
	}
}

simulated function ForceResetWeapon()
{
	ClearSwitchSightMode();
	SwitchSightMode( 0, 0.0 );
	Super.ForceResetWeapon();
}

/** forces the client to reload; called by the server when clip ammo has been used up */
reliable client simulated function ClientReload()
{
	ForceReload();
}

simulated function ForceReload()
{
	// Reload 를 하기 위해서는 sightmode 를 푼다.
	ClearSwitchSightMode();
	SwitchSightMode( 0, 0.0 );

	if ( PreReloadTime > 0.0 )
		GotoState('PreReloading');
	else	
		GotoState('WeaponReloading');
}


simulated function EndFire(byte FireModeNum)
{
	Super.EndFire(FireModeNum);
}

function ServerSwitchToNextSightMode()
{
	if ( Role < ROLE_Authority )
		return;
	
	SwitchToNextSightMode();
	ClientSwitchToNextSightMode( SightMode );
}

reliable client simulated function ClientSwitchToNextSightMode( int inSightMode )
{
	if ( Role == ROLE_Authority )
		return;
	SwitchSightMode( inSightMode, sightinfos[inSightMode].ChangeTime );
}

simulated state Active
{
	simulated function BeginFire(byte FireModeNum)
	{
		if ( FireModeNum == 1 )
		{
			if ( bSyncSightMode == TRUE )
			{
				ServerSwitchToNextSightMode();
			}
			else
			{
				SwitchToNextSightMode();
			}

			ClearPendingFire(1);
		}
		else Super.BeginFire( FireModeNum );
	}

	simulated function BeginState( name PrevStateName )
	{
		Super.BeginState( PrevStateName );
		if( Role == Role_Authority )
		{
			if ( ReloadCnt == 0 && GetAmmoCount() > 0 )
			{
				DoReload();
			}
		}
	}

Begin:
	// Active 상태로 들어왔을때 현재 ReloadCnt 가 없다면 Auto Reload 하도록 한다...
	if ( Role == ROLE_Authority && ReloadCnt == 0 && GetAmmoCount()  > 0 )
	{
		DoReload();
	}
}

function bool DropWeapon(vector StartLocation, optional bool bSwitch )
{
	if ( Super.DropWeapon(StartLocation, bSwitch) )
	{
		bForceReload = false;
		return true;
	}
	return false;
}
/*********************************************************************************************
 * Firing
 *********************************************************************************************/

/*
   These 3 functions are overriden here to allow for stats collection.  If your custom weapon
   does not use these 3 base firing functions, you need to implement the stats tracking on
   your own by calling UpdateFireStats()
*/

simulated function GetWeaponDebug( out Array<String> DebugInfo )
{
	super.GetWeaponDebug( DebugInfo );
	DebugInfo[DebugInfo.Length] = "ShotsFired:" $ iShotsFired $ " Accuracy:" $ Accuracy $ 
		" Spread:"$ CurrentSpread $" Inaccuracy:" $ Inaccuracy $ 
		" Acc.Accuracy:" $ AccumulatedAccuracy $ 
		" ElapsedTime:" $ ( WorldInfo.TimeSeconds - LastFire ) $		
		" Current.Effective.Acc.Accuracy: " $ (AccumulatedAccuracy * (1 - FMin( 1, ( WorldInfo.TimeSeconds - LastFire ) / SpreadDecayTime ))) $
		" Zoom State: " $ SightMode $
		" PunchAngle: " $ avaPawn(Instigator).PunchAngle;
		
}

simulated native function KickBack( RifleKickback rk );
simulated native function InstantFireEx();

function float GetExpectableMaxDamage()
{
	return GetHitDamage() + bulletType.default.DamageOffset + bulletType.default.DamageRandom;
}

simulated function GetBulletTypeParameters( 	
	out int iPenetrationPower,
	out float fPenetrationDistance,	
	out float CurrentDamage,
	out float ArmorParam )
{	
	iPenetrationPower = bulletType.default.PenetrationPower;
	fPenetrationDistance = bulletType.default.PenetrationDistance;		
	CurrentDamage += bulletType.default.DamageOffset + Rand(bulletType.default.DamageRandom);
	ArmorParam = bulletType.default.ArmorRatio;	
}

simulated function rotator AddSpread(rotator BaseAim)
{
	local vector X, Y, Z;
	local float RandY, RandZ, q;	

	// Add in any spread.
	GetAxes(BaseAim, X, Y, Z);

	while (true)
	{
		RandY = FRand() - 0.5;
		RandZ = FRand() - 0.5;
		q = RandY * RandY + RandZ * RandZ;

		if (q <= 1) break;
	}

	return rotator(X + RandY * CurrentSpread * Y + RandZ * CurrentSpread * Z);	
}

simulated native function AccumulateAccuracy();

simulated function CeaseToFire()
{
	if ( iShotsFired > 0 )
	{
		AccumulateAccuracy();
		iShotsFired = 0;
	}
}

simulated function ClearPendingFire(int FireMode)
{
	super.ClearPendingFire( FireMode );

	if ( FireMode == 0 )
		CeaseToFire();
}

simulated function DiscontinuityOnFiring()
{
	/// auto fire하는 경우에만 따닥을 지원한다.
	if (bAutoFire)
	{
		CeaseToFire();
	}
}

simulated function float GetRangeModifier()
{
	if ( bMountSilencer )	return RangeModifierS;
	return RangeModifier;
}

simulated function byte GetPenetration()
{
	if ( bMountSilencer )	return PenetrationS;
	return Penetration;
}

simulated function float GetHitDamage()
{
	if ( bMountSilencer )	return HitDamageS * avaPawn(Instigator).WeapTypeAmp[WeaponType].DamageAmp;
	return HitDamage * avaPawn(Instigator).WeapTypeAmp[WeaponType].DamageAmp;
}

simulated function class<avaDamageType>	GetInstantHitDamageTypes( int FireMode )
{
	if ( bMountSilencer )	return	class<avaDamageType>(InstantHitDamageTypesS[ FireMode ]);
	return class<avaDamageType>(InstantHitDamageTypes[ FireMode ]);
}

simulated function ProcessInstantHit2( ImpactInfo Impact, float CurrentDamage, float ArmorParam, optional bool bWallShot )
{
	local avaPawn		AP;
	local DamageData	damageData;

	if( Role == Role_Authority && Impact.HitActor != None && ( Pawn(Impact.HitActor) != none ) )
	{
		UpdateHitStats(true, Instigator.IsSameTeam( Pawn(Impact.HitActor) ), VSize( Instigator.Location - Impact.HitActor.Location )/16*0.3 );
	}

	// cause damage to locally authoritative actors
	if (Impact.HitActor != None )
	{
		AP = avaPawn(Impact.HitActor);
		// 맞은 대상이 Pawn 이라면 Local 에서 판단한것을 Host 로 올려 보내준다...
		if ( AP != None  )
		{
			if ( Role == ROLE_Authority )
				AP.bWallShot = bWallShot;

			if ( Instigator.IsLocallyControlled() )
			{
				LastHitTime = WorldInfo.TimeSeconds;
				LastHitPawn = AP;

				// client인 경우
				// 2007/9/14 추가 : 연습채널에서는 Damage 를 안 주기 때문에 Effect 를 Play 하지 않도록 한다....
				if ( Role != ROLE_Authority && 
					 WorldInfo.NetMode != NM_ListenServer && 
					 WorldInfo.NetMode != NM_DedicatedServer &&
					 ( class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentChannelFlag() != EChannelFlag_Practice ) )
				{
					// play clientside effects
					AP.PlayZeroLatencyEffect( Impact, Instigator.Controller );
				}

				damageData.Damage		=	Min( 255, CurrentDamage );
				damageData.BoneIndex	=	AP.Mesh.MatchRefBone( Impact.HitInfo.BoneName );
				damageData.Reserve1		=	Rand(255);
				damageData.Reserve2		=	Rand(255);
				AP.Client_RequestTakeGunDamage( damageData, Instigator.Controller );
			}			
		}
		else if ( Impact.HitActor.Role == ROLE_Authority )
		{
			Impact.HitActor.TakeDamage(	CurrentDamage,
										Instigator.Controller,
										Impact.HitLocation,
										InstantHitMomentum[CurrentFireMode] * Impact.RayDir,
										GetInstantHitDamageTypes(CurrentFireMode),
										Impact.HitInfo,
										Class );
		}
	}
}

simulated function ShakeView();

simulated function InstantFire()
{	
	// 내부적으로 RifleFire를 호출해준다.
	InstantFireEx();
	// avaWeapon.UpdateFiredStats()함숫에서 avaGame.GameStats.WeaponEvent를 발생시킨다.
	UpdateFiredStats(1);
}

simulated function ApplyPunchAngleToWeapon( out rotator rot )
{
	avaPawn( Instigator ).ApplyPunchAngleToWeapon( rot );	
}

simulated event function RifleFire( int ShotNum )
{
	local vector						StartTrace, EndTrace;
	local Array<ImpactInfo>				ImpactList;	
	local ImpactInfo					RealImpact;
	local vector						dir;			
	local int							iPenetration;
	local int							iRound, iImpactRound;
	local vector						org;
	local int							iPenetrationPower;
	local float							fPenetrationDistance;
	local float							CurrentDamage;
	local float							fDamageModifier;
	local avaPhysicalMaterialProperty	PhysicalProperty;
	local float							fCurrentDistance, fDistance;	
	local vector						NextStart, PrevHit, PrevNormal;
	local actor							HitActor;
	local vector						HitNormal, HitLocation;
	local TraceHitInfo					HitInfo;
	local bool							blood, glass;
	local float							ArmorParam;
	local float							PenetrationMinDistance;
	local float							CheckDistance;
	local rotator						TmpRot;
	local vector						RealHitLocation;

	blood = false;

	iPenetration	=	GetPenetration();

	CurrentDamage	=	GetHitDamage();

	GetBulletTypeParameters( iPenetrationPower, fPenetrationDistance, CurrentDamage, ArmorParam );

	GetFireLocAndRot( StartTrace, TmpRot );

	fDistance		=	GetTraceRange();	

	ApplyPunchAngleToWeapon( TmpRot );

	dir = vector(AddSpread(TmpRot));

	EndTrace = StartTrace + dir * GetTraceRange();

	org = StartTrace;
	
	fDamageModifier = 0.5;	

	while (iPenetration != 0 || blood)
	{        
		// Perform shot
		RealImpact = CalcWeaponFire(StartTrace, EndTrace, ImpactList);

		fCurrentDistance = VSize(RealImpact.HitLocation - StartTrace);

		if ( fCurrentDistance == 0.0 )	break;
		
		/// 뒷면 뚫고 나간 것 체크
		if (iRound > 0)
		{	
			CheckDistance = VSize( StartTrace - EndTrace );
			PenetrationMinDistance = fCurrentDistance > CheckDistance ? CheckDistance : fCurrentDistance;

			GetTraceOwner().Trace( HitLocation, HitNormal, EndTrace, StartTrace, true, vect(0,0,0), HitInfo, TRACEFLAG_Bullet );

			if ( PenetrationMinDistance > VSize(HitLocation - StartTrace) )	break;

			if (RealImpact.HitActor != none)
			{
				HitActor = GetTraceOwner().Trace(HitLocation, HitNormal, PrevHit + dir * 8, RealImpact.HitLocation - dir * 8, true, vect(0,0,0), HitInfo, TRACEFLAG_Bullet );			
			}
			else
			{
				HitActor = GetTraceOwner().Trace(HitLocation, HitNormal, PrevHit + dir * 8, EndTrace, true, vect(0,0,0), HitInfo, TRACEFLAG_Bullet );			
			}

			if (HitActor != None)
			{				
				if ((PrevNormal dot HitNormal) < 0)
				{
//					`log( "Backward trace "@HitLocation );
					SetImpactLocation(HitLocation,true);
				}
			}
		}

		/// 시작점이 invalid한 경우!
		if (iRound > 0 && VSize(StartTrace - RealImpact.HitLocation) < 2)
			break;

		glass = RealImpact.HitActor != none && RealImpact.HitActor.IsA( 'avaShatterGlassActor' );		

		if ( iImpactRound == 0 )
		{
			RealHitLocation = RealImpact.HitLocation;
			
		}

		/// 이것은 사실 Bounce==0에 대해서만 해야 한다. PlayImpactEffects가 Bulletwhip sound까지 포함하기 때문에!!
		if ( !glass )
		{
			//`log( "FlashLoctation "@iRound@RealImpact.HitLocation );
			if (iImpactRound == 0 && ShotNum == 0 )						
			{
				SetFlashLocation(RealImpact.HitLocation);
			}
			else
				SetImpactLocation(RealImpact.HitLocation,,blood);
						
			iImpactRound++;
		}

		iRound++;

		if (RealImpact.HitActor == None) break;								
		
		if (RealImpact.HitInfo.PhysMaterial != None)
			PhysicalProperty = avaPhysicalMaterialProperty(RealImpact.HitInfo.PhysMaterial.GetPhysicalMaterialProperty(class'avaPhysicalMaterialProperty'));
		else
			PhysicalProperty = None;

		/// 유리창 통과 시 그냐아앙!
		if (glass)
		{
			fDamageModifier = 1.0;
			iPenetration++;
		}
		else
		{
			if (PhysicalProperty != None)
			{
				iPenetrationPower = int( iPenetrationPower * PhysicalProperty.PenetrationDamper );
				fDamageModifier = PhysicalProperty.DamageModifier;
			}
			else
			{
				iPenetrationPower = iPenetrationPower / 2;
				fDamageModifier = 0.6;
			}
		}

		/* 피 안나오는 것 고침 */
		if (iPenetration == 0 && blood)
			break;

		iPenetration--;		

		//CurrentDamage = CurrentDamage * pow( RangeModifer, VSize(org-RealImpact.HitLocation) / 500 );
		CurrentDamage = CurrentDamage * Exp( Loge(GetRangeModifier()) * VSize(org-RealImpact.HitLocation) / 500 );

		if (fCurrentDistance > fPenetrationDistance)
			iPenetration = 0;				

		ProcessInstantHit2( RealImpact, CurrentDamage, ArmorParam, iImpactRound > 1 );

		if ( avaPawn(RealImpact.HitActor) != None)
		{		
			//if ( !Instigator.IsSameTeam( Pawn(RealImpact.HitActor ) ) )	

			if ( Instigator.GetTeamNum() != Pawn(RealImpact.HitActor ).GetTeamNum() && 
				 class'avaNetHandler'.static.GetAvaNetHandler().IsPlayerAdult() &&
				 class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentChannelFlag() != EChannelFlag_Practice )
			{
				blood = true;
			}
			NextStart = RealImpact.HitLocation + dir * 42;
			fDistance = (fDistance - fCurrentDistance) * 0.75;
		}
		else
		{
			NextStart = RealImpact.HitLocation + dir * iPenetrationPower;
			fDistance = (fDistance - fCurrentDistance) * 0.5;
			blood = false;
		}		

		StartTrace = NextStart;
		
		EndTrace = StartTrace + dir * fDistance;
		CurrentDamage *= fDamageModifier;

		PrevHit = RealImpact.HitLocation;
		PrevNormal = RealImpact.HitNormal;
	}	

	if ( Instigator.IsLocallyControlled() && ShotNum == 0  )
		PlayFireEffects( CurrentFireMode, RealHitLocation );
}

simulated function Destroyed()
{	
	super.Destroyed();
}

simulated function Activate()
{
	Super.Activate();
	SwitchSightMode( 0, 0.0 );
}

simulated event ReleaseZoomAfterFire()
{
	local int sm;			
	sm = SightMode;
	SwitchSightMode( 0, fReleaseZoomAfterFireInterval, false );
	ReleasedSightMode = sm;
}

simulated function float GetFireInterval( byte FireModeNum )
{
	local float result;

	if ( ReloadCnt == 0 && FireModeNum == 0 && LastFireAnim != '' )			
		result = LastFireInterval;		
	else 
		result = Super.GetFireInterval( FireModeNum );

	if (SightMode > 0)
	{
		return result * FireIntervalMultiplierA;
	}
	else
	{
		return result;
	}
}

simulated function PlayFiringSound2()
{
	local SoundCue SoundCueToPlay;
	Local PlayerController PC;

	if ( bMountSilencer )
	{	
		SoundCueToPlay = WeaponSilencerFireSnd;	
	}
	else
	{
		MakeNoise(1.0);

		SoundCueToPlay = WeaponFireSnd;		
	}
	
	if ( SoundCueToPlay != None )
	{
		foreach WorldInfo.LocalPlayerControllers(PC)
		{			
			PC.ClientHearSound( SoundCueToPlay, Instigator, Instigator.Location, false,false,AudioChannel_Weapon );
		}
	}
}

simulated function PlayFireEffects( byte FireModeNum, optional vector HitLocation )
{
	local int	i;
	local float TotalRate;
	local float	ResultRate;
	local array<FireAnimInfo>	AnimInfo;	

	if ( bEjectBulletWhenFire )
		EjectBullet();

	if ( bReleaseZoomAfterFire && SightMode > 0 )
	{		
		ReleaseZoomAfterFire();
	}
	else
	{
		ReleasedSightMode = 0;
	}
	
	if (TrailInterval > 0 && SightMode == 0 )
	{
		if (TrailCount % TrailInterval == 0)
			PlayTrailEffect( HitLocation );

		TrailCount++;
	}

	// Last Fire
	if ( ReloadCnt == 1 )
	{
		if ( FireModeNum == 0 && LastFireAnim != '' )	
		{
			PlayWeaponAnimation( LastFireAnim, 0.0 );
			CauseMuzzleFlash();
			ShakeView();
			return;
		}
		else if ( FireModeNum == 1 && LastAltFireAnim != '' )
		{
			PlayWeaponAnimation( LastAltFireAnim, 0.0 );
			CauseMuzzleFlash();
			ShakeView();
			return;
		}
	}

	
	if ( FireModeNum == 0 )	
	{
		AnimInfo = FireAnimInfos;
		if ( iShotsFired <= 1 )	TotalRate = FireAnimFirstShotTotalRate;
		else					TotalRate = FireAnimOtherShotTotalRate;
	}
	else if ( FireModeNum == 1 )	
	{
		AnimInfo = AltFireAnimInfos;
		if ( iShotsFired <= 1 )	TotalRate = AltFireAnimFirstShotTotalRate;
		else					TotalRate = AltFireAnimOtherShotTotalRate;
	}

	if ( TotalRate > 0.0 )
	{
		ResultRate	= FRand() * TotalRate;
		TotalRate	= 0;

		for ( i = 0 ; i < AnimInfo.length ; ++ i )
		{
			if ( iShotsFired <= 1 )	TotalRate += AnimInfo[i].FirstShotRate;
			else					TotalRate += AnimInfo[i].OtherShotRate;

			if ( TotalRate > ResultRate )
			{
				PlayWeaponAnimation( AnimInfo[i].AnimName, 0.0 );
				break;
			}
		}

		CauseMuzzleFlash();

		ShakeView();

		return;
	}

	Super.PlayFireEffects( FireModeNum, HitLocation );
}

simulated function PlayTrailEffect( vector HitLocation )
{
	local rotator	r;
	local vector	StartTrace;
	if (SkeletalMeshComponent(Mesh).GetSocketWorldLocationAndRotation( 'MuzzleFlashSocket', StartTrace, r ) )
	{				
		StartTrace  = StartTrace;// - Vector(r) * 30;
		/*EndTrace 	= StartTrace + Vector(r) * GetTraceRange();
		RealImpact = CalcWeaponFire(StartTrace, EndTrace, ImpactList);

		if (RealImpact.HitActor != none)
		{
			EndTrace = RealImpact.HitLocation;
		}*/

		BulletTrailComponent.Fire( StartTrace, HitLocation, Vect(0,0,0) );
	}
}


simulated state WeaponPuttingDown
{
	simulated function BeginState(Name PreviousStateName)
	{
		ReleasedSightMode = 0;
		SwitchSightMode( 0, 0.0 );
		Super.BeginState( PreviousStateName );
	}
}


/*===============================================================================
	Local 에서 SightMode 변경을 요청했음
	
	1. Specator 모드에서 1인칭 표현이 가능하도록 하기 위해서는 SightMode 가 replication 되어야 한다.
	2. SightMode 의 변경은 실패 할 수도 있다.
		1) SightMode 변경 중 Mode 변경을 요청하면 실패
		2) Reloading 중 Mode 변경을 요청하면 실패
		3) AWP 같이 SightMode 를 강제적으로 풀었다가 Recover 하는 도중에는 SightMode 변경 실패
	3. SightMode 변경시 Weapon 을 보여줄지 안보여줄지는 bHideWeaponInSightMode 의 value 를 보고 판단한다.

===============================================================================*/
simulated function SwitchToNextSightMode()
{
	local int nextSightMode;
	nextSightMode = (SightMode + 1) % SightInfos.length;
	SwitchSightMode( nextSightMode, sightinfos[nextSightMode].ChangeTime );
}

simulated function bool SwitchSightMode( int requestMode, float transitionTime, optional bool bPlayAnim = true )
{
	local int curSightMode;

	if ( SightMode == requestMode )
	{
		if ( transitionTime == 0.0 && fov_target != fov_current )	// fov 가 아직 바뀌고 있는 중이라면 완료해줘야 한다...
			ChangeFOV( fov_target, 0.0 );							
		return false;	// 현재 Sight Mode 와 같다면 변경할 수 없음.
	}
	if ( ReleasedSightMode > 0 )	return false;	// SightMode 를 Recover 하는 중에는 SightMode 변경 불가
	curSightMode	=	SightMode;
	SightMode		=	requestMode;

	if ( requestMode == 0 )	StartSightOut( transitionTime, bPlayAnim );
	else					StartSightIn( curSightMode, requestMode, transitionTime );
	return true;
}

native simulated function ChangeFOV( float targetFOV, float transition_time );

simulated function StartSightIn( int curSightMode, int nextSightMode, float transition_time )
{
	
	ChangeFOV( SightInfos[nextSightMode].FOV, transition_time );

	if ( curSightMode == 0 )
	{
		avaPawn(Instigator).ChangeWeaponState( 1 );

		if ( transition_time == 0.0 || SightInAnim == '' )
		{
			ShowScopeHUD( true );
		}
		else
		{
			GotoState( 'SightIn' );
		}
	}
}

simulated function StartSightOut( float transition_time, optional bool bPlayAnim = true )
{
	ChangeFOV( SightInfos[0].FOV, transition_time );

	avaPawn(Instigator).ChangeWeaponState( 0 );

	if ( transition_time == 0.0 )
	{
		EndSightOut();
	}
	else
	{
		if ( SightOutAnim == '' || bPlayAnim == false )
		{
			SetTimer( transition_time, false, 'EndSightOut' );
		}
		else
		{
			GotoState( 'SightOut' );
		}
	}
}

simulated function EndSightOut()
{
	ShowScopeHUD( false );
}

// if true show scope component, hide weapon mesh
simulated function ShowScopeHUD( bool bShow )
{
	if ( !bHideWeaponInSightMode )				return;	// HideInSightMode 가 false 라는 얘기는, Scope 를 따로 그리지 않겠다는 의미.
	if ( !Instigator.IsFirstPerson() )			return;	// 3인칭에서는 Scope 를 그리지 않음.
	if ( !Instigator.IsLocallyControlled() )	return;	// Local Controller 만 해당함. [1인칭Spectator] 지원한다면...?
	ChangeVisibility( !bShow );	
	//if ( ScopeComp != None )	ScopeComp.SetHidden( !bShow );
}

simulated function ClearSwitchSightMode()
{
	ClearTimer( 'EndSightInState' );
	ClearTimer( 'EndSightOutState' );
}

simulated function EndSightInState()
{
	ClearTimer( 'EndSightInState' );
	ShowScopeHUD( true );
	GotoState( 'Active' );
}

simulated state SightIn
{
	ignores SwitchToNextSightMode;

	simulated function StartFire(byte FireModeNum)
	{
		if ( bAvailableInstantZoomFire )
		{
			global.StartFire( FireModeNum );
		}
	}

	simulated function BeginState( name PrevState )
	{
		PlayWeaponAnimation( SightInAnim, fov_transitionTime );
		SetTimer( fov_transitionTime, false, 'EndSightInState' );
	}

	simulated function EndState( name NextState )
	{
	}
}

simulated function EndSightOutState()
{
	ClearTimer( 'EndSightOutState' );
	GotoState( 'Active' );
}

simulated state SightOut
{
	ignores SwitchToNextSightMode, StartFire;

	simulated function BeginState( name PrevState )
	{
		ShowScopeHUD( false );
		PlayWeaponAnimation( SightOutAnim, fov_transitionTime );
		SetTimer( fov_transitionTime, false, 'EndSightOutState' );
	}



	//simulated function EndState( name NextState )
	//{
	//	if ( IsTimerActive( 'EndSightOut' ) )
	//	{
	//		EndSightOut();
	//	}
	//}
}

simulated function ChangeVisibility(bool bIsVisible)
{
	if ( ScopeComp != None )
	{
		if ( Instigator.IsLocallyControlled() && Instigator.IsFirstPerson() && SightMode > 0 )	ScopeComp.SetHidden( false );
		else																					ScopeComp.SetHidden( true );
	}
	// 3인칭에서 Zoom 을 한 상태에서 다시 1인칭으로 온 경우이다. 2006/3/15 bug fix by OZ
	if ( bIsVisible == true && SightMode != 0 && bHideWeaponInSightMode )	
		bIsVisible	= false;
	Super.ChangeVisibility( bIsVisible );
	ChangeSilencerVisibility( bMountSilencer );
	//UpdateFlashLight();
}

simulated state UnMountSilencerState
{
	ignores SwitchToNextSightMode;

	simulated function ToggleSilencer();

	simulated function BeginState(name PrevStateName)
	{
		if ( Role == ROLE_Authority )	avaPawn(Instigator).PlayMountSilencerAnimation( false );
		ChangeSilencerVisibility( true );
		PlayUnMountSilencerAnim( UnMountSilencerAnim, UnMountSilencerTime );
	}

	simulated function PlayUnMountSilencerAnim( name animName, float animTime )
	{	
		PlayWeaponAnimation( animName, animTime );
		SetTimer( animTime, false, 'UnMountSilencerDone');
	}

	simulated function UnMountSilencerDone()
	{
		if ( Role == ROLE_Authority )	
		{
			bMountSilencer = false;	
			UpdateSilencerState();	
		}
		ChangeSilencerVisibility( false );
		GotoState('Active');
	}

	simulated function EndState(name NextStateName)
	{
		ClearTimer('UnMountSilencerDone');
	}

	simulated function bool TryPutDown()
	{
		ClearTimer('UnMountSilencerDone');
		PutDownWeapon();
		return true;
	}
}

// 소음기 탈착중
simulated state MountSilencerState
{
	ignores SwitchToNextSightMode;

	simulated function ToggleSilencer();
	
	simulated function BeginState(name PrevStateName)
	{
		if ( Role == ROLE_Authority )	avaPawn(Instigator).PlayMountSilencerAnimation( true );
		bMountSilencer = true;
		ChangeSilencerVisibility( true );
		PlayMountSilencerAnim( MountSilencerAnim, MountSilencerTime );
	}

	simulated function PlayMountSilencerAnim( name animName, float animTime )
	{	
		PlayWeaponAnimation( animName, animTime );
		SetTimer( animTime, false, 'MountSilencerDone');
	}

	simulated function MountSilencerDone()
	{
		GotoState('Active');
	}

	simulated function ForceResetWeapon()
	{
		bMountSilencer = false;
		ClearTimer('MountSilencerDone');
		global.ForceEquipWeapon();
	}

	simulated function EndState(name NextStateName)
	{
		if ( Role == ROLE_Authority )	
		{
			UpdateSilencerState();
		}
		ClearTimer('MountSilencerDone');
	}

	simulated function bool TryPutDown()
	{
		bMountSilencer = false;
		ClearTimer('MountSilencerDone');
		PutDownWeapon();
		return true;
	}
}

// PlayFiringSound2 에서 처리하도록 수정 - Not Replicate
simulated function PlayFiringSound()
{
	//if ( bMountSilencer )
	//{
	//	if ( WeaponSilencerFireSnd != None )
	//		WeaponPlaySound( WeaponSilencerFireSnd );
		InvManager.OwnerEvent('WeaponFiringSound');
	//}
	//else
	//{
	//	Super.PlayFiringSound();
	//}
}

simulated state WeaponEquipping
{
	ignores SwitchToNextSightMode;

	simulated function BeginState(Name PreviousStateName)
	{
		SwitchSightMode( 0, 0.0 );
		Super.BeginState( PreviousStateName );
	}
}

// 현재 Reload 가 가능한가???
simulated function bool IsReloadable()
{
	if ( ReloadCnt < ClipCnt && GetAmmoCount() - ReloadCnt > 0  )
		return true;
	else
		return false;
}

simulated function bool CanFire( byte FireModeNum )
{
	return ( ReloadCnt >= 1 );
}

simulated state PreReloading
{
	ignores SwitchToNextSightMode, OnAnimEnd;

	simulated function BeginState( name PrevStateName )
	{
		PlayWeaponAnimation( WeaponPreReloadAnim, PreReloadTime );
		SetTimer( PreReloadTime, false, 'PreReloadDone');
		// 3인칭용
		if ( Role == Role_Authority )
			avaPawn(Instigator).PlayReloadAnimation( EBT_PreReload );
	}

	simulated function EndState( name NextStateName )
	{
		ClearTimer( 'PreReloadDone' );
	}

	simulated function PreReloadDone()
	{
		GotoState( 'WeaponReloading' );
	}

	simulated function BeginFire(byte FireModeNum)
	{
		if ( FireModeNum == 0 && CanFire(FireModeNum) && !bDeleteMe && Instigator != None )
		{
			Global.BeginFire(FireModeNum);
			if( PendingFire(FireModeNum) )
				SendToFiringState(FireModeNum);
		}
	}
}

simulated state PostReloading
{
	ignores SwitchToNextSightMode, OnAnimEnd;

	simulated function BeginState( name PrevStateName )
	{
		PlayWeaponAnimation( WeaponPostReloadAnim, PostReloadTime );
		SetTimer( PostReloadTime, false, 'PostReloadDone');
		// 3인칭용
		if ( Role == Role_Authority )
			avaPawn(Instigator).PlayReloadAnimation( EBT_PostReload );
	}

	simulated function EndState( name NextStateName )
	{
		PlayIdleAnim( None );
		ClearTimer( 'PostReloadDone' );
	}

	simulated function PostReloadDone()
	{
		GotoState( 'Active' );
	}

	simulated function BeginFire(byte FireModeNum)
	{
		if ( FireModeNum == 0 && CanFire(FireModeNum) && !bDeleteMe && Instigator != None )
		{
			Global.BeginFire(FireModeNum);
			if( PendingFire(FireModeNum) )
				SendToFiringState(FireModeNum);
		}
	}
}

simulated function int GetReloadableCount()
{
	return Min( ClipCnt, AmmoCount ) - ReloadCnt;
}

simulated state WeaponReloading
{
	ignores SwitchToNextSightMode, OnAnimEnd;

	function DoReload();

	simulated function BeginState(name PrevStateName)
	{
		Super.BeginState(PrevStateName);
		StartReload();
	}

	simulated function StartReload( optional bool bFirstReload = true )
	{
		local float	RealReloadTime;
		local array<avaPawn>	FriendlyPawn;

		ReleasedSightMode	= 0;
		bForceReload		= false;

		if ( bFirstReload == true && bReloadClip == false )
		{
			ReloadableCount = GetReloadableCount();
		}
		
		RealReloadTime = ReloadTime * avaPawn(Instigator).WeapTypeAmp[WeaponType].ReloadAmp;
		PlayWeaponAnimation( WeaponReloadAnim, RealReloadTime );
		SetTimer( RealReloadTime, false, 'ReloadDone');

		// 3인칭용
		if ( Role == Role_Authority )
			avaPawn(Instigator).PlayReloadAnimation( EBT_Reload );

		
		if ( Instigator.IsLocallyControlled() && bFirstReload == true )
		{
			// 자동 외침...
			if ( avaPawn(Instigator).GetNearPawn( `AUTORADIOMSG_DISTANCE, true, FriendlyPawn, 1 ) == true )	
			{
				if( avaPlayerController( Instigator.Controller ) != None )
					avaPlayerController( Instigator.Controller ).RaiseAutoMessage( AUTOMESSAGE_Reloading, false );
			}
			//class'avaRadioAutoMessage'.Static.ClientReceive( PlayerController(Instigator.Controller), 
			//												 AUTOMESSAGE_Reloading, 
			//												 Instigator.Controller.PlayerReplicationInfo );;

			if ( Instigator.IsFirstPerson() )
				ChangeVisibility(true);
		}
	}

	simulated function ReloadDone()
	{
		if ( Role == ROLE_Authority )
		{
			if ( bReloadClip == true )
			{
				if( GetAmmoCount() > ClipCnt )
					ReloadCnt = ClipCnt;
				else
					ReloadCnt = GetAmmoCount();
			}
			else
			{
				++ReloadCnt;
			}
		}

		if ( PendingFire(0) )
		{
			SendToFiringState(0);
		}
		else
		{
			--ReloadableCount;
			if ( bReloadClip == false && ReloadableCount > 0 )
			{
				StartReload( false );
			}
			else 
			{
				if ( PostReloadTime > 0.0 )
					GotoState('PostReloading');
				else	
					GotoState('Active');
			}
		}
	}

	simulated function StartFire(byte FireModeNum)
	{
		if ( bReloadClip == false )
			global.StartFire( FireModeNum );
	}

	simulated function EndState(name NextStateName)
	{
		PlayIdleAnim( None );
		ClearTimer( 'ReloadDone' );
		Super.EndState(NextStateName);
	}

	simulated function BeginFire(byte FireModeNum)
	{
		if ( FireModeNum == 0 && CanFire(FireModeNum) && !bDeleteMe && Instigator != None && bEnableFireWhenReload )
		{
			Global.BeginFire(FireModeNum);
			if( PendingFire(FireModeNum) )
				SendToFiringState(FireModeNum);
		}
	}

	/**
	 * Ignore PlayFireEffects calls 
	 */

	simulated function PlayFireEffects( byte FireModeNum, optional vector HitLocation );

	
	/**
	 * When attempting to put the weapon down, look to see if our MinReloadPct has been met.  If so just put it down
	 */
	simulated function bool TryPutDown()
	{
		local float MinTimerTarget;
		local float TimerCount;

		bWeaponPutDown = true;

		MinTimerTarget = GetTimerRate('ReloadDone') * 0.75;
		TimerCount = GetTimerCount('ReloadDone');

		if (TimerCount < MinTimerTarget)
		{
			ClearTimer('ReloadDone');
			PutDownWeapon();
			if (ROLE == ROLE_Authority)
			{
				bForceReload=true;
			}
			return true;
		}
		return false;
	}
}

exec simulated function EjectBullet()
{
	//local vector v;
	//if ( BulletPSC == None && TryAttachBulletPSC == false )
	//{
	//	AttachBulletEjector();
	//	return;
	//}

	//if ( BulletPSC != None && Mesh.HiddenGame == false )
	//{
	//	v	= Instigator.Velocity;
	//	if ( v.z > 0.0 )	v.z = 0.0;
	//	BulletPSC.PartSysVelocity = v;
	//	BulletPSC.ActivateSystem();
	//	//SetTimer( 3, FALSE, 'StopBulletEjector' );
	//}
	local Emitter	Bullet;			// The Effects for the explosion 
	local vector	l;
	local rotator	r;
	local vector	v;

	if ( BulletTemplete != None && Mesh.HiddenGame == false )
	{
		v = Instigator.Velocity;
		v.z = 0.0;
		
		//BulletEmitter.ParticleSystemComponent.PartSysVelocity = v;					
		//BulletEmitter.ParticleSystemComponent.ActivateSystem();

		if ( SkeletalMeshComponent(Mesh).GetSocketWorldLocationAndRotation( 'ejector', l, r ) )
		{
			r.Pitch = 0;
			r.Roll	= 0;
			Bullet = avaGameReplicationInfo(WorldInfo.GRI).ObjectPool.GetBulletEmitter(BulletTemplete,l, r);						
			Bullet.ParticleSystemComponent.PartSysVelocity = v;					
			Bullet.ParticleSystemComponent.ActivateSystem();
			Bullet.ParticleSystemComponent.SetOcclusionGroup(Mesh);
			
			Bullet.SetTimer( 3, FALSE, 'HideSelf' );
		}
	}
}

// Dash 중에는 SightMode 변경 불가
simulated state WeaponDash
{
	ignores SwitchToNextSightMode, StartFire;
}

// Zoom 상태에서는 Dash 가 불가능하다.
simulated function bool IsAvailableDash()
{
	return ( SightMode == 0 );
}

simulated function float GetTraceRange()
{
	return WeaponRange * avaPawn( Instigator ).WeapTypeAmp[WeaponType].RangeAmp;
}

simulated function AttachMuzzleFlash()
{
	if (InSightMuzzleFlashPSC != none)
	{
		InSightMuzzleFlashPSC.DeactivateSystem();
	}
	Super.AttachMuzzleFlash();
}

simulated event CauseMuzzleFlash()
{
	if ( bMountSilencer )	return;
	if ( SightMode > 0 && bHideWeaponInSightMode )
	{
		if (InSightMuzzleFlashPSC != none && InSightMuzzleFlashPSCTemplate != None )
		{
			InSightMuzzleFlashPSC.SetTemplate(InSightMuzzleFlashPSCTemplate);
			InSightMuzzleFlashPSC.ActivateSystem();
		}
	}
	Super.CauseMuzzleFlash();
}

simulated event StopMuzzleFlash()
{
	if ( InSightMuzzleFlashPSC != none )
	{
		InSightMuzzleFlashPSC.DeactivateSystem();
	}
	Super.StopMuzzleFlash();
}

simulated function StopFireEffects(byte FireModeNum)
{
	Super.StopFireEffects( FireModeNum );
	//StopBulletEjector();
}

simulated event MuzzleFlashTimer()
{
	if ( InSightMuzzleFlashPSC != none )
	{
		InSightMuzzleFlashPSC.DeactivateSystem();
	}
	Super.MuzzleFlashTimer();
}

simulated function AttachBulletEjector()
{
	//local SkeletalMeshComponent	SKMesh;
	//local vector				v;


	//SKMesh = SkeletalMeshComponent(Mesh);
	//TryAttachBulletPSC = true;
	//if ( SkMesh.GetSocketByName( 'ejector' ) == None )
	//	return;

	//if ( BulletTemplete != none )
	//{
	//	BulletPSC = new(self) class'avaParticleSystemComponent';
	//	SKMesh.AttachComponentToSocket( BulletPSC, 'ejector' );
	//	BulletPSC.SetTemplate( BulletTemplete );

	//	v	= Instigator.Velocity;
	//	if ( v.z > 0.0 )	v.z = 0.0;
	//	BulletPSC.PartSysVelocity = v;
	//	BulletPSC.ActivateSystem();
	//}
//	BulletEmitter = avaEmitter( avaGameReplicationInfo(WorldInfo.GRI).ObjectPool.GetBulletEmitter(BulletTemplete,v, r) );

	//SKMesh = SkeletalMeshComponent(Mesh);
	//SKMesh.AttachComponentToSocket(BulletEmitter.ParticleSystemComponent, 'ejector' );
	//BulletEmitter.HideSelf();
	//`log( "avaWeapon.AttachBulletEjector" );
}

simulated function DetachBulletEjector()
{
	local SkeletalMeshComponent SKMesh;
	TryAttachBulletPSC = false;
	SKMesh = SkeletalMeshComponent(Mesh);
	if (BulletPSC != none)
		SKMesh.DetachComponent( BulletPSC );
}

//simulated event StopBulletEjector()
//{
//	if ( BulletPSC != None )
//	{
//		//BulletPSC.DeactivateSystem();
//		ClearTimer( 'StopBulletEjector' );
//	}
//}

static simulated function PreCache( out array< object > outList )
{
	Super.PreCache( outList );
	DLO( default.ScopeMeshName, outList );	
	DLO( default.SilencerMeshName, outList );	
}

static event LoadDLOs()
{
	local array< object > outList;
	super.LoadDLOs();
	PreCache( outList );
}

simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	//local string T;
	super.DisplayDebug(HUD, out_YL, out_YPos);
	HUD.Canvas.DrawText( "Weapon :" @self@ "SightMode :" @SightMode@ "Current State" @GetStateName() );
	out_YPos += out_YL;
	//if ( Instigator == None ) return;
	//T = "iShotsFired :" @iShotsFired@ "LastFire :" @LastFire;
	//HUD.Canvas.DrawText(T, false);
	//out_YPos += out_YL;
}

defaultproperties
{
	/* 고의적으로 Components에 추가하지 않은 것입니다 */
	Begin Object Class=avaBulletTrailComponent name=BulletTrailComponent0				
	End Object
	BulletTrailComponent=BulletTrailComponent0

	/* 고의적으로 0으로 set */
	TrailInterval			=	0

	WeaponColor=(R=32,G=255,B=32,A=255)
	WeaponColorInNVG=(R=255,G=32,B=32,A=255)

	FireInterval(0)=+0.0875	
	
	//WeaponFireSnd=SoundCue'A_Weapon.Enforcers.Cue.A_Weapon_Enforcers_Fire01_Cue'	

	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InventoryGroup=2
	GroupWeight=0.5

	AmmoCount=45
	MaxAmmoCount=90	

	WeaponFireTypes(0)=EWFT_InstantHit

	RangeModifier = 1.0	
	
	HitDamage = 30

	AccuracyDivisor = 200
	AccuracyOffset = 0.35
	MaxInaccuracy = 1.25

	ClipCnt=35
	ReloadCnt=35

	EquipTime=+0.2
	PutDownTime=+0.2

	InstantHitMomentum(0)		=	50000.0
	InstantHitMomentum(1)		=	0.0
	InstantHitDamageTypes(0)	=	class'avaDmgType_Gun'
	InstantHitDamageTypesS(0)	=	class'avaDmgType_GunSilencer'

	Kickback_WhenMoving			=	(UpBase=1.0,LateralBase=0.45,UpModifier=0.28,LateralModifier=0.045,UpMax=3.75,LateralMax=3,DirectionChange=7)
	Kickback_WhenFalling		=	(UpBase=1.2,LateralBase=0.5,UpModifier=0.23,LateralModifier=0.15,UpMax=5.5,LateralMax=3.5,DirectionChange=6)
	Kickback_WhenDucking		=	(UpBase=0.6,LateralBase=0.3,UpModifier=0.2,LateralModifier=0.0125,UpMax=3.25,LateralMax=2,DirectionChange=7)
	Kickback_WhenSteady			=	(UpBase=0.65,LateralBase=0.35,UpModifier=0.25,LateralModifier=0.015,UpMax=3.5,LateralMax=2.25,DirectionChange=7)

	Spread_WhenMoving			=	(param1=0.04,param2=0.07)
	Spread_WhenFalling			=	(param1=0.04,param2=0.3)
	Spread_WhenDucking			=	(param1=0,param2=0.0375)
	Spread_WhenSteady			=	(param1=0,param2=0.0375)

	SightInfos(0) = (FOV=90,ChangeTime=0.1)
	NumFiresPerShot = 1

	CrossHairSpeed = 2.5

	WeaponReloadAnim=WeaponReload

	WeaponNoAmmoSnd=SoundCue'avaWeaponSounds.Common.ClipEmpty.Clip_Empty_1'

	bCanThrow			= true
	PickupClass			= class'avaSwappedPickUp'

	NoAmmoSoundInterval = 1.0

	BulletTemplete = ParticleSystem'avaEffect.Gun_Effect.Ps_Wp_Rifle_cartridge'

	SpreadDecayTime = 0.4

	UnZoomPenalty	= 0.0

	Begin Object Class=avaParticleSystemComponent Name=ParticleComponent0
		bOnlyOwnerSee=true
		DepthPriorityGroup=SDPG_Foreground
		Translation=(X=60.0,Y=4.0,Z=-15.0)
	End Object
	InSightMuzzleFlashPSC	=	ParticleComponent0
	Components.Add(ParticleComponent0)

	BobDampingInSight		=	0.8500
	bReloadClip				=	true
//	CrossHairMtrl=Texture2D'EngineResources.BlendedMaterialIcon'

	SightInAnim					=	Zoom_In 
	SightOutAnim				=	Zoom_Out

	RecoverZoomTime				=	0.1			
	bHideCursorInSightMode		=	true
	bHideWeaponInSightMode		=	true

	MuzzleFlashLightClass=class'avaGame.avaGunMuzzleFlashLight'

	
	//Begin Object Class=LightFunction Name=FlashLightFunction
	//	SourceMaterial=Material'avaEffect.Light.M_FlashLight'
	//	Scale=(X=1024.0,Y=1024.0,Z=1024.0)
	//End Object
	//
	//Begin Object Class=avaFlashlightComponent Name=SpotLightComponent0
	//    LightAffectsClassification	=	LAC_DYNAMIC_AND_STATIC_AFFECTING
	//    CastShadows					=	FALSE
	//    CastStaticShadows			=	FALSE
	//    CastDynamicShadows			=	FALSE
	//    bForceDynamicLight			=	FALSE
	//    UseDirectLightMap			=	FALSE
	//    LightingChannels			=	(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
	//	bEnabled					=	TRUE
	//	InnerConeAngle				=	12
	//	OuterConeAngle				=	20
	//	Brightness					=	2.0
	//	Function					=	FlashLightFunction
	//End Object
	//FlashlightComponent = SpotLightComponent0
	
	//Begin Object Class=StaticMeshComponent Name=SpotLightComponent1
	//	bUseAsOccluder		=	FALSE
	//	StaticMesh			=	StaticMesh'avaEffect.Light.MS_Flash'
	//	HiddenGame			=	TRUE
	//	CastShadow			=	FALSE
	//	bAcceptsLights		=	FALSE
	//	bOnlyOwnerSee		=	TRUE
	//	DepthPriorityGroup	=	SDPG_Foreground
	//	BlockZeroExtent		=	false
	//	BlockNonZeroExtent	=	false
	//End Object
	//LightConeComponent=SpotLightComponent1

	bEjectBulletWhenFire			=	true
	CrossHairMultiflier				=	1

	FireIntervalMultiplierA			=	1

	UnZoomSpreadAmp					=	1.0
	ZoomSpreadAmp					=	1.0

	bDropWhenDead					=	true

	Kickback_LateralLimit = 10
	Kickback_UpLimit = 10
	DisplayedSpreadMax = 0.05
}
