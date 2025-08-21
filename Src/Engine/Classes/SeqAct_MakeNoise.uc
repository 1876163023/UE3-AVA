/** Causes target actor to make a noise that AI will hear */
class SeqAct_MakeNoise extends SequenceAction;

var() float Loudness;

defaultproperties
{
	ObjName="MakeNoise"
	ObjCategory="AI"
	ObjClassVersion=2

	Loudness=1.f
}