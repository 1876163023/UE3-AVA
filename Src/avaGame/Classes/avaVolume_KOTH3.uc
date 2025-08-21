class avaVolume_KOTH3 extends avaMissionTriggerVolume;

var()	float			TimeLimit;
var()	ETeamType		IgnoreTeam;				// 무시할 Team....
var		bool			bDone;

simulated event Tick( float DeltaTime )
{	
	local avaPawn		AP;
	local int			TeamIndex;
	local avaGameReplicationInfo GRI;

	if ( Role != ROLE_Authority )					return;

	// 이미 끝났으면 땡!
	if ( bDone ) return;

	GRI = avaGameReplicationInfo(WorldInfo.GRI);	

	GRI.KOTH3[0].NumPlayersInside = 0;
	GRI.KOTH3[1].NumPlayersInside = 0;	
	
	ForEach TouchingActors(class'avaPawn', AP)
	{
		TeamIndex = AP.GetTeamNum();

		if ((TeamIndex == 0 || TeamIndex == 1) && (IgnoreTeam == TEAM_Unknown || TeamIndex != IgnoreTeam) && AP.Health > 0)
		{
			GRI.KOTH3[TeamIndex].NumPlayersInside++;
		}
	}

	for (TeamIndex=0; TeamIndex<2; TeamIndex++)
	{
		if (GRI.KOTH3[TeamIndex].NumPlayersInside > 0)
		{
//			`log( "KOTH3" @ GRI.KOTH3[TeamIndex].TimeRemains  );

			if (GRI.KOTH3[TeamIndex].TimeRemains > 0 )
			{
				if (GRI.KOTH3[TeamIndex].TimeRemains == TimeLimit)
				{
					OnStart(TeamIndex);
				}

				GRI.KOTH3[TeamIndex].TimeRemains -= DeltaTime;

				if (GRI.KOTH3[TeamIndex].TimeRemains < 0)
				{
					OnFinish(TeamIndex);

					GRI.KOTH3[TeamIndex].TimeRemains = 0;
				}
			}
		}
		else
		{
			if (GRI.KOTH3[TeamIndex].TimeRemains < TimeLimit)
			{
				OnAbort(TeamIndex);

				GRI.KOTH3[TeamIndex].TimeRemains = TimeLimit;
			}
		}
	}
}

function OnStart( int TeamIndex )
{
	ActivateEvent( 'Start', TeamIndex );
}

function OnFinish( int TeamIndex )
{
	ActivateEvent( 'Success', TeamIndex );

	bDone = true;
}

function OnAbort( int TeamIndex )
{
	ActivateEvent( 'Reset', TeamIndex );
}

function Reset()
{
	local int TeamIndex;
	local avaGameReplicationInfo GRI;	

	GRI = avaGameReplicationInfo(WorldInfo.GRI);	

	for (TeamIndex=0; TeamIndex<2; TeamIndex++)
	{
		GRI.KOTH3[TeamIndex].NumPlayersInside = 0;	// 각 진영별 있는 사람 수
		GRI.KOTH3[TeamIndex].TimeRemains = TimeLimit;			// 현재까지 획득한 시간
	}

	bDone = false;
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	Reset();
}

function ActivateEvent( name EventName, int nTeam )
{
	local	avaSeqEvent_KOTH	eventKOTH;
	local	int i;

	for ( i=0 ; i<GeneratedEvents.Length; ++i )
	{
		eventKOTH = avaSeqEvent_KOTH( GeneratedEvents[i] );
		if ( eventKOTH != none )
			eventKOTH.ActivateEvent( EventName, nTeam );
	}
}


defaultproperties
{
	TickGroup=TG_PreAsyncWork
	bColored=true
	BrushColor=(R=255,G=0,B=0,A=255)	
	SupportedEvents.Add(class'avaSeqEvent_KOTH')
	bStatic=false
	bSkipActorPropertyReplication=false
	RemoteRole=ROLE_SimulatedProxy
	bStasis=false
	
	IgnoreTeam			= TEAM_Unknown
}