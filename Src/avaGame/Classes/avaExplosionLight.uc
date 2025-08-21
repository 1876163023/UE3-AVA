class avaExplosionLight extends PointLightComponent
	native;

/** set false after frame rate dependent properties have been tweaked. */
var bool bCheckFrameRate;	

/** HighDetailFrameTime - if frame rate is above this, force super high detail.  @todo steve - move to more appropriate actor */
var float HighDetailFrameTime;

/** Lifetime - how long this explosion has been going */
var float Lifetime;

/** Index into TimeShift array */
var int TimeShiftIndex;

struct native LightValues
{
	var float StartTime;
	var float Radius;
	var float Brightness;
	var color LightColor; 
};

var() array<LightValues> TimeShift;

final native function ResetLight();

cpptext
{
	void Tick(FLOAT DeltaTime);
}

defaultproperties
{
	HighDetailFrameTime=+0.015
	bCheckFrameRate=true
	Brightness=8
	Radius=256
	CastShadows=false
	LightColor=(R=255,G=255,B=255,A=255)
}
