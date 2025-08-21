class avaPickupProvider extends TriggerVolume
	native; // [+] 20070503 dEAthcURe|HM

var()						bool				bActive;			// true 인 경우에만 공급해준다.
var()						Actor				ProviderActor;		//	
var()						array<Actor>		PickUpActor;		//	
var()						class<avaWeapon>	InventoryClass;		// 
var()						BYTE				TeamIdx;			// 0이나 1로 Setting이 되어 있는 경우 같은 팀만이 가질 수 있다.
var()						int					MaxPickUpCnt;		// 최대 PickUp Count
var()						int					RespawnTime;		// 0 보다 큰 경우 RespawnTime 마다 다시 공급해 준다.
var()						BOOL				bJustAddAmmo;		// true이면 Ammo 만 공급해 준다...
var()						BOOL				bJustAddHealth;		// true이면 Health 만 공급해 준다..
var()						int					AddHealth;			// bJustAddHealth 가 true 인 경우 공급할 Health 의 양
var()						int					AddAmmoInventoryGroup;
var()						int					AmmoCount;			// Ammo 만을 공급할 때 공급할 Ammo 의 수
var()						BOOL				bSwap;				// true이면 G키를 눌러야 가질 수 있다...
var()						BOOL				bDoNotSwitch;		// PickUp 을 주었을때 주운 Weapon 으로 switch 할지에 대한 Flag
var()						BOOL				bDrawInRadar;		//
var()						BOOL				bDrawInRadarOnlyThis;
var()						int					IconCode;			//
var()						float				PickUpLifeTime;
var()						int					MaxAmmo;			// 
var()						SoundCue			PickUpSound;

// FreezeTime 기능은 아직 구현되지 않았음....
var()						float				CoolTime;			// 시간이 지정되어 있다면 Freeze 동안은 받아 갈 수 없음.
var							bool				bCoolTime;

var	hmserialize	repnotify	BYTE				CurPickUpCnt;
var	repnotify				BYTE				HMRestorCnt;


var				avaPickUp_Indicator				Indicator;
var()						vector				IndicatorOffset;
var()						bool				bUseIndicator;
var()						int					IndicatorType;

replication
{
	if ( Role==ROLE_Authority )
		CurPickUpCnt, HMRestorCnt, ProviderActor;
}

function Reset()
{
	Super.Reset();
	ForceReset();
}

function ForceReset()
{
	CurPickUpCnt = MaxPickUpCnt;
	UpdatePickUpActor();
}


simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'CurPickUpCnt' )
		UpdatePickUpActor();
	else if ( VarName == 'HMRestorCnt' )
		UpdatePickUpActor();
	else Super.ReplicatedEvent( VarName );
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	if ( Role == ROLE_Authority )
	{
		CurPickUpCnt = MaxPickUpCnt;
		SetTimer( 0.5, true, 'Recheck' );
	}
	UpdatePickUpActor();
}

function bool CanUse(Pawn User)
{
	local vector HitLocation, HitNormal;
	local vector TraceEnd, TraceStart;
	local Actor	 HitActor;
	local int	 i;

	if ( bActive == false )																		return false;
	if ( bCoolTime == true )																	return false;
	if ( CurPickUpCnt <= 0 )																	return false;
	if ( bJustAddHealth == true && avaCharacter(User).Health >= avaCharacter(User).HealthMax )	return false;
	if ( ( TeamIdx == 0 || TeamIdx == 1 ) && TeamIdx != User.GetTeamNum() )						return false;
	if ( User == None || !User.bCanPickupInventory || (User.Controller == None))				return false;
	if ( !WorldInfo.Game.PickupQuery(User, InventoryClass, self) )								return false;
	if ( ProviderActor != None )		// Trace 를 해서 ProviderActor 나 PickUp Actors 가 보이는지 Check 한다...
	{
		TraceStart	=	User.GetPawnViewLocation();
		TraceEnd	=	TraceStart + vector( User.GetViewRotation() ) * 1000;
		HitActor	=	User.Trace( HitLocation, HitNormal, TraceEnd, TraceStart, true, vect(0,0,0) );
		if ( HitActor == ProviderActor )	return true;
		for ( i = 0 ; i < PickUpActor.length ; ++ i )
		{
			if ( HitActor == PickUpActor[i] )
				return true;
		}
		return false;
	}
	return true;
}

function GiveTo( Pawn P )
{
	local avaInventoryManager	InvManager;
	local Inventory				Inv;
	local bool					bAddAmmo;
	local avaWeapon				InvWeapon;

	if ( bJustAddHealth == true )
	{
		avaCharacter(P).AddHealth( AddHealth );
	}
	else if ( bJustAddAmmo == true )
	{
		InvManager = avaInventoryManager(P.InvManager);
		if ( InventoryClass != None )
		{
			if ( InvManager.HasInventoryOfClass(InventoryClass) == None )	return;
			if ( !InvManager.NeedsAmmo(InventoryClass) )					return;
			// ToDo. 특정 Inventory Class 의 Ammo 를 공급해준다.
			InvManager.AddAmmoToWeapon( AmmoCount ,InventoryClass);
		}
		else
		{
			bAddAmmo = false;
			foreach InvManager.InventoryActors( class'avaWeapon', InvWeapon )
			{
				if ( ( AddAmmoInventoryGroup == -1 || InvWeapon.InventoryGroup == AddAmmoInventoryGroup ) && InvWeapon.AmmoCount < InvWeapon.MaxAmmoCount)
				{
					InvWeapon.AddAmmo( AmmoCount );
					bAddAmmo = true;
				}
			}

			if ( !bAddAmmo == false )
				return;
		}
	}
	else 
	{
		if ( InventoryClass == None )	return;
		Inv	= Spawn( InventoryClass );
		Inv.AnnouncePickup( p );
		avaWeapon(Inv).GiveToEx( P, bDoNotSwitch );
		if ( Weapon( Inv ) != None && MaxAmmo > 0 )
			Weapon( Inv ).AddAmmo( MaxAmmo );
		avaWeapon(Inv).bDrawInRadar		= bDrawInRadar;
		avaWeapon(Inv).PickUpTeamNum	= TeamIdx;
		avaWeapon(Inv).PickUpLifeTime	= PickUpLifeTime;
		avaWeapon(Inv).RadarIconCode	= IconCode;
		avaWeapon(Inv).PickUpProvider	= self;
	}

	if ( PickUpSound != None )
	{
		P.PlaySound( PickUpSound );
	}

	--CurPickUpCnt;

	if ( RespawnTime > 0 )
	{
		ClearTimer( 'Supply' );
		SetTimer( RespawnTime, false, 'Supply' );
	}
	UpdatePickUpActor();

	if ( CoolTime > 0 )
	{
		bCoolTime = true;
		SetTimer( CoolTime, false, 'ClearCoolTime' );
	}
}

function ClearCoolTime()
{
	bCoolTime = false;
}

simulated function OnToggle(SeqAct_Toggle action)
{
	if (action.InputLinks[0].bHasImpulse)		Activate( true );		// turn on
	else if (action.InputLinks[1].bHasImpulse)	Activate( false );		// turn off
	else if (action.InputLinks[2].bHasImpulse)	Activate( !bActive );	// toggle
}	

function Activate( bool bFlag )
{	
	local Pawn P;
	bActive = bFlag;
	if ( bActive == true )	CheckTouch();
	else
	{
		foreach TouchingActors(class'Pawn', P)
		{
			avaPawn(P).RemovePickUpProvider( self );
		}
	}
}

function CheckTouch()
{
	local Pawn P;
	if ( bActive == true )
	{
		foreach TouchingActors(class'Pawn', P)
		{
			Touch( P, None, Location, vect(0,0,1) );
		}
	}
}

function Recheck()
{
	CheckTouch();
}


event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	local Pawn P;
	P = Pawn(Other);
	if ( bSwap == true )
	{
		if ( P!= None )
		{
			if ( CanUse(P) )
			{
				avaPawn(P).AddPickUpProvider( self );
			}
			else
			{
				avaPawn(P).RemovePickUpProvider( self );
			}
		}
		return;		// touch 만으로는 가질 수 없음...
	}
	if( P != None && CanUse(P) )
	{
		GiveTo(P);
	}
}

event UnTouch( Actor Other )
{
	local avaPawn P;
	P = avaPawn(Other);
	if ( P != None )
		P.RemovePickUpProvider( self );
}

function Supply()
{
	++ CurPickUpCnt;
	if ( CurPickUpCnt >= MaxPickUpCnt )	CurPickUpCnt = MaxPickUpCnt;
	else								SetTimer( RespawnTime, false, 'Supply' );
	UpdatePickUpActor();
}

simulated function UpdatePickUpActor()
{
	local int	i;
	for ( i = 0 ; i < PickUpActor.length ; ++ i )
	{
		if ( i < CurPickUpCnt )
		{
			PickUpActor[i].SetHidden( false );
		}
		else
		{
			PickUpActor[i].SetHidden( true );
		}
	}

	if ( bDrawInRadar == false && bDrawInRadarOnlyThis == false )	return;

	if ( CurPickUpCnt > 0 )	AddToHUD();
	else					RemoveFromHUD();
	CheckIndicatorTeam();
}

simulated function RemoveFromHUD()
{
	local PlayerController PC;
	ForEach LocalPlayerControllers(PC)
	{
		if ( avaHUD(PC.MyHUD) != None )
		{
			avaHUD(PC.MyHUD).RemoveActorFromRadar( self );
		}
	}
	ClearTimer( 'Timer_TryAddToHUD' );
}

simulated function AddToHUD()
{
	if ( TryAddToHUD() == false )
		SetTimer( 1.0, true, 'Timer_TryAddToHUD' );
}

simulated function Timer_TryAddToHUD()
{
	if ( TryAddToHUD() == true )
		ClearTimer( 'Timer_TryAddToHUD' );
}

simulated function bool TryAddToHUD()
{
	local PlayerController PC;
	ForEach LocalPlayerControllers(PC)
	{
		if ( avaHUD(PC.MyHUD) != None && PC.PlayerReplicationInfo != None )
		{
			CheckIndicatorTeam( PC );
			avaHUD(PC.MyHUD).AddActorToRadar( self, TeamIdx, IconCode );
			return true;
		}
	}
	return false;
}

event function onHmRestore()
{
	++HMRestorCnt;
	// {{ 20070907 dEAthcURe|HM pickup을 집은 상태에서 HM되면 respawn timer를 돌리도록	
	if ( CurPickUpCnt == 0 && RespawnTime > 0 )
	{
		ClearTimer( 'Supply' );
		SetTimer( RespawnTime, false, 'Supply' );
	}	
	// }} 20070907 dEAthcURe|HM pickup을 집은 상태에서 HM되면 respawn timer를 돌리도록
	UpdatePickUpActor();
}

simulated function CreateIndicator()
{
	local Rotator			R;

	if ( bUseIndicator == false )	return;
	if ( Indicator != None )		return;

	Indicator	= Spawn( class'avaPickUp_Indicator', self);
	Indicator.IndicatorType = IndicatorType;
	Indicator.SetBase( self );
	Indicator.bIgnoreBaseRotation = true;
	R.Yaw	= 0;
	R.Pitch = 0;
	R.Roll	= 0;
	Indicator.SetRotation( R );
	Indicator.Mesh.SetTranslation( IndicatorOffset );
	Indicator.Init();

	// 같은 Team 만 보이도록 한다....
	//CheckIndicatorTeam();
}

simulated function CheckIndicatorTeam( optional PlayerController LocalPC = None )
{
	local PlayerController PC;

	PC = LocalPC;
	if ( PC == None )
	{
		ForEach LocalPlayerControllers( PC )
		{
			break;
		}
	}
	if ( PC == None )	return;

	if ( Indicator == None )	CreateIndicator();

	if ( Indicator != None )
	{
		if ( ( ( TeamIdx == 0 || TeamIdx == 1 ) && !avaPlayerController( PC ).IsSameTeamByIndex( TeamIdx ) ) || CurPickUpCnt <= 0 )	Indicator.Mesh.SetHidden( true );
		else																										Indicator.Mesh.SetHidden( false );
	}
}

simulated function DestroyIndicator()
{
	if ( Indicator != None )
	{
		Indicator.Destroy();
		Indicator = None;
	}
}

defaultproperties
{
	bStatic=false
	bAlwaysRelevant=true
	bSkipActorPropertyReplication=false
	bUpdateSimulatedPosition=false
	bReplicateMovement=false
	RemoteRole=ROLE_SimulatedProxy

	Begin Object Name=BrushComponent0
		HiddenGame=true
	End Object

	bCollideActors		=	TRUE
	bProjTarget			=	FALSE
	bDoNotSwitch		=	TRUE
	AddAmmoInventoryGroup	=	-1
	PickUpLifeTime		=	16.0
}

