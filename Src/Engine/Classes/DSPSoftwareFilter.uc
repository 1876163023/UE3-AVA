class DSPSoftwareFilter extends DSPSoftwareBlock;

var() EFilterType FilterType;
var() float Cutoff, QWidth;
var() EQualityType Quality;

simulated function FillValue( out Descriptor desc )
{
	desc.type = 3;	
	desc.params[0] = FilterType;
	desc.params[1] = Cutoff;
	desc.params[2] = QWidth;
	desc.params[3] = Quality;
}