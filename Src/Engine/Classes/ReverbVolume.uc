/**
 * Copyright 2006 Epic Games, Inc. All Rights Reserved.
 *
 * Used to affect reverb settings in the game and editor.
 */
class ReverbVolume extends Volume
	native
	placeable
	hidecategories(Advanced,Attachment,Collision);
/**
 * Indicates a reverb preset to use.
 */
enum ReverbPreset
{	
	REVERB_Off,	
	REVERB_Generic,
	REVERB_PaddedCell,
	REVERB_Room,
    REVERB_Bathroom,
	REVERB_LivingRoom,
    REVERB_StoneRoom,
    REVERB_Auditorium,
    REVERB_ConcertHall,
    REVERB_Cave,
	REVERB_Arena,
	REVERB_CarpettedHallway,
    REVERB_Hallway,
    REVERB_StoneCorridor,
    REVERB_Alley,
    REVERB_Forest,
    REVERB_City,
    REVERB_Mountains,
    REVERB_Quarry,
    REVERB_Plain,
    REVERB_ParkingLot,
    REVERB_SewerPipe,
    REVERB_Underwater,    
};

/** Struct encapsulating settings for reverb effects. */
struct native ReverbSettings
{
	/** The reverb preset to employ. */
	var() ReverbPreset	ReverbType;
	var() ReverbProperty UserdefinedReverbPreset;

	/** Volume level of the reverb affect. */
	var() float			Volume;

	/** Time to fade from the current reverb settings into this setting, in seconds. */
	var() float			FadeTime;

	structdefaultproperties
	{
		ReverbType=REVERB_Off
		Volume=0.5
		FadeTime=2.0
	}
};

/**
 * Priority of this volume. In the case of overlapping volumes the one with the highest priority
 * is chosen. The order is undefined if two or more overlapping volumes have the same priority.
 */
var()							float				Priority;

/** Reverb settings to use for this volume. */
var()							DSPPreset			DSPPreset;
var()							SoundCue			AmbientSound;
var()							ReverbSettings		Settings;

/** Next volume in linked listed, sorted by priority in descending order. */
var const noimport transient	ReverbVolume		NextLowerPriorityVolume;

var() localized string VolumeName;

cpptext
{
	/**
	 * Removes the reverb volume to world info's list of reverb volumes.
	 */
	virtual void ClearComponents();

protected:
	/**
	 * Adds the reverb volume to world info's list of reverb volumes.
	 */
	virtual void UpdateComponentsInternal(UBOOL bCollisionUpdate = FALSE);
public:
	virtual void PostEditChange(UProperty* PropertyThatChanged);
}

defaultproperties
{
	Begin Object Name=BrushComponent0
		CollideActors=True
		BlockActors=False
//		BlockZeroExtent=False
// [2006/10/24 YTS] MultiLineCheck(TraceActors)로 검사할 수 있도록 수정
		BlockZeroExtent=True
//		PRI.PlayerLocationHint 에 포함되기 위해 수정했음.
//		BlockNonZeroExtent=False
		BlockRigidBody=False
	End Object

//  a localized-string Cannot be assigned here.
//	VolumeName = "nowhere"

//  PRI.PlayerLocationHint 에 포함되기 위해 (Touching Actors)수정했음
	bCollideActors=True
	bBlockActors=False
	bProjTarget=False
	SupportedEvents.Empty
	SupportedEvents(0)=class'SeqEvent_Touch'
	SupportedEvents(1)=class'SeqEvent_UnTouch'
}
