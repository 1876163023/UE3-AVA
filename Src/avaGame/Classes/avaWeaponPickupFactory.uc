/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaWeaponPickupFactory extends avaPickupFactory
	native
	nativereplication;

var() RepNotify	class<avaWeapon>			WeaponPickupClass;
var   			bool					bWeaponStay;

replication
{
	if (ROLE==ROLE_Authority && bNetInitial)
		WeaponPickupClass;
}

cpptext
{
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

simulated function PreBeginPlay()
{
	Super.PreBeginPlay();

	PickupClassChanged();
}

simulated function PickupClassChanged()
{
	InventoryType = WeaponPickupClass;

	//FIXME: Use a different mesh instead of just scale

	if (WeaponPickupClass.Default.bSuperWeapon)
	{
		if ( BaseMesh != None )
			BaseMesh.SetScale(2.0);
	}

	SetWeaponStay();
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'WeaponPickupClass' )
	{
		PickupClassChanged();
		SetPickupMesh();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

function bool CheckForErrors()
{
	if ( Super.CheckForErrors() )
		return true;

	if ( WeaponPickupClass == None )
	{
		`log(self$" no weapon pickup class");
		return true;
	}	

	return false;
}

/**
 * If our charge is not a super weapon and weaponstay is on, set weapon stay
 */

function SetWeaponStay()
{
	//bWeaponStay = ( !WeaponPickupClass.Default.bSuperWeapon && avaGame(WorldInfo.Game).bWeaponStay );
	bWeaponStay = !WeaponPickupClass.Default.bSuperWeapon;
}

function StartSleeping()
{
	if (!bWeaponStay)
	    GotoState('Sleeping');
}

function bool AllowRepeatPickup()
{
    return !bWeaponStay;
}

function SpawnCopyFor( Pawn Recipient )
{
	local Inventory Inv;

	if ( avaInventoryManager(Recipient.InvManager)!=None )
	{
		Inv = avaInventoryManager(Recipient.InvManager).HasInventoryOfClass(WeaponPickupClass);
		if ( avaWeapon(Inv)!=none )
		{
			avaWeapon(Inv).AddAmmo(WeaponPickupClass.Default.AmmoCount);
			avaWeapon(Inv).AnnouncePickup(Recipient);
			return;
		}
	}
	super.SpawnCopyFor(Recipient);

}

defaultproperties
{
	bWeaponStay=true
	bStatic=false
	bRotatingPickup=true
	bCollideActors=true
	bBlockActors=true

	Begin Object NAME=CollisionCylinder
		BlockZeroExtent=false
	End Object

	/*Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		StaticMesh=StaticMesh'S_Pickups.Bases.S_Pickups_Bases_TempSpawner'
		CollideActors=true
		BlockNonZeroExtent=false
		BlockZeroExtent=true
		BlockActors=true
		BlockRigidBody=true
		CastShadow=false
		bForceDirectLightMap=true
		bCastDynamicShadow=false
		Translation=(X=0.0,Y=0.0,Z=-44.0)
		Scale3D=(X=1.0,Y=1.0,Z=1.0)
		CullDistance=7000
	End Object
	BaseMesh=StaticMeshComponent0
 	Components.Add(StaticMeshComponent0)*/
}

