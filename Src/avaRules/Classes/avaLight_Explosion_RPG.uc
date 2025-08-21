class avaLight_Explosion_RPG extends avaExplosionLight;

defaultproperties
{
	//Brightness=5
	LightColor=(R=255,G=255,B=255,A=255)
	TimeShift(0)=(StartTime=0.0,Radius=256,Brightness=10,LightColor=(R=255,G=255,B=250,A=255) )
	TimeShift(1)=(StartTime=0.2,Radius=400,Brightness=8,LightColor=(R=255,G=255,B=220,A=255) )
	TimeShift(2)=(StartTime=0.5,Radius=0,Brightness=0,LightColor=(R=255,G=190,B=190,A=255) )
}