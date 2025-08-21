class avaUIAction_SetColorCorrectionParam extends UIAction
	native;

var float		HueOffset;
var float		SatOffset;
var float		LightOffset;
var float		ContrastOffset;
var vector		Shadows;
var vector		HighLights;
var vector		MidTones;
var float		Desaturation;

cpptext
{
	/** Callback for when the event is activated. */
	virtual void Activated();
}

defaultproperties
{
	bCallHandler=false
	ObjName="Set Color Param"
	ObjCategory="Adjust"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Hue",PropertyName="HueOffset",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Saturation",PropertyName="SatOffset",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Lightness",PropertyName="LightOffset",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Contrast",PropertyName="ContrastOffset",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Vector',LinkDesc="Shadows",PropertyName="Shadows",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Vector',LinkDesc="Hightlights",PropertyName="HighLights",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Vector',LinkDesc="MidTones",PropertyName="MidTones",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Desaturation",PropertyName="Desaturation",MaxVars=1))
}