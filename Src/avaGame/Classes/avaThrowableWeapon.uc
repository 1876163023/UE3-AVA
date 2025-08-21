//=============================================================================
//  avaWeap_C4
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/03/10 by OZ
//		
//		무기 발사 후 Switch Weapon 이 제대로 정상동작 하지 않으며,
//		1인칭 Mesh 잔상이 남는등의 문제로 인해 다시 제작함
//
//		두가지 Mode 로 발사 가능하도록 수정함
//
//=============================================================================

class avaThrowableWeapon extends avaWeapon
	native;
	// abstract /**< avaWeapon의 하위 클래스로 에디터에서 나타나지 않아 주석처리 */


// 준비동작관련 
var(Animations)	array<name>		PrepareAnim;
var(Animations) array<float>	PrepareTime;
var(Sounds)		array<SoundCue>	PrepareSnd;

// Grenade 류는 EndFire 에서 던지지만 C4 는 Guage가 차면 알아서 떨어짐, 즉 C4 는 bSkipReady 가 true 이다.
var()			array<bool>		bSkipReady;

// bCancelPrepare 가 가능하면 Prepare 도중에 Fire 가 취소되면 Active 로 간다. 불가능하다면 무조건 던지게 된다.
var()			array<bool>		bCancelPrepare;

// bCancelPrepare 가 ture 인 경우에 사용된다.
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

// 'Fire in the hole!' avaQuickVoiceMessage에 등록되어있는 사운드의 인덱스. -1이면 사용하지 않음
var				array<int>		ThrowEndRadioSoundIndex;	

// throw animation 이 play 된 후 몇초후에 발사할 것인가?
var()			array<float>	FireTime;

// true 이면 던진후 Weapon 을 바꾼다.
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

// 수류탄류의 경우, 미묘한 Timing 의 문제가 심각한 문제를 발생할 수 있으므로 한번 더 Check 하도록 한다....
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

// 준비동작... 수류탄의 핀을 뽑는다던가... C4의 버튼을 누른다던가...
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

		// PullPin State 로 바꿔준다.
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

	// Host 와의 Sync 문제가 발생한다....
	// Client 에서 단독적으로 Active 상태로 가서는 안된다...
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
			// Stop Fire 를 호출했을때 서버와의 time 차에 의해서 
			// 서버가 WeaponPreparing 상태이면 대략 난감한 상황이 발생한다.
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
		//PutDownWeapon();			// 'WeaponPuttingDown' 으로 전이함 \.
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
		// 3인칭에서 던지는 Animation 을 Play 해준다.
		// 원래는 ProjectileFire 에 있었지만 그럼 Timing 이 맞지 않는다.
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

			// 총알 발사시간은 Throw Time 보다는 잛아야 한다.
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

		// 서버는 "Fire In the Hole"과 같은 라디오 메시지를 날려준다
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

	// WeaponEmpty 일 경우에 Active 로 보내기 때문에 override 했음
	simulated function WeaponEmpty()
	{
	}

	simulated function ThrowEndDone()
	{
		// 투척무기는 던진 후 무기가 바뀐다.
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

		// Normal State 로 바꿔준다.
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

// IncrementFlashCount() 를 여기서 하지 않기 위해서 Override 했음.
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
	
	// 수류탄의 기본값들임
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
	FireTime(0)			=	0.0733		//  던지는 Animation 으로 전환후 몇초 있다 Projectile 을 생성할 것인가?

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

	// Weapon 에 의한 Pawn 의 속도 제한
	BaseSpeed			= 285	// 기본속도
	AimSpeedPct			= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.4	// 걷기시 보정치
	CrouchSpeedPct		= 0.25	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.6	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1.1	// 앉아서 스프린트시 보정치

	bCanThrow			=	false
	PickUpClass			=	class'avaPickUp'
	PickUpAddAmmo		=	false
	bDropWhenDead		=	true
	bDropIfHasAmmo		=	true

	CrossHairMtrl		=	Texture2D'avaUI.HUD.CrossImage'	
}
