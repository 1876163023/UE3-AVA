/*=============================================================================
	UnAudio.h: Unreal base audio.
	Copyright 1997-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/** 
* Number of ticks an inaudible source remains alive before being stopped
*/
#define AUDIOSOURCE_TICK_LONGEVITY 60

/**
 * Audio stats
 */
enum EAudioStats
{
	STAT_AudioUpdateTime = STAT_AudioFirstStat,
	STAT_AudioComponents,
	STAT_WaveInstances,
	STAT_AudioMemorySize,
	STAT_WavesDroppedDueToPriority,
	STAT_AudioCPUUsage_DSP,
	STAT_AudioCPUUsage_Stream,
	STAT_AudioCPUUsage_Update,
	STAT_AudioCPUUsage_Total,
};

/**
 * Channel definitions for multistream waves
 *
 * These are in the sample order OpenAL expects for a 7.1 sound
 * 
 */
enum EAudioSpeakers
{							//	4.0	5.1	6.1	7.1
	SPEAKER_FrontLeft,		//	*	*	*	*
	SPEAKER_FrontRight,		//	*	*	*	*
	SPEAKER_FrontCenter,	//		*	*	*
	SPEAKER_LowFrequency,	//		*	*	*
	SPEAKER_SideLeft,		//	*	*	*	*
	SPEAKER_SideRight,		//	*	*	*	*
	SPEAKER_BackLeft,		//			*	*		If there is no BackRight channel, this is the BackCenter channel
	SPEAKER_BackRight,		//				*
	SPEAKER_Count
};

//@todo: change enums to not be game specific but rather ini configurable names
/**
 * Structure containing configurable properties of a sound group.
 *
 * @warning: manually mirrored in AudioDevice.uc
 */
enum ESoundMode
{
	SOUNDMODE_NORMAL = 0,
	SOUNDMODE_DYING,
	SOUNDMODE_DEATH,
	SOUNDMODE_GRENADE,
	SOUNDMODE_COVER,
	SOUNDMODE_ROADIE_RUN,
	SOUNDMODE_TACCOM,
	SOUNDMODE_COUNT
};

// Forward declarations.
class UAudioComponent;
class FSoundSource;
struct FReverbSettings;
struct FSampleLoop;

/*-----------------------------------------------------------------------------
	UAudioDevice.
-----------------------------------------------------------------------------*/

// Listener.
struct FListener
{
	FVector		Location,
				Up,
				Right,
				Front;
};

/**
 * Structure containing configurable properties of a sound group.
 *
 * @warning: manually mirrored in AudioDevice.uc
 */
struct FSoundGroupProperties
{
	/** Volume multiplier.												*/
	FLOAT			Volume;
	/** Priority multiplier.											*/
	FLOAT			Priority;
	/** Voice center channel volume - Not a multiplier (no propagation)	*/
	FLOAT			VoiceCenterChannelVolume;
	/** Radio volume multiplier  - Not a multiplier (no propagation)	*/
	FLOAT			VoiceRadioVolume;
	/** Sound mode voice - Which sound mode voice applies (no propagation)	*/
	INT				SoundModeVoice;
	/** Whether or not this is music (propagates only if parent is TRUE)*/
	BITFIELD		bIsMusic:1;
	/** Whether or not to exclude this sound group from the reverb EQ	*/
	BITFIELD		bNoReverb:1;
};

/**
 * Structure containing information about a sound group.
 *
 * @warning: manually mirrored in AudioDevice.uc
 *
 */
struct FSoundGroup
{
	/** Configurable properties like volume and priority.				*/
	FSoundGroupProperties	Properties;
	/** Name of this sound group.										*/
	FName					GroupName;
	/** Array of names of child sound groups. Empty for leaf groups.	*/
	TArray<FName>			ChildGroupNames;
};

//     2 UU == 1"
// <=> 1 UU == 0.0127 m
#define AUDIO_DISTANCE_FACTOR ( 0.0127f )

/**
 * Base audio class
 *
 * @warning: manually mirrored in AudioDevice.uc
 */
class UAudioDevice : public USubsystem
{
	DECLARE_CLASS(UAudioDevice,USubsystem,CLASS_Config|CLASS_NoExport|CLASS_Transient,Engine)
	friend class FSoundSource;

	// Constructor.
	UAudioDevice( void ) {}

	// UAudioDevice interface.
	virtual UBOOL Init();
	virtual void Flush();
	virtual void FinishDestroy();
	virtual void Update( UBOOL Realtime );
	void SetListener(INT ViewportIndex, const FVector& Location, const FVector& Up, const FVector& Right, const FVector& Front);

	/**
	 * Stops all realtime (and possibly nonrealtime) sounds
	 *
	 * @param bShouldStopNonRealtimeSounds If TRUE, this function will stop non-realtime sounds as well (UI sounds, etc)
	 */
	virtual void StopAllSounds(UBOOL bShouldStopNonRealtimeSounds=FALSE);

	/**
	 * Pushes the specified reverb settings onto the reverb settings stack.
	 *
	 * @param	ReverbSettings		The reverb settings to use.
	 */
	virtual void SetReverbSettings(const FReverbSettings& ReverbSettings);

	virtual void OnReverbSettingsChanged() {}

	/**
	 * Frees the bulk resource data assocated with this SoundNodeWave.
	 *
	 * @param	SoundNodeWave	wave object to free associated bulk data
	 */
	virtual void FreeResource( USoundNodeWave* SoundNodeWave ) 
	{
	}

	/**
	 * Precaches the passed in sound node wave object.
	 *
	 * @param	SoundNodeWave	Resource to be precached.
	 */
	virtual void Precache( USoundNodeWave* SoundNodeWave ) 
	{
	}

	/** 
	 * Attempts to start recording input from the microphone
	 *
	 * @return	TRUE if recording started successfully
	 */
	virtual UBOOL CaptureStart( void ) { return( FALSE ); }

	/** 
	 * Stops recording input from the microphone
	 *
	 * @return	Number of samples recorded. 0 is returned if the the capture was not or failed to start.
	 */
	virtual INT CaptureStop( void ) { return( 0 ); }

	/**
	 * Returns a pointer the start of the captured data
	 *
	 * @return SHORT * to captured samples
	 */
	virtual SWORD * CaptureGetSamples( void ) { return( NULL ); }

	/**
	 * Returns the sound group properties associates with a sound group taking into account
	 * the group tree.
	 *
	 * @param	SoundGroupName	name of sound group to retrieve properties from
	 * @return	sound group properties if a sound group with name SoundGroupName exists, NULL otherwise
	 */
	FSoundGroupProperties* GetSoundGroupProperties( FName SoundGroupName );

	/**
	 * Returns an array of existing sound group names.
	 *
	 * @return array of sound group names
	 */
	TArray<FName> GetSoundGroupNames();

	/**
	 * Parses the sound groups and propagates multiplicative properties down the tree.
	 */
	void ParseSoundGroups();

	// Audio components.
	void AddComponent( UAudioComponent* AudioComponent );
	void RemoveComponent( UAudioComponent* AudioComponent );
	static UAudioComponent* CreateComponent( USoundCue* SoundCue, FSceneInterface* Scene, AActor* Actor = NULL, UBOOL Play = TRUE, UBOOL bStopWhenOwnerDestroyed = FALSE, FVector* Location = NULL, EAudioChannel Channel = (EAudioChannel)0 ); 

	// UObject interface.
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog ) 
	{ 
		return false; 
	}
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void Serialize( FArchive& Ar );

	/**
	 * Updates sound group volumes
	 */
	void SetGroupVolume( FName Group, FLOAT Volume );

	/**
	 * 특정 그룹의 볼륨을 받아옴
	 */
	FLOAT GetGroupVolume( FName Group );

	/**
	 * Sets a new sound mode and applies it to all appropriate sound groups - Must be defined per platform
	 */
	virtual void SetSoundMode(ESoundMode NewSoundMode) { /* This is a no-op unless overloaded on your platform */ };

	/** 
	 * Platform dependent call to associate the sound output with an audio effect
	 */
	virtual void * LinkEffect( UINT SourceId, UBOOL Enable );

	/** 
	 * Checks to see if a coordinate is within a distance of any listener
	 */
	UBOOL LocationIsAudible( FVector Location, FLOAT MaxDistance );

	virtual void LockStream( UBOOL bLock ) {}
	virtual INT GetStreamWriteable( INT Stream, UBOOL bLinear ) { return 0; }
	virtual signed short* GetStreamWritePointer( INT Stream ) { return NULL; }
	virtual void CommitStream( INT Stream, INT NumSample ) {}
	virtual FLOAT GetStreamTime( INT Stream ) { return 0.0f; }


	virtual void PlayStream( INT Stream ) {}
	virtual void StopStream( INT Stream ) {}

	void SetMaxChannels ( INT MaxChannels );
	INT GetMaxChannels() const { return MaxChannels; }

protected:
	void HandlePause( UBOOL bRealtime );
	INT GetSortedActiveWaveInstances( TArray<FWaveInstance*>& WaveInstances, UBOOL bRealtime );
	void StopSources( TArray<FWaveInstance*>& WaveInstances, INT FirstActiveIndex );
	void StartSources( TArray<FWaveInstance*>& WaveInstances, INT FirstActiveIndex, UBOOL bRealtime );

	// Internal.
	void SortWaveInstances( INT MaxChannels );

	/**
	 * Internal helper function used by ParseSoundGroups to traverse the tree.
	 *
	 * @param CurrentGroup			Subtree to deal with
	 * @param ParentProperties		Propagated properties of parent node
	 */
	void RecurseIntoSoundGroups( FSoundGroup* CurrentGroup, FSoundGroupProperties* ParentProperties );

	// Configuration.
	INT											MaxChannels;		
	UBOOL										UseEffectsProcessing;

	// Variables.		
	TArray<UAudioComponent*>					AudioComponents;
	TArray<FSoundSource*>						Sources;	
	TArray<FSoundSource*>						FreeSources;
	TDynamicMap<FWaveInstance*,FSoundSource*>	WaveInstanceSourceMap;
	UBOOL										bLastRealtime;	
	TArray<FListener>							Listeners;
	QWORD										CurrentTick;

	/** Map from name to the sound group												*/
	TMap<FName,FSoundGroup*>					NameToSoundGroupMap;
	/** Map from name to the "propagated" leaf sound group properties					*/
	TMap<FName,FSoundGroupProperties>			NameToPropagatedSoundGroupPropertiesMap;
	/** Array of sound groups read from ini file										*/
	TArrayNoInit<FSoundGroup>					SoundGroups;

	/** Interface to audio effects processing */
	FAudioEffectsManager *						Effects;
};

/*-----------------------------------------------------------------------------
	FSoundSource.
-----------------------------------------------------------------------------*/

class FSoundSource
{
public:
	// Constructor/ Destructor.
	FSoundSource( UAudioDevice* InAudioDevice )
	:	AudioDevice( InAudioDevice ),
		WaveInstance( NULL ),
		Playing( FALSE ),
		Paused( FALSE ),
		ReverbApplied( FALSE ),
		LastUpdate( 0 ),
		LastHeardUpdate( 0 )
	{}
	virtual ~FSoundSource( void ) {}

	// Initialization & update.
	virtual UBOOL Init( FWaveInstance* WaveInstance ) = 0;
	virtual void Update( void ) = 0;

	// Playback.
	virtual void Play( void ) = 0;
	virtual void Stop( void );
	virtual void Pause( void ) = 0;

	// Query.
	virtual	UBOOL IsFinished( void ) = 0;
	/**
	 * Returns whether associated audio component is a realtime- only component, aka one that will
	 * not play unless we're in realtime mode.
	 *
	 * @return FALSE if associated component has bNonRealtime set, TRUE otherwise
	 */
	virtual UBOOL IsRealtimeOnly( void );

protected:
	// Variables.	
	UAudioDevice *		AudioDevice;
	FWaveInstance *		WaveInstance;

	/** Cached status information whether we are playing or not. */
	UBOOL				Playing;
	/** Cached status information whether we are paused or not. */
	UBOOL				Paused;
	/** Cached sound mode value used to detect when to switch outputs. */
	UBOOL				ReverbApplied;

	/** Last tick when this source was active */
	INT					LastUpdate;
	/** Last tick when this source was active *and* had a hearable volume */
	INT					LastHeardUpdate;

	friend class UAudioDevice;
};

/*-----------------------------------------------------------------------------
	UDrawSoundRadiusComponent. 
-----------------------------------------------------------------------------*/

class UDrawSoundRadiusComponent : public UDrawSphereComponent
{
	DECLARE_CLASS(UDrawSoundRadiusComponent,UDrawSphereComponent,CLASS_NoExport,Engine);

	// UPrimitiveComponent interface.
	/**
	 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void UpdateBounds();
};

/**
 * A line of subtitle text and the time at which it should be displayed.
 */
struct FSubtitleCue
{
	/** The text too appear in the subtitle. */
	FString		Text;

	/** The time at which the subtitle is to be displayed, in seconds relative to the beginning of the line. */
	FLOAT		Time;
};

/*-----------------------------------------------------------------------------
	USoundNodeWave. 
-----------------------------------------------------------------------------*/

class USoundNodeWave : public USoundNode
{
public:
	DECLARE_CLASS(USoundNodeWave,USoundNode,CLASS_NoExport,Engine)

	//## BEGIN PROPS SoundNodeWave
	INT CompressionQuality;
	BITFIELD					bCompressedSample:1;
	FLOAT Volume;
	FLOAT Pitch;
	FLOAT Duration;
	INT NumChannels;
	INT SampleRate;
	INT SampleDataSize;

	/** Offsets into the bulk data for the source wav data - always 0 for mono and stereo wavs */
	TArrayNoInit<INT>			ChannelOffsets;
	/** Sizes of the bulk data for the source wav data - always 0 for mono and stereo wavs */
	TArrayNoInit<INT>			ChannelSizes;
	/** Uncompressed wav data 16 bit in mono or stereo - stereo not allowed for multichannel data */
	FByteBulkData				RawData;
	/** Pointer to 16 bit PCM data - used to preview sounds */
	SWORD* RawPCMData;	
	BITFIELD DynamicResource:1;
	/** Cached ogg vorbis data. */
	FByteBulkData				CompressedPCData;
	/** Cached cooked Xbox 360 data to speed up iteration times. */
	FByteBulkData				CompressedXbox360Data;
	/** Cached cooked PS3 data to speed up iteration times. */
	FByteBulkData				CompressedPS3Data;

	INT							ResourceID;

	INT ResourceSize;
	const BYTE* ResourceData;

	/** A localized version of the text that is actually spoken in the audio. */
	FString						SpokenText;

	/**
	 * Subtitle cues.  If empty, use SpokenText as the subtitle.  Will often be empty,
	 * as the contents of the subtitle is commonly identical to what is spoken.
	 */
	TArray<FSubtitleCue>		Subtitles;

	/** Provides contextual information for the sound to the translator. */
	FString						Comment;

	/** TRUE if this sound is considered mature. */
	BITFIELD					bMature:1;

	/** TRUE if the subtitles have been split manually. */
	BITFIELD					bManualWordWrap:1;	

	// UObject interface.
	virtual void Serialize( FArchive& Ar );

	/** 
	* Frees up all the resources allocated in this class
	*/
	void FreeResources( void );

	/**
	 * Frees the sound resource data.
	 */
	virtual void FinishDestroy( void );

	/**
	 * Outside the Editor, uploads resource to audio device and performs general PostLoad work.
	 *
	 * This function is being called after all objects referenced by this object have been serialized.
	 */
	virtual void PostLoad( void );

	/** 
	 * Invalidate compressed data
	 */
	virtual void PostEditChange( UProperty* PropertyThatChanged );

	/** 
	* Copy the compressed audio data from the bulk data
	*/
	void InitAudioResource( FByteBulkData& CompressedData );

	/** 
	* Remove the compressed audio data associated with the passed in wave
	*/
	void RemoveAudioResource( void );

	// USoundNode interface.
	virtual void ParseNodes( USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual INT GetMaxChildNodes( void ) { return 0; }
	virtual FLOAT GetDuration( void );

	/** 
	 * Get the name of the class used to help out when handling events in UnrealEd.
	 * @return	String name of the helper class.
	 */
	virtual const FString GetEdHelperClassName( void ) const
	{
		return FString( TEXT("UnrealEd.SoundNodeWaveHelper") );
	}

	/**
	 * Returns whether this wave file is a localized resource.
	 *
	 * @return TRUE if it is a localized resource, FALSE otherwise.
	 */
	virtual UBOOL IsLocalizedResource( void );

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @return size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	virtual INT GetResourceSize( void );

	/** 
	 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser
	 */
	virtual FString GetDesc( void );

	/** 
	 * Returns detailed info to populate listview columns
	 */
	virtual FString GetDetailedDescription( INT Index );

	/**
	* Used by various commandlets to purge Editor only data from the object.
	* 
	* @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
	*/
	virtual void StripData(UE3::EPlatformType TargetPlatform);
};


// Hash function. Needed to avoid UObject v FResource ambiguity due to multiple inheritance
inline DWORD GetTypeHash( const USoundNodeWave* A )
{
	return A ? A->GetIndex() : 0;
}


/*-----------------------------------------------------------------------------
	FWaveModInfo. 
-----------------------------------------------------------------------------*/

//
// Structure for in-memory interpretation and modification of WAVE sound structures.
//
class FWaveModInfo
{
public:

	// Pointers to variables in the in-memory WAVE file.
	DWORD* pSamplesPerSec;
	DWORD* pAvgBytesPerSec;
	WORD* pBlockAlign;
	WORD* pBitsPerSample;
	WORD* pChannels;

	DWORD  OldBitsPerSample;

	DWORD* pWaveDataSize;
	DWORD* pMasterSize;
	BYTE*  SampleDataStart;
	BYTE*  SampleDataEnd;
	DWORD  SampleDataSize;
	BYTE*  WaveDataEnd;

	INT	   SampleLoopsNum;
	FSampleLoop*  pSampleLoop;

	DWORD  NewDataSize;
	UBOOL  NoiseGate;

	// Constructor.
	FWaveModInfo()
	{
		NoiseGate   = false;
		SampleLoopsNum = 0;
	}
	
	// 16-bit padding.
	DWORD Pad16Bit( DWORD InDW )
	{
		return ((InDW + 1)& ~1);
	}

	// Read headers and load all info pointers in WaveModInfo. 
	// Returns 0 if invalid data encountered.
	UBOOL ReadWaveInfo( BYTE* WaveData, INT WaveDataSize );
};

/*-----------------------------------------------------------------------------
	USoundNode helper macros. 
-----------------------------------------------------------------------------*/

#define DECLARE_SOUNDNODE_ELEMENT(Type,Name)													\
	Type& Name = *((Type*)(Payload));															\
	Payload += sizeof(Type);														

#define DECLARE_SOUNDNODE_ELEMENT_PTR(Type,Name)												\
	Type* Name = (Type*)(Payload);																\
	Payload += sizeof(Type);														

#define	RETRIEVE_SOUNDNODE_PAYLOAD( Size )														\
		BYTE*	Payload					= NULL;													\
		UBOOL*	RequiresInitialization	= NULL;													\
		{																						\
			UINT* TempOffset = AudioComponent->SoundNodeOffsetMap.Find( this );					\
			UINT Offset;																		\
			if( !TempOffset )																	\
			{																					\
				Offset = AudioComponent->SoundNodeData.AddZeroed( Size + sizeof(UBOOL));		\
				AudioComponent->SoundNodeOffsetMap.Set( this, Offset );							\
				RequiresInitialization = (UBOOL*) &AudioComponent->SoundNodeData(Offset);		\
				*RequiresInitialization = 1;													\
				Offset += sizeof(UBOOL);														\
			}																					\
			else																				\
			{																					\
				RequiresInitialization = (UBOOL*) &AudioComponent->SoundNodeData(*TempOffset);	\
				Offset = *TempOffset + sizeof(UBOOL);											\
			}																					\
			Payload = &AudioComponent->SoundNodeData(Offset);									\
		}
