class DSPEcho extends DSPBlock native(DSP);

var() float Delay, DecayRatio, DryMix, WetMix;

defaultproperties
{
	Delay = 500
	DecayRatio = 0.5
	DryMix = 1.0
	WetMix = 1.0
}