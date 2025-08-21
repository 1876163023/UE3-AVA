/*=============================================================================
	ALAudioDevice.h: Unreal OpenAL audio interface object.
	Copyright ?1999-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_ALAUDIODEVICE
#define _INC_ALAUDIODEVICE

/*------------------------------------------------------------------------------------
	Dependencies, helpers & forward declarations.
------------------------------------------------------------------------------------*/

class UALAudioDevice;

#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,8)
#endif
#define AL_NO_PROTOTYPES 1
#define ALC_NO_PROTOTYPES 1
#include "fmod.hpp"
#include "fmod_errors.h"
#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif

#define MAX_AUDIOCHANNELS 128
#define MIN_AUDIOCHANNELS 16
#if __WIN32__
#define AL_DLL TEXT("OpenAL32.dll")
#else
#define AL_DLL TEXT("libopenal.so")
#endif

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#define MAX_DSP_SLOT 2
#define MAX_AUDIOSTREAMS 1


/*------------------------------------------------------------------------------------
	FALSoundBuffer
------------------------------------------------------------------------------------*/

class FALSoundBuffer
{
public:
	/** 
	 * Constructor
	 *
	 * @param AudioDevice	audio device this sound buffer is going to be attached to.
	 */
	FALSoundBuffer( UALAudioDevice* AudioDevice );
	
	/**
	 * Destructor 
	 * 
	 * Frees wave data and detaches itself from audio device.
 	 */
	~FALSoundBuffer( void );

	/**
	 * Static function used to create a buffer.
	 *
	 * @param	InWave		USoundNodeWave to use as template and wave source
	 * @param	AudioDevice	Audio device to attach created buffer to
	 * @return	FALSoundBuffer pointer if buffer creation succeeded, NULL otherwise
	 */
	static FALSoundBuffer* Init( USoundNodeWave* InWave, UALAudioDevice* AudioDevice );

	/**
	 * Locate and precache if necessary the ogg vorbis data. Decompress and validate the header.
	 *
	 * @param	InWave		USoundNodeWave to use as template and wave source
	 */
	void PrepareDecompression( USoundNodeWave* Wave );

	/**
	 * Decompress the next chunk of sound into CurrentBuffer
	 *
	 * @param	bLoop		Whether to loop the sound or pad with 0s
	 * @return	UBOOL		TRUE if the sound looped, FALSE otherwise
	 */
	UBOOL DecodeCompressed( UBOOL bLoop );

	/**
	 * Returns the size of this buffer in bytes.
	 *
	 * @return Size in bytes
	 */
	INT GetSize( void ) { return( BufferSize ); }

	/** 
	 * Returns the number of channels for this buffer
	 */
	INT GetNumChannels( void ) { return( NumChannels ); }
		
	/** Audio device this buffer is attached to */
	UALAudioDevice*		AudioDevice;
	/** Index for the current buffer id - used for double buffering packets. Set to zero for resident sounds. */
	INT					CurrentBuffer;
	/** Array of buffer ids used to reference the data stored in AL. */	

#define FMOD_SOUND_SOFTWARE		0
#define FMOD_SOUND_HARDWARE_2D	1
#define FMOD_SOUND_HARDWARE_3D	2	
	FMOD::Sound*		Sound[3];	
	/** Resource ID of associated USoundNodeWave */
	INT					ResourceID;
	/** Pointer to decompression state */
	void *				DecompressionState;
	/** Human readable name of resource, most likely name of UObject associated during caching. */
	FString				ResourceName;	
	/** Number of bytes stored in OpenAL, or the size of the ogg vorbis data */
	INT					BufferSize;
	/** The number of channels in this sound buffer - should be directly related to InternalFormat */
	INT					NumChannels;

	float				BaseFrequency;

private:
	/**
	 * Static function used to create a double buffered buffer for dynamic decompression of comrpessed sound
	 *
	 * @param	InWave		USoundNodeWave to use as template and wave source
	 * @param	AudioDevice	Audio device to attach created buffer to
	 * @return	FALSoundBuffer pointer if buffer creation succeeded, NULL otherwise
	 */
	static FALSoundBuffer* CreateQueuedBuffer( USoundNodeWave* Wave, UALAudioDevice* AudioDevice );

	/**
	 * Static function used to create a buffer to a sound that is completely uploaded to OpenAL
	 *
	 * @param	InWave		USoundNodeWave to use as template and wave source
	 * @param	AudioDevice	Audio device to attach created buffer to
	 * @return	FALSoundBuffer pointer if buffer creation succeeded, NULL otherwise
	 */
	static FALSoundBuffer* CreateNativeBuffer( USoundNodeWave* Wave, UALAudioDevice* AudioDevice );

	/**
	* Static function used to create a buffer that can be used for listening to raw PCM data
	*
	* @param	Buffer		Existing buffer that needs new sound data
	* @param	InWave		USoundNodeWave to use as template and wave source
	* @param	AudioDevice	Audio device to attach created buffer to
	* @return	FALSoundBuffer pointer if buffer creation succeeded, NULL otherwise
	*/
	static FALSoundBuffer* CreatePreviewBuffer( FALSoundBuffer* Buffer, USoundNodeWave* Wave, UALAudioDevice* AudioDevice );
};

/**
 * OpenAL implementation of FSoundSource, the interface used to play, stop and update sources
 */
class FALSoundSource : public FSoundSource
{
public:
	/**
	 * Constructor
	 *
	 * @param	InAudioDevice	audio device this source is attached to
	 */
	FALSoundSource( UAudioDevice* InAudioDevice )
	:	FSoundSource( InAudioDevice ),
		Playing( FALSE ),
		Paused( FALSE ),
		Buffer( NULL ),
		bUsingHardware( FALSE )
	{}

	/**
	 * Initializes a source with a given wave instance and prepares it for playback.
	 *
	 * @param	WaveInstance	wave instance being primed for playback
	 * @return	TRUE			if initialization was successful, FALSE otherwise
	 */
	UBOOL Init( FWaveInstance* WaveInstance );

	/**
	 * Updates the source specific parameter like e.g. volume and pitch based on the associated
	 * wave instance.	
	 */
	void Update( void );

	/**
	 * Plays the current wave instance.	
	 */
	void Play( void );

	/**
	 * Stops the current wave instance and detaches it from the source.	
	 */
	void Stop( void );

	/**
	 * Pauses playback of current wave instance.
	 */
	void Pause( void );

	/**
	 * Queries the status of the currently associated wave instance.
	 *
	 * @return	TRUE if the wave instance/ source has finished playback and FALSE if it is 
	 *			currently playing or paused.
	 */
	UBOOL IsFinished( void );

	/**
	 * Handles feeding new data to a real time decompressed sound and waits for the sound to finish if necessary.
	 */
	void HandleRealTimeSource( void );

protected:
	/** Cached status information whether we are playing or not. */
	UBOOL				Playing;
	/** Cached status information whether we are paused or not. */
	UBOOL				Paused;
	/** There are queued buffers that need to play before the sound is finished. */
	UBOOL				BuffersToFlush;	
	UBOOL				bUsingHardware;
	/** Cached sound buffer associated with currently bound wave instance. */
	FALSoundBuffer *	Buffer;
	
	FMOD::Channel*		Channel;
	float				BaseFrequency;

	friend class UALAudioDevice;
};


/*------------------------------------------------------------------------------------
	UALAudioDevice
------------------------------------------------------------------------------------*/

struct DSPBase;

class UALAudioDevice : public UAudioDevice
{
	DECLARE_CLASS(UALAudioDevice,UAudioDevice,CLASS_Config|CLASS_Intrinsic,ALAudio)

	/**
	 * Static constructor, used to associate .ini options with member variables.	
	 */
	void StaticConstructor( void );

	/**
	 * Initializes the audio device and creates sources.
	 *
	 * @return TRUE if initialization was successful, FALSE otherwise
	 */
	UBOOL Init( void );

	/**
	 * Update the audio device and calculates the cached inverse transform later
	 * on used for spatialization.
	 *
	 * @param	Realtime	whether we are paused or not
	 */
	void Update( UBOOL Realtime );

	/**
	 * Precaches the passed in sound node wave object.
	 *
	 * @param	SoundNodeWave	Resource to be precached.
	 */
	void Precache( USoundNodeWave* SoundNodeWave );

	/**
	 * Frees the bulk resource data associated with this SoundNodeWave.
	 *
	 * @param	SoundNodeWave	wave object to free associated bulk data
	 */
	virtual void FreeResource( USoundNodeWave* SoundNodeWave );

	// UObject interface.
	virtual void Serialize( FArchive& Ar );	

	/**
	 * Shuts down audio device. This will never be called with the memory image codepath.
	 */
	virtual void FinishDestroy( void );
	
	/**
	 * Special variant of Destroy that gets called on fatal exit. 
	 */
	void ShutdownAfterError( void );

	/**
	 * Exec handler used to parse console commands.
	 *
	 * @param	Cmd		Command to parse
	 * @param	Ar		Output device to use in case the handler prints anything
	 * @return	TRUE if command was handled, FALSE otherwise
	 */
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );

	/**
	 * Sets a new sound mode and applies it to all appropriate sound groups
	 * 
	 * @param	NewSoundMode	The sound mode index from ESoundMode, with 0 being no sound mode EQ applied.
	 */
	virtual void SetSoundMode( ESoundMode NewSoundMode );


protected:	

	/** Lists all the loaded sounds and their memory footprint */
	void ListSounds( FOutputDevice& Ar );

	void ListWaves( FOutputDevice& Ar );

public :
	virtual void SetReverbSettings(const FReverbSettings& ReverbSettings);

	/** Called by SetReverbSettings when a new set of reverb settings have been pushed onto the reverb settings stack. */
	virtual void OnReverbSettingsChanged( void );

protected :
	/**
	 * Tears down audio device by stopping all sounds, removing all buffers,  destroying all sources, ... Called by both Destroy and ShutdownAfterError
	 * to perform the actual tear down.
	 */
	void Teardown( void );

	// Variables.

	/** The name of the OpenAL Device to open - defaults to "Generic Software" */
	FStringNoInit								DeviceName;
	/** All buffers that are currently loaded */
	TArray<FALSoundBuffer *>					Buffers;
	/** Map from resource ID to sound buffer */
	TDynamicMap<INT, FALSoundBuffer*>			WaveBufferMap;
	DOUBLE										LastHWUpdate;
	/** Next resource ID value used for registering USoundNodeWave objects */
	INT											NextResourceID;

public :
	// AL specific.
	FMOD::System*								System;			
	FMOD::ChannelGroup*							FXChannelGroup;
	FMOD::ChannelGroup*							MasterChannelGroup;

	// Configuration.
	FLOAT										TimeBetweenHWUpdates;
	FLOAT										MinOggVorbisDuration;

	/**
	 * Balance between the previous and current reverb settings.
	 * In [0,1], with 0.0 meaning full previous reverb and 1.0 meaning full current reverb.
	 */
	FLOAT										ReverbBalance;

	/** Time when OnReverbSettingsChanged() was last called. */
	DOUBLE										LastReverbSettingsChangeTime;

	TArray<DSPBase*>							DSPs;
	INT											ActiveDSP[MAX_DSP_SLOT];

	FMOD_REVERB_PROPERTIES						CurrentFMODReverb, PreviousFMODReverb, TargetFMODReverb;
	FLOAT										ReverbChangedTime;

	FMOD::Sound*								FMOD_Stream[MAX_AUDIOSTREAMS];
	FMOD::Channel*								FMOD_Channel[MAX_AUDIOSTREAMS];	

	FReverbSettings								CurrentReverbSettings;

	INT											NumHWChannels, NumHW2DChannels, NumHW3DChannels;
	INT											NumHW2DChannels_Playing, NumHW3DChannels_Playing;

	UBOOL IsHardwareChannelAvailable( UBOOL bSpatialize ) const
	{
		return (
			NumHW2DChannels_Playing + NumHW3DChannels_Playing < NumHWChannels &&
			bSpatialize && NumHW2DChannels_Playing < NumHW2DChannels || !bSpatialize && NumHW3DChannels_Playing < NumHW3DChannels
			);
	}

	void UseHardwareChannel( UBOOL bSpatialize );
	void UnuseHardwareChannel( UBOOL bSpatialize );
	
	void CreateStreams();
	void ReleaseStreams();

	virtual void LockStream( UBOOL bLock );
	virtual INT GetStreamWriteable( INT Stream, UBOOL bLinear );
	virtual signed short* GetStreamWritePointer( INT Stream );
	virtual void CommitStream( INT Stream, INT NumSample );

	virtual void PlayStream( INT Stream );
	virtual void StopStream( INT Stream );

	virtual FLOAT GetStreamTime( INT Stream );

	friend class FALSoundBuffer;
	friend class FALSoundSource;
};

#endif

