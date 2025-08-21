
class avaSoundNodePlaytime extends SoundNode
	native
	collapsecategories
	hidecategories(Object)
	editinlinenew;

// ���۵Ǹ� StartDelayTime��ŭ ����, PlayTime��ŭ Play�ǰ� �ߴܵȴ�
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

