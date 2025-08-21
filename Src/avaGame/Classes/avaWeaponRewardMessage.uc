/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaWeaponRewardMessage extends avaLocalMessage;

var	localized string 	RewardString[4];
var name RewardSoundName[4];

static function string GetString(
	optional int Switch,
	optional bool bPRI1HUD,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	return Default.RewardString[Switch];
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
	return Default.RewardSoundName[MessageIndex];
}

static function bool IsRewardAnnouncement(byte MessageIndex)
{
	return true;
}

defaultproperties
{
	RewardSoundName(0)=HeadShot
	RewardSoundName(1)=FlakMonkey
	RewardSoundName(2)=HeadHunter
	RewardSoundName(3)=ComboWhore
	bIsSpecial=True
	bIsUnique=True
	Lifetime=3
	bBeep=False

	DrawColor=(R=255,G=255,B=0)
	FontSize=2

	PosY=0.242
}
