/*=============================================================================
  avaSwappedPickUp
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
  2006/03/21 by OZ

	G 키를 눌렀을 경우에만 Pawn 이 가지고 있는 Weapon 과 Swap 한다.
	
	ToDo
	
		1. HostMigration 이 된다면 TouchedPawn 은 replication 되어야 한다.
			
============================================================================*/
class avaSwappedPickUp extends avaPickUp;

var	array<avaPawn>	TouchedPawn;

function RemoveFromLevel()
{
	RemoveAll();
	Super.RemoveFromLevel();
}

// TouchedPawn 에 Insert 하고 Pawn 에 Notify 한다.
function AddToPawn( avaPawn P )
{
	local int i;
	// TouchedPawn 이 중복되는지 Check 한다.
	for ( i = 0 ; i < TouchedPawn.length ; ++ i )
		if ( P == TouchedPawn[i] )
		{
			`warn( "avaSwappedPickUp.AddToPawn already add" @P );
			return;
		}

	TouchedPawn[TouchedPawn.length] = P;
	P.AddPickUp( self );
}

// TouchedPawn 에서 Remove 하고 Pawn 에 Notify 한다.
function RemoveFromPawn( avaPawn P )
{
	local int idx;
	idx = TouchedPawn.Find(P);
	if ( idx < 0 )	
	{
		return;
	}

	TouchedPawn.Remove( Idx, 1 );
	P.RemovePickUp( self );
}

// TouchedPawn 을 Clear 하고 Pawn 에 Notify 한다.
function RemoveAll()
{
	local int i;
	for ( i = 0 ; i < TouchedPawn.length ; ++ i )
	{
		TouchedPawn[i].RemovePickUp( self );
	}
	// 이렇게 하면 array 가 clear 되나?
	TouchedPawn.length = 0;
}

state Pickup
{
	event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
	{
		local avaPawn	P;
		local avaWeapon InvWeapon;
		P = avaPawn( Other );
		if ( P != None && ValidTouch(P) )
		{

			// Swapped PickUp 이지만 InventoryGroup 이 같은 Weapon 을 하나도 가지고 있지 못한 경우에는 그냥 주도록 하자...
			ForEach P.InvManager.InventoryActors( class'avaWeapon', InvWeapon )
			{
				if ( InventoryClass.default.InventoryGroup == InvWeapon.InventoryGroup )
				{
					AddToPawn( P );
					return;
				}
			}
			GiveTo( P );
			//AddToPawn( P );
		}
	}

	event UnTouch( Actor Other )
	{
		local avaPawn P;
		P = avaPawn(Other);
		if ( P != None )
		{
			RemoveFromPawn( P );
		}
	}
}

defaultproperties
{
	bDoNotSwitch=false
}