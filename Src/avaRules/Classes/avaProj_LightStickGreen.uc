class avaProj_LightStickGreen extends avaProj_BaseLightStick;

defaultproperties
{
	Begin Object Name=ProjectileMesh		
		StaticMesh=StaticMesh'Wp_LightStick.MS_LightStick_NRF_3p'
	End Object

	ExplodeTime			= 25.0

	ProjectileLightClass	= class'avaLightStickComponent_Green'
}