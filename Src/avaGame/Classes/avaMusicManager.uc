/**
 * Copyright ?2005 Epic Games, Inc. All Rights Reserved.
 */
class avaMusicManager extends Info
	config(Game);

var	float	Tempo;					/** Tempo in Beats per Minute.  @Todo Steve - should be stored in MusicPackage */
var float	MusicStartTime;			/** Time at which current track started playing */
var	int		LastBeat;				/** Count of beats since MusicStartTime */
var avaPlayerController PlayerOwner;	/** Owner of this MusicManager */

var globalconfig float MusicVolume;	/** Maximum volume for music audiocomponents (max value for VolumeMultiplier). */

var float CrossFadeTime;			/** How long it takes to cross-fade between tracks */
var float FadeFactor;				/** Pre-computed MusicVolume/CrossFadeTime deltatime multiplier for cross-fading */

var float LastActionEventTime;		/** Time at which last "action event" occured - used to determine when to fade out action track. */
var	bool	bPendingAction;			/** If true, switch to action on next beat */

enum EMusicState
{
	MST_Ambient,
	MST_Tension,
	MST_Suspense,
	MST_Action,
	MST_Victory,
};
var EMusicState CurrentState;		/** Current Music state (reflects which track is active). */

var int PendingEvent;				/** Pending music event - will be processed on next beat. */
var int	CurrentEvent;				/** Current music event (stinger being played). */

var AudioComponent CurrentTrack;	/** Track being ramped up, rather than faded out */
var AudioComponent  MusicTracks[6]; /** Music Tracks - see ChangeTrack() for definition of slots. */

/** Music track cues */
var SoundCue	AmbientCue;
var SoundCue	ActionCue;
var SoundCue	IntroCue;
var SoundCue	SuspenseCue;
var	SoundCue	TensionCue;
var	SoundCue	VictoryCue;

var AudioComponent	EventTrack;				/** Track for stingers */

/** Stinger cues */

/** StartMusic()
* Initialize MusicManager and start intro track.  
*/
function StartMusic()
{
	MusicTracks[5] = CreateNewTrack(IntroCue);
	CurrentTrack = MusicTracks[5];
	LastBeat = 0;
	CurrentState = MST_Ambient;
	MusicTracks[5].VolumeMultiplier = MusicVolume;
	MusicTracks[5].Play();
	MusicStartTime = WorldInfo.TimeSeconds;
	PlayerOwner = avaPlayerController(Owner);
	FadeFactor = MusicVolume/CrossFadeTime;
}

/* CreateNewTrack()
* Create a new AudioComponent to play MusicCue.
* @param MusicCue:  the sound cue to play
* @returns the new audio component
*/
function AudioComponent CreateNewTrack(SoundCue MusicCue)
{
	local AudioComponent AC;

	AC = CreateAudioComponent( MusicCue, false, true );
	AC.bAllowSpatialization = false;
	AC.bShouldRemainActiveIfDropped = true;
	return AC;
}

/* MusicEvent()
Music Manager interface for musical events.
@param NewEventIndex:  see list below
0 - enemy action (shooting at you, you shooting at them)
1 - kill
2 - died
3 - returned flag
4 - enemy took flag (from base)
5 - enemy returned flag NOT IMPLEMENTED, CHANGE?
6 - major kill (first blood, killed flag carrier, or took lead in DM)
7 - took flag
8 - killing spree
9 - double kill
10 - long spree
11 - ultrakill
12 - monster kill
13- score behind
14 - score increase lead
15 - score take lead
*/
function MusicEvent(int NewEventIndex)
{
	// set pendingevent - will be processed on the next beat
	PendingEvent = Max(PendingEvent, NewEventIndex);

	// request change to action track if appropriate
    if ( (PendingEvent != 2)
		&& (PendingEvent != 3)
		&& (PendingEvent != 4)
		&& (PendingEvent != 5)
		&& (PendingEvent != 7)
		&& (PendingEvent < 13) )
	{
		if ( CurrentState != MST_Action ) 
			bPendingAction = true;
		LastActionEventTime = WorldInfo.TimeSeconds;
	}
}

/** ProcessMusicEvent()
process PendingEvent.  Called from Tick() on a beat.
*/
function ProcessMusicEvent()
{
	local SoundCue EventCue;

	// change to action track if appropriate
    if ( bPendingAction )
	{
		if ( CurrentState != MST_Action ) 
			ChangeTrack(MST_Action);
		bPendingAction = false;
	}

	if ( (EventTrack != None) && EventTrack.bWasPlaying )
	{
		// skip if higher priority stinger already playing
		if ( CurrentEvent > PendingEvent )
			return;

		// stop already playing stinger
		EventTrack.Stop();
		EventTrack = None;
	}

	EventCue = none;

	// play appropriate stinger
	/*Switch ( PendingEvent )
	{
		case 1:		EventCue = KillCue; break;
		case 2:		EventCue = DiedCue; break;
		case 3:		EventCue = ReturnFlagCue; break;
		case 4:		EventCue = EnemyGrabFlagCue; break;
		case 5:		EventCue = FlagReturnedCue; break;
		case 6:		EventCue = MajorKillCue; break;
		case 7:		EventCue = GrabFlagCue; break;
		case 8:		EventCue = FirstSpreeCue; break;
		case 9:		EventCue = DoubleKillCue; break;
		case 10:	EventCue = LongSpreeCue; break;
		case 11:	EventCue = MultiKillCue; break;
		case 12:	EventCue = MonsterKillCue; break;
		case 13:	EventCue = ScoreLosingCue; break;
		case 14:	EventCue = ScoreTieCue; break;
		case 15:	EventCue = ScoreWinningCue; break;
	}*/

	if (EventCue == none) return;

	CurrentEvent = PendingEvent;
	EventTrack = CreateNewTrack(EventCue);
	EventTrack.bAutoDestroy = true;
	EventTrack.VolumeMultiplier = FMin(1.0, 1.5*MusicVolume);
	EventTrack.Play();
}

function Tick(float DeltaTime)
{
	local float NumBeats;
	local int i;
	local EMusicState NewState;

	if (CurrentTrack == None) return;

	// Cross-fade
	if ( CurrentTrack.VolumeMultiplier < MusicVolume )
	{
		// ramp up current track
		CurrentTrack.VolumeMultiplier = FMin(MusicVolume, CurrentTrack.VolumeMultiplier + FadeFactor*DeltaTime);
	}

	for ( i=0; i<6; i++ )
	{
		// ramp down other tracks
		if ( (MusicTracks[i] != None) && (MusicTracks[i] != CurrentTrack) && (MusicTracks[i].VolumeMultiplier > 0.f) )
		{
			MusicTracks[i].VolumeMultiplier = MusicTracks[i].VolumeMultiplier - FadeFactor*DeltaTime;
			if ( MusicTracks[i].VolumeMultiplier <= 0.f )
			{
				MusicTracks[i].VolumeMultiplier = 0.f;
				MusicTracks[i].Stop();
			}
		}
	}

	// only process music on every other beat
	NumBeats = (WorldInfo.TimeSeconds - MusicStartTime) * Tempo/60;
	if ( NumBeats - LastBeat < 1 )
	{
		return;
	}

	LastBeat = int(NumBeats);
	if ( LastBeat % 2 != 0 )
	{
		return;
	}

	// process any outstanding pending events
	if ( PendingEvent > 0 )
	{
		ProcessMusicEvent();
		PendingEvent = 0;
		return;
	}

	// check if there is current game action (to keep the action track going)
	if ( PlayerOwner.Pawn != None )
	{
		if ( (PlayerOwner.Pawn.Weapon != None) && PlayerOwner.Pawn.Weapon.IsFiring() )
		{
			LastActionEventTime = WorldInfo.TimeSeconds;
		}
		else if ( PlayerOwner.Pawn.InCombat() )
		{
			LastActionEventTime = WorldInfo.TimeSeconds;
			if ( CurrentState != MST_Action )
				ChangeTrack(MST_Action);
		}
	}
	else
	{
		LastActionEventTime = 0;
	}

	if ( (CurrentState != MST_Action) || (WorldInfo.TimeSeconds - LastActionEventTime > 8) )
	{
		// determine if music state needs to change
		if ( (PlayerOwner.Pawn != None) && PlayerOwner.Pawn.PoweredUp() ) 
		{
			NewState = MST_Victory;
		}
		else if ( !avaGameReplicationInfo(WorldInfo.GRI).FlagsAreHome() )
		{
			if ( PlayerOwner.PlayerReplicationInfo.bHasFlag )
				NewState = MST_Victory;
			else
			{
				if ( avaGameReplicationInfo(WorldInfo.GRI).FlagIsHome(PlayerOwner.PlayerReplicationInfo.Team.TeamIndex) )
					NewState = MST_Suspense;
				else
					NewState = MST_Tension;
			}
		}
		else
		{
			NewState = MST_Ambient;
		}

		if ( NewState != CurrentState )
			ChangeTrack(NewState);
	}
}

/** ChangeTrack()
* @param NewState  New music state (track to ramp up).
*/
function ChangeTrack(EMusicState NewState)
{
	local AudioComponent NewTrack;

	if ( CurrentState == NewState )
		return;
	CurrentState = NewState;

	// select appropriate track
	Switch( NewState )
	{
		case MST_Ambient:
			if ( MusicTracks[0] == None ) MusicTracks[0] = CreateNewTrack(AmbientCue);
			NewTrack = MusicTracks[0];
			break;
		case MST_Tension:
			if ( MusicTracks[1] == None ) MusicTracks[1] = CreateNewTrack(TensionCue);
			NewTrack = MusicTracks[1];
			break;
		case MST_Suspense:
			if ( MusicTracks[2] == None ) MusicTracks[2] = CreateNewTrack(SuspenseCue);
			NewTrack = MusicTracks[2];
			break;
		case MST_Action:
			if ( MusicTracks[3] == None ) MusicTracks[3] = CreateNewTrack(ActionCue);
			NewTrack = MusicTracks[3];
			break;
		case MST_Victory:
			if ( MusicTracks[4] == None ) MusicTracks[4] = CreateNewTrack(VictoryCue);
			NewTrack = MusicTracks[4];
			break;
	}

	if ( CurrentTrack == NewTrack )
		return;

	// play selected track
	CurrentTrack = NewTrack;
	MusicStartTime = WorldInfo.TimeSeconds;
	LastBeat = 0;
	CurrentTrack.VolumeMultiplier = 0.0;
	CurrentTrack.Play();
}

defaultproperties
{
	Tempo=167.8934
	CrossFadeTime=+4.0

	//ActionCue=SoundCue'A_Music_Interactive.Music.ForegoneMusicActionCue'
	//AmbientCue=SoundCue'A_Music_Interactive.Music.ForegoneMusicAmbientCue'
	//IntroCue=SoundCue'A_Music_Interactive.Music.ForegoneMusicIntroCue'
	//SuspenseCue=SoundCue'A_Music_Interactive.Music.ForegoneMusicSuspenseCue'
	//TensionCue=SoundCue'A_Music_Interactive.Music.ForegoneMusicTensionAmbientCue'
	//VictoryCue=SoundCue'A_Music_Interactive.Music.ForegoneMusicVictoryCue'
}


