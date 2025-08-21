class avaUIAction_TransPosition extends avaUIAction_TransitionBase
	native;

struct native TransPosInfo
{
	var UIScreenObject	ScreenObj;
	var float			InitRawPos[4 /*UIFACE_MAX*/];
};

var transient float					TimeElapsed;
var transient array<TransPosInfo>	TransPosData;

var() CurveEdPresetCurve	PositionCurve;
var() float					PositionDuration;
var() UIScreenObject		PositionTargetObject;

cpptext
{
	virtual void Activated();
	virtual UBOOL UpdateOp(FLOAT deltaTime);
}

defaultproperties
{
	ObjName="Positioning"

	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="PosCurve", PropertyName=PositionCurve, MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="PosDur", PropertyName=PositionDuration, MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="PosTarget", PropertyName=PositionTargetObject, MaxVars=1))
}