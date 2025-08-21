class DSPSoftwarePitchShifter extends DSPSoftwareBlock;

var() float Pitch, TimeSlice, Crossfade;

simulated function FillValue( out Descriptor desc )
{
	desc.type = 5;
	desc.params[0] = Pitch;
	desc.params[1] = TimeSlice;
	desc.params[2] = Crossfade;	
}