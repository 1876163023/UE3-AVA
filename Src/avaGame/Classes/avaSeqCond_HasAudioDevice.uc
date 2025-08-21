/**
 * does it have an audio device ?
 */
class avaSeqCond_HasAudioDevice extends SequenceCondition
	native;

cpptext
{
	virtual void Activated();
}

defaultproperties
{
	ObjName="Has AudioDevice"

	OutputLinks(0)=(LinkDesc="Has")
	OutputLinks(1)=(LinkDesc="Not")
}
