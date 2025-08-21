class avaGunMuzzleFlashLight extends PointLightComponent native;

var float LastRenderTime;
var float LastReallyRenderedTime;

simulated native function ResetLight();

cpptext
{
	void Tick( FLOAT DeltaTime );
}

defaultproperties
{	
	CastShadows=false
	Brightness=16
	Radius=70
	LightColor=(R=255,G=255,B=255,A=255)	
	LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
}
