// 20070417 dEAthcURe

class avaKismetState extends Actor	
	native	
	placeable;
	
var hmserialize repnotify bool bActivated;
var hmserialize repnotify string eventName;
//var hmserialize repnotify string events[16]; // 20070420 static string array test

replication
{	
	if (Role == ROLE_Authority )
		bActivated, eventName; // , events; // 20070420 static string array test
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'bActivated' || VarName == 'eventName')
	{
		`log("          <---------- avaKismetStateActor::bActivated,eventName replicated");
	}
	
	/* 20070420 static string array test
	if(VarName == 'events') {
		for(lpp=0;lpp<16;lpp++) {
			`log("ReplicatedEvent for events" @ lpp @ events[lpp]);
		}	
	}
	*/	
}
	
function ActivateRemoteEvent( string _EventName )
{
	// code from avaNetEntryGameEx.uc
	
	local Sequence					GameSeq;
	local int						i;
	local array<SequenceObject>		AllSeqEvents;
	local SeqEvent_RemoteEvent		RemoteEvent;

	// World상의 Level Kismet의 Sequence를 얻는다.
	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != None )
	{
		// RemoteEvent를 모두 찾아서.
		GameSeq.FindSeqObjectsByClass( class'SeqEvent_RemoteEvent', true, AllSeqEvents );

		for ( i = 0; i < AllSeqEvents.Length; ++ i )
		{
			RemoteEvent = SeqEvent_RemoteEvent( AllSeqEvents[i] );

			if ( RemoteEvent == None )
				continue;

			if ( RemoteEvent.bEnabled != true )
				continue;

			// 발생하고 싶은 이벤트 이름의 RemoteEvent만 발생시킨다.
			if( string(RemoteEvent.EventName) == _EventName )
			{
				`log("### RemoteEvent - " @ _EventName @"###");
				RemoteEvent.CheckActivate( WorldInfo, None );
			}
		}
	}

}

event function HmActivate(bool _bActivate = True, string _eventName = "N/A")
{
	bActivated = _bActivate;
	eventName = _eventName;
	NetUpdateTime	= WorldInfo.TimeSeconds - 1; // Force replication
	
	`log("          <---------- [dEAthcURe|avaKismetState::HmActivate] " @ bActivated @ eventName);
	
	/* 20070420 static string array test
	for(lpp=0;lpp<16;lpp++) {
		events[lpp] = "event" @ lpp;	
		`log(events[lpp]);
	}
	*/
}

simulated event function onHmBackup()
{
	`log("[dEAthcURe|avaKismetState::onHmBackup]");
}

event function onHmRestore()
{
	`log("[dEAthcURe|avaKismetState::onHmRestore]" @ bActivated @ eventName);
	if(bActivated) ActivateRemoteEvent(eventName);	
	
	/* 20070420 static string array test
	for(lpp=0;lpp<16;lpp++) {
		`log(events[lpp]);
	}
	*/
}

function Reset()
{
	`log("          <---------- [dEAthcURe|avaKismetState::Reset]");
	bActivated = false;
	eventName = "N/A";
}

defaultproperties
{
	bStatic=false
	bAlwaysRelevant=true
	bSkipActorPropertyReplication=false
	bUpdateSimulatedPosition=false
	bReplicateMovement=false
	RemoteRole=ROLE_SimulatedProxy
	
	bActivated = false;
	eventName = "N/A";	
}
