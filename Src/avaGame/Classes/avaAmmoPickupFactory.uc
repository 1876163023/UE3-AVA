/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// Ammo.
//=============================================================================
class avaAmmoPickupFactory extends avaItemPickupFactory
	abstract;

/** The amount of ammo to give */
var int AmmoAmount;
/** The class of the weapon this ammo is for. */
var class<avaWeapon> TargetWeapon;

function SpawnCopyFor( Pawn Recipient )
{
	if ( avaInventoryManager(Recipient.InvManager) != none )
		avaInventoryManager(Recipient.InvManager).AddAmmoToWeapon(AmmoAmount, TargetWeapon);

	super.SpawnCopyFor(Recipient);
}


simulated static function UpdateHUD(avaHUD H)
{
	local Weapon CurrentWeapon;

	Super.UpdateHUD(H);

	if ( H.PawnOwner != None )
	{
		CurrentWeapon = H.PawnOwner.Weapon;
		if ( CurrentWeapon == None )
			return;
	}

	if ( Default.TargetWeapon == CurrentWeapon.Class )
		H.LastAmmoPickupTime = H.LastPickupTime;
}

auto state Pickup
{
	/* ValidTouch()
	 Validate touch (if valid return true to let other pick me up and trigger event).
	*/
	function bool ValidTouch( Pawn Other )
	{
		if ( !Super.ValidTouch(Other) )
		{
			return false;
		}

		if ( avaInventoryManager(Other.InvManager) != none)
		  return avaInventoryManager(Other.InvManager).NeedsAmmo(TargetWeapon);

		return true;
	}	
}

defaultproperties
{
     MaxDesireability=+00000.200000

	Begin Object Name=CollisionCylinder
		CollisionRadius=20
		CollisionHeight=9.6
	End Object

	Begin Object Class=StaticMeshComponent Name=MeshComponent0
		bUseAsOccluder = FALSE
		CastShadow=false
		bForceDirectLightMap=true
		bCastDynamicShadow=false
		CollideActors=false
		Scale=1.8
	End Object
	PickupMesh=MeshComponent0
	Components.Add(MeshComponent0)

}
