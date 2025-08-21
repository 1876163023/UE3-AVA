/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaTeamScoreMessage extends avaLocalMessage;

var name TeamScoreSoundName[7];

static function PrecacheGameAnnouncements(avaAnnouncer Announcer, class<GameInfo> GameClass)
{
	local int i;

	for ( i=0; i<6; i++ )
	{
		Announcer.PrecacheStatusSound(Default.TeamScoreSoundName[i]);
	}
	Announcer.PrecacheRewardSound(Default.TeamScoreSoundName[6]);
}

static function byte AnnouncementLevel(byte MessageIndex)
{
	return 2;
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
	return Default.TeamScoreSoundName[MessageIndex];
}
defaultproperties
{
    TeamScoreSoundName(0)=RedTeamScores
    TeamScoreSoundName(1)=BlueTeamScores
    TeamScoreSoundName(2)=RedTeamIncreasesTheirLead
    TeamScoreSoundName(3)=BlueTeamIncreasesTheirLead
    TeamScoreSoundName(4)=RedTeamTakesTheLead
    TeamScoreSoundName(5)=BlueTeamTakesTheLead
    TeamScoreSoundName(6)=HatTrick
}
