/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
//
// A Death Message.
//
// Switch 0: Kill
//	RelatedPRI_1 is the Killer.
//	RelatedPRI_2 is the Victim.
//	OptionalObject is the DamageType Class.
//

class avaDeathMessage extends avaLocalMessage
	config(game);

var(Message) localized string KilledString, SomeoneString;
var globalconfig bool bNoConsoleDeathMessages;

static function color GetConsoleColor( PlayerReplicationInfo RelatedPRI_1 )
{
    return class'HUD'.Default.GreenColor;
}

static function string GetString(
	optional int Switch,
	optional bool bPRI1HUD,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	local string KillerName, VictimName;

	if (Class<DamageType>(OptionalObject) == None)
		return "";

	if (RelatedPRI_1 == None)
		KillerName = Default.SomeoneString;
	else
		KillerName = RelatedPRI_1.PlayerName;

	if (RelatedPRI_2 == None)
		VictimName = Default.SomeoneString;
	else
		VictimName = RelatedPRI_2.PlayerName;

	if ( Switch == 1 )
	{
		// suicide
		return class'GameInfo'.Static.ParseKillMessage(
			KillerName,
			VictimName,
			Class<DamageType>(OptionalObject).Static.SuicideMessage(RelatedPRI_2) );
	}

	return class'GameInfo'.Static.ParseKillMessage(
		KillerName,
		VictimName,
		Class<DamageType>(OptionalObject).Static.DeathMessage(RelatedPRI_1, RelatedPRI_2) );
}

static function ClientReceive(
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
}

defaultproperties
{
	DrawColor=(R=255,G=0,B=0,A=255)
    bIsSpecial=false
}
