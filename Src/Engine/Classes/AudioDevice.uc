/**
 * Copyright 2006 Epic Games, Inc. All Rights Reserved.
 */
class AudioDevice extends Subsystem
	config(engine)
	native
	transient
	noexport;


struct ListenerStruct
{
	var vector Location;
	var vector Up;
	var vector Right;
	var vector Front;
};

/**
 * Enum describing the sound modes available for use in game.
 *
 * @warning: manually mirrored in UnAudio.h
 */
enum ESoundMode
{
	/** Normal - No EQ applied											 */
	SOUNDMODE_NORMAL,
	/** Dying - Ranged EQ applied depending on health					 */
	SOUNDMODE_DYING,
	/** Death - Death EQ applied										 */
	SOUNDMODE_DEATH,
	/** Grenade - Shellshocked EQ applied								 */
	SOUNDMODE_GRENADE,
	/** Cover - EQ applied to indicate player is in cover				 */
	SOUNDMODE_COVER,
	/** Roadie Run - Accentuates high-pitched bullet whips, etc.		 */
	SOUNDMODE_ROADIE_RUN,
	/** TacCom - Tactical command EQ lowers game volumes				 */
	SOUNDMODE_TACCOM,
};

/**
 * Structure containing configurable properties of a sound group.
 *
 * @warning: manually mirrored in UnAudio.h
 */
struct SoundGroupProperties
{
	/** Volume multiplier.												*/
	var() float Volume;
	/** Priority multiplier.											*/
	var() float Priority;
	/** Voice center channel volume - Not a multiplier (no propagation)	*/
	var() float VoiceCenterChannelVolume;
	/** Radio volume multiplier  - Not a multiplier (no propagation)	*/
	var() float VoiceRadioVolume;
	/** Sound mode voice - Which sound mode voice applies (no propagation)	*/
	var() int SoundModeVoice;
	/** Whether or not this is music (propagates only if parent is TRUE)*/
	var() bool bIsMusic;
	/** Whether or not this sound group is excluded from reverb EQ		*/
	var() bool bNoReverb;

	structdefaultproperties
	{
		Volume=1
		Priority=1
		VoiceCenterChannelVolume=0
		VoiceRadioVolume=0
		SoundModeVoice=0
		bIsMusic=FALSE
		bNoReverb=FALSE
	}
};

/**
 * Structure containing information about a sound group.
 *
 * @warning: manually mirrored in AudioDevice.uc
 *
 */
struct SoundGroup
{
	/** Configurable properties like volume and priority.				*/
	var() SoundGroupProperties	Properties;
	/** Name of this sound group.										*/
	var() name					GroupName;
	/** Array of names of child sound groups. Empty for leaf groups.	*/
	var() array<name>			ChildGroupNames;
};

var		config const	int						MaxChannels;
var		config const	bool					UseEffectsProcessing;

var		transient const	array<AudioComponent>	AudioComponents;
var		native const	array<pointer>			Sources;
var		native const	array<pointer>			FreeSources;
var		native const	DynamicMap_Mirror		WaveInstanceSourceMap;

var		native const	int						bLastRealtime;

var		native const	array<ListenerStruct>	Listeners;
var		native const	QWORD					CurrentTick;
/** Map from name to the sound group												*/
var		native const	Map_Mirror				NameToSoundGroupMap;
/** Map from name to the "propagated" leaf sound group properties					*/
var		native const	Map_Mirror				NameToPropagatedSoundGroupPropertiesMap;

/** Array of sound groups read from ini file										*/
var()	config			array<SoundGroup>		SoundGroups;

/** Interface to audio effects processing */
var		native const	pointer					Effects;