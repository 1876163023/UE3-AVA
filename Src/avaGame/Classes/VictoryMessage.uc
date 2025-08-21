/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class VictoryMessage extends avaLocalMessage;

var name VictorySoundName[6];

static function byte AnnouncementLevel(byte MessageIndex)
{
	return 2;
}

static function Name AnnouncementSound(byte MessageIndex)
{
	return Default.VictorySoundName[MessageIndex];
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

static function PrecacheGameAnnouncements(avaAnnouncer Announcer, class<GameInfo> GameClass)
{
	Announcer.PrecacheStatusSound(Default.VictorySoundName[0]);
	Announcer.PrecacheStatusSound(Default.VictorySoundName[1]);
	if ( GameClass.Default.bTeamGame )
	{
		Announcer.PrecacheStatusSound(Default.VictorySoundName[4]);
		Announcer.PrecacheStatusSound(Default.VictorySoundName[5]);
	}
	else
	{
		Announcer.PrecacheStatusSound(Default.VictorySoundName[2]);
		Announcer.PrecacheStatusSound(Default.VictorySoundName[3]);
	}
}

defaultproperties
{
	VictorySoundName(0)=FlawlessVictory
	VictorySoundName(1)=HumiliatingDefeat
	VictorySoundName(2)=WonTheMatch
	VictorySoundName(3)=LostTheMatch
	VictorySoundName(4)=RedTeamWins
	VictorySoundName(5)=BlueTeamWinner
}
