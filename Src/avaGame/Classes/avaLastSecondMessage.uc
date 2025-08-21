/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaLastSecondMessage extends avaLocalMessage;

var localized string LastSecondRed, LastSecondBlue;

static function string GetString(
	optional int Switch,
	optional bool bPRI1HUD,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	if ( TeamInfo(OptionalObject) == None )
		return "";
	if ( TeamInfo(OptionalObject).TeamIndex == 0 )
		return Default.LastSecondRed;
	else
		return Default.LastSecondBlue;
}

static function bool IsRewardAnnouncement(byte MessageIndex)
{
	return ( MessageIndex == 1 );
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
}

static function Name AnnouncementSound(byte MessageIndex)
{
	if ( MessageIndex == 1 )
		return 'Denied';
	else
		return 'LastSecondSave';
}

static function PrecacheGameAnnouncements(avaAnnouncer Announcer, class<GameInfo> GameClass)
{
	Announcer.PrecacheRewardSound('LastSecondSave');
	Announcer.PrecacheRewardSound('Denied');
}

defaultproperties
{
	bBeep=false
	bIsUnique=True
    FontSize=1
    PosY=0.1
	DrawColor=(R=0,G=160,B=255,A=255)
}
