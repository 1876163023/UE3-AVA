/*=============================================================================
  avaVolume_KOTH
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
  
	2006/06/29 by OZ

	Ȯ��� King Of The Hill Rule �̴�.

=============================================================================*/
class avaVolume_KOTHEx extends avaVolume_KOTH;

var()	float	IncRate;		// �츮���� Dominance �� ��� OccupationTime �� �����ϴ� �ӵ�...
var()	float	DecRate;		// ������� Dominance �� ��� OccupationTime �� �������� �ӵ�...
var()	float	NeutralDecRate;	// �ƹ��� �������� �������� OccupationTime �� �������� �ӵ�....
var		int		DominanceTeam;	// ���� �켼�� Team

function OccupationStart( int nTeam )
{
	OccupationTeam		= nTeam;
	bStopOccupationTime = false;
	ActivateEvent( 'Start', OccupationTeam );

	// ���� ó������ ������ ����̴�....
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
	// Timer �� Reset �� �Ǵ°� �ƴϰ� �����...
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
