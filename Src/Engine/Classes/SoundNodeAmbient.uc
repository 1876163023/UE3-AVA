/**
 * Copyright 2005 Epic Games, Inc. All Rights Reserved.
 */
class SoundNodeAmbient extends SoundNode
	native(Sound)
	collapsecategories
	hidecategories(Object)
	dependson(SoundNodeAttenuation)
	editinlinenew;


var()								SoundDistanceModel		DistanceModel;

var()								rawdistributionfloat	MinRadius;
var()								rawdistributionfloat	MaxRadius;

var()								bool					bSpatialize;
var()								bool					bAttenuate;

var()								SoundNodeWave			Wave;

var()								rawdistributionfloat		VolumeModulation;
var()								rawdistributionfloat		PitchModulation;


cpptext
{
	// USoundNode interface.
	virtual void ParseNodes( USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual void GetNodes( class UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes );
	virtual void GetAllNodes( TArray<USoundNode*>& SoundNodes ); // Like above but returns ALL (not just active) nodes.
	virtual FLOAT MaxAudibleDistance(FLOAT CurrentMaxDistance) { return ::Max<FLOAT>(CurrentMaxDistance,MaxRadius.GetValue(0.9f)); }
	virtual INT GetMaxChildNodes() { return 0; }
		
	/**
	 * Notifies the sound node that a wave instance in its subtree has finished.
	 *
	 * @param WaveInstance	WaveInstance that was finished 
	 */
	virtual void NotifyWaveInstanceFinished( struct FWaveInstance* WaveInstance );

	/**
	 * We're looping indefinitely so we're never finished.
	 *
	 * @param	AudioComponent	Audio component containing payload data
	 * @return	FALSE
	 */
	virtual UBOOL IsFinished( class UAudioComponent* /*Unused*/ ) { return FALSE; }
}


defaultproperties
{
	Begin Object Class=DistributionFloatUniform Name=DistributionMinRadius
		Min=400
		Max=400
	End Object
	MinRadius=(Distribution=DistributionMinRadius)

	Begin Object Class=DistributionFloatUniform Name=DistributionMaxRadius
		Min=5000
		Max=5000
	End Object
	MaxRadius=(Distribution=DistributionMaxRadius)

	Begin Object Class=DistributionFloatUniform Name=DistributionVolume
		Min=1
		Max=1
	End Object
	VolumeModulation=(Distribution=DistributionVolume)

	Begin Object Class=DistributionFloatUniform Name=DistributionPitch
		Min=1
		Max=1
	End Object
	PitchModulation=(Distribution=DistributionPitch)

	bSpatialize=true
	bAttenuate=true
}

