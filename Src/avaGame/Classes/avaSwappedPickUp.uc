/*=============================================================================
  avaSwappedPickUp
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
  2006/03/21 by OZ

	G Ű�� ������ ��쿡�� Pawn �� ������ �ִ� Weapon �� Swap �Ѵ�.
	
	ToDo
	
		1. HostMigration �� �ȴٸ� TouchedPawn �� replication �Ǿ�� �Ѵ�.
			
============================================================================*/
class avaSwappedPickUp extends avaPickUp;

var	array<avaPawn>	TouchedPawn;

function RemoveFromLevel()
{
	RemoveAll();
	Super.RemoveFromLevel();
}

// TouchedPawn �� Insert �ϰ� Pawn �� Notify �Ѵ�.
function AddToPawn( avaPawn P )
{
	local int i;
	// TouchedPawn �� �ߺ��Ǵ��� Check �Ѵ�.
	for ( i = 0 ; i < TouchedPawn.length ; ++ i )
		if ( P == TouchedPawn[i] )
		{
			`warn( "avaSwappedPickUp.AddToPawn already add" @P );
			return;
		}

	TouchedPawn[TouchedPawn.length] = P;
	P.AddPickUp( self );
}

// TouchedPawn ���� Remove �ϰ� Pawn �� Notify �Ѵ�.
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

// TouchedPawn �� Clear �ϰ� Pawn �� Notify �Ѵ�.
function RemoveAll()
{
	local int i;
	for ( i = 0 ; i < TouchedPawn.length ; ++ i )
	{
		TouchedPawn[i].RemovePickUp( self );
	}
	// �̷��� �ϸ� array �� clear �ǳ�?
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

			// Swapped PickUp ������ InventoryGroup �� ���� Weapon �� �ϳ��� ������ ���� ���� ��쿡�� �׳� �ֵ��� ����...
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