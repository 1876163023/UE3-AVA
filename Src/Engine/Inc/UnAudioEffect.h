/*=============================================================================
	UnAudio.cpp: Unreal base audio.
	Copyright 1997-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

class FAudioEffect
{
public:
	FAudioEffect( void ) {}
	virtual ~FAudioEffect( void ) {}
};

class FAudioReverbEffect : public FAudioEffect
{
public:
	/** Sets the default values for a reverb effect */
	FAudioReverbEffect( void );
	/** Sets the platform agnostic parameters */
	FAudioReverbEffect( FLOAT InRoom, FLOAT InRoomHF, FLOAT InRoomRolloffFactor, FLOAT InDecayTime, FLOAT InDecayHFRatio, FLOAT InReflections, FLOAT InReflectionsDelay, 
					 FLOAT InReverb, FLOAT InReverbDelay, FLOAT InDiffusion, FLOAT InDensity, FLOAT InAirAbsorption );

	/** Interpolates between Start and Ed reverb effect settings */
	void Interpolate( FLOAT InterpValue, FAudioReverbEffect & Start, FAudioReverbEffect & End );

	/** Time when this reverb was initiated or completed faded in */
	FLOAT		Time;

	/** Overall volume of effect */
	FLOAT		Volume;					// 0.0 to 1.0

	/** Platform agnostic parameters that define a reverb effect. Min < Default < Max */
	FLOAT		Density;				// 0.0 < 1.0 < 1.0
	FLOAT		Diffusion;				// 0.0 < 1.0 < 1.0
	FLOAT		Gain;					// 0.0 < 0.32 < 1.0 
	FLOAT		GainHF;					// 0.0 < 0.89 < 1.0
	FLOAT		DecayTime;				// 0.1 < 1.49 < 20.0	Seconds
	FLOAT		DecayHFRatio;			// 0.1 < 0.83 < 2.0
	FLOAT		ReflectionsGain;		// 0.0 < 0.05 < 3.16
	FLOAT		ReflectionsDelay;		// 0.0 < 0.007 < 0.3	Seconds
	FLOAT		LateGain;				// 0.0 < 1.26 < 10.0
	FLOAT		LateDelay;				// 0.0 < 0.011 < 0.1	Seconds
	FLOAT		AirAbsorptionGainHF;	// 0.892 < 0.994 < 1.0
	FLOAT		RoomRolloffFactor;		// 0.0 < 0.0 < 10.0
	UBOOL		DecayHFLimit;			// FALSE TRUE - TRUE
};

/** 
 * Manager class to handle the interface to various audio effects
 */
class FAudioEffectsManager
{
public:
	FAudioEffectsManager( UAudioDevice * Device );
	virtual ~FAudioEffectsManager( void ) {}

	/** 
	 * Engine hook to handle setting and fading in of reverb effects
	 */
	void SetReverbSettings( const FReverbSettings& ReverbSettings );

	/** 
	 * Calls the platform specific code to set the parameters that define reverb
	 */
	virtual void SetReverbEffect( const FAudioReverbEffect & ReverbEffectParameters ) {}

	/** 
	 * Platform dependent call to associate the sound output with an audio effect
	 */
	virtual void * LinkEffect( UINT SourceId, UBOOL Enable ) { return( NULL ); }

	/**
	 * Converts and volume (0.0f to 1.0f) to a MilliBel value (a Hundredth of a deciBel)
	 */
	LONG VolumeToMilliBels( FLOAT Volume );

private:
	/** 
	 * Helper function to interpolate between different reverb effects
	 */
	void Interpolate( FAudioReverbEffect & Current, FAudioReverbEffect & Start, FAudioReverbEffect & End );

	UAudioDevice *			Device;

	ReverbPreset			CurrentReverbType;

	FAudioReverbEffect		SourceReverbEffect;
	FAudioReverbEffect		CurrentReverbEffect;
	FAudioReverbEffect		DestinationReverbEffect;

	/** 
	 * The parameters that define the reverb presets
	 */
	static	FAudioReverbEffect FAudioEffectsManager::ReverbPresets[REVERB_MAX];
};

// end 
