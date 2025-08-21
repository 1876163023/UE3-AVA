/**
 * Copyright 1999-2006 Epic Games, Inc. All Rights Reserved.
 */

/*------------------------------------------------------------------------------------
	Audio includes.
------------------------------------------------------------------------------------*/

#include "ALAudioPrivate.h"
#include "UnNet.h"
#include "FMODDSPExtension.h"
#include "DSPInterface.h"
#include "EngineDSPClasses.h"

#undef AUDIO_DISTANCE_FACTOR 
#define AUDIO_DISTANCE_FACTOR ( 1/16.0f * 0.3f )

/*------------------------------------------------------------------------------------
	UALAudioDevice constructor and UObject interface.
------------------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UALAudioDevice);

void DSP_InitAll();
void DSP_FreeAll();

static FMOD_REVERB_PROPERTIES ReverbPresets[] = { 
	FMOD_PRESET_OFF,          
	FMOD_PRESET_GENERIC,          
	FMOD_PRESET_PADDEDCELL,       
	FMOD_PRESET_ROOM,             
	FMOD_PRESET_BATHROOM,         
	FMOD_PRESET_LIVINGROOM,       
	FMOD_PRESET_STONEROOM,        
	FMOD_PRESET_AUDITORIUM,       
	FMOD_PRESET_CONCERTHALL,      
	FMOD_PRESET_CAVE,             
	FMOD_PRESET_ARENA,            		
	FMOD_PRESET_CARPETTEDHALLWAY, 
	FMOD_PRESET_HALLWAY,          
	FMOD_PRESET_STONECORRIDOR,    
	FMOD_PRESET_ALLEY,            
	FMOD_PRESET_FOREST,           
	FMOD_PRESET_CITY,             
	FMOD_PRESET_MOUNTAINS,        
	FMOD_PRESET_QUARRY,           
	FMOD_PRESET_PLAIN,            
	FMOD_PRESET_PARKINGLOT,       
	FMOD_PRESET_SEWERPIPE,        
	FMOD_PRESET_UNDERWATER,   
};

UBOOL operator != ( const FReverbSettings& A, const FReverbSettings& B )
{
	return appMemcmp( &A, &B, sizeof(FReverbSettings) );
}

void UALAudioDevice::SetReverbSettings(const FReverbSettings& ReverbSettings)
{
	if (CurrentReverbSettings != ReverbSettings)
	{
		CurrentReverbSettings = ReverbSettings;

		OnReverbSettingsChanged();
	}	
}

void UALAudioDevice::OnReverbSettingsChanged()
{
	PreviousFMODReverb = CurrentFMODReverb;

	if (CurrentReverbSettings.UserdefinedReverbPreset)
	{
		UReverbProperty* src = CurrentReverbSettings.UserdefinedReverbPreset;

		FMOD_REVERB_PROPERTIES prop = 
		{
			0,
			src->Environment,
			src->EnvSize,
			src->EnvDiffusion,
			src->Room,
			src->RoomHF,
			src->RoomLF,
			src->DecayTime,
			src->DecayHFRatio,
			src->DecayLFRatio,
			src->Reflections,
			src->ReflectionsDelay,
			{ src->ReflectionsPan.X, src->ReflectionsPan.Y, src->ReflectionsPan.Z },
			src->Reverb,
			src->ReverbDelay,
			{ src->ReverbPan.X, src->ReverbPan.Y, src->ReverbPan.Z },
			src->EchoTime,
			src->EchoDepth,
			src->ModulationTime,
			src->ModulationDepth,
			src->AirAbsorptionHF,
			src->HFReference,
			src->LFReference,
			src->RoomRolloffFactor,
			src->Diffusion,
			src->Density,
			(src->bDecayTimeScale ? FMOD_REVERB_FLAGS_DECAYTIMESCALE : 0) |
			(src->bReflectionsScale ? FMOD_REVERB_FLAGS_REFLECTIONSSCALE : 0) |
			(src->bReflectionsDelayScale ? FMOD_REVERB_FLAGS_REFLECTIONSDELAYSCALE : 0) |
			(src->bReverbScale ? FMOD_REVERB_FLAGS_REVERBSCALE : 0) |
			(src->bReverbDelayScale ? FMOD_REVERB_FLAGS_REVERBDELAYSCALE : 0) |
			(src->bDecayHFLimit ? FMOD_REVERB_FLAGS_DECAYHFLIMIT : 0) |
			(src->bEchoTimeScale ? FMOD_REVERB_FLAGS_ECHOTIMESCALE : 0) |
			(src->bModulationTimeScale ? FMOD_REVERB_FLAGS_MODULATIONTIMESCALE : 0)
		};

		TargetFMODReverb = prop;
	}
	else
	{
		checkSlow( REVERB_MAX == sizeof(ReverbPresets) / sizeof(ReverbPresets[0]) );

		TargetFMODReverb = ReverbPresets[CurrentReverbSettings.ReverbType];
	}	

	ReverbChangedTime = appSeconds();
}

//
// UALAudioDevice::StaticConstructor
//
void UALAudioDevice::StaticConstructor()
{
	new(GetClass(),TEXT("TimeBetweenHWUpdates"	), RF_Public) UFloatProperty(CPP_PROPERTY(TimeBetweenHWUpdates	), TEXT("ALAudio"), CPF_Config );
}

FLOAT Audio_GetDuration( const BYTE* Bytes, INT NumBytes )
{
	UALAudioDevice* AudioDevice = Cast<UALAudioDevice>( GEngine->Client->GetAudioDevice() );
	if( AudioDevice )
	{
		FMOD_RESULT result;
		FMOD::Sound* sound;
		FMOD_CREATESOUNDEXINFO exinfo;
		memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
		exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
		exinfo.length = NumBytes;    		
		result = AudioDevice->System->createSound( (const char*)Bytes, FMOD_CREATECOMPRESSEDSAMPLE | FMOD_OPENMEMORY | FMOD_SOFTWARE | FMOD_3D, &exinfo, &sound);
		if (result != FMOD_OK)
		{			
			return 0.0f;
		}		

		unsigned int length = 0;
		sound->getLength( &length, FMOD_TIMEUNIT_MS );

		sound->release();

		return length / 1000.0f;
	}
	else
	{
		return 0.0f;
	}
}

//
//	UALAudioDevice::Teardown
//
void UALAudioDevice::Teardown()
{
	Flush();	

	ReleaseStreams();

	FXChannelGroup->release();
	System->close();

	for( INT i=0; i<DSPs.Num(); ++i)
	{
		DSP_Free( i );
	}

	DSP_FreeAll();	
	
	System->release();
}

//
//	UALAudioDevice::Serialize
//
void UALAudioDevice::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	if( Ar.IsCountingMemory() )
	{
		Ar.CountBytes( Buffers.Num() * sizeof(FALSoundBuffer), Buffers.Num() * sizeof(FALSoundBuffer) );
		Buffers.CountBytes( Ar );
		WaveBufferMap.CountBytes( Ar );
	}
}

//
//	UALAudioDevice::Destroy
//
void UALAudioDevice::FinishDestroy()
{
	if( !HasAnyFlags(RF_ClassDefaultObject) )
	{
		Teardown();
		debugf(NAME_Exit,TEXT("OpenAL Audio Device shut down."));
	}

	Super::FinishDestroy();
}

//
// UALAudioDevice::ShutdownAfterError
//
void UALAudioDevice::ShutdownAfterError()
{
	if( !HasAnyFlags(RF_ClassDefaultObject) )
	{
		Teardown();
		debugf(NAME_Exit,TEXT("UALAudioDevice::ShutdownAfterError"));
	}
	Super::ShutdownAfterError();
}

/*------------------------------------------------------------------------------------
	UAudioDevice Interface.
------------------------------------------------------------------------------------*/

#define ERRCHECK(x) if (result != FMOD_OK) { debugf(NAME_Error,TEXT("FMOD error! (%d)"), (int)result ); if (System) { System->close(); System->release(); } return 0; }

void* F_CALLBACK UE3_FMOD_Malloc(unsigned int size)
{
	return GMalloc->Malloc( size );
}

void* F_CALLBACK UE3_FMOD_Realloc(void *ptr, unsigned int size)
{
	return GMalloc->Realloc( ptr, size );
}

void  F_CALLBACK UE3_FMOD_Free(void *ptr)
{
	GMalloc->Free( ptr );
}

//
// UALAudioDevice::Init
//
UBOOL UALAudioDevice::Init()
{
	ReverbChangedTime = -1.0f;
	CurrentFMODReverb = PreviousFMODReverb = ReverbPresets[0];

	FMOD_RESULT result;
	unsigned int version;

	for (INT i=0; i<sizeof(ActiveDSP)/sizeof(ActiveDSP[0]); ++i)
	{
		ActiveDSP[i] = -1;
	}

	System = NULL;

	result = FMOD::Memory_Initialize( NULL, 0, UE3_FMOD_Malloc, UE3_FMOD_Realloc, UE3_FMOD_Free );
	ERRCHECK(result);
	
	result = FMOD::System_Create(&System);
	ERRCHECK(result);

	result = System->getVersion(&version);
	ERRCHECK(result);

	if (version < FMOD_VERSION)
	{
		printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
		return 0;
	}

	// No channels, no sound.
	if ( MaxChannels <= 8 )
	{
		result = System->close();
		ERRCHECK(result);
		result = System->release();
		ERRCHECK(result);
		return 0;
	}

	FMOD_SPEAKERMODE  speakermode;
	result = System->getDriverCaps(0, 0, 0, 0, &speakermode);
	ERRCHECK(result);

	result = System->setSpeakerMode(speakermode);
	ERRCHECK(result);	

	result = System->setSoftwareChannels( MaxChannels );
	ERRCHECK(result);

	result = System->setSoftwareFormat( 44100, FMOD_SOUND_FORMAT_PCM16, 0, 0, FMOD_DSP_RESAMPLER_LINEAR );
	ERRCHECK(result);

	result = System->init( MaxChannels, FMOD_INIT_NORMAL, 0);
	ERRCHECK(result);

	result = System->getHardwareChannels( &NumHW2DChannels, &NumHW3DChannels, &NumHWChannels );
	ERRCHECK(result);

	NumHW2DChannels_Playing = NumHW3DChannels_Playing = 0;

	debugf( NAME_Log, TEXT("FMOD:Hardware Channels (2D:%d, 3D:%d, Total:%d)"), NumHW2DChannels, NumHW3DChannels, NumHWChannels );

	/*if (NumHW3DChannels < 128)
	{
		debugf( NAME_Log, TEXT("FMOD:Software fallback used due to lack of HW 3D Channels (%d)"), NumHW3DChannels );
		NumHW3DChannels = 0;
		NumHW2DChannels = 0;
		NumHWChannels = 0;
	}*/
	
	/*result = System->set3DSettings( 0, 1, 0 );
	ERRCHECK(result);	*/

	result = System->getMasterChannelGroup( &MasterChannelGroup );
	ERRCHECK(result);

	result = System->createChannelGroup( "FXChannelGroup", &FXChannelGroup );
	ERRCHECK(result);

	result = MasterChannelGroup->addGroup( FXChannelGroup );
	ERRCHECK(result);
	
	// Initialize channels.	
	for( INT i=0; i<Min(MaxChannels, MAX_AUDIOCHANNELS); i++ )
	{		
		FALSoundSource* Source = new FALSoundSource(this);			
		Sources.AddItem( Source );
		FreeSources.AddItem( Source );		
	}

	// Update MaxChannels in case we couldn't create enough sources.
	MaxChannels = Sources.Num();	

	if( !Sources.Num() )
	{
		debugf(NAME_Error,TEXT("ALAudio: couldn't allocate sources"));
		return 0;
	}	
	DSP_InitAll();

	// Initialized.
	LastHWUpdate		= 0;
	NextResourceID		= 1;	

	debugf(NAME_Init,TEXT("ALAudio: Device initialized."));	

	CreateStreams();

	// Initialize base class last as it's going to precache already loaded audio.
	Super::Init();

	return TRUE;
}

//
//	UALAudioDevice::Update
//
void UALAudioDevice::Update( UBOOL Realtime )
{
	Super::Update( Realtime );

#if STATS
	{
		FLOAT dsp, stream, update, total;

		System->getCPUUsage( &dsp, &stream, &update, &total );

		SET_FLOAT_STAT(STAT_AudioCPUUsage_DSP,dsp);
		SET_FLOAT_STAT(STAT_AudioCPUUsage_Stream,stream);
		SET_FLOAT_STAT(STAT_AudioCPUUsage_Update,update);
		SET_FLOAT_STAT(STAT_AudioCPUUsage_Total,total);

		INT CurrentAllocated;
		FMOD::Memory_GetStats( &CurrentAllocated, 0 );

		SET_DWORD_STAT(STAT_AudioMemory, CurrentAllocated);
		SET_DWORD_STAT(STAT_AudioMemorySize, CurrentAllocated);
	}
#endif

	FLOAT CurrentTime = appSeconds();

	for (INT i=0; i<DSPs.Num(); ++i)
	{
		if (DSPs(i))
			DSPs(i)->ProcessFade();			
	}		

	// Set Player position and orientation.
	FVector Orientation[2];

	// See file header for coordinate System explanation.
	Orientation[0].X	= Listeners(0).Front.X;
	Orientation[0].Y	= Listeners(0).Front.Y; // Z/Y swapped on purpose, see file header	
	Orientation[0].Z	= Listeners(0).Front.Z; // Z/Y swapped on purpose, see file header
	
	// See file header for coordinate System explanation.
	Orientation[1].X	= Listeners(0).Up.X;
	Orientation[1].Y	= Listeners(0).Up.Y; // Z/Y swapped on purpose, see file header
	Orientation[1].Z	= Listeners(0).Up.Z; // Z/Y swapped on purpose, see file header

	// Make the listener still and the sounds move relatively -- this allows 
	// us to scale the doppler effect on a per-sound basis.
	FVector Velocity	= FVector(0,0,0),
			Location	= Listeners(0).Location;

	// See file header for coordinate System explanation.
	Location.X			= Listeners(0).Location.X;
	Location.Y			= Listeners(0).Location.Y; // Z/Y swapped on purpose, see file header
	Location.Z			= Listeners(0).Location.Z; // Z/Y swapped on purpose, see file header
	Location		   *= AUDIO_DISTANCE_FACTOR;

	FMOD_RESULT result;
	result = System->set3DListenerAttributes(0, (FMOD_VECTOR*)&Location, (FMOD_VECTOR*)&Velocity, (FMOD_VECTOR*)&Orientation[0], (FMOD_VECTOR*)&Orientation[1] );
	//ERRCHECK(result);	
	
	// Deferred commit (enforce min time between updates).
	if( CurrentTime < LastHWUpdate )
		LastHWUpdate = CurrentTime;
	if( (CurrentTime - LastHWUpdate) >= (TimeBetweenHWUpdates / 1000.f) )
	{
		LastHWUpdate = CurrentTime;
		System->update();
	}	

	/* ReverbChangedTime < 0일 때는 update 완료 상황임 */
	if (ReverbChangedTime > 0.0f)
	{
		FLOAT ElapsedTime = CurrentTime - ReverbChangedTime;

		if (ElapsedTime > CurrentReverbSettings.FadeTime || CurrentReverbSettings.FadeTime <= 0.0f)
		{
			System->setReverbProperties( &TargetFMODReverb );

			/// Do not update!
			ReverbChangedTime = -1.0f;

			CurrentFMODReverb = TargetFMODReverb;
		}
		else
		{
			FLOAT Ratio = Clamp( ElapsedTime / CurrentReverbSettings.FadeTime, 0.0f, 1.0f );
			FLOAT InvRatio = 1.0f - Ratio;

			CurrentFMODReverb.Instance = 0;
			CurrentFMODReverb.Environment = 0; /* Unknown */

#define EXPERP2(x,y,z) x = y!=z ? expf( log( y ) * InvRatio + log( z ) * Ratio ) : y
#define LERP2(x,y,z) x = y!=z ? (y * InvRatio + z * Ratio) : y
#define EXTRACT(x,y) (((FLOAT*)&x)[y])
#define EXPERP1(x,y) EXPERP2( EXTRACT(CurrentFMODReverb.x,y), EXTRACT(PreviousFMODReverb.x,y), EXTRACT(TargetFMODReverb.x, y ) )
#define LERP1(x,y) LERP2( EXTRACT(CurrentFMODReverb.x,y), EXTRACT(PreviousFMODReverb.x,y), EXTRACT(TargetFMODReverb.x, y ) )
#define EXPERP(x) for (INT i=0; i<sizeof(CurrentFMODReverb.x)/sizeof(FLOAT); ++i) { EXPERP1( x, i ); }
#define LERP(x) for (INT i=0; i<sizeof(CurrentFMODReverb.x)/sizeof(FLOAT); ++i) { LERP1( x, i ); }

			EXPERP(EnvSize);
			EXPERP(EnvDiffusion);
			LERP(Room);
			LERP(RoomHF);
			LERP(RoomLF);
			EXPERP(DecayTime);
			EXPERP(DecayHFRatio);
			EXPERP(DecayLFRatio);
			LERP(Reflections);
			EXPERP(ReflectionsDelay);
			LERP(ReflectionsPan);
			LERP(Reverb);
			EXPERP(ReverbDelay);
			LERP(ReverbPan);
			EXPERP(EchoTime);
			LERP(EchoDepth);
			EXPERP(ModulationTime);
			LERP(ModulationDepth);
			LERP(AirAbsorptionHF);
			EXPERP(HFReference);
			EXPERP(LFReference);
			LERP(RoomRolloffFactor);
			LERP(Diffusion);
			LERP(Density);

			CurrentFMODReverb.Flags = PreviousFMODReverb.Flags & TargetFMODReverb.Flags;

			System->setReverbProperties( &CurrentFMODReverb );
		}
	}	
}

/**
 * Precaches the passed in sound node wave object.
 *
 * @param	SoundNodeWave	Resource to be precached.
 */
void UALAudioDevice::Precache( USoundNodeWave* SoundNodeWave )
{
	FALSoundBuffer::Init( SoundNodeWave, this );
}

/**
 * Frees the bulk resource data assocated with this SoundNodeWave.
 *
 * @param	SoundNodeWave	wave object to free associated bulk data
 */
void UALAudioDevice::FreeResource( USoundNodeWave* SoundNodeWave )
{
	// Find buffer.
	FALSoundBuffer* Buffer = NULL;
	if( SoundNodeWave->ResourceID )
	{
		// Find buffer associated with resource id.
		Buffer = WaveBufferMap.FindRef( SoundNodeWave->ResourceID );
		
		// Remove from buffers array.
		Buffers.RemoveItem( Buffer );

		// Delete it. This will automatically remove itself from the WaveBufferMap.
		delete Buffer;
	}

	// .. or reference to compressed data
	SoundNodeWave->RemoveAudioResource();
}

/*------------------------------------------------------------------------------------
	FALSoundSource.
------------------------------------------------------------------------------------*/

void UALAudioDevice::UseHardwareChannel( UBOOL bSpatialize )
{
	check( IsHardwareChannelAvailable( bSpatialize ) );

	if (bSpatialize)
	{
		NumHW3DChannels_Playing++;
	}
	else
	{
		NumHW2DChannels_Playing++;
	}
}

void UALAudioDevice::UnuseHardwareChannel( UBOOL bSpatialize )
{
	if (bSpatialize)
	{
		NumHW3DChannels_Playing--;

		check( NumHW3DChannels_Playing >= 0);
	}
	else
	{
		NumHW2DChannels_Playing--;

		check( NumHW2DChannels_Playing >= 0);
	}
}

//
//	FALSoundSource::Init
//
UBOOL FALSoundSource::Init( FWaveInstance* InWaveInstance )
{
	UALAudioDevice* ALAudioDevice = ((UALAudioDevice*)AudioDevice);
	// Find matching buffer.	
	Buffer = FALSoundBuffer::Init( InWaveInstance->WaveData, ALAudioDevice );
	if( Buffer )
	{
		WaveInstance = InWaveInstance;

		FMOD::Sound* FMODSoundToPlay = Buffer->Sound[ WaveInstance->bUseSpatialization ? FMOD_SOUND_HARDWARE_3D : FMOD_SOUND_HARDWARE_2D ];

		UBOOL bUseHWBuffer = (FMODSoundToPlay != NULL);		

		bUsingHardware = FALSE;

		if (bUseHWBuffer)
		{
			if (ALAudioDevice->IsHardwareChannelAvailable( WaveInstance->bUseSpatialization ))
			{
				ALAudioDevice->UseHardwareChannel( WaveInstance->bUseSpatialization );
				bUsingHardware = TRUE;				
			}			
			else
			{
				bUseHWBuffer = FALSE;
			}
		}

		if (!bUseHWBuffer)
		{
			FMODSoundToPlay = Buffer->Sound[ FMOD_SOUND_SOFTWARE ];
		}

		if (WaveInstance->LoopingMode == LOOP_Forever)
		{
			unsigned int length;

			FMODSoundToPlay->getLength( &length, FMOD_TIMEUNIT_MS );

			InWaveInstance->Length = length / 1000.0f;			
		}
		else
		{
			InWaveInstance->Length = 0.0f;			
		}		

		FMOD_RESULT result;		
		result = Buffer->AudioDevice->System->playSound( FMOD_CHANNEL_FREE, FMODSoundToPlay, true, &Channel );

		if (result != FMOD_OK)
		{
			return FALSE;
		}

		// Looping or long sound should have lower priority :|
		Channel->setPriority( WaveInstance->LoopingMode == LOOP_Forever || InWaveInstance->Length > 3.0f ? 1 : 0 );

		if (!WaveInstance->bNoReverb)
		{
			Channel->setChannelGroup( Buffer->AudioDevice->FXChannelGroup );
		}		

		FMOD_MODE mode;
		
		Channel->getMode( &mode );

		mode &= ~(FMOD_LOOP_NORMAL | FMOD_LOOP_OFF | FMOD_LOOP_BIDI);
		
		if (WaveInstance->LoopingMode == LOOP_Forever)
			mode |= FMOD_LOOP_NORMAL;
		else
			mode |= FMOD_LOOP_OFF;	

		if (!bUseHWBuffer)
		{
			mode &= ~(FMOD_3D | FMOD_2D);

			if (WaveInstance->bUseSpatialization)
			{
				mode |= FMOD_3D;
			}
			else
			{
				mode |= FMOD_2D;
			}
		}
		
		result = Channel->setMode( mode );				

		/*if( WaveInstance->bUseSeamlessLooping && !WaveInstance->bShouldLoopIndefinitelyWithoutNotification )
		{		
			// We queue the sound twice for wave instances that use seamless looping so we can have smooth 
			// loop transitions. The downside is that we might play at most one frame worth of audio from the 
			// beginning of the wave after the wave stops looping.
			alSourceQueueBuffers( SourceId, Buffer->BufferIds.Num(), &Buffer->BufferIds(0) );
		}*/

		Update();

		result = Channel->setPaused(false);

		BaseFrequency  = Buffer->BaseFrequency;

		//result = Channel->getFrequency( &BaseFrequency );

		// Initialization was successful.
		return TRUE;
	}
	else
	{
		// Failed to initialize source.
		return FALSE;
	}
}

//
//	FALSoundSource::Update
//
void FALSoundSource::Update()
{
	if( !WaveInstance || Paused )
	{
		return;
	}	

	FMOD_RESULT result;

	FLOAT	Volume	= Clamp<FLOAT>( WaveInstance->Volume, 0.0f, 1.0f );
	FLOAT	Pitch	= Clamp<FLOAT>( WaveInstance->Pitch , 0.5f, 2.0f );

	// See file header for coordinate System explanation.
	FMOD_VECTOR pos = { WaveInstance->Location.X * AUDIO_DISTANCE_FACTOR, WaveInstance->Location.Y * AUDIO_DISTANCE_FACTOR, WaveInstance->Location.Z * AUDIO_DISTANCE_FACTOR };
	FMOD_VECTOR vel = { WaveInstance->Velocity.X * AUDIO_DISTANCE_FACTOR, WaveInstance->Velocity.Y * AUDIO_DISTANCE_FACTOR, WaveInstance->Velocity.Z * AUDIO_DISTANCE_FACTOR };	

	// We're using a relative coordinate System for un- spatialized sounds.
	if (WaveInstance->bUseSpatialization )
	{
		result = Channel->set3DAttributes(&pos, &vel);				
		result = Channel->set3DMinMaxDistance( WORLD_MAX, WORLD_MAX );
	}	

	result = Channel->setVolume( Volume );
	result = Channel->setFrequency( BaseFrequency * Pitch );	
}

//
//	FALSoundSource::Play
//
void FALSoundSource::Play()
{
	if( WaveInstance )
	{
		Channel->setPaused( false );
		Paused	= 0;
		Playing = 1;
	}
}

//
//	FALSoundSource::Stop
//
void FALSoundSource::Stop()
{
	if( WaveInstance )
	{	
		if (bUsingHardware)
		{
			((UALAudioDevice*)AudioDevice)->UnuseHardwareChannel( WaveInstance->bUseSpatialization );
		}

		Channel->stop();		
		
		Paused		= 0;
		Playing		= 0;
		Buffer		= NULL;
	}

	FSoundSource::Stop();
}

//
//	UALSoundSource::Pause
//
void FALSoundSource::Pause()
{
	if( WaveInstance )
	{
		Channel->setPaused( true );
		Paused = 1;
	}
}

//
//	FALSoundSource::IsFinished
//
UBOOL FALSoundSource::IsFinished()
{
	if( WaveInstance )
	{		
		bool isplaying = false;
		Channel->isPlaying( &isplaying );

		if(!isplaying)
		{
			// Notify the wave instance that it has finished playing.
			WaveInstance->NotifyFinished();
			return TRUE;
		}
		/*else if( WaveInstance->bUseSeamlessLooping && HasProcessedBuffers(SourceId, Buffer->BufferIds.Num()) )
		{
			// Notify the wave instance that the current buffer has finished playing.
			WaveInstance->NotifyFinished();

			// OpenAL requires a non NULL buffer argument so we just allocate a bit of throwaway data on the stack.
			ALuint* Buffers = (ALuint*) appAlloca( sizeof(ALuint) * Buffer->BufferIds.Num() );

			// Unqueue one full set worth of buffers.
			alSourceUnqueueBuffers( SourceId, Buffer->BufferIds.Num(), Buffers );

			// Queue another set of buffers to (potentially) loop it once more.
			alSourceQueueBuffers( SourceId, Buffer->BufferIds.Num(), &Buffer->BufferIds(0) );
		}*/

		return FALSE;
	}
	return TRUE;
}

/*------------------------------------------------------------------------------------
	FALSoundBuffer.
------------------------------------------------------------------------------------*/

//
//	FALSoundBuffer::FALSoundBuffer
//
FALSoundBuffer::FALSoundBuffer( UALAudioDevice* InAudioDevice )
{
	appMemzero( Sound, sizeof(Sound) );

	AudioDevice	= InAudioDevice;
}

//
//	FALSoundBuffer::~FALSoundBuffer
//
FALSoundBuffer::~FALSoundBuffer()
{
#if STATS
	INT CurrentAllocated;
	FMOD::Memory_GetStats( &CurrentAllocated, 0 );

	SET_DWORD_STAT(STAT_AudioMemory, CurrentAllocated);
	SET_DWORD_STAT(STAT_AudioMemorySize, CurrentAllocated);
#endif

	if( ResourceID )
	{
		AudioDevice->WaveBufferMap.Remove( ResourceID );

		// Delete AL buffers.
		if (Sound[FMOD_SOUND_SOFTWARE])
			Sound[FMOD_SOUND_SOFTWARE]->release();

		if (Sound[FMOD_SOUND_HARDWARE_2D])
			Sound[FMOD_SOUND_HARDWARE_2D]->release();

		if (Sound[FMOD_SOUND_HARDWARE_3D])
			Sound[FMOD_SOUND_HARDWARE_3D]->release();
	}
}

//
//	FALSoundBuffer::Init
//
FALSoundBuffer* FALSoundBuffer::Init( USoundNodeWave* Wave, UALAudioDevice* AudioDevice )
{
	// Can't create a buffer without any source data
	if( Wave == NULL || Wave->NumChannels == 0 )
	{
		return NULL;
	}

	FALSoundBuffer* Buffer = NULL;

	if( /*Wave->UseTTS || */Wave->RawPCMData )
	{
		// Find the existing buffer if any
		if( Wave->ResourceID )
		{
			Buffer = AudioDevice->WaveBufferMap.FindRef( Wave->ResourceID );
		}

		// Override with any new PCM data even if some already exists
		if( Wave->RawPCMData )
		{
			// Upload the preview PCM data to it
			Buffer = CreatePreviewBuffer( Buffer, Wave, AudioDevice );
		}
	}
	else
	{
		if( Wave->ResourceID )
		{
			Buffer = AudioDevice->WaveBufferMap.FindRef( Wave->ResourceID );
		}

		if( Buffer == NULL )
		{
			Buffer = CreateNativeBuffer( Wave, AudioDevice );
		}
	}

	return Buffer;
}

/**
* Static function used to create an OpenAL buffer and upload raw PCM data to.
*
* @param InWave		USoundNodeWave to use as template and wave source
* @param AudioDevice	audio device to attach created buffer to
* @return FALSoundBuffer pointer if buffer creation succeeded, NULL otherwise
*/
FALSoundBuffer* FALSoundBuffer::CreatePreviewBuffer( FALSoundBuffer* Buffer, USoundNodeWave* Wave, UALAudioDevice* AudioDevice )
{	
	FMOD_RESULT result;
	FMOD::Sound* sound;
	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO)); 

	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.length = Wave->SampleDataSize;    
	exinfo.format = FMOD_SOUND_FORMAT_PCM16;
	exinfo.defaultfrequency = Wave->SampleRate;
	exinfo.numchannels = Wave->NumChannels;
	if (exinfo.length == 0)
	{
		debugf( NAME_Warning, TEXT("%s ; not cooked"), *Wave->GetPathName() );			
		return Buffer;
	}
	void* pRawData = Wave->RawPCMData;
	result = AudioDevice->System->createSound( (const char*)pRawData, FMOD_CREATECOMPRESSEDSAMPLE | FMOD_OPENMEMORY | FMOD_SOFTWARE | FMOD_3D | FMOD_OPENRAW, &exinfo, &sound);						

	// Free up the data if necessary
	if( Wave->DynamicResource )
	{
		appFree( Wave->RawPCMData );
		Wave->RawPCMData = NULL;
		Wave->DynamicResource = FALSE;
	}

	if (result != FMOD_OK)
	{
		debugf( NAME_Warning, TEXT("%s ; FMOD error(%d) %s"), *Wave->GetPathName(), (INT)result, *FString( FMOD_ErrorString(result) ) );			
		return Buffer;
	}		

	if (Buffer != NULL && Buffer->Sound[FMOD_SOUND_SOFTWARE] != NULL)
	{
		Buffer->Sound[FMOD_SOUND_SOFTWARE]->release();
	}

	if (Buffer == NULL)
	{	
		// Create new buffer.
		Buffer = new FALSoundBuffer( AudioDevice );		

		// Allocate new resource ID and assign to USoundNodeWave. A value of 0 (default) means not yet registered.
		INT ResourceID		= AudioDevice->NextResourceID++;
		Buffer->ResourceID	= ResourceID;
		Wave->ResourceID	= ResourceID;		

		AudioDevice->Buffers.AddItem( Buffer );
		AudioDevice->WaveBufferMap.Set( ResourceID, Buffer );
	}	

	Buffer->Sound[FMOD_SOUND_SOFTWARE] = sound;
	Buffer->BufferSize = Wave->SampleDataSize;
	Buffer->BaseFrequency = Wave->SampleRate;
	Buffer->ResourceName = Wave->GetFullName();

	return( Buffer );
}

FALSoundBuffer* FALSoundBuffer::CreateNativeBuffer( USoundNodeWave* Wave, UALAudioDevice* AudioDevice )
{
	FALSoundBuffer*		Buffer = NULL;

	SCOPE_CYCLE_COUNTER(STAT_AudioResourceCreationTime);

	// Grab the data from the bulk data
	Wave->InitAudioResource( Wave->CompressedPCData );

	FMOD_RESULT result;
	FMOD::Sound* sound[3] = { NULL, NULL, NULL };
	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.length = Wave->CompressedPCData.GetBulkDataSize();    
	if (exinfo.length == 0)
	{
		debugf( NAME_Warning, TEXT("%s ; not cooked"), *Wave->GetPathName() );			
		return NULL;
	}
	void* pRawData = Wave->CompressedPCData.Lock(LOCK_READ_ONLY);	

	FMOD_MODE OpenMode = FMOD_OPENMEMORY;
	
	if (Wave->bCompressedSample)
		OpenMode |= FMOD_CREATECOMPRESSEDSAMPLE | FMOD_SOFTWARE;	

	// Software만 사용해야 하는 경우	
	result = AudioDevice->System->createSound( (const char*)pRawData, OpenMode | FMOD_SOFTWARE | FMOD_3D | FMOD_CREATECOMPRESSEDSAMPLE, &exinfo, &sound[FMOD_SOUND_SOFTWARE] );
	
	if (!(OpenMode & FMOD_SOFTWARE) && AudioDevice->NumHWChannels > 0)
	{			
		OpenMode |= FMOD_HARDWARE;	

		if (AudioDevice->NumHW2DChannels >= 128)
		{
			result = AudioDevice->System->createSound( (const char*)pRawData, OpenMode | FMOD_2D, &exinfo, &sound[FMOD_SOUND_HARDWARE_2D] );
		}
		
		if (AudioDevice->NumHW3DChannels >= 32)
		{
			result = AudioDevice->System->createSound( (const char*)pRawData, OpenMode | FMOD_3D, &exinfo, &sound[FMOD_SOUND_HARDWARE_3D] );
		}
	}

	// Unload the data.
	Wave->CompressedPCData.Unlock();

	if (result != FMOD_OK)
	{
		debugf( NAME_Warning, TEXT("%s ; FMOD error(%d) %s"), *Wave->GetPathName(), (INT)result, *FString( FMOD_ErrorString(result) ) );			
		return NULL;
	}		

	// Create new buffer.
	Buffer = new FALSoundBuffer( AudioDevice );		

	// Allocate new resource ID and assign to USoundNodeWave. A value of 0 (default) means not yet registered.
	INT ResourceID		= AudioDevice->NextResourceID++;
	Buffer->ResourceID	= ResourceID;
	Wave->ResourceID	= ResourceID;

	Buffer->Sound[FMOD_SOUND_SOFTWARE] = sound[FMOD_SOUND_SOFTWARE];
	Buffer->Sound[FMOD_SOUND_HARDWARE_2D] = sound[FMOD_SOUND_HARDWARE_2D];
	Buffer->Sound[FMOD_SOUND_HARDWARE_3D] = sound[FMOD_SOUND_HARDWARE_3D];
	Buffer->BufferSize = Wave->RawData.GetBulkDataSize();

	Buffer->BaseFrequency = Wave->SampleRate;
	Buffer->ResourceName = Wave->GetFullName();

	AudioDevice->Buffers.AddItem( Buffer );
	AudioDevice->WaveBufferMap.Set( ResourceID, Buffer );

	Wave->RemoveAudioResource();

#if STATS
	INT CurrentAllocated;
	FMOD::Memory_GetStats( &CurrentAllocated, 0 );

	SET_DWORD_STAT(STAT_AudioMemory, CurrentAllocated);
	SET_DWORD_STAT(STAT_AudioMemorySize, CurrentAllocated);
#endif	

	return Buffer;
}

#define RINGSIZE (44100 * 2 * 2) 
#define FENCE_CHECK 0xfefa08cd

static FCriticalSection*							GStreamCriticalSection;

struct FAudioRingBuffer
{
	FAudioRingBuffer()
		: Fence(FENCE_CHECK)
	{
		Reset();
	}

	void Reset()
	{
		ReadCursor = 0;
		WriteCursor = 0;
		DataRemains = 0;
	}

	INT GetNumWriteable( UBOOL bLinear )
	{
		check( Fence == FENCE_CHECK );

		if (bLinear)
		{
			if (ReadCursor <= WriteCursor)
				return Min( RINGSIZE - DataRemains, RINGSIZE - WriteCursor );
			else
				return Min( RINGSIZE - DataRemains, ReadCursor - WriteCursor );
		}
		else
		{
			return RINGSIZE - DataRemains;
		}		
	}

	BYTE* Write( INT NumSamples )
	{
		BYTE* Result = Data + WriteCursor;		

//		debugf( TEXT("PRE  Write %d bytes WC:%d R:%d"), NumSamples, WriteCursor, DataRemains );
		WriteCursor += NumSamples;
		WriteCursor %= RINGSIZE;

		DataRemains += NumSamples;
//		debugf( TEXT("POST Write %d bytes WC:%d R:%d"), NumSamples, WriteCursor, DataRemains );		

		check( Fence == FENCE_CHECK );

		//debugf( TEXT("%d Samples written for stream, %d samples remains"), NumSamples, DataRemains );

		return Result;
	}	

	BYTE* GetStreamWritePointer() 
	{
		return Data + WriteCursor;
	}

	void Read( BYTE* Out, INT NumSamples )
	{		
		INT Bytes1, Bytes2;
		BYTE* Buffer1;
		BYTE* Buffer2;

		{
			FScopeLock ScopedLock(GStreamCriticalSection);

			INT NumAvailableBytes = DataRemains;

			if (WriteCursor <= ReadCursor && NumAvailableBytes > 0)
			{
				Bytes1 = RINGSIZE - ReadCursor;
				Bytes2 = WriteCursor;
				Buffer1 = Data + ReadCursor;
				Buffer2 = Data;
			}
			else
			{
				Bytes1 = WriteCursor - ReadCursor;
				Bytes2 = 0;
				Buffer1 = Data + ReadCursor;
				Buffer2 = NULL;
			}

			check( Bytes1 + Bytes2 == NumAvailableBytes );

			if (Bytes1 > NumSamples)
			{
				Bytes1 = NumSamples;
				Bytes2 = 0;
			}
			else if (Bytes1 + Bytes2 > NumSamples)
			{
				Bytes2 = NumSamples - Bytes1;
			}

			ReadCursor = (ReadCursor + Bytes1 + Bytes2 + RINGSIZE) % RINGSIZE;
			DataRemains -= Bytes1 + Bytes2;
		}
		
		appMemcpy( Out, Buffer1, Bytes1 );
		
		if (Bytes2 > 0)
			appMemcpy( Out + Bytes1, Buffer2, Bytes2 );
		

//		debugf( TEXT("POST Read %d bytes RC:%d R:%d"), NumSamples, ReadCursor, DataRemains );		
	}	

	void Lock( INT* Bytes1, void** Buffer1, INT* Bytes2, void** Buffer2 )
	{
		INT NumAvailableBytes = RINGSIZE - DataRemains;

		// Begin, Read, Write, End --> [Write-End], [Begin-Read]
		// Begin, Write, Read, End --> [Write-Read], NULL

		if (ReadCursor <= WriteCursor && NumAvailableBytes > 0)
		{
			*Bytes1 = RINGSIZE - WriteCursor;
			*Bytes2 = ReadCursor;
			*Buffer1 = Data + WriteCursor;
			*Buffer2 = Data;
		}
		else
		{
			*Bytes1 = ReadCursor - WriteCursor;
			*Bytes2 = 0;
			*Buffer1 = Data + WriteCursor;
			*Buffer2 = NULL;
		}
	
		check( *Bytes1 + *Bytes2 == NumAvailableBytes );
	}

	void Unlock( INT Bytes1, void* Buffer1, INT Bytes2, void* Buffer2 )
	{
		Write( Bytes1 + Bytes2 );
	}

	BYTE										Data[RINGSIZE];
	INT											Fence;
	INT											ReadCursor, WriteCursor;
	INT 										DataRemains;
};

static FAudioRingBuffer GAudioStreamBuffer[MAX_AUDIOSTREAMS];

void AudioDevice_LockStream( INT* Bytes1, void** Buffer1, INT* Bytes2, void** Buffer2 )
{
	UALAudioDevice* AudioDevice = Cast<UALAudioDevice>( GEngine->Client->GetAudioDevice() );
	if (AudioDevice == NULL)
	{
		*Bytes1 = *Bytes2 = 0;
		*Buffer1 = *Buffer2 = NULL;
		return;
	}

	GAudioStreamBuffer[0].Lock( Bytes1, Buffer1, Bytes2, Buffer2 );
}

void GenerateSine( INT Bytes, void* Buffer )
{
	static FLOAT Phase = 0;

	const INT NumChannels = 2;
	INT Samples = Bytes / sizeof(SHORT) / NumChannels;
	SHORT* SampleBuffer = (SHORT*)Buffer;

	FLOAT Hz = 440.0f;

	while (Samples--)
	{
		Phase += Hz * (1/44100.0f);

		SHORT Val = (SHORT)(32767.0f * sinf( Phase ));

		for (INT i=0; i<NumChannels; ++i)
		{
			*SampleBuffer++ = Val;
		}		
	}
}

void AudioDevice_UnlockStream( INT Bytes1, void* Buffer1, INT Bytes2, void* Buffer2 )
{
	UALAudioDevice* AudioDevice = Cast<UALAudioDevice>( GEngine->Client->GetAudioDevice() );
	if (AudioDevice == NULL)
	{
		return;
	}

	/*if (Bytes1 > 0)
		GenerateSine( Bytes1, Buffer1 );

	if (Bytes2 > 0)
		GenerateSine( Bytes2, Buffer2 );*/

	GAudioStreamBuffer[0].Unlock( Bytes1, Buffer1, Bytes2, Buffer2 );
}

FMOD_RESULT F_CALLBACK pcmreadcallback(FMOD_SOUND *sound, void *data, unsigned int datalen)
{
	FAudioRingBuffer* Buffer;
	FMOD_Sound_GetUserData( sound, (void**)&Buffer );

	Buffer->Read( (BYTE*)data, datalen );		

	return FMOD_OK;
}


FMOD_RESULT F_CALLBACK pcmsetposcallback(FMOD_SOUND *sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
	/*
	This is useful if the user calls FMOD_Sound_SetPosition and you want to seek your data accordingly.
	*/
	return FMOD_OK;
}

void UALAudioDevice::CreateStreams()
{
	GStreamCriticalSection = GSynchronizeFactory->CreateCriticalSection();	

	for (INT i=0; i<MAX_AUDIOSTREAMS; ++i)
	{		
		FMOD_Channel[i] = 0;
		FMOD_Stream[i] = 0;		
	}
}

void UALAudioDevice::ReleaseStreams()
{
	for (INT i=0; i<MAX_AUDIOSTREAMS; ++i)
	{
		if (FMOD_Stream[i])
		{
			if (FMOD_Channel[i])
			{				
				FMOD_Channel[i] = 0;
			}

			FMOD_Stream[i]->release();		
			FMOD_Stream[i] = 0;
		}	
	}

	if(GStreamCriticalSection)
	{
		GSynchronizeFactory->Destroy(GStreamCriticalSection);
		GStreamCriticalSection = NULL;
	}
}

static UBOOL IsValidStream( INT Stream )
{
	return Stream >= 0 && Stream < MAX_AUDIOSTREAMS;
}

void UALAudioDevice::LockStream( UBOOL bLock ) 
{
	if (bLock)
	{
		GStreamCriticalSection->Lock();
	}
	else
	{
		GStreamCriticalSection->Unlock();
	}
}

INT UALAudioDevice::GetStreamWriteable( INT Stream, UBOOL bLinear )
{
	if (!IsValidStream(Stream)) return 0;	

	return GAudioStreamBuffer[Stream].GetNumWriteable( bLinear ) / 2;
}

signed short* UALAudioDevice::GetStreamWritePointer( INT Stream )
{
	if (!IsValidStream(Stream)) return NULL;	

	return (signed short*)(GAudioStreamBuffer[Stream].GetStreamWritePointer());
}

void UALAudioDevice::CommitStream( INT Stream, INT NumSample )
{
	if (!IsValidStream(Stream)) return;

	GAudioStreamBuffer[Stream].Write( NumSample * 2 );	
}

void UALAudioDevice::PlayStream( INT Stream ) 
{
	if (!IsValidStream(Stream)) return;

	StopStream( Stream );

	INT Rate = 44100;

	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.numchannels      = 2;
	exinfo.format           = FMOD_SOUND_FORMAT_PCM16;
	exinfo.defaultfrequency = Rate;
	exinfo.decodebuffersize  = 4096;                                       /* Chunk size of stream update in samples.  This will be the amount of data passed to the user callback. */
	exinfo.length            = 4096 * 2;									/* Length of PCM data in bytes of whole song (for Sound::getLength) */
	exinfo.pcmreadcallback   = pcmreadcallback;                             /* User callback for reading. */
	exinfo.pcmsetposcallback = pcmsetposcallback;                           /* User callback for seeking. */	

	exinfo.userdata = &GAudioStreamBuffer[Stream];
	FMOD_Channel[Stream] = 0;
	System->createSound( 0, FMOD_2D | FMOD_OPENUSER | FMOD_LOOP_NORMAL | FMOD_SOFTWARE | FMOD_CREATESTREAM, &exinfo, &FMOD_Stream[Stream] );		

	if (!FMOD_Channel[Stream]) 
	{		
		System->playSound( FMOD_CHANNEL_FREE, FMOD_Stream[Stream], false, &FMOD_Channel[Stream] );	
	}
}

void UALAudioDevice::StopStream( INT Stream ) 
{
	if (!IsValidStream(Stream)) return;

	if (FMOD_Stream[Stream])
	{
		if (FMOD_Channel[Stream]) 
		{
			FMOD_Channel[Stream]->stop();
			FMOD_Channel[Stream] = 0;
		}	

		FMOD_Stream[Stream]->release();
		FMOD_Stream[Stream] = 0;
	}	

	LockStream( TRUE );

	GAudioStreamBuffer[Stream].Reset();

	LockStream( FALSE );
}

static FLOAT LastTime = -1;

FLOAT UALAudioDevice::GetStreamTime( INT Stream ) 
{ 
	if (!IsValidStream(Stream)) return 0.0f;

	if (!FMOD_Channel[Stream]) return 0.0f;	

	unsigned int position = 0;
	
	FMOD_Channel[Stream]->getPosition( &position, FMOD_TIMEUNIT_MS );

	FLOAT Time = position / 1000.0f;

	/// Wrapped!
	if (LastTime > Time)
	{
		Time += 5.0f;
	}

	LastTime = Time;	

	Time = Max( 0.0f, Time );

	return Time;
}

/** 
* Displays debug information about the loaded sounds
*/
void UALAudioDevice::ListSounds( FOutputDevice& Ar )
{
	INT	TotalResident, TotalRealTime;
	INT	ResidentCount, RealTimeCount;

	Ar.Logf( TEXT( "Uploaded resident sound resources:" ) );
	TotalResident = 0;
	ResidentCount = 0;
	for( INT BufferIndex = 0; BufferIndex < Buffers.Num(); BufferIndex++ )
	{
		FALSoundBuffer* Buffer = Buffers( BufferIndex );
		if( Buffer->DecompressionState == NULL )
		{
			Ar.Logf( TEXT( "%6i Kb for %d channel(s) in sound %s" ), Buffer->GetSize() / 1024, Buffer->GetNumChannels(), *Buffer->ResourceName );
			TotalResident += Buffer->GetSize() / 1024;
			ResidentCount++;
		}
	}

	Ar.Logf( TEXT( "Real time decompressed sound resources:" ) );
	TotalRealTime = 0;
	RealTimeCount = 0;
	for( INT BufferIndex = 0; BufferIndex < Buffers.Num(); BufferIndex++ )
	{
		FALSoundBuffer* Buffer = Buffers( BufferIndex );
		if( Buffer->DecompressionState != NULL )
		{
			Ar.Logf( TEXT( "%6i Kb for %d channel(s) in sound %s" ), Buffer->GetSize() / 1024, Buffer->GetNumChannels(), *Buffer->ResourceName );
			TotalRealTime += Buffer->GetSize() / 1024;
			RealTimeCount++;
		}
	}

	Ar.Logf( TEXT( "%8i Kb for %d resident sounds" ), TotalResident, ResidentCount );
	Ar.Logf( TEXT( "%8i Kb for %d real time decompressed sounds" ), TotalRealTime, RealTimeCount );
}

/** List the WaveInstances and whether they have a source */
void UALAudioDevice::ListWaves( FOutputDevice& Ar )
{
	TArray<FWaveInstance*> WaveInstances;
	INT FirstActiveIndex = GetSortedActiveWaveInstances( WaveInstances, false );

	for( INT InstanceIndex = FirstActiveIndex; InstanceIndex < WaveInstances.Num(); InstanceIndex++ )
	{
		FWaveInstance* WaveInstance = WaveInstances( InstanceIndex );
		FSoundSource* Source = WaveInstanceSourceMap.FindRef( WaveInstance );
		Ar.Logf( TEXT( "%4i.    %s %6.2f  %s"), InstanceIndex, Source ? TEXT( "Yes" ) : TEXT( " No" ), WaveInstance->Volume, *WaveInstance->WaveData->GetPathName() );
	}
}


/**
* Exec handler used to parse console commands.
*
* @param	Cmd		Command to parse
* @param	Ar		Output device to use in case the handler prints anything
* @return	TRUE if command was handled, FALSE otherwise
*/
UBOOL UALAudioDevice::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( ParseCommand( &Cmd, TEXT( "LISTSOUNDS" ) ) )
	{
		ListSounds( Ar );
		return TRUE;
	}
	else if( ParseCommand( &Cmd, TEXT( "ListWaves" ) ) )
	{
		ListWaves( Ar );
		return( TRUE );
	}
#if 0
	else if( ParseCommand(&Cmd,TEXT("SETSOUNDMODE")) )
	{
		// Takes a number representing an ESoundMode and applies the appropriate mode
		ESoundMode SoundMode = (ESoundMode) appAtoi( Cmd );
		check( (SoundMode >= 0) && (SoundMode < SOUNDMODE_COUNT) );
		SetSoundMode( SoundMode );
	}
	else if( ParseCommand(&Cmd,TEXT("SETRANGEDSOUNDMODE")) )
	{
		Ar.Logf(TEXT("Set sound mode: Dying (ranged)"));

		INT RangedSoundMode = SOUNDMODE_NORMAL;
		FLOAT RangeLow = 0.0f;
		FLOAT RangeHigh = 1.0f;
		FLOAT CurrentHealth = 0.0f;
		INT result = swscanf( Cmd, TEXT("%d %f %f %f"), &RangedSoundMode, &RangeLow, &RangeHigh, &CurrentHealth );
		// Make sure the input string parses correctly
		check (result == 4);
		//SubmixHelper.SetRangedSoundMode( ESoundMode(RangedSoundMode), RangeLow, RangeHigh, CurrentHealth );
		SetSoundMode( ESoundMode(RangedSoundMode) );
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SOUNDMODENORMAL")) )
	{
		Ar.Logf(TEXT("Set sound mode: Normal"));
		SetSoundMode(SOUNDMODE_NORMAL);
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SOUNDMODEDYINGHI")) )
	{
		Ar.Logf(TEXT("Set sound mode: Dying (high health)"));
		SetSoundMode(SOUNDMODE_DYING);
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SOUNDMODEDYINGLO")) )
	{
		Ar.Logf(TEXT("Set sound mode: Dying (low health)"));
		SetSoundMode(SOUNDMODE_DYING);
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SOUNDMODEDEATH")) )
	{
		Ar.Logf(TEXT("Set sound mode: Death"));
		SetSoundMode(SOUNDMODE_DEATH);
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SOUNDMODEGRENADE")) )
	{
		Ar.Logf(TEXT("Set sound mode: Grenade"));
		SetSoundMode(SOUNDMODE_GRENADE);
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SOUNDMODECOVER")) )
	{
		Ar.Logf(TEXT("Set sound mode: Cover"));
		SetSoundMode(SOUNDMODE_COVER);
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SOUNDMODEROADIERUN")) )
	{
		Ar.Logf(TEXT("Set sound mode: Roadie run"));
		SetSoundMode(SOUNDMODE_ROADIE_RUN);
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SOUNDMODETACCOM")) )
	{
		Ar.Logf(TEXT("Set sound mode: TacCom"));
		SetSoundMode(SOUNDMODE_TACCOM);
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("RESETVOLUMES")) )
	{
		Ar.Logf(TEXT("All volumes reset to their defaults"));
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TOGGLEDRYAUDIO")) )
	{
		//if (DryVolume == 0.0f)
		//{
		//	Ar.Logf(TEXT("Dry audio on"));
		//	DryVolume = 1.0f;
		//}
		//else
		//{
		//	Ar.Logf(TEXT("Dry audio off"));
		//	DryVolume = 0.0f;
		//}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("ISOLATEDRYAUDIO")) )
	{
		{
			Ar.Logf(TEXT("Dry audio isolated"));
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TOGGLEREVERB")) )
	{
		//if (ReverbVolume == 0.0f)
		//{
		//	Ar.Logf(TEXT("Reverb audio on"));
		//	ReverbVolume = 1.0f;
		//}
		//else
		//{
		//	Ar.Logf(TEXT("Reverb audio off"));
		//	ReverbVolume = 0.0f;
		//}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("ISOLATEREVERB")) )
	{
		//{
		//	Ar.Logf(TEXT("Reverb audio isolated"));
		//}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TOGGLERADIO")) )
	{
		//if (RadioVolume == 0.0f)
		//{
		//	Ar.Logf(TEXT("Radio voice on"));
		//	RadioVolume = 1.0f;
		//}
		//else
		//{
		//	Ar.Logf(TEXT("Radio voice off"));
		//	RadioVolume = 0.0f;
		//}
	}
	else if( ParseCommand(&Cmd,TEXT("ISOLATERADIO")) )
	{
		//{
		//	Ar.Logf(TEXT("Radio voice audio isolated"));
		//	DryVolume = 0.0f;
		//	ReverbVolume = 0.0f;
		//	RadioVolume = 1.0f;
		//	CenterVolume = 0.0f;
		//}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TOGGLECENTERVOICE")) )
	{
		//if (CenterVolume == 0.0f)
		//{
		//	Ar.Logf(TEXT("Center channel voice on"));
		//	CenterVolume = 1.0f;
		//}
		//else
		//{
		//	Ar.Logf(TEXT("Center channel voice off"));
		//	CenterVolume = 0.0f;
		//}
	}
	else if( ParseCommand(&Cmd,TEXT("ISOLATECENTERVOICE")) )
	{
		//{
		//	Ar.Logf(TEXT("Center channel voice audio isolated"));
		//	DryVolume = 0.0f;
		//	ReverbVolume = 0.0f;
		//	RadioVolume = 0.0f;
		//	CenterVolume = 1.0f;
		//}
		return TRUE;
	}
#endif
	return FALSE;
}

/**
* Sets a new sound mode and applies it to all appropriate sound groups
* 
* @param	NewSoundMode	The sound mode index from ESoundMode, with 0 being no sound mode EQ applied.
*/
void UALAudioDevice::SetSoundMode(ESoundMode NewSoundMode)
{
}