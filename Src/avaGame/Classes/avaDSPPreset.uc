class avaDSPPreset extends Object
	collapsecategories
	hidecategories(Object)
	native;

var() editinline array<avaDSPBlock> DSPBlocks;
var private transient int Handle;
var private transient bool Dirty;
var() float Duration;
var() float Fade;

var() DSPSlotIndex DSPSlot;

cpptext
{
	UavaDSPPreset();
	virtual void FinishDestroy();

	void PostEditChange(UProperty* PropertyThatChanged)
	{
		__super::PostEditChange(PropertyThatChanged);

		Reload();
	}
}

native simulated function Reload();
native simulated function Apply();
static native simulated function Stop();

defaultproperties
{
	Duration = 0
	Fade = 1.0
	DSPSlot = DSP_Room
}