class avaUIChatCam extends avaUIVideo native(Video);

var() float Duration;
var() float FadeOut;
var() vector CameraLocation, TargetOffset;
var() float Radius;
var() float Border_Thickness;
var() color Border_Color;
var private transient float Progress, StartedTime;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );
}

defaultproperties
{
	Duration = 1.0
	FadeOut = 0.15
	CameraLocation = ( X=75, Y=0, Z=0) 
	TargetOffset = ( X=5, Y=0, Z=-7.5 )
	Radius = 20.0f
	Border_Thickness = 2
	Border_Color = (R=0,G=0,B=0)
}