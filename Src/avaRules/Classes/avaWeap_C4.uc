/*=============================================================================
  avaWeap_C4
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/03/06 by OZ
		Alt-Fire �� C4 �� ����߸��� ���� Code �߰�
=============================================================================*/

class avaWeap_C4 extends avaThrowableWeapon;

var float		ExplodeTime;
var float		DefuseTime;

var MaterialInstanceConstant LCDMaterialInstanceConstant;
var MaterialInstanceConstant LampMIC;

var LinearColor					Param1, Param2;
var SoundCue					PowerOnSC, ClickSC;
var TriggerVolume				BombVolume;

simulated function AttachItems()
{
	Super.AttachItems();
	LampMIC						= Mesh.CreateMaterialInstance( 1 );
	LCDMaterialInstanceConstant = Mesh.CreateMaterialInstance( 2 );
	LampMIC.SetScalarParameterValue( 'Lamp', 0.0 );
	LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV1', MakeLinearColor( 0, 0.0, 0, 0 ) );
	LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV2', MakeLinearColor( 0, 0.0, 0, 0 ) );
}

function bool DenyPickupQuery(class<Inventory> Inv, Actor Pickup)
{
	// Light Stick �� �ϳ��� ������ �Ѵ�....
	if ( class<avaWeap_C4>(Inv) != None )
		return true;
	return false;
}

simulated function float GetPrepareTime()
{
	return PrepareTime[CurrentFireMode];
}

simulated event Notify_SetTimer()
{
	Param1 = MakeLinearColor( 0, 0.0, 0, 1 );
	Param2 = MakeLinearColor( 0, 0.0, 0, 1 );
	LampMIC.SetScalarParameterValue( 'Lamp', 1.0 );
	
	WeaponPlaySound( PowerOnSC );
}

simulated event Notify_PushButton01()
{
	Param2 = Param1;
	Param1 = MakeLinearColor( 0.5, 0.0, 0, 1 );	
	WeaponPlaySound( ClickSC );
}

simulated event Notify_PushButton02()
{
	Param2 = Param1;
	Param1 = MakeLinearColor( 0.0, 0.125, 0, 1 );	
	WeaponPlaySound( ClickSC );
}

simulated event Notify_PushButton03()
{
	Param2 = Param1;
	Param1 = MakeLinearColor( 0.5, 0.125, 0, 1 );	
	WeaponPlaySound( ClickSC );
}

simulated event Notify_PushButton04()
{
	Param2 = Param1;
	Param1 = MakeLinearColor( 0.0, 0.25, 0, 1 );
	WeaponPlaySound( ClickSC );
}

simulated event Notify_PushButton05()
{
	Param2 = Param1;
	Param1 = MakeLinearColor( 0.5, 0.25, 0, 1 );
	WeaponPlaySound( ClickSC );
}


//	��ź ��ġ�� ī��ó�� ��ġ���� �������� ���� ���� Code 2006/02/09 by OZ
simulated state WeaponPreparing
{
	simulated function BeginState( Name PreviousStateName )
	{
		IncrementFlashCount();
		if ( Role == Role_Authority )
		{
			avaPlayerController( Instigator.Controller ).Server_IgnoreMoveInput( true );
			//avaPlayerController( Instigator.Controller ).IgnoreMoveInputEx( true, true );
		}

		// initialize
		Param1 = MakeLinearColor( 0, 0, 0, 0 );
		Param2 = MakeLinearColor( 0, 0, 0, 0 );

		Super.BeginState( PreviousStateName );
	}

	simulated function EndState(Name NextStateName)
	{
		ClearFlashCount();
		if ( Role == Role_Authority )
		{
			avaPlayerController( Instigator.Controller ).Server_IgnoreMoveInput( false );
		}
		if ( NextStateName == 'Active' )
		{
			LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV1', MakeLinearColor( 0.0, 0.0, 0, 0 ) );
			LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV2', MakeLinearColor( 0.0, 0.0, 0, 0 ) );
			LampMIC.SetScalarParameterValue( 'Lamp', 0.0 );
		}
		avaPlayerController( Instigator.Controller ).IgnoreMoveInputEx( false );
		Super.EndState( NextStateName );
	}

	simulated function Tick( float DeltaTime )
	{
		super.Tick( DeltaTime );

		Param2.A = FMax( Param2.A - DeltaTime / 0.25, 0 );

		LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV1', Param1 );
		LCDMaterialInstanceConstant.SetVectorParameterValue( 'UV2', Param2 );
	}

}

simulated function FireAmmunition()
{
	ConsumeAmmo( CurrentFireMode );

	PlayFiringSound();

	CustomFire();
}

simulated function CustomFire()
{
	// �Ѿ� ��� ���� ���� Ȯ��.
	if ( CurrentFireMode == 0 )
	{
		// tell remote clients that we fired, to trigger effects
		//IncrementFlashCount();
		if ( Role == ROLE_Authority )
		{
			SpawnBomb();
		}

		Super.CustomFire();
	}
}

function vector GetThrowVel()
{
	local vector X, Y, Z;
	GetAxes( Instigator.GetViewRotation(), X, Y, Z );
	return DropVelocity * X;
}

function SpawnBomb()
{
	local avaProj_C4	c4;
	c4 = Spawn( class'avaProj_C4', self, , Instigator.GetPawnViewLocation() + (ThrowOffset>>Instigator.GetViewRotation()), Instigator.Rotation );
	c4.SetBombVolume( BombVolume );
}

simulated function bool CanBombHere()
{
	if ( avaPawn(Instigator) == None )
		return false;
	BombVolume = avaPawn( Instigator ).GetBombVolume();
	if ( BombVolume == None )	return false;
	if ( ( avaBombVolume( BombVolume ).TeamIdx == 0 || avaBombVolume( BombVolume ).TeamIdx == 1 ) && avaBombVolume( BombVolume ).TeamIdx != Instigator.GetTeamNum() )
		return false;
	return true;
}

simulated function StartFire(byte FireModeNum)
{
	if ( FireModeNum == 0 && Instigator.Physics == PHYS_Walking )
	{
		Super.StartFire( FireModeNum );
	}
}

simulated state Active
{
	simulated function BeginState( Name PreviousStateName )
	{
		avaPlayerController( Instigator.Controller ).IgnoreMoveInputEx( false );
		Super.BeginState( PreviousStateName );
	}

	/// ��ź ��ó�� �ƴ϶�� �� �� ����.
	simulated function BeginFire( byte FireModeNum )
	{
		local avaPlayerController	C;
		local avaHUD				H;
		if ( FireModeNum == 0 )
		{
			if ( CanBombHere() )
			{
				if ( Instigator.Physics != PHYS_Walking )
				{
					ClearPendingFire( FireModeNum );
				}
				else
				{
					avaPlayerController( Instigator.Controller ).IgnoreMoveInputEx( true, true );
					Super.BeginFire( FireModeNum );
				}
			}
			else
			{
				if ( Instigator.IsLocallyControlled() )
				{
					C = avaPlayerController( Instigator.Controller );
					H = avaHUD( C.myHUD );
					H.GameInfoMessage( class'avaLocalizedMessage'.Default.Not_In_Bomb_Area );
				}
				ClearPendingFire( FireModeNum );
			}
		}
	}
}

simulated function EndFire(byte FireModeNum)
{
	Super.EndFire( FireModeNum );
	avaPlayerController( Instigator.Controller ).IgnoreMoveInputEx( false );
}

// C4 �� Alt-Fire �� ������ ���̴�.
function ConsumeAmmo( byte FireModeNum )
{
	if ( FireModeNum == 0 )
	{
		Super.ConsumeAmmo( FireModeNum );
	}
}

// C4 �� Ammo �� ������� ���� �� �ִ�.
simulated function bool CanThrow()
{
	return true;
}

simulated function bool IsProjectileSpin()
{
	return false;
}

// C4 ����Ǵ°��� ���� �� �ִ�....
simulated function GotoWeaponReady()
{
	if ( Role == ROLE_Authority )
	{
		GotoState( 'WeaponReady' );
		ForceGotoWeaponReady();
	}
}

simulated function GotoActive()
{
	if ( Role == ROLE_Authority )
	{
		GotoState( 'Active' );
		ForceGotoActive();
	}
}

function bool ConsumeAmmoWhenPracticeMode()
{
	return true;
}

defaultproperties
{
	BaseSpeed		= 230	// �⺻�ӵ�
	AimSpeedPct		= 0.8	// ���ؽ� ����ġ
	WalkSpeedPct		= 0.42	// �ȱ�� ����ġ
	CrouchSpeedPct		= 0.3	// �ɾ��̵��� ����ġ
	CrouchAimSpeedPct	= 0.2	// �ɾƼ� ���� �̵��� ����ġ
	SwimSpeedPct		= 0.7	// ������ ����ġ
	SprintSpeedPct		= 1.3	// ������Ʈ�� ����ġ
	CrouchSprintSpeedPct	= 1	// �ɾƼ� ������Ʈ�� ����ġ

	BaseSkelMeshName	=	"Wp_New_C4.MS_C4"
	BaseAnimSetName		=	"Wp_New_C4.Ani_C4"

	AmmoCount=1
	MaxAmmoCount=1

	WeaponRange=110
	
	WeaponFireTypes(0)=EWFT_Custom
	WeaponFireTypes(1)=EWFT_Custom

	FiringStatesArray(1)=WeaponFiring

	InventoryGroup=5
	GroupWeight=0.5

	WeaponColor=(R=255,G=0,B=0,A=255)

	AttachmentClass=class'avaAttachment_C4'

/*	WeaponLoadSnd=SoundCue'A_Weapon.RocketLauncher.Cue.A_Weapon_RL_Load_Cue'
	WeaponLoadedSnd=SoundCue'A_Pickups.Ammo.Cue.A_Pickup_Ammo_Sniper_Cue'*/
	//WeaponFireSnd=SoundCue'A_Weapon.RocketLauncher.Cue.A_Weapon_RL_Fire_Cue'	

	WeaponProjectiles(0)=class'avaProj_C4'

	FireOffset=(X=20,Y=12,Z=-5)
	
	ShouldFireOnRelease(0)=1

	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	//FireShake=(OffsetMag=(X=-5.0,Y=-5.00,Z=-5.00),OffsetRate=(X=-1000.0,Y=-1000.0,Z=-1000.0),OffsetTime=2,RotMag=(X=0.0,Y=0.0,Z=0.0),RotRate=(X=0.0,Y=0.0,Z=0.0),RotTime=2)	

	FireInterval(0)=1.0

	WeaponEquipAnim		=	BringUp
	EquipTime			=	0.8333
	PutDownTime			=	0.0
	WeaponIdleAnims(0)	=	Idle

	PrepareAnim(0)	=	Fire
	PrepareTime(0)	=	3.5333
	//PrepareSnd(0)	=
	CancelAnim(0)	=	Idle
	CancelTime(0)	=	0.0
	//CancelSnd=
	ThrowAnim(0)	=	None
	ThrowTime(0)	=	0.0
	//ThrowSnd=

	// ThrowEndTime �� 0.0 �̸� Bug �� ����...
	ThrowEndAnim(0)	=	None
	ThrowEndTime(0)	=	0.01
	//ThrowEndSnd=
	FireTime(0)		=	0.0

	/// cass c4�� ������ ���߿� �ߴ��� �� �ִ�.
	bCancelPrepare(0)	=	true
	/// cass c4�� ���� ������ �Ϸ� �Ǹ� ��ٷ� ����߸���.
	bSkipReady(0)	=	true

	// Weapon Throw ���� default properties
	bCanThrow		=	true
	PickUpClass		=	class'avaPickUp_C4'
	ThrowOffset		=	(X=0,Y=0,Z=-12)			// X �� 0���� Ŭ ���, ���� �پ ������ �Ǹ� ���� �հ� ���� �� �ִ�.

	ManualIncrementFlashCount = true

	ExplodeTime			=	40
	DefuseTime			=	7

	bDisableChangeState	=	true
	bDropWhenDead		=	true

	ProjStartVel(0)				=	12
	ProjZAng(0)				=	0
	ProjZAng(1)				=	0

	PowerOnSC				=	SoundCue'avaWeaponSounds.Mission_C4.Mission_C4_PowerOn'
	ClickSC					=	SoundCue'avaWeaponSounds.Mission_C4.Mission_C4_click'

	RadarIconColor	=	(R=134,G=234,B=40,A=255)
	RadarIconCode	=	-1

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.7
	BobDampingInDash		=	0.45
	bSpecialInventory		=	true

	GIMIndexWhenPickUp	=	102
	GIMIndexWhenEquip	=	102
	bAvailableAbandonWeapon	=	true
}

