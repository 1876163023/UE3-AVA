/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
//
// OptionalObject is an Pickup class
//
class avaPickupMessage extends avaLocalMessage;

static simulated function ClientReceive( 
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	Super.ClientReceive(P, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);

	if ( avaHUD(P.MyHUD) != None )
	{
		if ( class<avaPickupFactory>(OptionalObject) != None )
			class<avaPickupFactory>(OptionalObject).static.UpdateHUD(avaHUD(P.MyHUD));
		else
			avaHUD(P.MyHUD).LastPickupTime = avaHUD(P.MyHUD).WorldInfo.TimeSeconds;
	}		
}

defaultproperties
{
	bIsUnique=True

	DrawColor=(R=255,G=255,B=0,A=255)
	FontSize=1

    PosY=0.9
}
