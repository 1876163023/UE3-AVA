// this class is expired...
/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
//
// Switch is the note.
// RelatedPRI_1 is the player on the spree.
//
class avaKillingSpreeMessage extends avaLocalMessage;

var	localized string EndSpreeNote, EndSelfSpree, EndFemaleSpree, MultiKillString;
var	localized string SpreeNote[10];
var	localized string SelfSpreeNote[10];
var name SpreeSoundName[10];
var	localized string EndSpreeNoteTrailer;

static function int GetFontSize( int Switch, PlayerReplicationInfo RelatedPRI1, PlayerReplicationInfo RelatedPRI2, PlayerReplicationInfo LocalPlayer )
{
	local Pawn ViewPawn;
	if ( RelatedPRI2 == None )
	{
		if ( LocalPlayer == RelatedPRI1 )
			return 2;
		if ( LocalPlayer.bOnlySpectator )
		{
			ViewPawn = Pawn(PlayerController(LocalPlayer.Owner).ViewTarget);
			if ( (ViewPawn != None) && (ViewPawn.PlayerReplicationInfo == RelatedPRI1) )
				return 2;
		}
	}
	return Default.FontSize;
}

static function string GetString(
	optional int Switch,
	optional bool bPRI1HUD,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	if (RelatedPRI_2 == None)
	{
		if ( bPRI1HUD )
			return Default.SelfSpreeNote[Switch];
		if (RelatedPRI_1 == None)
			return "";

		if (RelatedPRI_1.PlayerName != "")
			return RelatedPRI_1.PlayerName@Default.SpreeNote[Switch];
	}
	else
	{
		if (RelatedPRI_1 == None)
		{
			if (RelatedPRI_2.PlayerName != "")
			{
				if ( RelatedPRI_2.bIsFemale )
					return RelatedPRI_2.PlayerName@Default.EndFemaleSpree;
				else
					return RelatedPRI_2.PlayerName@Default.EndSelfSpree;
			}
		}
		else
		{
			return RelatedPRI_1.PlayerName$Default.EndSpreeNote@RelatedPRI_2.PlayerName@Default.EndSpreeNoteTrailer;
		}
	}
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
	Super.ClientReceive(P, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);

	if (RelatedPRI_2 != None)
		return;
}

static function Name AnnouncementSound(byte MessageIndex);

static function bool IsRewardAnnouncement(byte MessageIndex)
{
	return true;
}

defaultproperties
{
	 FontSize=1
	 bBeep=False
	 SpreeSoundName(0)=KillingSpree
	 SpreeSoundName(1)=Rampage
	 SpreeSoundName(2)=Dominating
	 SpreeSoundName(3)=Unstoppable
	 SpreeSoundName(4)=GodLike
}
