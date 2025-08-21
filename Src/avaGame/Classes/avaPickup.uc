/*=============================================================================
  avaPickUp
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/03/08 by OZ

		1. Editor �󿡼� ��ġ�� �����ϵ��� �Ѵ�.
			Inventory �� Null �̶�� Editor ���� ��ġ�� Actor �̴�.
			Editor ���� ��ġ�� ��� LifeSpan �� 0 �� ������,
			Destroy ���� �ʰ� ShutDown �� �Ǹ�, Reset �� Recover �ȴ�.

		2. TeamIdx �� ������.
			bTeamOnly �� True �� ��쿡�� ���� Team ���� PickUp �� �� �ִ�.

		3. ���� ����� �ٷ� PickUp �� �� ������ 
			1�� ���Ŀ� Pickup State �� �� �� �ֵ��� �����Ͽ���

		4. HUD �� �ٸ� Team �� �Ⱥ��̵��� �ϱ� ���ؼ� TeamIdx �� ����ȭ �ؾ� �Ѵ�.

	2006/04/18 by OZ

		1. GameReplicationInfo �� bReinforcement �� true �� ��쿡�� LifeSpan �� 16 �ʷ� �����Ѵ�.

	2006/10/31 by OZ

		1. bDrawInRadar �� true �̸� World �󿡵� Indicator �� ǥ���� �ֵ��� �Ѵ�.
			
============================================================================*/
class avaPickup extends avaKActor
	native				/** avaPawn.TouchedActor�� avaHUD���� �����ϱ� ���� native�� ��ȯ */
	placeable;

var					repnotify	Inventory						Inventory;			// the dropped inventory item which spawned this pickup
var()				repnotify	class<avaWeapon>				InventoryClass;		// Class of the inventory object to pickup // 20061130 dEAthcURe|HM repnotify �߰�
var()	hmserialize repnotify	BYTE							TeamIdx;			// [!] 20070323 dEAthcURe|HM 'hmserialize'
var()							BOOL							bDoNotSwitch;		// PickUp �� �־����� �ֿ� Weapon ���� switch ������ ���� Flag
var()							BOOL							bJustAddAmmo;		// PickUp �� �־����� ���� Weapon �� �ִٸ� Ammo �� �����ش�.(����ź�̳� ����ź��...)
var()							BOOL							bDynamicSpawned;	// ������ ����߸� PickUp �̴�.
var()							int								MaxAmmo;			// 
var()							float							RespawnTime;		// Static �� ���̶�� PickUp �� ���� �ٽ� Respawn �� �� �ִ�.
var()	hmserialize				BOOL							bDrawInRadar;		// true �̸� Radar �� PickUp �� ��ġ�� ǥ�����ش�.
var()	hmserialize				int								IconCode;			// bDrawInRadar �� true �� ��쿡 Radar �� ǥ�����ִ� Icon Code�� �ǹ��Ѵ�.
var()							float							LifeTime;
var()	hmserialize				BYTE							IndicatorType;		// Indicator type �Դϴ�.

var								BOOL							bDoNotRemove;		// �̰� ���������� � ��쿡�� LifeSpan�� ������ �ʴ´�.
var								CylinderComponent				cylinderComponent;
var								SkeletalMeshComponent			SocMesh;			// Socket �� ���� MeshComponent �̴�...
var								string							SocMeshName;		// Socket Mesh Name....
var								string							StaticMeshName;		// PickUp �� Static Mesh Name...
var								array< StaticMeshComponent >	Items;				// �ΰ������� �ٴ� Item ��...
var								avaPickUp_Indicator				Indicator;
var		hmserialize	repnotify	BYTE							nAddToLvl;			// [!] 20070323 dEAthcURe|HM 'hmserialize'
var		hmserialize	repnotify	BYTE							nRemoveFromLvl;		// [!] 20070323 dEAthcURe|HM 'hmserialize'

var		hmserialize	repnotify	avaPickupProvider				PickupProvider;


replication
{
	if (bNetDirty && Role == ROLE_Authority)
		TeamIdx, Inventory, InventoryClass ,nAddToLvl,nRemoveFromLvl, IndicatorType, IconCode, PickupProvider;
}

simulated function PostBeginPlay()
{
	// Owner �� ���� ���� Editor �󿡼� ��ġ�� ����̴�...
	if ( Owner == None )
	{
		CreateComponent();
	}
	Super.PostBeginPlay();
}

simulated event ReplicatedEvent(name VarName)
{
	// if ( VarName == 'Inventory' )			CreateComponent(); // [-] dEAthcURe|HM
	// {{ [+] dEAthcURe|HM
	if ( VarName == 'Inventory' || VarName == 'InventoryClass')	{		
		CreateComponent(); // 20061130 dEAthcURe|HM InventoryClass�߰�
	}
	// }} [+] dEAthcURe|HM
	else if ( VarName == 'nAddToLvl' )		AddToHUD();
	else if ( VarName == 'nRemoveFromLvl' )	RemoveFromHUD();
	else if ( VarName == 'TeamIdx' )		ChangedTeam();
	Super.ReplicatedEvent(VarName);
}

function Throw(vector ThrowVel)
{
	local vector X, Y, Z;

	if ( Instigator != None )
	{
		CollisionComponent.AddImpulse( ThrowVel,,,true );
		GetAxes( Instigator.GetViewRotation(), X, Y, Z );
		X.X = 600;
		X.Y = (FRand() - 0.5) * 2 * 1200;
		X.Z = 0;
		CollisionComponent.SetRBAngularVelocity( X );
	}
}

simulated function Destroyed()
{
	RemoveFromHUD();

	RemoveFromLevel();

	if (Inventory != None )
		Inventory.Destroy();

	DestroyIndicator();
}

simulated function Reset()
{
	// recover from shut down
	if ( bDynamicSpawned == false )
	{
		RecoverShutDown();
	}
	else
	{
		PickedUpBy(None);
	}
}

simulated function ShutDown()
{
	DestroyIndicator();
	Super.ShutDown();
}

// {{ 20061128 dEAthcURe|HM
event HmShutdown()
{
	ShutDown();	
}

event avaPickup HmCreateDynamicPickup(vector Loc, rotator Rot)
{
	local avaPickUp		P;	
	P = Spawn(class,,,Loc, Rot);
	
	P.InventoryClass	= InventoryClass; //!!
	P.Instigator		= None;
	P.bDynamicSpawned	= true;
	P.bJustAddAmmo		= bJustAddAmmo;
	P.bDrawInRadar		= bDrawInRadar;	
	P.TeamIdx			= TeamIdx;
	P.NetUpdateTime		= WorldInfo.TimeSeconds - 1;	
	P.IndicatorType		= IndicatorType;
	P.IconCode			= IconCode;
	
	P.Inventory = None; //Inventory;
	P.CreateComponent();
	
	return P;
}

event HmSetupPickup(String invClassName)
{
	local class<avaWeapon> InvClass;
	InvClass = class<avaWeapon>( DynamicLoadObject( invClassName, class'Class' ) );	

	InventoryClass	= InvClass; //!!
	
	bDynamicSpawned	= true;
	CreateComponent();
}
// }} 20061128 dEAthcURe|HM

// {{ 20070105 dEAthcURe|HM
event HmReinit()
{
	//`log("[dEAthcURe] avaPickup::HmReinit");
	super.HmReinit();
	DestroyIndicator();
}
// }} 20070105 dEAthcURe|HM

function RecoverShutDown()
{
	Super.RecoverShutDown();
	AddToLevel();
	CreateIndicator();
}

event EncroachedBy(Actor Other)
{
	Destroy();
}

auto state PrePickup
{
	function BeginState(Name PreviousStateName)
	{
		SetTimer(1.0,false);
	}

	function Timer()
	{
		GotoState('Pickup');
	}
}

function bool ValidTouch(Pawn Other);

state Pickup
{
	function bool ValidTouch(Pawn Other)
	{
		// ���� Class �� ���ؼ� FastTrace �κ��� �����ߴ�.
		// TemaIdx �� Setting �� �Ǿ��ִٸ� ���� ���� ���� �� �ִ�.
		if ( TeamIdx == 0 || TeamIdx == 1 )
		{
			if ( TeamIdx != Other.GetTeamNum() )
				return false;
		}

		if (Other == None || !Other.bCanPickupInventory || (Other.Controller == None))
		{
			return false;
		}

		// make sure game will let player pick me up
		if (!WorldInfo.Game.PickupQuery(Other, InventoryClass, self))
		{
			return false;
		}
		return true;
	}

	// When touched by an actor.
	event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
	{
		local Pawn P;

		// If touched by a player pawn, let him pick this up.
		P = Pawn(Other);
		if( P != None && ValidTouch(P) )
		{
			GiveTo(P);
		}
	}

	function Timer()
	{
		PickedUpBy(None);
	}

	function CheckTouching()
	{
		local Pawn P;

		foreach TouchingActors(class'Pawn', P)
		{
			Touch( P, None, Location, vect(0,0,1) );
		}
	}

	function BeginState(Name PreviousStateName)
	{
		if ( bDynamicSpawned && LifeSpan == 0.0 )
		{
			// Spawn �� �Ǵ� ���� Ÿ���̸� LifeSpan �� �����Ѵ�.
			if ( avaGameReplicationInfo( WorldInfo.GRI ).bReinforcement == true && !bDoNotRemove )
				LifeSpan = LifeTime;
		}

		if ( bDynamicSpawned == true && LifeSpan > 0 && !bDoNotRemove )
		{
			SetTimer( LifeSpan, false );
		}
		AddToLevel();
	}

Begin:
	CheckTouching();
}

function GiveTo( Pawn P )
{
	local Inventory Inv;
	local int		AmmoAmountToAdd;	

	// �Ⱥ��̴µ� ���� ���� ���ݾ�.
	if ( bHidden )	return;

	if ( bJustAddAmmo == true && avaInventoryManager(P.InvManager).HasInventoryOfClass(InventoryClass) != None )
	{
		// �̹� MaxAmmo�� ������ �ֱ� ������ �� �ʿ䰡 ����.
		if ( !avaInventoryManager(P.InvManager).NeedsAmmo(InventoryClass) )	return;

		if ( Inventory != None )	AmmoAmountToAdd	= avaWeapon(Inventory).GetAmmoCount();
		else						AmmoAmountToAdd	= InventoryClass.Default.AmmoCount;
		avaInventoryManager(P.InvManager).AddAmmoToWeapon( AmmoAmountToAdd ,InventoryClass);
	}
	else
	{
		if ( Inventory != None )	Inv = Inventory;
		else
		{
			Inv = spawn(InventoryClass);
			if ( Weapon( Inv ) != None && MaxAmmo > 0 )
				Weapon( Inv ).AddAmmo( MaxAmmo );
			avaWeapon( Inv ).PickUpLifeTime		= LifeTime;
			avaWeapon( Inv ).PickUpTeamNum		= TeamIdx;
			avaWeapon( Inv ).bDrawInRadar		= bDrawInRadar;
			avaWeapon( Inv ).IndicatorType		= IndicatorType;
			avaWeapon( Inv ).RadarIconCode		= IconCode;
			avaWeapon( Inv ).PickupProvider		= PickupProvider;
			
		}

		GiveToEx( P, Inv );
	}

	PickedUpBy(P);
}

function GiveToEx( Pawn P, Inventory Inv )
{
	if ( Inv == None )	return;

	// ammo �� ��������� �Ҹ��� ���ٸ� GiveTo �Լ��� ������.
	Inv.AnnouncePickup( p );

	if ( avaWeapon(Inv) != None )
	{
		avaWeapon(Inv).GiveToEx(P,bDoNotSwitch);
		Inventory = None;	// �־����ϱ� Inventory �� None ���� �����.
	}
	else
	{
		Inv.GiveTo(P);
	}
}

function PickedUpBy(Pawn P)
{
	RemoveFromLevel();
	if ( bDynamicSpawned == false )
	{
		ShutDown();

		if ( RespawnTime > 0.0 )
		{
			SetTimer( RespawnTime, false, 'RespawnDone' );
		}
	}
	else
	{
		Destroy();
	}
}

event function RespawnDone()
{
	RecoverShutDown();
}

function SetInventory( Inventory Inv )
{
	Inventory = Inv;
	CreateComponent();
}

function SetTeam( int nTeamIdx )
{
	TeamIdx = nTeamIdx;
	ChangedTeam();
	NetUpdateTime = WorldInfo.TimeSeconds - 1;
}

// Component �� ������!!!!
simulated function CreateComponent()
{
	local SkeletalMesh		tmpSkelMesh;
	local StaticMesh		tmpStaticMesh;
	local avaWeapon			w;
	local class<avaWeapon>	weaponClass;

	weaponClass = Inventory != None ? class<avaWeapon>( Inventory.class ) : InventoryClass;

	if ( Inventory != None && Role != ROLE_Authority )	Inventory.Inventory = None;

	if ( weaponClass == None ) return;

	if ( weaponClass.default.AttachmentClass.default.SocMeshName != "" )
	{
		SocMeshName		= weaponClass.default.AttachmentClass.default.SocMeshName;
		if ( StaticMeshName == "" )
			StaticMeshName	= weaponClass.default.AttachmentClass.default.BasicMeshName;	
	}
	else
	{
		if ( StaticMeshName == "" )
			StaticMeshName	= weaponClass.default.AttachmentClass.default.MeshName;
	}

	if ( SocMeshName != "" )
	{
		if ( SocMesh == None )	SocMesh = new(outer) class'SkeletalMeshComponent';
		SocMesh.bUseAsOccluder = FALSE;
		tmpSkelMesh = SkeletalMesh( DynamicLoadObject( SocMeshName, class'SkeletalMesh' ) );
		if ( tmpSkelMesh != None )	SocMesh.SetSkeletalMesh( tmpSkelMesh );
		SocMesh.bForceNotToDraw = true;
		SocMesh.SetHidden( true );
		SocMesh.SetActorCollision(false,false);
		SocMesh.SetBlockRigidBody(false);
		SocMesh.bUpdateSkelWhenNotRendered=false;
		SocMesh.bNoSkeletonUpdate=1;
		SocMesh.bCastDynamicShadow=false;
		AttachComponent( SocMesh );
	}

	if ( StaticMeshName != "" )
	{
		tmpStaticMesh = StaticMesh( DynamicLoadObject( StaticMeshName, class'StaticMesh' ) );
		if ( tmpStaticMesh != None )	StaticMeshComponent.SetStaticMesh( tmpStaticMesh );
		StaticMeshComponent.WakeRigidBody();
		StaticMeshComponent.SetLightEnvironment( LightEnvironment );
	}

	if ( SocMesh != None )
	{
		SocMesh.AttachComponentToSocket( StaticMeshComponent, 'root' );		
	}

	CreateIndicator();

	// Weapon �� �پ��ִ� Item �� ������!!!
	if ( Inventory == None )	return;
	w = avaWeapon( Inventory );
	if ( w == None )			return;
	
	AttachItems( w );
	SetSkins( w );

	if(StaticMeshComponent.bNotifyRigidBodyCollision)
	{
		SetPhysicalCollisionProperties();
	}
}

simulated function CreateIndicator()
{
	local Rotator			R;

	if ( bDrawInRadar == false )	return;
	if ( Indicator != None )		return;
	if ( bShutDown == true )		return;	// Shut Down ���̸� ����� �ȵȴ�...
	
	Indicator	= Spawn( class'avaPickUp_Indicator');
	Indicator.IndicatorType = IndicatorType;
	Indicator.SetBase( self );
	Indicator.bIgnoreBaseRotation = true;
	R.Yaw	= 0;
	R.Pitch = 0;
	R.Roll	= 0;
	Indicator.SetRotation( R );
	Indicator.Init();

	// ���� Team �� ���̵��� �Ѵ�....
	CheckIndicatorTeam();
}

simulated function DestroyIndicator()
{
	if ( Indicator != None )
	{
		Indicator.Destroy();
		Indicator = None;
	}
}

simulated function AttachItems( avaWeapon w )
{
	local int			i;
	local int			id;
	local StaticMesh	tmpStaticMesh;
	local string		n;

	if ( SocMesh == None )	return;	// Socket Mesh �� ���ٸ� ��� ���ϰ��ΰ�...??
	for ( i = 0 ; i < w.ItemParts.Length ; ++ i )
	{
		if ( w.ItemParts[i].MeshName != "" )
		{
			id				= items.length;
			Items.Length	= id + 1;
			Items[id]		= new(outer) class'StaticMeshComponent';
			Items[id].bUseAsOccluder = FALSE;
			n				= w.ItemParts[i].MeshName$"_3p";	// 3��Ī���� 1��Ī�� �ڿ� _3p �� �ٿ��� ����...
			tmpStaticMesh	= StaticMesh( DynamicLoadObject( n, class'StaticMesh' ) );
			if ( tmpStaticMesh != None )	Items[id].SetStaticMesh( tmpStaticMesh );
			else							`warn( "avaPickUp.AttachItems Cannot Load StaticMesh" @n );
			Items[id].SetActorCollision(false,false);
			Items[id].bCastDynamicShadow=false;
			SocMesh.AttachComponentToSocket( Items[id], w.ItemParts[i].SocketName );
			if ( w.ItemParts[i].MaxVisibleDistance > 0 )
				SetFadeOut( Items[id], w.ItemParts[i].MaxVisibleDistance );
			Items[id].SetLightEnvironment( LightEnvironment );
			if ( SocMesh != None )	
			{
				Items[id].SetShadowParent( SocMesh );
				Items[id].SetOcclusionGroup( SocMesh );
			}
			else					
			{
				Items[id].SetShadowParent( StaticMeshComponent );
				Items[id].SetOcclusionGroup( StaticMeshComponent );
			}
		}
	}
}

simulated function SetSkins( avaWeapon w )
{
	local int i;
	local Material		tmpMtrl;
	for ( i = 0 ; i < 3 ; ++ i )
	{
		if ( w.WeaponSkin[i] == "" )	continue;	// ������ Material �� ����
		tmpMtrl = Material( DynamicLoadObject( w.WeaponSkin[i], class'Material' ) );
		if ( tmpMtrl != None )
			StaticMeshComponent.SetMaterial( i, tmpMtrl );
		else					`warn( "avaPickUp.SetSkins Cannot Load Material" @w.WeaponSkin[i] );
	}
}

function AddToLevel()			// Level �� ��ġ�� ����̴�.
{
	if ( !bDrawInRadar )	return;
	AddToHUD();
	++nAddToLvl;
	NetUpdateTime = WorldInfo.TimeSeconds - 1;
}

function RemoveFromLevel()		// Level ���� ���ŵ� ����̴�.
{
	if ( !bDrawInRadar )	return;
	RemoveFromHUD();
	++nRemoveFromLvl;
	NetUpdateTime = WorldInfo.TimeSeconds - 1;
}

simulated function AddToHUD()
{
	// ���𼭹��� HUD �� �׷��� �ʿ䰡 ����.
	if ( WorldInfo.NetMode == NM_DedicatedServer )	return;
	if ( bShutDown == true )						return;	// Shut Down ���̸� ����� �ȵȴ�...
	bDrawInRadar = true;
	CreateIndicator();
	if ( TryAddToHUD() == false )
		SetTimer( 1.0, true, 'Timer_TryAddToHUD' );
}

event Timer_TryAddToHUD()
{
	if ( TryAddToHUD() == true )
		ClearTimer( 'Timer_TryAddToHUD' );
}

simulated function bool TryAddToHUD()
{
	local PlayerController PC;
	ForEach LocalPlayerControllers(PC)
	{
		if ( avaHUD(PC.MyHUD) != None )
		{
			avaHUD(PC.MyHUD).AddActorToRadar( self, TeamIdx, IconCode );
			return true;
		}
	}
	return false;
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

simulated function Client_ShutDown()
{
	DestroyIndicator();
	Super.Client_ShutDown();
}

simulated function ClientReset()
{
	CreateIndicator();
	Super.ClientReset();
}

simulated function ChangedTeam()
{
	CreateIndicator();
	CheckIndicatorTeam();
}

simulated function CheckIndicatorTeam( optional PlayerController LocalPC = None )
{
	local PlayerController PC;
	if ( Indicator == None )	return;

	PC = LocalPC;
	
	if ( PC == None )
	{
		ForEach LocalPlayerControllers( PC )
		{
			break;
		}
	}

	if ( !avaPlayerController( PC ).IsSameTeamByIndex( TeamIdx ) )	Indicator.Mesh.SetHidden( true );
	else															Indicator.Mesh.SetHidden( false );		
}

simulated function SetFadeOut( PrimitiveComponent Comp, optional float MaxVisibleDistance = 300.0 )
{	
	Comp.bFadingOut = true;
	Comp.FadeStart	= MaxVisibleDistance;
	Comp.FadeEnd	= Comp.FadeStart;	
}


simulated event FellOutOfWorld(class<DamageType> dmgType)
{
	if ( PickupProvider != None )
	{
		if ( PickupProvider.InventoryClass.default.bSpecialInventory == true )
			PickupProvider.ForceReset();
	}
	Super.FellOutOfWorld( dmgType );
}

cpptext
{	
	#ifdef EnableHostMigration
	virtual void hmSerialize(FArchive& Ar); // 20061128 dEAthcURe
	#endif
}

// {{ 20070523 dEAthcURe|HM
event function OnHmRestore()
{	
	CreateIndicator();	
}
// }} 20070523 dEAthcURe|HM

defaultproperties
{
	Begin Object Class=CylinderComponent NAME=CollisionCylinder
		CollisionRadius=+00020.000000
		CollisionHeight=+00010.000000
		CollideActors=true
		BlockZeroExtent=false
	End Object
	cylinderComponent=CollisionCylinder
	Components.Add(CollisionCylinder)
	
	Begin Object Name=StaticMeshComponent0		
		StaticMesh=StaticMesh'Wp_New_C4.MS_C4_3p'
		CollideActors=true
		CastShadow=true		
		CullDistanceEx=(Value[0]=7500,Value[1]=7500,Value[2]=7500,Value[3]=7500,Value[4]=7500,Value[5]=7500,Value[6]=7500,Value[7]=7500)		
		BlockActors=false
		bCastDynamicShadow=true
		bNotifyRigidBodyCollision=true
		RBChannel=RBCC_GameplayPhysics
		bAcceptsDecals=false
		RBCollideWithChannels=(Default=TRUE,GameplayPhysics=TRUE,EffectPhysics=TRUE)
	End Object	

	DrawScale		=	1
	bCollideActors	=	true
	bBlockActors	=	false
	bNoDelete		=	false
	bDoNotSwitch	=	true
	bJustAddAmmo	=	false
	LifeTime		=	16.0
	TeamIdx			=	255
	Physics			=	PHYS_RigidBody
}
