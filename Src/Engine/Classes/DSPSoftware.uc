class DSPSoftware extends DSPBlock native(DSP);

var() editinline array<DSPSoftwareBlock> Blocks;
var() float Gain;

defaultproperties
{	
	Gain = 1.0
}