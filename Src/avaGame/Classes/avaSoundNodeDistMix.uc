/**
 * SoundNodeDistanceCrossFade
 * 
 * This node's purpose is to play different sounds based on the distance to the listener.  
 * The node mixes between the N different sounds which are valid for the distance.  One should
 * think of a SoundNodeDistanceCrossFade as Mixer node which determines the set of nodes to
 * "mix in" based on their distance to the sound.
 * 
 * Example:
 * You have a gun that plays a fire sound.  At long distances you want a different sound than
 * if you were up close.   So you use a SoundNodeDistanceCrossFade which will calculate the distance
 * a listener is from the sound and play either:  short distance, long distance, mix of short and long sounds.
 *
 * A SoundNodeDistanceCrossFade differs from an SoundNodeAttenuation in that any sound is only going
 * be played if it is within the MinRadius and MaxRadius.  So if you want the short distance sound to be 
 * heard by people close to it, the MinRadius should probably be 0
 *
 * The volume curve for a SoundNodeDistanceCrossFade will look like this:
 *
 *                          Volume (of the input) 
 *    FadeInDistance.Max --> _________________ <-- FadeOutDistance.Min
 *                          /                 \
 *                         /                   \
 *                        /                     \
 * FadeInDistance.Min -->/                       \ <-- FadeOutDistance.Max
 *
 * Copyright ?2006 Epic Games, Inc. All Rights Reserved.
 **/

class avaSoundNodeDistMix extends SoundNode
	native
	collapsecategories
	hidecategories(Object)
	editinlinenew;

var() float OneShot_Solo_Threshold;
var() float NearDistance, FarDistance;

cpptext
{
	// USoundNode interface.

	virtual void ParseNodes( USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual INT GetMaxChildNodes() { return 3; }
	virtual void CreateStartingConnectors()
	{		
		// Mixers default with two connectors.
		InsertChildNode( ChildNodes.Num() );
		InsertChildNode( ChildNodes.Num() );
		InsertChildNode( ChildNodes.Num() );
	}
}

defaultproperties
{
	OneShot_Solo_Threshold = 0.25
	NearDistance = 320
	FarDistance = 1760
}
