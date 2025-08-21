/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaIdleKickWarningMessage extends LocalMessage;

static function string GetString(
	optional int Switch,
	optional bool bPRI1HUD,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
    return class'GameMessage'.Default.KickWarning;
}

defaultproperties
{
	bIsUnique=false
	bIsPartiallyUnique=true
	bIsConsoleMessage=False
	Lifetime=1

	DrawColor=(R=255,G=255,B=64,A=255)
	FontSize=1

    PosY=0.242
}
