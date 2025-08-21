class avaSoundScape extends Keypoint
	native;

var() avaSoundScapeProperty Property;

cpptext
{
	AavaSoundScape();
	virtual void FinishDestroy();
}

static native function avaSoundScape FindSoundscape( vector pos );

defaultproperties
{
	Begin Object NAME=Sprite
		Sprite=Texture2D'EngineResources.S_Ambient'
	End Object

	RemoteRole=ROLE_None
}
