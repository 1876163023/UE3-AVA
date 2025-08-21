class avaDSPSoftwareAmp extends avaDSPSoftwareBlock;

var() float Gain, DistortionThreshold, DistortionMix, DistortionFeedback;

simulated function FillValue( out Descriptor desc )
{
	desc.type = 11;
	desc.params[0] = Gain;
	desc.params[1] = DistortionThreshold;
	desc.params[2] = DistortionMix;	
	desc.params[3] = DistortionFeedback;	
}