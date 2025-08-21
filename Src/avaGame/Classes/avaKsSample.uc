// 20070417 dEAthcURe

class avaKsSample extends avaKismetState
	native	
	placeable;
	
var hmserialize int sampleInt;

replication
{	
	if (Role == ROLE_Authority )
		sampleInt;
}
	
function Init()
{
	`log("%%%%%%%%%% avaKsSample initialized");
}

event function setup()
{
	`log("%%%%%%%%%% avaKsSample setup event");
}

event function HmActivate(bool _bActivate = True, string _eventName = "N/A")
{
	`log("[dEAthcURe|avaKsSample::HmActivate] ");
	Super.HmActivate(_bActivate, _eventName);
}

simulated event function onHmBackup()
{
	Super.onHmBackup();
	`log("[dEAthcURe|avaKsSample::onHmBackup]");
}

event function onHmRestore()
{
	Super.onHmRestore();
	`log("[dEAthcURe|avaKsSample::onHmRestore]");
	
}

defaultproperties
{
}
