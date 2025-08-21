/*=============================================================================
	UnAudio.cpp: Unreal base audio.
	Copyright 1997-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h" 
#include "EngineSoundClasses.h"

/** 
 * Default settings for a null reverb effect
 */
FAudioReverbEffect::FAudioReverbEffect( void )
{
	Time = 0.0f;
	Volume = 0.0f;

	Density = 1.0f;					
	Diffusion = 1.0f;				
	Gain = 0.32f;					
	GainHF = 0.89f;					
	DecayTime = 1.49f;				
	DecayHFRatio = 0.83f;			
	ReflectionsGain = 0.05f;		
	ReflectionsDelay = 0.007f;		
	LateGain = 1.26f;				
	LateDelay = 0.011f;				
	AirAbsorptionGainHF = 0.994f;	
	RoomRolloffFactor = 0.0f;		
	DecayHFLimit = TRUE;			
}

/** 
 * Construct generic reverb settings based in the I3DL2 standards
 */
FAudioReverbEffect::FAudioReverbEffect( FLOAT InRoom, 
								 		FLOAT InRoomHF, 
								 		FLOAT InRoomRolloffFactor,	
								 		FLOAT InDecayTime,			
								 		FLOAT InDecayHFRatio,		
								 		FLOAT InReflections,		
								 		FLOAT InReflectionsDelay,	
								 		FLOAT InReverb,				
								 		FLOAT InReverbDelay,		
								 		FLOAT InDiffusion,			
								 		FLOAT InDensity,			
								 		FLOAT InAirAbsorption )
{
	Time = 0.0f;
	Volume = 0.0f;

	Density = InDensity;
	Diffusion = InDiffusion;
	Gain = InRoom;
	GainHF = InRoomHF;
	DecayTime = InDecayTime;
	DecayHFRatio = InDecayHFRatio;
	ReflectionsGain = InReflections;
	ReflectionsDelay = InReflectionsDelay;
	LateGain = InReverb;
	LateDelay = InReverbDelay;
	RoomRolloffFactor = InRoomRolloffFactor;
	AirAbsorptionGainHF = InAirAbsorption;
	DecayHFLimit = TRUE;
}

/** 
 * Get interpolated reverb parameters
 */
void FAudioReverbEffect::Interpolate( FLOAT InterpValue, FAudioReverbEffect & Start, FAudioReverbEffect & End )
{
	FLOAT	InvInterpValue = 1.0f - InterpValue;

	Volume = ( Start.Volume * InvInterpValue ) + ( End.Volume * InterpValue );
	Density = ( Start.Density * InvInterpValue ) + ( End.Density * InterpValue );				
	Diffusion = ( Start.Diffusion * InvInterpValue ) + ( End.Diffusion * InterpValue );				
	Gain = ( Start.Gain * InvInterpValue ) + ( End.Gain * InterpValue );					
	GainHF = ( Start.GainHF * InvInterpValue ) + ( End.GainHF * InterpValue );					
	DecayTime = ( Start.DecayTime * InvInterpValue ) + ( End.DecayTime * InterpValue );				
	DecayHFRatio = ( Start.DecayHFRatio * InvInterpValue ) + ( End.DecayHFRatio * InterpValue );			
	ReflectionsGain = ( Start.ReflectionsGain * InvInterpValue ) + ( End.ReflectionsGain * InterpValue );		
	ReflectionsDelay = ( Start.ReflectionsDelay * InvInterpValue ) + ( End.ReflectionsDelay * InterpValue );		
	LateGain = ( Start.LateGain * InvInterpValue ) + ( End.LateGain * InterpValue );				
	LateDelay = ( Start.LateDelay * InvInterpValue ) + ( End.LateDelay * InterpValue );				
	AirAbsorptionGainHF = ( Start.AirAbsorptionGainHF * InvInterpValue ) + ( End.AirAbsorptionGainHF * InterpValue );	
	RoomRolloffFactor = ( Start.RoomRolloffFactor * InvInterpValue ) + ( End.RoomRolloffFactor * InterpValue );		
	/** Turn on halfway through the interpolation */
	DecayHFLimit = ( InterpValue > 0.5f );
}

/** 
 * Reverb preset table - may be exposed in the future
 */
FAudioReverbEffect FAudioEffectsManager::ReverbPresets[REVERB_MAX] =
{
	FAudioReverbEffect( 0.0000f, 0.0000f, 0.00f, 1.00f, 0.50f, 0.0000f, 0.020f, 0.0000f, 0.040f, 1.00f, 1.00f, 0.994f ),	// DEFAULT
	FAudioReverbEffect( 0.3162f, 0.2511f, 0.00f, 1.49f, 0.54f, 0.6531f, 0.007f, 0.3055f, 0.011f, 1.00f, 0.60f, 0.994f ),	// BATHROOM
	FAudioReverbEffect( 0.3162f, 0.7079f, 0.00f, 2.31f, 0.64f, 0.4411f, 0.012f, 0.9089f, 0.017f, 1.00f, 1.00f, 0.994f ),	// STONEROOM
	FAudioReverbEffect( 0.3162f, 0.5781f, 0.00f, 4.32f, 0.59f, 0.4032f, 0.020f, 0.7170f, 0.030f, 1.00f, 1.00f, 0.994f ),	// AUDITORIUM
	FAudioReverbEffect( 0.3162f, 0.5623f, 0.00f, 3.92f, 0.70f, 0.2427f, 0.020f, 0.9977f, 0.029f, 1.00f, 1.00f, 0.994f ),	// CONCERTHALL
	FAudioReverbEffect( 0.3162f, 1.0000f, 0.00f, 2.91f, 1.30f, 0.5000f, 0.015f, 0.7063f, 0.022f, 1.00f, 1.00f, 0.994f ),	// CAVE
	FAudioReverbEffect( 0.3162f, 0.7079f, 0.00f, 1.49f, 0.59f, 0.2458f, 0.007f, 0.6019f, 0.011f, 1.00f, 1.00f, 0.994f ),	// HALLWAY
	FAudioReverbEffect( 0.3162f, 0.7612f, 0.00f, 2.70f, 0.79f, 0.2472f, 0.013f, 0.6346f, 0.020f, 1.00f, 1.00f, 0.994f ),	// STONECORRIDOR
	FAudioReverbEffect( 0.3162f, 0.7328f, 0.00f, 1.49f, 0.86f, 0.2500f, 0.007f, 0.9954f, 0.011f, 1.00f, 1.00f, 0.994f ),	// ALLEY
	FAudioReverbEffect( 0.3162f, 0.0224f, 0.00f, 1.49f, 0.54f, 0.0525f, 0.162f, 0.4937f, 0.088f, 0.79f, 1.00f, 0.994f ),	// FOREST
	FAudioReverbEffect( 0.3162f, 0.3981f, 0.00f, 1.49f, 0.67f, 0.0730f, 0.007f, 0.0779f, 0.011f, 0.50f, 1.00f, 0.994f ),	// CITY
	FAudioReverbEffect( 0.3162f, 0.0562f, 0.00f, 1.49f, 0.21f, 0.0407f, 0.300f, 0.0984f, 0.100f, 0.27f, 1.00f, 0.994f ),	// MOUNTAINS
	FAudioReverbEffect( 0.3162f, 0.3162f, 0.00f, 1.49f, 0.83f, 0.0000f, 0.061f, 0.5623f, 0.025f, 1.00f, 1.00f, 0.994f ),	// QUARRY
	FAudioReverbEffect( 0.3162f, 0.1000f, 0.00f, 1.49f, 0.50f, 0.0585f, 0.179f, 0.0553f, 0.100f, 0.21f, 1.00f, 0.994f ),	// PLAIN
	FAudioReverbEffect( 0.3162f, 1.0000f, 0.00f, 1.65f, 1.50f, 0.2082f, 0.008f, 0.2652f, 0.012f, 1.00f, 1.00f, 0.994f ),	// PARKINGLOT
	FAudioReverbEffect( 0.3162f, 0.3162f, 0.00f, 2.81f, 0.14f, 1.6387f, 0.014f, 0.4742f, 0.021f, 0.80f, 0.60f, 0.994f ),	// SEWERPIPE
	FAudioReverbEffect( 0.3162f, 0.0100f, 0.00f, 1.49f, 0.10f, 0.5963f, 0.007f, 0.1412f, 0.011f, 1.00f, 1.00f, 0.994f ),	// UNDERWATER
	FAudioReverbEffect( 0.3162f, 0.5011f, 0.00f, 1.10f, 0.83f, 0.6310f, 0.005f, 0.5623f, 0.010f, 1.00f, 1.00f, 0.994f ),	// SMALLROOM
	FAudioReverbEffect( 0.3162f, 0.5011f, 0.00f, 1.30f, 0.83f, 0.3162f, 0.010f, 0.7943f, 0.020f, 1.00f, 1.00f, 0.994f ),	// MEDIUMROOM
	FAudioReverbEffect( 0.3162f, 0.5011f, 0.00f, 1.50f, 0.83f, 0.1585f, 0.020f, 0.3162f, 0.040f, 1.00f, 1.00f, 0.994f ),	// LARGEROOM
	FAudioReverbEffect( 0.3162f, 0.5011f, 0.00f, 1.80f, 0.70f, 0.2239f, 0.015f, 0.3981f, 0.030f, 1.00f, 1.00f, 0.994f ),	// MEDIUMHALL
	FAudioReverbEffect( 0.3162f, 0.5011f, 0.00f, 1.80f, 0.70f, 0.1000f, 0.030f, 0.1885f, 0.060f, 1.00f, 1.00f, 0.994f ),	// LARGEHALL
	FAudioReverbEffect( 0.3162f, 0.7943f, 0.00f, 1.30f, 0.90f, 1.0000f, 0.002f, 1.0000f, 0.010f, 1.00f, 0.75f, 0.994f )		// PLATE
};																													 

/**
 * Converts and volume (0.0f to 1.0f) to a MilliBel value (a Hundredth of a deciBel)
 */
LONG FAudioEffectsManager::VolumeToMilliBels( FLOAT Volume )
{
	if( Volume > 0.0f )
	{
		return( ( LONG )( 2000.0f * log10f( Volume ) ) );
	}

	return( -10000 );
}

/** 
 * Gets the parameters for reverb based on settings and time
 */
void FAudioEffectsManager::Interpolate( FAudioReverbEffect & Current, FAudioReverbEffect & Start, FAudioReverbEffect & End )
{
	FLOAT InterpValue = ( ( FLOAT )appSeconds() - Start.Time ) / ( End.Time - Start.Time );

	if( InterpValue > 1.0f )
	{
		Current = End;
		return;
	}

	if( InterpValue < 0.0f )
	{
		Current = Start;
		return;
	}

	Current.Interpolate( InterpValue, Start, End );
}

/** 
 * Clear out any reverb settings
 */
FAudioEffectsManager::FAudioEffectsManager( UAudioDevice * InDevice )
{
	Device = InDevice;

	// Clear out the default reverb settings
	FReverbSettings ReverbSettings;
	ReverbSettings.ReverbType = REVERB_Off;
	ReverbSettings.Volume = 0.0f;
	ReverbSettings.FadeTime = 1.0f;
	SetReverbSettings( ReverbSettings );
}

/**
 * Called every tick from UGameViewportClient::Draw
 * 
 * Sets new reverb mode if necessary. Otherwise interpolates to the current settings and calls SetEffect to handle
 * the platform specific aspect.
 */
void FAudioEffectsManager::SetReverbSettings( const FReverbSettings& ReverbSettings )
{
	/** Update the settings if the reverb type has changed */
	if( ReverbSettings.ReverbType != CurrentReverbType )
	{
		debugf( NAME_DevSound, TEXT( "UAudioDevice::SetReverbSettings(): Old - %i  New - %i:%f(%f)" ),
			( INT )CurrentReverbType, ReverbSettings.ReverbType, ReverbSettings.Volume, ReverbSettings.FadeTime );

		SourceReverbEffect = CurrentReverbEffect;
		SourceReverbEffect.Time = ( FLOAT )appSeconds();

		DestinationReverbEffect = ReverbPresets[ReverbSettings.ReverbType];
		DestinationReverbEffect.Time = SourceReverbEffect.Time + ReverbSettings.FadeTime;
		DestinationReverbEffect.Volume = ReverbSettings.Volume;

		CurrentReverbType = ( ReverbPreset )ReverbSettings.ReverbType;
	}

	/** Interpolate the settings depending on time */
	Interpolate( CurrentReverbEffect, SourceReverbEffect, DestinationReverbEffect );

	/** Call the platform specific code to set the effect */
	SetReverbEffect( CurrentReverbEffect );
}

// end 
