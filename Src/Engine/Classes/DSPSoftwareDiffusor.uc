class DSPSoftwareDiffusor extends DSPSoftwareBlock;

var() float Size, Density, Decay;

simulated function FillValue( out Descriptor desc )
{
	desc.type = 10;
	desc.params[0] = Size;
	desc.params[1] = Density;
	desc.params[2] = Decay;	
}