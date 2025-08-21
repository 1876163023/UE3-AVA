
class avaSoundNodeAttenuation extends SoundNode
	native
	collapsecategories
	hidecategories(Object)
	editinlinenew;


enum avaSoundDistanceModel
{
	avaATTENUATION_Linear,
	avaATTENUATION_CalcDecibel,
};


var()					avaSoundDistanceModel	DistanceModel;

/**
 * This is the point at which to start attenuating.  Prior to this point the sound will be
 * at full volume.
 **/
var()					float MinRadius;
var()					float MaxRadius;

var()	float								SoundPressure;

var()					bool				bSpatialize;
var()					bool				bAttenuate;
var		transient float						ObstructionGain, ObstructionGainTarget, LastCheckTime;

cpptext
{
	// USoundNode interface.

	virtual void ParseNodes( USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual FLOAT MaxAudibleDistance(FLOAT CurrentMaxDistance);
}


defaultproperties
{
	MinRadius=400
	MaxRadius=5000

	DistanceModel=avaATTENUATION_CalcDecibel
	
	bSpatialize=true
	bAttenuate=true
	SoundPressure=70	
	ObstructionGainTarget = 1.0
	ObstructionGain = 1.0
	LastCheckTime = -1.0
}

