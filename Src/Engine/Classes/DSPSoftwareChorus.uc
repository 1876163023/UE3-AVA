class DSPSoftwareChorus extends DSPSoftwareBlock;

var() ELFOType LFOType;
var() float Rate, Depth, Mix;

simulated function FillValue( out Descriptor desc )
{
	desc.type = 4;
	desc.params[0] = LFOType;
	desc.params[1] = Rate;
	desc.params[2] = Depth;	
	desc.params[3] = Mix;	
}