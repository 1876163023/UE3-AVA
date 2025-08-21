class SunLightComponent extends DirectionalLightComponent
	native	
	hidecategories(Object)
	editinlinenew;

var(AVA_Radiosity)	interp float	AmbientBrightness;
var(AVA_Radiosity)	color			AmbientLightColor;

var(AVA_Radiosity)	String			SkyEnvironmentMap;
var(AVA_Radiosity)	float			SkyMax;
var(AVA_Radiosity)	float			SkyScale;
var(AVA_Radiosity)	float			Yaw;
var(AVA_Radiosity)	float			HDRTonemapScale;

defaultproperties
{
	SkyMax=1.0
	SkyScale=1.0
	Yaw = 0.0	
	HDRTonemapScale=0
//	MinAutoExposure=0.1
//	MaxAutoExposure=10
//	KeyValue=0.18
//	bUseDynamicTonemapping=true
}