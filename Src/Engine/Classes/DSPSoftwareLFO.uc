class DSPSoftwareLFO extends DSPSoftwareBlock;

var() ELFOType Type;
var() float Rate;

simulated function FillValue( out Descriptor desc )
{
	desc.type = 7;
	desc.params[0] = Type;
	desc.params[1] = Rate;
	desc.params[2] = 0;	
}