class avaUIAction_TransScale extends avaUIAction_TransitionBase
	native;

struct native TransScaleInfo
{
	var UIScreenObject	ScreenObj;
	var float			InitBound[2/*UIORIENT_MAX*/];
};

var transient float					TimeElapsed;
var transient array<TransScaleInfo>	TransScaleData;

var() bool					bDrawComp<ToolTip=DrawCompOpacity or WidgetOpacity>;
var() CurveEdPresetCurve	ScaleCurveHorz;
var() CurveEdPresetCurve	ScaleCurveVert;
var() float					ScaleAxisHorz;
var() float					ScaleAxisVert;
var() float					ScaleDuration;

cpptext
{
	virtual void Activated();
	virtual UBOOL UpdateOp(FLOAT deltaTime);
}

defaultproperties
{
	ObjName="Scaling"
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="DrawComp", PropertyName=bDrawComp, MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="SclCurveH", PropertyName=ScaleCurveHorz, MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="SclCurveV", PropertyName=ScaleCurveVert, MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="SclAxisH", PropertyName=ScaleAxisHorz, MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="SclAxisV", PropertyName=ScaleAxisVert, MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="OpaDur", PropertyName=ScaleDuration, MaxVars=1))
}