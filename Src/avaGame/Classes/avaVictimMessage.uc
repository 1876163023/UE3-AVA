/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaVictimMessage extends avaLocalMessage;

var(Message) localized string YouWereKilledBy, KilledByTrailer;

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

	if (RelatedPRI_1.PlayerName != "")
		return Default.YouWereKilledBy@RelatedPRI_1.PlayerName$Default.KilledByTrailer;
}

defaultproperties
{
	bIsUnique=True
	Lifetime=6
	DrawColor=(R=255,G=0,B=0,A=255)
	FontSize=1
    PosY=0.10
}
