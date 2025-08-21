class avaPDACamNavPoint extends NavigationPoint
	placeable
	native;

var(PDACamera) EPDACameraHeight PDACameraHeight;
var(PDACamera) float FOV;
var(PDACamera) bool bFixedLocation;

defaultproperties
{
	PDACameraHeight=PDACameraHeight_High
	Rotation=(Pitch=-16384,Yaw=32767,Roll=32767)
	FOV=90
	bFixedLocation=false

	Begin Object Name=Sprite
		Scale=5.0
	End Object

	Begin Object NAME=CollisionCylinder
		CollisionRadius=+00010.000000
		CollisionHeight=+00010.000000
	End Object

	bNotBased=true
}