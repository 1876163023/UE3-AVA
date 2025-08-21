/*=============================================================================
  avaWeap_BaseMachineGun
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/06/09 by OZ
			MachingGun 구현을 위한 Class

	2006/07/18 by OZ
			기본 Machine Gun 의 경우 거치가 안되도록 기획 수정, 거치 관련 Code 주석 처리함

ToDo.
	1. 거치
		1-1. 마우스 좌클릭으로 총기를 거치한다.
			1-1-1. 거치가능 여부의 판별
					거치가능 여부는 거치볼륨을 이용하도록 한다.
			1-1-2. 거치가 가능한 곳을 UI 에서 표시해줘야 한다.
			1-1-3. 거치가 실패했을 경우 Message 를 찍어줘야 한다.
		1-2. 거치 상태에서는 크로스헤어를 움직일 수 있는 각도가 제한된다.						[완료]
		1-3. 거치 상태일 경우에는 FOV 가 60 도로 확대된다.										[완료]
		1-4. 거치 상태에서 마우스 좌클릭으로 거치를 푼다.										[완료]
		1-5. 일반 상태와 거치 상태에서의 총기 수치에 차이가 있다.
		1-6. 거치가 불가능 할 경우 화면상에 '거치가 불가능 하다' 라는 메세지를 표시한다.
		1-7. 거치중에는 이동이 불가능하다.														[완료]
		1-8. 거치중에는 총기 교환이 불가능하다.													[완료]
		1-9. 거치 상태에서는 총이 화면의 중앙에 위치하게 된다.									[완료]
			1-9-1.	거치시에는 Fire, Idle Animation 이 따로 들어가게 된다.


		

=============================================================================*/
class avaWeap_BaseMachineGun extends avaWeap_BaseRifle;

//var bool	bInstalled;
//var float	InstalledFOV;			// 거치상태에서의 FOV
//
//// Animation & Time
//var name	InstallAnimName;			// MachineGun 설치 Animation Name
//var name	UnInstallAnimName;			// MachineGun 해체 Animation Name
//var float	InstallAnimTime;			// MachineGun 설치 시간
//var float	UnInstallAnimTime;			// MachineGun 해체 시간 
//
//var array<name>	InstallIdleAnimNames;	// MachineGun 설치 후 Idle Animation Name
//var name	InstallFireAnimName;		// MachineGun 설치 후 Fire Animation Name
//
//var float	InstallYawAng;				// 거치시 Volume 의 Yaw Angle
//var bool	bCrouchWhenInstall;			// treu 이면 거치시 앉아야만 한다.
//
//var float	LimitYawAng;				// 거치시 Yaw Angle 제한
//var float	LimitMaxPitchAng;			// 거치시 Pitch Angle 제한
//var float	LimitMinPitchAng;			// 거치시 Pitch Angle 제한

// 거치를 시도한다.
//simulated function AltFire()
//{
//	ServerDoInstall( true );
//}
//
//function avaVolume_MGInstall IsInInstallVolume()
//{
//	local avaVolume_MGInstall V;
//	ForEach Instigator.TouchingActors( class'avaVolume_MGInstall', V )
//		return V;
//	return None;
//}
//
//// 거치가 가능하면 true, 아니면 false 를 return 한다.
//function bool IsAvailableInstall()
//{
//	local avaVolume_MGInstall	V;
//	local float					Incidence;
//	if ( !IsInState( 'Active' ) )												return false;	// Active 상태에서만 거치가 가능하다.
//	if ( Instigator.Physics != PHYS_Walking )									return false;	// PHYS_Walking 상태에서만 거치가 가능하다. Swim, Falling, Ladder 중에는 당연히 불가다.
//
//	// 거치 볼륨안에 있는지 Check 한다.
//	V = IsInInstallVolume();
//	if ( V == None )															return false;
//	// Install Volume 과 같은 Team 인지 Check 한다.
//	if ( V.TeamIdx != TEAM_Unknown && V.TeamIdx != Instigator.GetTeamNum() )	return false;
//	// 앉은 상태에서만 Install 할 수 있는 Volume 인지 Check 한다.
//	if ( V.bCrouch && !Instigator.bIsCrouched )									return false;
//	if ( !V.bCrouch && Instigator.bIsCrouched )									return false;
//
//	// Instigator 의 각도와 Install Volume 의 각도가 어느정도 맞아야 한다. 현재는 90 도 이상 벗어나면 안된다.
//	Incidence = vector( Instigator.GetViewRotation() ) dot vector( V.Rotation );
//	if ( Incidence < 0 )														return false;
//	bCrouchWhenInstall	= V.bCrouch;
//	InstallYawAng		= V.Rotation.Yaw;
//	return true;
//}
//
//function ServerDoInstall( bool bInstall )
//{
//	if ( bInstall && !IsAvailableInstall() )	return;
//	ClientDoInstall( bInstall );
//	ForceDoInstall( bInstall );
//}
//
//simulated function ClientDoInstall( bool bInstall )
//{
//	ForceDoInstall( bInstall );
//}
//
//simulated function ForceDoInstall( bool bInstall )
//{
//	if ( bInstall )	GotoState( 'Installing' );
//	else			GotoState( 'UnInstalling' );
//}
//
//function SetUp( bool bSetUp )
//{
//	local float					MinYawAng, MaxYawAng, MinPitchAng, MaxPitchAng;
//	local avaPlayerController	PC;
//	local avaPawn				Pawn;
//
//	PC		= avaPlayerController( Instigator.Controller );
//	Pawn	= avaPawn( Instigator );
//
//	if ( bSetUp )	// 거치중 이동및 회전 제한...
//	{
//		if ( !PC.IsMoveInputIgnored() )	PC.IgnoreMoveInput( true );
//		PC.IgnoreCrouch( true );
//		if ( bCrouchWhenInstall )		PC.SetAlwaysCrouch( true );
//		
//
//		MinYawAng	= InstallYawAng - LimitYawAng * 65535 / 360;
//		MaxYawAng	= InstallYawAng + LimitYawAng * 65535 / 360;
//		MinPitchAng	= LimitMinPitchAng * 65535 / 360;
//		MaxPitchAng = LimitMaxPitchAng * 65535 / 360;
//
//		Pawn.InstallHeavyWeapon( EXC_InstallHeavyWeapon, MinYawAng, MaxYawAng, MinPitchAng, MaxPitchAng );
//	}
//	else			// 거치해제, 이동및 회전 Default 로...
//	{
//		PC.IgnoreMoveInput( false );
//		Pawn.InstallHeavyWeapon( EXC_None );
//		PC.IgnoreCrouch( false );
//		if ( bCrouchWhenInstall )		PC.SetAlwaysCrouch( false );
//	}
//}
//
//simulated function float AdjustForegroundFOVAngle( float FOV )
//{
//	return bInstalled ? WeaponFOV * InstalledFOV / FOV  : WeaponFOV;
//}
//
//simulated function float AdjustFOVAngle(float FOVAngle)
//{
//	return bInstalled ? InstalledFOV : FOVAngle;
//}
//
//// 거치 상태 전환 중
//simulated state Installing
//{
//	// 거치중 해제 불가
//	ignores AltFire;
//	// 거치중 무기교체 불가
//	simulated function bool DenyClientWeaponSet()	{	return true;	}
//
//	simulated function BeginState( name prvstate )
//	{
//		if ( Role == ROLE_Authority )	SetUp( true );
//		PlayWeaponAnimation( InstallAnimName, InstallAnimTime );	// 거치 Animation Play
//		SetTimer( InstallAnimTime, false, 'InstallDone' );
//	}
//
//	simulated function InstallDone()
//	{
//		GotoState( 'Installed' );
//	}
//
//	simulated function EndState( name nextstate )
//	{
//		ClearTimer( 'InstallDone' );
//		if ( nextstate != 'Installed' )	SetUp( false );
//	}
//}
//
//// 거치 해제중
//simulated state UnInstalling
//{
//	// 거치중 해제 불가
//	ignores AltFire;
//	// 거치중 무기교체 불가
//	simulated function bool DenyClientWeaponSet()	{	return true;	}
//
//	simulated function BeginState( name prvstate )
//	{
//		PlayWeaponAnimation( UnInstallAnimName, UnInstallAnimTime );
//		SetTimer( UnInstallAnimTime, false, 'UnInstallDone' );
//	}
//
//	simulated function UnInstallDone()
//	{
//		GotoState( 'Active' );
//	}
//
//	simulated function EndState( name nextstate )
//	{
//		ClearTimer( 'UnInstallDone' );
//		if ( Role == ROLE_Authority )	SetUp( false );
//	}
//}
//
//// 거치중
//simulated state Installed extends Active
//{
//	// 거치중 무기교체 불가
//	simulated function bool DenyClientWeaponSet()	{	return true;	}
//
//	simulated function AltFire()
//	{
//		ServerDoInstall( false );
//	}
//
//	simulated function BeginState( name prvstate )
//	{
//		bInstalled = true;
//		Super.BeginState( prvstate );
//	}
//
//	simulated function EndState( name nextstate )
//	{
//		Super.EndState( nextstate );
//
//		if ( nextstate != 'WeaponFiring' )
//		{
//			bInstalled = false;
//		}
//	}
//}
//
//
//// Firing State - Active 에서 오는 경우와 Installed 에서 오는 경우 두가지가 있다.
//simulated state WeaponFiring
//{
//	simulated function RefireCheckTimer()
//	{
//		// if switching to another weapon, abort firing and put down right away
//		if( bWeaponPutDown )
//		{
//			PutDownWeapon();
//			return;
//		}
//
//		// If weapon should keep on firing, then do not leave state and fire again.
//		if( ShouldRefire() )
//		{
//			FireAmmunition();
//			return;
//		}
//
//		// Otherwise we're done firing, so go back to active state.
//		if ( bInstalled )	GotoState( 'Installed' );
//		else				GotoState( 'Active' );
//
//		// if out of ammo, then call weapon empty notification
//		if( !HasAnyAmmo() )
//		{
//			WeaponEmpty();
//		}
//	}
//}

//simulated function PlayFireEffects( byte FireModeNum, optional vector HitLocation )
//{
//	`log( "avaWeap_BaseMachineGun.PlayFireEffects" );
//	if ( bInstalled )
//	{
//		if ( InstallFireAnimName != '' )
//			PlayWeaponAnimation( InstallFireAnimName, 0.0 );
//	}
//	else
//	{
//		`log( "avaWeap_BaseMachineGun.PlayFireEffects" @WeaponFireAnim.Length @WeaponFireAnim[FireModeNum] );
//		if ( WeaponFireAnim.Length > FireModeNum && WeaponFireAnim[FireModeNum] != '' )
//		{
//			PlayWeaponAnimation( WeaponFireAnim[FireModeNum], 0.0 );
//			`log( "avaWeap_BaseMachineGun.PlayFireEffects" @WeaponFireAnim[FireModeNum] );
//		}
//	}
//
//	// Start muzzle flash effect
//	CauseMuzzleFlash();
//
//	ShakeView();
//}

//simulated event OnAnimEnd(AnimNodeSequence SeqNode)
//{
//	local int IdleIndex;
//
//	if ( bWeaponPutDown != true )
//	{
//		if ( bInstalled )
//		{
//			if ( WorldInfo.NetMode != NM_DedicatedServer && InstallIdleAnimNames.Length > 0 )
//			{
//				IdleIndex = InstallIdleAnimNames.Length * frand();
//				PlayWeaponAnimation( InstallIdleAnimNames[IdleIndex], 0.0 );
//			}
//		}
//		else
//		{
//			Super.OnAnimEnd( SeqNode );
//		}
//	}
//}
//
//// 거치 상태에서 틀려지는 총기 Parameter 적용
//simulated function ApplyKickback()
//{
//	Super.ApplyKickback();
//}
//
//simulated function float CalcSpread(float fAccuracy)
//{
//	return Super.CalcSpread( fAccuracy );
//}
//
//simulated function float CalcAccuracy( int ShotsFired, float DeltaTime )
//{
//	return Super.CalcAccuracy( ShotsFired, DeltaTime );
//}

defaultproperties
{
	BaseSkelMeshName	=	"WP_Heavy_M249.WP_M249"
	BaseAnimSetName		=	"WP_Heavy_M249.M249_Ani_1P"

	InventoryGroup		=	1

	// Projectile 관련 Properties
	WeaponFireTypes(0)		=	EWFT_InstantHit
	FireOffset				=	(X=22,Y=10)					//

	WeaponFireAnim(0)	=	Fire
 	WeaponPutDownAnim	=	Down
	WeaponEquipAnim		=	BringUp
	WeaponReloadAnim	=	Reload
	WeaponIdleAnims(0)	=	Idle

	//InstallAnimName		=	Reload		// MachineGun 설치 Animation Name
	//UnInstallAnimName	=	Reload		// MachineGun 해체 Animation Name
	//InstallAnimTime		=	2.0			// MachineGun 설치 시간
	//UnInstallAnimTime	=	2.0			// MachineGun 해체 시간 

	//InstallIdleAnimNames(0)	=	Idle;	// MachineGun 설치 후 Idle Animation Name
	//InstallFireAnimName		=	Fire;	// MachineGun 설치 후 Fire Animation Name

	FireInterval(0)		=	0.0875
	EquipTime			=	1.4333
	PutDownTime			=	0.0333
	ReloadTime			=	5.0333

	// Ammo 관련
	ClipCnt=100
	AmmoCount=200
	MaxAmmoCount=200

	// Attachmetn
	

	// Velocity Limit Parameter of Pawn
	BaseSpeed			= 260	// 기본속도
	AimSpeedPct			= 0.7	// 조준시 보정치
	WalkSpeedPct		= 0.4	// 걷기시 보정치
	CrouchSpeedPct		= 0.25	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.25	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	//
	
	bAutoFire			= true
	//InstalledFOV		= 60

	//LimitYawAng			= 45
	//LimitMaxPitchAng	= 45
	//LimitMinPitchAng	= -45

	Penetration			=	1
	WeaponType			=	WEAPON_MACHINEGUN

}
