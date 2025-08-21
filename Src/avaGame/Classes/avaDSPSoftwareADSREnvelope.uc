class avaDSPSoftwareADSREnvelope extends avaDSPSoftwareBlock;

var() EEnvelopeType Type;
var() float Amp1, Amp2, Amp3, Attack, Decay, Sustain, Release;

simulated function FillValue( out Descriptor desc )
{
	desc.type = 6;
	desc.params[0] = Type;
	desc.params[1] = Amp1;
	desc.params[2] = Amp2;
	desc.params[3] = Amp3;
	desc.params[4] = Attack;
	desc.params[5] = Decay;
	desc.params[6] = Sustain;
	desc.params[7] = Release;
}