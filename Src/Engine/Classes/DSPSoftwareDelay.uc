class DSPSoftwareDelay extends DSPSoftwareBlock;

var() EDelayType Type;
var() float Delay;
var() float Feedback, Gain;
var() EFilterType FilterType;
var() float Cutoff, QWidth;
var() EQualityType Quality;

simulated function FillValue( out Descriptor desc )
{
	desc.type = 1;
	desc.params[0] = Type;
	desc.params[1] = Delay;
	desc.params[2] = Feedback;
	desc.params[3] = Gain;
	desc.params[4] = FilterType;
	desc.params[5] = Cutoff;
	desc.params[6] = QWidth;
	desc.params[7] = Quality;
}