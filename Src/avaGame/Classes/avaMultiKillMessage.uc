/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaMultiKillMessage extends avaLocalMessage;

var	localized string 	KillString[6];
var name KillSoundName[6];

static function string GetString(
	optional int Switch,
	optional bool bPRI1HUD,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	return "";
}

static simulated function ClientReceive(
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	if (P.GamePlayEndedState())
	{
		`warn("Possible incorrect multikill message" @ P @ Switch @ RelatedPRI_1 @ RelatedPRI_2 @ RelatedPRI_1.PlayerName @ RelatedPRI_2.PlayerName);
	}
	Super.ClientReceive(P, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);
}

static function Name AnnouncementSound(byte MessageIndex);


static function bool IsRewardAnnouncement(byte MessageIndex)
{
	return true;
}

static function int GetFontSize( int Switch, PlayerReplicationInfo RelatedPRI1, PlayerReplicationInfo RelatedPRI2, PlayerReplicationInfo LocalPlayer )
{
	if ( Switch < 4 )
		return 1;
	if ( Switch == 4 )
		return 1;
	if ( Switch == 7 )
		return 3;
	return 2;
}

defaultproperties
{
	KillSoundName(0)=DoubleKill
	KillSoundName(1)=MultiKill
	KillSoundName(2)=MegaKill
	KillSoundName(3)=UltraKill
	KillSoundName(4)=MonsterKill
	KillSoundName(5)=HolyShit
	bIsSpecial=True
	bIsUnique=True
	Lifetime=3
	bBeep=False

	DrawColor=(R=255,G=0,B=0)
	FontSize=1

    PosY=0.242

}
