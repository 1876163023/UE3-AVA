//=============================================================================
//  avaWeap_C4
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/03/10 by OZ
//		
//		���� �߻� �� Switch Weapon �� ����� ������ ���� ������,
//		1��Ī Mesh �ܻ��� ���µ��� ������ ���� �ٽ� ������
//
//		�ΰ��� Mode �� �߻� �����ϵ��� ������
//
//=============================================================================

class avaThrowableWeapon extends avaWeapon
	native;
	// abstract /**< avaWeapon�� ���� Ŭ������ �����Ϳ��� ��Ÿ���� �ʾ� �ּ�ó�� */


// �غ��۰��� 
var(Animations)	array<name>		PrepareAnim;
var(Animations) array<float>	PrepareTime;
var(Sounds)		array<SoundCue>	PrepareSnd;

// Grenade ���� EndFire ���� �������� C4 �� Guage�� ���� �˾Ƽ� ������, �� C4 �� bSkipReady �� true �̴�.
var()			array<bool>		bSkipReady;

// bCancelPrepare �� �����ϸ� Prepare ���߿� Fire �� ��ҵǸ� Active �� ����. �Ұ����ϴٸ� ������ ������ �ȴ�.
var()			array<bool>		bCancelPrepare;

// bCancelPrepare �� ture �� ��쿡 ���ȴ�.
var(Animations)	array<name>		CancelAnim;		
var(Animations)	array<float>	CancelTime;
var(Sounds)		array<SoundCue>	CancelSnd;

// throw animation
var(Animations)	array<name>		ThrowAnim;
var(Animations)	array<float>	ThrowTime;
var(Sounds)		array<SoundCue>	ThrowSnd;

// throw end animation
var(Animations)	array<name>		ThrowEndAnim;
var(Animations)	array<float>	ThrowEndTime;
var(Sounds)		array<SoundCue>	ThrowEndSnd;

// 'Fire in the hole!' avaQuickVoiceMessage�� ��ϵǾ��ִ� ������ �ε���. -1�̸� ������� ����
var				array<int>		ThrowEndRadioSoundIndex;	

// throw animation �� play �� �� �����Ŀ� �߻��� ���ΰ�?
var()			array<float>	FireTime;

// true �̸� ������ Weapon �� �ٲ۴�.
var	bool	SwitchWeaponAfterFire;



var array<float>	ProjStartVel;
var array<float>	ProjZAng;
var	array<float>	ProjTossZ;

var	bool	ManualIncrementFlashCount;
var bool	bDisableChangeState;
var bool	bDrop;


simulated function float GetPrepareTime()
{
	return PrepareTime[CurrentFireMode] * avaPawn(Instigator).ThrowableWeapReadyAmp;
}

simulated function ForcePrepare(byte FireModeNum)
{
	if( firemodenum >= firingstatesarray.length )
	{
		weaponlog("invalid firemodenum", "weapon::sendtofiringstate");
		return;
	}
	// needs a state name, and ignores a none fire type
	if( firingstatesarray[firemodenum] == '' ||
		weaponfiretypes[firemodenum] == ewft_none )
	{
		return;
	}
	// set current fire mode
	setcurrentfiremode(firemodenum);
	// transition to firing mode state
	gotostate(firingstatesarray[firemodenum]);
}

reliable client function ClientForcePrepare(byte FireModeNum)
{
	if ( Role == ROLE_Authority )	return;
	ForcePrepare(FireModeNum);
}

// ����ź���� ���, �̹��� Timing �� ������ �ɰ��� ������ �߻��� �� �����Ƿ� �ѹ� �� Check �ϵ��� �Ѵ�....
simulated function SendToFiringState(byte FireModeNum)
{
	if ( Role < ROLE_Authority )
	{
		GotoState( 'WeaponPrepareReserved' );
		return;
	}
	ClientForcePrepare(FireModeNum);
	ForcePrepare(FireModeNum);
}


reliable client function ForceGotoWeaponReady()
{
	GotoState( 'WeaponReady' );
}

reliable client function ForceGotoActive()
{
	GotoState( 'Active' );
}

simulated function GotoWeaponReady()
{
	GotoState( 'WeaponReady' );
}

simulated function GotoActive()
{
	GotoState( 'Active' );
}

simulated state WeaponPrepareReserved
{
	simulated function bool TryPutDown()
	{
		return false;
	}
}

// �غ���... ����ź�� ���� �̴´ٴ���... C4�� ��ư�� �����ٴ���...
simulated state WeaponPreparing
{
	ignores OnAnimEnd;

	simulated function BeginState( Name PreviousStateName )
	{
		local float	Time;

		`log( "avaThrowableWeapon.WeaponPreparing.BeginState" @PreviousStateName );
		Time = GetPrepareTime();
		if ( Time > 0 )
		{
//			`log( "avaThrowableWeapon.WeaponPreparing.BeginState SetTimer" @PrepareTime[CurrentFireMode] );
			SetTimer( Time, false, 'EndPrepare' );
			if ( PrepareAnim.length > CurrentFireMode && PrepareAnim[CurrentFireMode] != '' )
				PlayWeaponAnimation( PrepareAnim[CurrentFireMode], Time );
		}
		else
		{
			EndPrepare();
		}

		if ( PrepareSnd.length > CurrentFireMode )
			WeaponPlaySound( PrepareSnd[CurrentFireMode] );

		// PullPin State �� �ٲ��ش�.
		if ( Role==ROLE_Authority && bDisableChangeState == false )
		{
			avaPawn(Instigator).PlayPullPinAnimation();
			avaPawn(Instigator).ChangeWeaponState( 1 );
		}
	}

	simulated event EndPrepare()
	{
		//if ( Role == ROLE_Authority )
		//{
			if ( StillFiring( CurrentFireMode ) || bCancelPrepare[CurrentFireMode] == false )
			{
				//ForceGotoWeaponReady();
				GotoWeaponReady();
				//GotoState( 'WeaponReady' );
			}
			else
			{
				//ForceGotoActive();
				GotoActive();
			}
				//GotoState( 'Active' );
		//}
	}	

	// Host ���� Sync ������ �߻��Ѵ�....
	// Client ���� �ܵ������� Active ���·� ������ �ȵȴ�...
	simulated function EndFire(byte FireModeNum)
	{
		super.EndFire( FireModeNum );
		//if ( Role == ROLE_Authority )
		//{
		if ( bCancelPrepare[CurrentFireMode] )
		{
			//ForceGotoActive();
			GotoActive();
		}
				//GotoState( 'Active' );
		//}
	}

	simulated function bool TryPutDown()
	{
		return false;
	}

	simulated function EndState(Name NextStateName)
	{
		`log( "avaThrowableWeapon.WeaponPreparing.EndState" @NextStateName );
		ClearTimer('EndPrepare');
		if ( NextStateName == 'Active' )
			PlayCancelingAnim();
	}

	simulated function PlayCancelingAnim()
	{
		if ( CancelSnd.length > CurrentFireMode )
			WeaponPlaySound( CancelSnd[CurrentFireMode] );
		if ( CancelAnim.length > CurrentFireMode )
			PlayWeaponAnimation( CancelAnim[CurrentFireMode], CancelTime[CurrentFireMode] );
	}

	simulated event bool IsFiring()
	{
		return true;
	}

	simulated function bool DenyClientWeaponSet()
	{
		return true;
	}

	simulated function bool CanEnterVehicle()
	{
		return false;
	}
}

simulated state WeaponReady
{
	ignores OnAnimEnd;

	simulated function BeginState( Name PreviousStateName )
	{
		`log( "avaThrowableWeapon.WeaponReady.BeginState" @PreviousStateName );
		if ( !StillFiring(CurrentFireMode) )
		{
			GotoState( 'WeaponThrow' );
		}
		else if ( bSkipReady[CurrentFireMode] )
		{
			//ClearPendingFire( 0 );
			GotoState( 'WeaponThrow' );
			// Stop Fire �� ȣ�������� �������� time ���� ���ؼ� 
			// ������ WeaponPreparing �����̸� �뷫 ������ ��Ȳ�� �߻��Ѵ�.
			//StopFire( CurrentFireMode );	
		}
	}

	simulated event bool IsFiring()
	{
		return true;
	}

	simulated function EndFire(byte FireModeNum)
	{
		super.EndFire( FireModeNum );
		if ( CurrentFireMode == FireModeNum )
		{
			GotoState( 'WeaponThrow' );
		}
	}

	simulated function bool TryPutDown()
	{
		return false;
		//bWeaponPutDown = true;
		//PutDownWeapon();			// 'WeaponPuttingDown' ���� ������ \.
		//return true;
	}

	simulated function bool DenyClientWeaponSet()
	{
		return true;
	}

	simulated function EndState( Name NextStateName )
	{
		`log( "avaThrowableWeapon.WeaponReady.EndState" @NextStateName );
	}

	simulated function bool CanEnterVehicle()
	{
		return false;
	}
}

simulated state WeaponThrow
{
	ignores OnAnimEnd, StartFire;;

	simulated function BeginState( Name PreviousStateName )
	{
		`log( "avaThrowableWeapon.WeaponThrow.BeginState" @PreviousStateName );
		PlayThrowingAnim();
	}

	simulated function PlayThrowingAnim()
	{
		//if ( bSkipReady[CurrentFireMode] )
		//	StopFire( CurrentFireMode );
		// 3��Ī���� ������ Animation �� Play ���ش�.
		// ������ ProjectileFire �� �־����� �׷� Timing �� ���� �ʴ´�.
		if ( ManualIncrementFlashCount == false )
		{
//			`log( "IncremetFlashCount" );
			IncrementFlashCount();
		}

		if ( ThrowSnd.length > CurrentFireMode )
			WeaponPlaySound( ThrowSnd[CurrentFireMode] );

		if ( ThrowTime[CurrentFireMode] > 0 )
		{
			SetTimer( ThrowTime[CurrentFireMode], false, 'ThrowDone' );

			if ( ThrowAnim.length > CurrentFireMode && ThrowAnim[CurrentFireMode] != '' )
				PlayWeaponAnimation( ThrowAnim[CurrentFireMode], ThrowTime[CurrentFireMode] );

			// �Ѿ� �߻�ð��� Throw Time ���ٴ� ��ƾ� �Ѵ�.
			if ( FireTime[CurrentFireMode] > ThrowTime[CurrentFireMode] )
			{	
				`warn( "FireTime is too long" );
				FireTime[CurrentFireMode] = ThrowTime[CurrentFireMode] - 0.1;
			}

			if ( FireTime[CurrentFireMode] <= 0.0 )	
				Fire();
			else					
				SetTimer( FireTime[CurrentFireMode], false, 'Fire' );
		}
		else
		{
			Fire();

			ThrowDone();
		}
	}

	simulated function Fire()
	{
		local avaPlayerController avaPC;

		FireAmmunition();
		if ( bSkipReady[CurrentFireMode] )
			ClearPendingFire( CurrentFireMode );

		// ������ "Fire In the Hole"�� ���� ���� �޽����� �����ش�
		if( Role == ROLE_Authority )
		{
			if( ThrowEndRadioSoundIndex[CurrentFireMode] != -1 && Instigator != None )
			{
				avaPC = avaPlayerController( Instigator.Controller );
				if( avaPC != None )
					avaPC.RaiseAutoMessage( ThrowEndRadioSoundIndex[CurrentFireMode], false );
			}
		}
	}

	simulated function ThrowDone()
	{
		PlayThrowEndAnim();
	}

	simulated function PlayThrowEndAnim()
	{
		if ( ThrowEndTime[CurrentFireMode] > 0.0 )
		{
			if ( ThrowEndAnim.length > CurrentFireMode && ThrowEndAnim[CurrentFireMode] != '' )
				PlayWeaponAnimation( ThrowEndAnim[CurrentFireMode], ThrowEndTime[CurrentFireMode] );
		}

		if ( ThrowEndSnd.length > CurrentFireMode )
		{
			WeaponPlaySound( ThrowEndSnd[CurrentFireMode] );
		}

		if ( ThrowEndTime[CurrentFireMode] > 0 )
			SetTimer( ThrowEndTime[CurrentFireMode], false, 'ThrowEndDone' );
		else
			ThrowEndDone();
	}

	// WeaponEmpty �� ��쿡 Active �� ������ ������ override ����
	simulated function WeaponEmpty()
	{
	}

	simulated function ThrowEndDone()
	{
		// ��ô����� ���� �� ���Ⱑ �ٲ��.
		// 2006/02/23 by OZ
		GotoState( 'WeaponThrowDone' );
	}

	simulated function EndState( Name NextStateName )
	{
		`log( "avaThrowableWeapon.WeaponThrow.EndState" @NextStateName );
		if ( ManualIncrementFlashCount == false )
		{
			ClearFlashCount();
			ClearFlashLocation();
		}
		ClearTimer('ThrowDone');
		ClearTimer('ThrowEndDone');

		// Normal State �� �ٲ��ش�.
		if ( Role==ROLE_Authority && bDisableChangeState == false )
		{
			avaPawn(Instigator).ChangeWeaponState( 0 );
		}

		if ( SwitchWeaponAfterFire )
		{
			//GotoState( 'Inactive' );
			if ( DestroyWeaponEmpty && !HasAnyAmmo() )
			{
				bRemoveReserved = true;
				bHideWeaponMenu	= true;
				bNoSelectable	= true;
				`log( "avaThrowableWeapon.WeaponThrowDone" @self @bRemoveReserved );
				//LifeSpan = 1.0;
			}
			//if( Role == ROLE_Authority && Pawn(Owner) != None )
			//	avaInventoryManager( Instigator.InvManager ).SwitchToBestWeapon( true );
				//avaInventoryManager( Instigator.InvManager ).SwitchWeapon( 1 );
		}
		//else
		//{
		//	GotoState( NextStateName );
		//}
	}

	simulated event bool IsFiring()
	{
		return true;
	}

	simulated function bool DenyClientWeaponSet()
	{
		return true;
	}

	simulated function bool CanEnterVehicle()
	{
		return false;
	}
}

simulated state WeaponThrowDone
{
	ignores	StartFire;
	simulated function BeginState( name PrevState )
	{
		if ( !SwitchWeaponAfterFire )
		{
			GotoState( 'Active' );
		}
		else
		{
			if ( Instigator.IsLocallyControlled() && avaPawn(Instigator) != None )
				avaInventoryManager( Instigator.InvManager ).SwitchToBestWeapon( true );
		}
	}

	simulated function bool CanEnterVehicle()
	{
		return false;
	}

	simulated function EndState( name NextState )
	{
		`log( "avaThrowableWeapon.WeaponThrowDone.EndState" @NextState );
		Super.EndState( NextState );
	}

}

function SpawnProjectile()
{
	local vector			RealStartLoc;
	local Projectile		SpawnedProjectile;

	// this is the location where the projectile is spawned.
	RealStartLoc = GetPhysicalFireStartLoc();

	// Spawn projectile
	if ( WeaponProjectiles[CurrentFireMode] != None )
	{
		SpawnedProjectile = Spawn( WeaponProjectiles[CurrentFireMode], Self,, RealStartLoc, , , true);
		if( SpawnedProjectile != None && !SpawnedProjectile.bDeleteMe )
		{
			SpawnedProjectile.Init( Vector(GetAdjustedAim( RealStartLoc )) );
			if ( avaProjectile(SpawnedProjectile) != none )
			{
				avaProjectile(SpawnedProjectile).InitStats(self);
			}
		}
	}
	bDrop = true;
}

// IncrementFlashCount() �� ���⼭ ���� �ʱ� ���ؼ� Override ����.
simulated function Projectile ProjectileFire()
{
	if( Role == ROLE_Authority )
	{
		SpawnProjectile();
		UpdateFiredStats(1);
	}
	return None;
}

function float GetProjectileVel()
{
	return ProjStartVel[CurrentFireMode] * avaPawn(Instigator).ProjectileVelAmp;
}

function float GetProjectileZAng()
{
	return ProjZAng[CurrentFireMode];
}

function float GetProjectileTossz()
{
	return ProjTossZ[CurrentFireMode];
}

simulated function bool ThrowWeapon( optional bool bDoNotSwitch )
{
	if ( IsInState( 'WeaponThrow' ) || IsInState( 'WeaponReady' ) || IsInState( 'WeaponPreparing' ) )
	{
		if (bDrop == false )
		{
			GotoState('Dead');
			SpawnProjectile();
		}
		return true;
	}
	else
	{
		return Super.ThrowWeapon(bDoNotSwitch);	
	}
}

state Dead
{
	function float GetProjectileVel()
	{
		return 0.0;
	}

	function float GetProjectileZAng()
	{
		return -0.1;
	}
}

simulated function bool IsProjectileSpin()
{
	return true;
}

defaultproperties
{
	WeaponColor=(R=255,G=0,B=64,A=255)
	FireInterval(0)=+0.0875	

	//WeaponFireSnd=SoundCue'A_Weapon.Enforcers.Cue.A_Weapon_Enforcers_Fire01_Cue'	

	//PickupSound=SoundCue'A_Pickups.Weapons.Cue.A_Pickup_Weapons_Enforcer_Cue'

	InventoryGroup=2
	GroupWeight=0.5

	AmmoCount=45
	MaxAmmoCount=90	

	bNeverForwardPendingFire=true

	Begin Object Name=MeshComponent0
		Rotation=(Yaw=-16384)
	End Object

	WeaponFireAnim(0)	=	None
 	WeaponPutDownAnim	=	None
	WeaponEquipAnim		=	BringUp
	EquipTime			=	0.5667
	PutDownTime			=	0.0
	WeaponIdleAnims(0)	=	Idle
	
	// ����ź�� �⺻������
	PrepareAnim(0)		=	PinUpd
	PrepareTime(0)		=	0.60
	PrepareSnd(0)		=	None

	bCancelPrepare(0)	=	false
	bSkipReady(0)		=	false

	//CancelAnim		=	WeaponAltFireLaunch1End
	//CancelTime=   0.3
	//CancelSnd=

	ThrowAnim(0)		=	Fire1
	ThrowTime(0)		=	0.3667
	//ThrowSnd=
	//ThrowEndAnim		=	BringUp
	ThrowEndTime(0)		=	0.5667
	//ThrowEndSnd=
	FireTime(0)			=	0.0733		//  ������ Animation ���� ��ȯ�� ���� �ִ� Projectile �� ������ ���ΰ�?

	FiringStatesArray(0)=	WeaponPreparing
	FiringStatesArray(1)=	none

	WeaponFireTypes(0)	=	EWFT_Projectile
	WeaponFireTypes(1)	=	EWFT_None

	FireOffset			=	(X=20,Y=16,Z=0)

	ThrowEndRadioSoundIndex(0)	=	-1

	DestroyWeaponEmpty		=	true
	SwitchWeaponAfterFire	=	true

	ProjStartVel(0)			=	1600
	ProjZAng(0)				=	0.1
	ProjZAng(1)				=	-0.1

	// Weapon �� ���� Pawn �� �ӵ� ����
	BaseSpeed			= 285	// �⺻�ӵ�
	AimSpeedPct			= 0.8	// ���ؽ� ����ġ
	WalkSpeedPct		= 0.4	// �ȱ�� ����ġ
	CrouchSpeedPct		= 0.25	// �ɾ��̵��� ����ġ
	CrouchAimSpeedPct	= 0.2	// �ɾƼ� ���� �̵��� ����ġ
	SwimSpeedPct		= 0.7	// ������ ����ġ
	SprintSpeedPct		= 1.6	// ������Ʈ�� ����ġ
	CrouchSprintSpeedPct= 1.1	// �ɾƼ� ������Ʈ�� ����ġ

	bCanThrow			=	false
	PickUpClass			=	class'avaPickUp'
	PickUpAddAmmo		=	false
	bDropWhenDead		=	true
	bDropIfHasAmmo		=	true

	CrossHairMtrl		=	Texture2D'avaUI.HUD.CrossImage'	
}
