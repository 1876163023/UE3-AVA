class avaMissionTriggerVolume extends TriggerVolume;

var() vector LocationOffset;

function vector GetMissionLocation()
{
	return Location + LocationOffset;
}

defaultproperties
{
	LocationOffset=(X=0,Y=0,Z=0)
}