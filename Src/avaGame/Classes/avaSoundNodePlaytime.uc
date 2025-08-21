
class avaSoundNodePlaytime extends SoundNode
	native
	collapsecategories
	hidecategories(Object)
	editinlinenew;

// 시작되면 StartDelayTime만큼 쉬고, PlayTime만큼 Play되고 중단된다
var()	float	StartDelayTime;
var()	float	PlayTime;

cpptext
{
	// USoundNode interface.
	virtual void NotifyWaveInstanceFinished( struct FWaveInstance* WaveInstance );

	void ParseNodes( USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	FLOAT MaxAudibleDistance(FLOAT CurrentMaxDistance) { return WORLD_MAX; }
	FLOAT GetDuration();

}

defaultproperties
{
}

