class avaReplicatableEmitter extends Emitter;

simulated event PostBeginPlay()
{
	Super.PostBeginPlay();
	if ( bCurrentlyActive == FALSE )
	{
		NetUpdateFrequency = 0.1;
	}
}

simulated function OnToggle(SeqAct_Toggle action)
{
	Super.OnToggle( action );
	if ( bCurrentlyActive == TRUE )
	{
		NetUpdateFrequency	= default.NetUpdateFrequency;
		NetUpdateTime		= WorldInfo.TimeSeconds - 1.0f;
	}
	else
	{
		NetUpdateFrequency	= 0.1;
	}
}


defaultproperties
{
	RemoteRole=ROLE_SimulatedProxy
	bUpdateSimulatedPosition=true
	bReplicateMovement=true
	bSkipActorPropertyReplication=false
	bStatic=false
	bAlwaysRelevant=true
}
