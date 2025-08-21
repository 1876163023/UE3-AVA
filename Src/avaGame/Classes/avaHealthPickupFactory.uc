/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaHealthPickupFactory extends avaItemPickupFactory
	abstract;

var		int					HealingAmount;
var		bool				bSuperHeal;

simulated static function UpdateHUD(avaHUD H)
{
	Super.UpdateHUD(H);
	H.LastHealthPickupTime = H.LastPickupTime;
}

function SpawnCopyFor( Pawn Recipient )
{
	// Give health to recipient
	Recipient.Health += HealAmount(Recipient);

	super.SpawnCopyFor(Recipient);

}

function int HealAmount(Pawn Recipient)
{
	if ( bSuperHeal )
	{
		return FClamp(2*Recipient.Default.Health - Recipient.Health - 1, 0, HealingAmount);
	}
	return FClamp(Recipient.Default.Health - Recipient.Health, 0, HealingAmount);
}

//=============================================================================
// Pickup state: this inventory item is sitting on the ground.

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

		// superhealth can always be picked up in DM (to deny it to other players)
		if ( bSuperHeal && !WorldInfo.Game.bTeamGame )
		{
			return true;
		}

		// does Other need health?
		return ( HealAmount(Other) > 0 );
	}
}

defaultproperties
{
     HealingAmount=20
}
