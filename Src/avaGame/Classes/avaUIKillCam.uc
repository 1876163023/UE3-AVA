class avaUIKillCam extends avaUIVideo native(Video);

var() float Duration;
var() float FadeIn, FadeOut;
var() bool bKillerCamera;
var() float Killer_FOV, Killee_FOV;
var() float CameraDistance, FOV_Speed;
var() float Border_Thickness;
var() color Border_Color;
var private transient float Progress, StartedTime;
var private transient vector KillCamLocation;

cpptext
{	
	virtual void Render_Widget( FCanvas* Canvas );
}

defaultproperties
{
	Duration = 2.0
	FadeIn = 0.02
	FadeOut = 0.15

	Killer_FOV = 80.0
	Killee_FOV = 40.0
	CameraDistance = 156
	FOV_Speed = 4.0
	Border_Thickness = 2
	Border_Color = (R=0,G=0,B=0)
}