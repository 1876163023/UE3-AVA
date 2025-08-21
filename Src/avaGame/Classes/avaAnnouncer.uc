/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAnnouncer extends Info
	config(Game);

/** information about an announcer package */
struct AnnouncerPackageInfo
{
	/** name of the package */
	var string PackageName;
	/** prefix for the names of all sounds in this package */
	var string SoundPrefix;
};

var globalconfig AnnouncerPackageInfo StatusSoundPackage;
var globalconfig AnnouncerPackageInfo RewardSoundPackage;
var AnnouncerPackageInfo FallbackStatusSoundPackage;
var AnnouncerPackageInfo FallbackRewardSoundPackage;
var globalconfig	byte			AnnouncerLevel;				// 0=none, 1=no possession announcements, 2=all
// FIXMESTEVE var globalconfig	byte			AnnouncerVolume;			// 1 to 4

struct CachedSound
{
	var name CacheName;
	var SoundNodeWave CacheSound;
};

var array<CachedSound> CachedSounds;	// sounds which had to be gotten from backup package

var bool bPrecachedBaseSounds;
var bool bPrecachedGameSounds;
var const bool bEnglishOnly;		// this announcer has not been translated into other languages

var byte PlayingAnnouncementIndex;
var class<avaLocalMessage> PlayingAnnouncementClass; // Announcer Sound

var	avaQueuedAnnouncement	Queue;

var	float				GapTime;			// Time between playing 2 announcer sounds

var avaPlayerController PlayerOwner;

/** the sound cue used for announcer sounds. We then use a wave parameter named Announcement to insert the actual sound we want to play.
 * (this allows us to avoid having to change a whole lot of cues together if we want to change SoundCue options for the announcements)
 */
var SoundCue AnnouncerSoundCue;

function Destroyed()
{
	local avaQueuedAnnouncement A;

	Super.Destroyed();

	for ( A=Queue; A!=None; A=A.nextAnnouncement )
		A.Destroy();
}

function PostBeginPlay()
{
	Super.PostBeginPlay();

	PlayerOwner = avaPlayerController(Owner);
}

function AnnouncementFinished(AudioComponent AC)
{
	if (GapTime > 0.0)
	{
		SetTimer(GapTime, false, 'PlayNextAnnouncement');
	}
	else
	{
		PlayNextAnnouncement();
	}
}

function PlayNextAnnouncement()
{
	local avaQueuedAnnouncement PlayedAnnouncement;

	PlayingAnnouncementClass = None;

	if (Queue != None)
	{
		PlayedAnnouncement = Queue;
		Queue = PlayedAnnouncement.nextAnnouncement;
		PlayAnnouncementNow(PlayedAnnouncement.AnnouncementClass, PlayedAnnouncement.MessageIndex);
		PlayedAnnouncement.Destroy();
	}
}

function PlayAnnouncementNow(class<avaLocalMessage> InMessageClass, byte MessageIndex)
{
	local SoundNodeWave ASound;
	local AudioComponent AC;

	ASound = GetSound(InMessageClass.Static.AnnouncementSound(MessageIndex), InMessageClass.Static.IsRewardAnnouncement(MessageIndex));

	if ( ASound != None )
	{
		AC = PlayerOwner.CreateAudioComponent(AnnouncerSoundCue, false, false);
		// AC will be none if -nosound option used
		if ( AC != None )
		{
			AC.SetWaveParameter('Announcement', ASound);
			AC.bAutoDestroy = true;
			AC.bShouldRemainActiveIfDropped = true;
			AC.OnAudioFinished = AnnouncementFinished;
			AC.Play();
		}
		PlayingAnnouncementClass = InMessageClass;
		PlayingAnnouncementIndex = MessageIndex;
	}
}

function PlayAnnouncement(class<avaLocalMessage> InMessageClass, byte MessageIndex)
{
	if ( InMessageClass.Static.AnnouncementLevel(MessageIndex) > AnnouncerLevel )
	{
		/* FIXMESTEVE
		if ( AnnouncementLevel == 2 )
			PlayerOwner.ClientPlaySound(Soundcue'GameSounds.DDAverted');
		else if ( AnnouncementLevel == 1 )
			PlayerOwner.PlayBeepSound();
		*/
		return;
	}

	if ( PlayingAnnouncementClass == None )
	{
		// play immediately
		PlayAnnouncementNow(InMessageClass, MessageIndex);
		return;
	}
	InMessageClass.static.AddAnnouncement(self, MessageIndex);
}

/** constructs the fully qualified name of the sound given a package info and cue name */
function string GetFullSoundName(AnnouncerPackageInfo Package, name CueName)
{
	return (Package.PackageName $ "." $ Package.SoundPrefix $ CueName);
}

function SoundNodeWave GetSound(name AName, bool bIsReward)
{
	local SoundNodeWave NewSound;
	local string FullName;
	local int i;

	// check fallback sounds
	for (i = 0; i < CachedSounds.Length; i++)
	{
		if (AName == CachedSounds[i].CacheName)
		{
			return CachedSounds[i].CacheSound;
		}
	}

	FullName = GetFullSoundName(bIsReward ? RewardSoundPackage : StatusSoundPackage, AName);

	// DLO is cheap if already loaded
	NewSound = SoundNodeWave(DynamicLoadObject(FullName, class'SoundNodeWave', true));

	if (NewSound == None)
	{
		NewSound = bIsReward ? PrecacheRewardSound(AName) : PrecacheStatusSound(AName);
	}

	return NewSound;
}

function SoundNodeWave PrecacheRewardSound(name AName)
{
	local SoundNodeWave NewSound;
	local string FullName;

	FullName = GetFullSoundName(RewardSoundPackage, AName);
	NewSound = SoundNodeWave(DynamicLoadObject(FullName, class'SoundNodeWave', true));

	if (NewSound == None && FallBackRewardSoundPackage.PackageName != "")
	{
		NewSound = PrecacheFallbackPackage(FallBackRewardSoundPackage, AName);
	}

	if (NewSound == None)
	{
		`warn("Could not find" @ AName @ "in" @ RewardSoundPackage.PackageName @ "nor in fallback package" @ FallBackRewardSoundPackage.PackageName);
	}

	return NewSound;
}

function SoundNodeWave PrecacheStatusSound(name AName)
{
	local SoundNodeWave NewSound;
	local string FullName;

	FullName = GetFullSoundName(StatusSoundPackage, AName);
	NewSound = SoundNodeWave(DynamicLoadObject(FullName, class'SoundNodeWave', true));

	if (NewSound == None && FallBackStatusSoundPackage.PackageName != "")
	{
		NewSound = PrecacheFallbackPackage(FallBackStatusSoundPackage, AName);
	}

	if (NewSound == None)
	{
		`warn("Could not find" @ AName @ "in" @ StatusSoundPackage.PackageName @ "nor in fallback package" @ FallBackStatusSoundPackage.PackageName);
	}

	return NewSound;
}

function SoundNodeWave PrecacheFallbackPackage(AnnouncerPackageInfo Package, name AName)
{
	local SoundNodeWave NewSound;
	local int i;
	local string FullName;

	FullName = GetFullSoundName(Package, AName);
	NewSound = SoundNodeWave(DynamicLoadObject(FullName, class'SoundNodeWave', true));
	if (NewSound != None)
	{
		for (i = 0; i < CachedSounds.Length; i++)
		{
			if (CachedSounds[i].CacheName == AName)
			{
				CachedSounds[i].CacheSound = NewSound;
				return NewSound;
			}
		}

		CachedSounds.Length = CachedSounds.Length + 1;
		CachedSounds[CachedSounds.Length - 1].CacheName = AName;
		CachedSounds[CachedSounds.Length - 1].CacheSound = NewSound;

		return NewSound;
	}

	return None;
}

function PrecacheAnnouncements()
{
	//local class<avaGame> GameClass;

	//if ( !bPrecachedGameSounds && (WorldInfo.GRI != None) && (WorldInfo.GRI.GameClass != None) )
	//{
	//	bPrecachedGameSounds = true;
	//	GameClass = class<avaGame>(WorldInfo.GRI.GameClass);
	//	if ( GameClass != None )
	//		GameClass.Static.PrecacheGameAnnouncements(self);
	//}

	/* FIXMESTEVE
	ForEach DynamicActors(class'Actor', A)
		A.PrecacheAnnouncer(self);
	*/
	if ( !bPrecachedBaseSounds )
	{
		bPrecachedBaseSounds = true;

		/*PrecacheStatusSound('Count01');
		PrecacheStatusSound('Count02');
		PrecacheStatusSound('Count03');
		PrecacheStatusSound('Count04');
		PrecacheStatusSound('Count05');
		PrecacheStatusSound('Count06');
		PrecacheStatusSound('Count07');
		PrecacheStatusSound('Count08');
		PrecacheStatusSound('Count09');
		PrecacheStatusSound('Count10');*/
	}
}

defaultproperties
{
	//FallbackStatusSoundPackage=(PackageName="A_Announcer_UTFemale01.wav",SoundPrefix="A_Ann_UTFem_")
	//FallbackRewardSoundPackage=(PackageName="A_Announcer_UTMale01.wav",SoundPrefix="A_Announcer_UTMale_")
	//AnnouncerSoundCue=SoundCue'A_Gameplay.Gameplay.A_Gameplay_AnnouncerCue'
}
