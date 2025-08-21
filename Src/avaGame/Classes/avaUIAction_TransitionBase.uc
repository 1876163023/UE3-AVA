class avaUIAction_TransitionBase extends UIAction
	abstract
	native;

var transient int				QueueIndex;
var transient array<int>		WorkingDelta;

native static function GetUIDrawComponents( UIObject TargetObj, out array<UIComp_DrawComponents> OutComps, optional bool bRecursive );

native function float GetStableDeltaTime( float RawDeltaTime );

defaultproperties
{
	ObjCategory="Transition"

	OutputLinks(0)=(LinkDesc="Complete")
	OutputLinks(1)=(LinkDesc="Abort")

	bLatentExecution=true
	bAutoActivateOutputLinks=false


	WorkingDelta(5)=0
}