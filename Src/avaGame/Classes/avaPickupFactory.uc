/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaPickupFactory extends PickupFactory
	abstract
	native
	nativereplication
	hidecategories(Display,Collision);

var		bool			bRotatingPickup;	// if true, the pickup mesh rotates
var		float			YawRotationRate;
var		Controller		TeamOwner[4];		// AI controller currently going after this pickup (for team coordination)

var 	PrimitiveComponent		BaseMesh;	// pickup base mesh (optional)

cpptext
{
	virtual void TickSpecial( FLOAT DeltaSeconds );
	virtual void PostEditMove(UBOOL bFinished=TRUE);
	virtual void Spawned();
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}


/* UpdateHUD()
Called for the HUD of the player that picked it up
*/
simulated static function UpdateHUD(avaHUD H)
{
	H.LastPickupTime = H.WorldInfo.TimeSeconds;
}

// FIXMESTEVE - implement respawneffect
function RespawnEffect();

/* epic ===============================================
* ::StopsProjectile()
*
* returns true if Projectiles should call ProcessTouch() when they touch this actor
*/
simulated function bool StopsProjectile(Projectile P)
{
	local Actor HitActor;
	local vector HitNormal, HitLocation;

	if ( (P.CylinderComponent.CollisionRadius > 0) || (P.CylinderComponent.CollisionHeight > 0) )
	{
		// only collide if zero extent trace would also collide
		HitActor = Trace(HitLocation, HitNormal, P.Location, P.Location - 100*Normal(P.Velocity), true, vect(0, 0, 0));
		if ( HitActor != self )
			return false;
	}

	return bProjTarget || bBlockActors;
}

defaultproperties
{
	Components.Remove(Sprite)
	Components.Remove(Arrow)

	YawRotationRate=32768

	Begin Object NAME=CollisionCylinder
		CollisionRadius=+00034.000000
		CollisionHeight=+00044.000000
		CollideActors=true
	End Object

}
