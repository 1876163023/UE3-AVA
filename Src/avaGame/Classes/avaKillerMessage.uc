/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaKillerMessage extends avaLocalMessage;

var(Message) localized string YouKilled;
var(Message) localized string YouKilledTrailer;

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

	if (RelatedPRI_2.PlayerName != "")
		return Default.YouKilled@RelatedPRI_2.PlayerName@Default.YouKilledTrailer;
}

defaultproperties
{
	bIsSpecial=True
	bIsUnique=True
	DrawColor=(R=0,G=160,B=255,A=255)
	FontSize=1
    PosY=0.10
}
