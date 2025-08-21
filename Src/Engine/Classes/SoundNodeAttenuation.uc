/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SoundNodeAttenuation extends SoundNode
	native(Sound)
	collapsecategories
	hidecategories(Object)
	editinlinenew;


enum SoundDistanceModel
{
	ATTENUATION_Linear,
	ATTENUATION_Logarithmic,
	ATTENUATION_Inverse,
	ATTENUATION_LogReverse
};

var()					SoundDistanceModel		DistanceModel;

/**
 * This is the point at which to start attenuating.  Prior to this point the sound will be
 * at full volume.
 **/
var()					rawdistributionfloat	MinRadius;
var()					rawdistributionfloat	MaxRadius;

var()					bool					bSpatialize;
var()					bool					bAttenuate;

cpptext
{
	// USoundNode interface.

	virtual void ParseNodes( USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual FLOAT MaxAudibleDistance(FLOAT CurrentMaxDistance) { return ::Max<FLOAT>(CurrentMaxDistance,MaxRadius.GetValue(0.9f)); }
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

	bSpatialize=true
	bAttenuate=true
}
