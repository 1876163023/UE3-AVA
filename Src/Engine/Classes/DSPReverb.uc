class DSPReverb extends DSPBlock native(DSP);

var() float RoomSize, Damp, DryMix, WetMix, Width;

defaultproperties
{
	RoomSize = 0.5
	Damp = 0.5
	DryMix = 0.66
	WetMix = 0.33
	Width = 1.0
}