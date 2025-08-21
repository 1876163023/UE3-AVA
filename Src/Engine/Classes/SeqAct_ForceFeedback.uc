class SeqAct_ForceFeedback extends SequenceAction;

var() editinline ForceFeedbackWaveform FFWaveform;

defaultproperties
{
	ObjName="Force Feedback"
	ObjCategory="Misc"

	InputLinks(0)=(LinkDesc="Start")
	InputLinks(1)=(LinkDesc="Stop")
}
