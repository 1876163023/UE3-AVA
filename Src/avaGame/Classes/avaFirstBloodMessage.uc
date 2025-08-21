// This class is expired
/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaFirstBloodMessage extends avaLocalMessage;

var localized string FirstBloodString;

static function string GetString(
	optional int Switch,
	optional bool bPRI1HUD,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	if (RelatedPRI_1 == None)
		return "";
	if (RelatedPRI_1.PlayerName == "")
		return "";
	return RelatedPRI_1.PlayerName@Default.FirstBloodString;
}

static simulated function ClientReceive(
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	Super.ClientReceive(P, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);

	if (RelatedPRI_1 != P.PlayerReplicationInfo)
		return;
}

static function Name AnnouncementSound(byte MessageIndex);

static function bool IsRewardAnnouncement(byte MessageIndex)
{
	return true;
}

defaultproperties
{
	bBeep=False
	DrawColor=(R=255,G=0,B=0)
}
