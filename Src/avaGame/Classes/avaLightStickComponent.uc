class avaLightStickComponent extends PointLightComponent
	native;

var		float		ElapsedTime;
var()	float		DurationTime;	// 지속시간...
var()	float		FallOffTime;	// 사그라지는 시간...
var()	float		IncTime;		// 

var()	float		TargetRadius;

cpptext
{
	void Tick(FLOAT DeltaTime);
}

defaultproperties
{
	Brightness	=	1
	Radius		=	0
	TargetRadius=	1024
	CastShadows	=	false
	LightColor	=	(R=180,G=180,B=255,A=255)

	DurationTime	= 12.0
	FallOffTime		= 3.0
	IncTime			= 1.0
}