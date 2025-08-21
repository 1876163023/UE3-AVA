class avaDSPHighPass extends avaDSPBlock native;

var() float CutOff;
var() float Resonance;

defaultproperties
{
	CutOff = 5000
	Resonance = 1.0
}