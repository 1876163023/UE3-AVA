/*=============================================================================
  avaWeap_BaseMachineGun
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/06/09 by OZ
			MachingGun ������ ���� Class

	2006/07/18 by OZ
			�⺻ Machine Gun �� ��� ��ġ�� �ȵǵ��� ��ȹ ����, ��ġ ���� Code �ּ� ó����

ToDo.
	1. ��ġ
		1-1. ���콺 ��Ŭ������ �ѱ⸦ ��ġ�Ѵ�.
			1-1-1. ��ġ���� ������ �Ǻ�
					��ġ���� ���δ� ��ġ������ �̿��ϵ��� �Ѵ�.
			1-1-2. ��ġ�� ������ ���� UI ���� ǥ������� �Ѵ�.
			1-1-3. ��ġ�� �������� ��� Message �� ������ �Ѵ�.
		1-2. ��ġ ���¿����� ũ�ν��� ������ �� �ִ� ������ ���ѵȴ�.						[�Ϸ�]
		1-3. ��ġ ������ ��쿡�� FOV �� 60 ���� Ȯ��ȴ�.										[�Ϸ�]
		1-4. ��ġ ���¿��� ���콺 ��Ŭ������ ��ġ�� Ǭ��.										[�Ϸ�]
		1-5. �Ϲ� ���¿� ��ġ ���¿����� �ѱ� ��ġ�� ���̰� �ִ�.
		1-6. ��ġ�� �Ұ��� �� ��� ȭ��� '��ġ�� �Ұ��� �ϴ�' ��� �޼����� ǥ���Ѵ�.
		1-7. ��ġ�߿��� �̵��� �Ұ����ϴ�.														[�Ϸ�]
		1-8. ��ġ�߿��� �ѱ� ��ȯ�� �Ұ����ϴ�.													[�Ϸ�]
		1-9. ��ġ ���¿����� ���� ȭ���� �߾ӿ� ��ġ�ϰ� �ȴ�.									[�Ϸ�]
			1-9-1.	��ġ�ÿ��� Fire, Idle Animation �� ���� ���� �ȴ�.


		

=============================================================================*/
class avaWeap_BaseMachineGun extends avaWeap_BaseRifle;

//var bool	bInstalled;
//var float	InstalledFOV;			// ��ġ���¿����� FOV
//
//// Animation & Time
//var name	InstallAnimName;			// MachineGun ��ġ Animation Name
//var name	UnInstallAnimName;			// MachineGun ��ü Animation Name
//var float	InstallAnimTime;			// MachineGun ��ġ �ð�
//var float	UnInstallAnimTime;			// MachineGun ��ü �ð� 
//
//var array<name>	InstallIdleAnimNames;	// MachineGun ��ġ �� Idle Animation Name
//var name	InstallFireAnimName;		// MachineGun ��ġ �� Fire Animation Name
//
//var float	InstallYawAng;				// ��ġ�� Volume �� Yaw Angle
//var bool	bCrouchWhenInstall;			// treu �̸� ��ġ�� �ɾƾ߸� �Ѵ�.
//
//var float	LimitYawAng;				// ��ġ�� Yaw Angle ����
//var float	LimitMaxPitchAng;			// ��ġ�� Pitch Angle ����
//var float	LimitMinPitchAng;			// ��ġ�� Pitch Angle ����

// ��ġ�� �õ��Ѵ�.
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
//// ��ġ�� �����ϸ� true, �ƴϸ� false �� return �Ѵ�.
//function bool IsAvailableInstall()
//{
//	local avaVolume_MGInstall	V;
//	local float					Incidence;
//	if ( !IsInState( 'Active' ) )												return false;	// Active ���¿����� ��ġ�� �����ϴ�.
//	if ( Instigator.Physics != PHYS_Walking )									return false;	// PHYS_Walking ���¿����� ��ġ�� �����ϴ�. Swim, Falling, Ladder �߿��� �翬�� �Ұ���.
//
//	// ��ġ �����ȿ� �ִ��� Check �Ѵ�.
//	V = IsInInstallVolume();
//	if ( V == None )															return false;
//	// Install Volume �� ���� Team ���� Check �Ѵ�.
//	if ( V.TeamIdx != TEAM_Unknown && V.TeamIdx != Instigator.GetTeamNum() )	return false;
//	// ���� ���¿����� Install �� �� �ִ� Volume ���� Check �Ѵ�.
//	if ( V.bCrouch && !Instigator.bIsCrouched )									return false;
//	if ( !V.bCrouch && Instigator.bIsCrouched )									return false;
//
//	// Instigator �� ������ Install Volume �� ������ ������� �¾ƾ� �Ѵ�. ����� 90 �� �̻� ����� �ȵȴ�.
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
//	if ( bSetUp )	// ��ġ�� �̵��� ȸ�� ����...
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
//	else			// ��ġ����, �̵��� ȸ�� Default ��...
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
//// ��ġ ���� ��ȯ ��
//simulated state Installing
//{
//	// ��ġ�� ���� �Ұ�
//	ignores AltFire;
//	// ��ġ�� ���ⱳü �Ұ�
//	simulated function bool DenyClientWeaponSet()	{	return true;	}
//
//	simulated function BeginState( name prvstate )
//	{
//		if ( Role == ROLE_Authority )	SetUp( true );
//		PlayWeaponAnimation( InstallAnimName, InstallAnimTime );	// ��ġ Animation Play
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
//// ��ġ ������
//simulated state UnInstalling
//{
//	// ��ġ�� ���� �Ұ�
//	ignores AltFire;
//	// ��ġ�� ���ⱳü �Ұ�
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
//// ��ġ��
//simulated state Installed extends Active
//{
//	// ��ġ�� ���ⱳü �Ұ�
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
//// Firing State - Active ���� ���� ���� Installed ���� ���� ��� �ΰ����� �ִ�.
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
//// ��ġ ���¿��� Ʋ������ �ѱ� Parameter ����
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

	// Projectile ���� Properties
	WeaponFireTypes(0)		=	EWFT_InstantHit
	FireOffset				=	(X=22,Y=10)					//

	WeaponFireAnim(0)	=	Fire
 	WeaponPutDownAnim	=	Down
	WeaponEquipAnim		=	BringUp
	WeaponReloadAnim	=	Reload
	WeaponIdleAnims(0)	=	Idle

	//InstallAnimName		=	Reload		// MachineGun ��ġ Animation Name
	//UnInstallAnimName	=	Reload		// MachineGun ��ü Animation Name
	//InstallAnimTime		=	2.0			// MachineGun ��ġ �ð�
	//UnInstallAnimTime	=	2.0			// MachineGun ��ü �ð� 

	//InstallIdleAnimNames(0)	=	Idle;	// MachineGun ��ġ �� Idle Animation Name
	//InstallFireAnimName		=	Fire;	// MachineGun ��ġ �� Fire Animation Name

	FireInterval(0)		=	0.0875
	EquipTime			=	1.4333
	PutDownTime			=	0.0333
	ReloadTime			=	5.0333

	// Ammo ����
	ClipCnt=100
	AmmoCount=200
	MaxAmmoCount=200

	// Attachmetn
	

	// Velocity Limit Parameter of Pawn
	BaseSpeed			= 260	// �⺻�ӵ�
	AimSpeedPct			= 0.7	// ���ؽ� ����ġ
	WalkSpeedPct		= 0.4	// �ȱ�� ����ġ
	CrouchSpeedPct		= 0.25	// �ɾ��̵��� ����ġ
	CrouchAimSpeedPct	= 0.2	// �ɾƼ� ���� �̵��� ����ġ
	SwimSpeedPct		= 0.7	// ������ ����ġ
	SprintSpeedPct		= 1.25	// ������Ʈ�� ����ġ
	CrouchSprintSpeedPct= 1	// �ɾƼ� ������Ʈ�� ����ġ

	//
	
	bAutoFire			= true
	//InstalledFOV		= 60

	//LimitYawAng			= 45
	//LimitMaxPitchAng	= 45
	//LimitMinPitchAng	= -45

	Penetration			=	1
	WeaponType			=	WEAPON_MACHINEGUN

}
