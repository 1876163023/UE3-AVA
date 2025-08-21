class avaVolume_TeamTrigger extends avaMissionTriggerVolume;

var()	ETeamType	TriggerTeam;		// Event 를 발생할 Team
var		array<Pawn>	TouchList;

event Tick( float DeltaTime )
{
	local avaPawn	AP;
	local int		TeamIndex;
	local int		PrvNumPlayerInside;

	if ( Role != ROLE_Authority )										return;
	if ( avaGameReplicationInfo( WorldInfo.GRI ).bWarmupRound == true )	return;

	PrvNumPlayerInside	=	TouchList.length;
	TouchList.length	=	0;

	ForEach TouchingActors(class'avaPawn', AP)
	{
		TeamIndex = AP.GetTeamNum();
		if ( ( TeamIndex == TriggerTeam ) && AP.Health > 0)
		{
			TouchList[TouchList.length] = AP;
		}
	}

	if ( TouchList.length != PrvNumPlayerInside )
	{
		if ( TouchList.length > 0 )
		{	
			if ( PrvNumPlayerInside == 0 )
			{	
				// On Activate
				ActivateEvent( 'Activate' );
			}
			else
			{
				// On Change
				ActivateEvent( 'CountChange' );
			}	
		}
		else
		{
			// On Deactivate
			ActivateEvent( 'Deactivate' );
		}
	}
}

function ActivateEvent( name EventName )
{
	local avaSeqEvent_TeamTrigger	eventTeamTrigger;
	local int						i;
	for ( i = 0 ; i < GeneratedEvents.length ; ++ i )
	{
		eventTeamTrigger = avaSeqEvent_TeamTrigger( GeneratedEvents[i] );
		if ( eventTeamTrigger != None )
		{
			eventTeamTrigger.Trigger( EventName, TouchList );
		}
	}
}

//! RisingDust, 수리 후 호출.(2007/10/01 고광록)
function OnRefresh(avaSeqAct_Refresh action)
{
	// 강제로 이벤트가 발생해도록 해준다.
	if ( TouchList.length > 0 )
	{	
		// On Activate
		ActivateEvent( 'Activate' );
	}
	else
	{
		// On Deactivate
		ActivateEvent( 'Deactivate' );
	}
}

defaultproperties
{
	TickGroup						=	TG_PreAsyncWork
	bColored						=	true
	BrushColor						=	(R=255,G=0,B=0,A=255)
	bStatic							=	false
	bSkipActorPropertyReplication	=	false
	RemoteRole						=	ROLE_SimulatedProxy
	bStasis							=	false
	TriggerTeam						=	TEAM_Unknown
	SupportedEvents.Add(class'avaSeqEvent_TeamTrigger')
}

