class DSPSoftwareReverb extends DSPSoftwareBlock;

var() float RoomSize, Density, Decay;
var() EFilterType FilterType;
var() float Cutoff, QWidth;
var() EQualityType Quality;
var() bool bParallel;

simulated function FillValue( out Descriptor desc )
{
	desc.type = 2;
	desc.params[0] = RoomSize;
	desc.params[1] = Density;
	desc.params[2] = Decay;	
	desc.params[3] = FilterType;
	desc.params[4] = Cutoff;
	desc.params[5] = QWidth;
	desc.params[6] = Quality;
	if (bParallel)
	{
		desc.params[7] = 1;
	}
	else
	{
		desc.params[7] = 0;
	}
}