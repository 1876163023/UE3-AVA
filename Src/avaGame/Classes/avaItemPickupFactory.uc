/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaItemPickupFactory extends avaPickupFactory
	abstract;

var		SoundCue			PickupSound;
var		localized string	PickupMessage;			// Human readable description when picked up.
var		float				RespawnTime;

simulated function InitializePickup()
{
}

static function string GetLocalString(
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2
	)
{
	return Default.PickupMessage;
}

/**
 * Give the benefit of this pickup to the recipient
 */
function SpawnCopyFor( Pawn Recipient )
{
	Recipient.PlaySound( PickupSound );

	if ( PlayerController(Recipient.Controller) != None )
	{
		PlayerController(Recipient.Controller).ReceiveLocalizedMessage(MessageClass,,,,class);
	}
}

// Set up respawn waiting if desired.
//
function SetRespawn()
{
	if( WorldInfo.Game.ShouldRespawn(self) )
		StartSleeping();
	else
		GotoState('Disabled');
}

function float GetRespawnTime()
{
	return RespawnTime;
}


function PickedUpBy(Pawn P)
{
	//local avaGame GI;
	//Super.PickedUpBy(P);
	//GI = avaGame(WorldInfo.Game);

	//if (GI != none && GI.GameStats != none && P.PlayerReplicationInfo != none)
	//{
	//	GI.GameStats.PickupInventoryEvent(Self.Class,P.PlayerReplicationInfo,'pickup');
	//}
}

defaultproperties
{
     RespawnTime=30.000000
	 MessageClass=class'avaPickupMessage'
	InventoryType=class'avaGame.avaPickupInventory'
}
