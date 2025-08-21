class avaUIAction_TransOpacity extends avaUIAction_TransitionBase
	native;

var transient float			TimeElapsed;

var() bool					bDrawComp<ToolTip=DrawCompOpacity or WidgetOpacity>;
var() CurveEdPresetCurve	OpacityCurve;
var() float					OpacityDuration;

cpptext
{
	virtual void Activated();
	virtual UBOOL UpdateOp(FLOAT deltaTime);
}

defaultproperties
{
	ObjName="Trans Opacity"

	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="DrawComp", PropertyName=bDrawComp, MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="OpaCurve", PropertyName=OpacityCurve, MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="OpaDur", PropertyName=OpacityDuration, MaxVars=1))
}