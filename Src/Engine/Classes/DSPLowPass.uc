class DSPLowPass extends DSPBlock native(DSP);

var() float CutOff;
var() float Resonance;

defaultproperties
{
	CutOff = 5000
	Resonance = 1.0
}