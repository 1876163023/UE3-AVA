/*=============================================================================
  avaVolume_KOTH
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
  
	2006/06/29 by OZ

	확장된 King Of The Hill Rule 이다.

=============================================================================*/
class avaVolume_KOTHEx extends avaVolume_KOTH;

var()	float	IncRate;		// 우리편이 Dominance 일 경우 OccupationTime 이 증가하는 속도...
var()	float	DecRate;		// 상대편이 Dominance 일 경우 OccupationTime 이 떨어지는 속도...
var()	float	NeutralDecRate;	// 아무도 점령하지 못했을때 OccupationTime 이 떨어지는 속도....
var		int		DominanceTeam;	// 현재 우세한 Team

function OccupationStart( int nTeam )
{
	OccupationTeam		= nTeam;
	bStopOccupationTime = false;
	ActivateEvent( 'Start', OccupationTeam );

	// 제일 처음으로 점령한 경우이다....
	if ( DominanceTeam == -1 )	DominanceTeam = OccupationTeam;

	if ( DominanceTeam == OccupationTeam )
		avaGameReplicationInfo( WorldInfo.GRI ).StartMissionTime( OccupationTeam, OccupationTime, OccupationMaxTime, IncRate, DominanceTeam );
	else
		avaGameReplicationInfo( WorldInfo.GRI ).StartMissionTime( OccupationTeam, OccupationTime, 0.0, DecRate, DominanceTeam );
}

function ResetOccupationTeam()
{
	OccupationTeam		= -1;
	bStopOccupationTime = false;
	ActivateEvent( 'Reset', -1 );
	// Timer 가 Reset 이 되는게 아니고 멈춘다...
	if ( OccupationTime > 0 )
		avaGameReplicationInfo( WorldInfo.GRI ).StartMissionTime( OccupationTeam, OccupationTime, 0.0, NeutralDecRate, DominanceTeam );
	else
		avaGameReplicationInfo( WorldInfo.GRI ).StopMissionTime();
	
	avaGameReplicationInfo( WorldInfo.GRI ).SetMissionIndicatorIdx( 2 );
}

simulated event Tick( float DeltaTime )
{
	if ( Role != ROLE_Authority )	return;
	if ( bOccupationDone )			return;
	if ( !bStopOccupationTime )
	{
		if ( OccupationTeam >= 0 ) 
		{
			if ( ActivateVolume[OccupationTeam] != 0 )
			{
				if ( DominanceTeam == OccupationTeam )
				{
					OccupationTime += DeltaTime * IncRate;
					if ( OccupationTime > OccupationMaxTime )
						OccupationSuccess( OccupationTeam );
				}
				else
				{
					OccupationTime -= DeltaTime * DecRate;
					if ( OccupationTime < 0 )
					{
						OccupationTime	= -OccupationTime;
						DominanceTeam	= OccupationTeam;
						avaGameReplicationInfo( WorldInfo.GRI ).StartMissionTime( OccupationTeam, OccupationTime, OccupationMaxTime, IncRate, DominanceTeam );
					}
				}
			}
		}
		else
		{
			if ( OccupationTime > 0 )
			{
				OccupationTime -= DeltaTime * NeutralDecRate;
				if ( OccupationTime < 0 )
				{
					OccupationTime	= 0;
					DominanceTeam	= -1;
					avaGameReplicationInfo( WorldInfo.GRI ).StopMissionTime();
				}
			}
		}
	}
}

function Reset()
{
	DominanceTeam	= -1;
	Super.Reset();
}

defaultproperties
{
	DominanceTeam	= -1
	IncRate			= 1.0
	DecRate			= 1.0
	NeutralDecRate	= 0.25
}
