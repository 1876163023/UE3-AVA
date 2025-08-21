/*=============================================================================
  avaVolume_KOTH
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
  
	2006/04/18 by OZ
		King Of The Hill �� ���� Volume �̴�.

=============================================================================*/
class avaVolume_KOTH extends avaMissionTriggerVolume;

var()	float			OccupationMaxTime;	// �����ϰ� �־�� �ϴ� �ð��̴�
var		float			OccupationTime;		// ���� �����ϰ� �ִ� �ð��̴�.
var		bool			bStopOccupationTime;// �ð� Stop
var		bool			bOccupationDone;	// ���� �Ϸ�
var		int				OccupationTeam;		// ���� KOTH Volume �� �����ϰ� �ִ� Team
var		array<Pawn>		EUTouchList;		// ���� KOTH Volume �ȿ� �ִ� Pawn ��
var		array<Pawn>		USSRTouchList;		// ���� KOTH Volume �ȿ� �ִ� Pawn ��
var()	array<vector>	SimpleRegion;		// HUD �� ǥ���� ������ ����
var()	int				ActivateVolume[2];	// �� ������ ���ؼ� Activate �Ǿ���...

var()	ETeamType		IgnoreTeam;			// ������ Team....

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	if ( WorldInfo.GRI != None )
	{
		avaGameReplicationInfo( WorldInfo.GRI ).SetMissionMaxTime( OccupationMaxTime );
		avaGameReplicationInfo( WorldInfo.GRI ).SetMissionIndicatorIdx( 2 );
	}
}

event Touch(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	local int nTeam;

	if ( avaGameReplicationInfo( WorldInfo.GRI ).bWarmupRound == true )	return;
	
	if ( bOccupationDone )							return;
	if ( Pawn( Other ) == None )					return;	// Pawn �� �ƴϴ�.

	nTeam = Other.GetTeamNum();
	if ( IgnoreTeam != TEAM_Unknown && nTeam == IgnoreTeam )	return;
	if ( EUTouchList.Find( Pawn( Other ) ) >= 0 )	return;	// �̹� ��� �ִ� Pawn �̴�.
	if ( USSRTouchList.Find( Pawn( Other ) ) >= 0 )	return;	// �̹� ��� �ִ� Pawn �̴�.

	if ( nTeam == 0 )		EUTouchList[ EUTouchList.length ] = Pawn(Other);
	else if ( nTeam == 1 )	USSRTouchList[ USSRTouchList.length ] = Pawn(Other);
	UpdateTouch();
}

event UnTouch( Actor Other )
{
	local int nIdx;
	local int nTeam;

	if ( avaGameReplicationInfo( WorldInfo.GRI ).bWarmupRound == true )	return;

	if ( bOccupationDone )							return;
	if ( Pawn( Other ) == None )					return;	// Pawn �� �ƴϴ�.
	nTeam = Other.GetTeamNum();
	if ( IgnoreTeam != TEAM_Unknown && nTeam == IgnoreTeam )	return;
	nIdx = EUTouchList.Find( Pawn( Other ) );
	if ( nIdx >= 0 )
	{
		EUTouchList.Remove( nIdx, 1 );
	}
	else
	{
		nIdx = USSRTouchList.Find( Pawn( Other ) );
		if ( nIdx >= 0 )	USSRTouchList.Remove( nIdx, 1 );
	}
	UpdateTouch();
}

simulated event Tick( float DeltaTime )
{
	if ( Role != ROLE_Authority )					return;
	if ( bOccupationDone )							return;
	if ( !bStopOccupationTime && OccupationTeam >= 0 )
	{
		if ( ActivateVolume[OccupationTeam] != 0 )
			OccupationTime += DeltaTime;
	}
	if ( OccupationTime > OccupationMaxTime )
		OccupationSuccess( OccupationTeam );
}

function Reset()
{
	OccupationTime			=	0.0;
	bStopOccupationTime		=	false;
	bOccupationDone			=	false;
	OccupationTeam			=	-1;		// ���� KOTH Volume �� �����ϰ� �ִ� Team
	EUTouchList.length		=	0;		// ���� KOTH Volume �ȿ� �ִ� Pawn ��
	USSRTouchList.length	=	0;		// ���� KOTH Volume �ȿ� �ִ� Pawn ��
	avaGameReplicationInfo( WorldInfo.GRI ).ResetMissionTime();
	avaGameReplicationInfo( WorldInfo.GRI ).SetMissionIndicatorIdx( 2 );
}

function ResetOccupationTeam()
{
	OccupationTeam		= -1;
	OccupationTime		=  0;
	bStopOccupationTime = false;
	ActivateEvent( 'Reset', -1 );
	avaGameReplicationInfo( WorldInfo.GRI ).ResetMissionTime();
	avaGameReplicationInfo( WorldInfo.GRI ).SetMissionIndicatorIdx( 2 );
}

function OccupationStart( int nTeam )
{
	OccupationTeam		= nTeam;
	OccupationTime		= 0;
	bStopOccupationTime = false;
	ActivateEvent( 'Start', OccupationTeam );
	avaGameReplicationInfo( WorldInfo.GRI ).StartMissionTime( OccupationTeam, OccupationMaxTime, 0.0  );
}

function OccupationRestart()
{
	bStopOccupationTime = false;
	// need Restart Sequence Event?
	avaGameReplicationInfo( WorldInfo.GRI ).RestartMissionTime();
}

function OccupationStop()
{
	ActivateEvent( 'Stop', OccupationTeam );
	bStopOccupationTime = true;
	avaGameReplicationInfo( WorldInfo.GRI ).StopMissionTime();
}

function OccupationSuccess( int nTeam )
{
	ActivateEvent( 'Success', nTeam );
	OccupationTime		= OccupationMaxTime;
	bStopOccupationTime = true;
	bOccupationDone		= true;
	avaGameReplicationInfo( WorldInfo.GRI ).EndMissionTime();
}

function UpdateTouch()
{
	local int EUTouchCnt, USSRTouchCnt;
	EUTouchCnt		= EUTouchList.length;
	USSRTouchCnt	= USSRTouchList.length;
	if ( EUTouchCnt > 0 )
	{
		if ( USSRTouchCnt > 0 )		
		{
			if ( OccupationTeam >= 0 && bStopOccupationTime == false )	// �����ϰ� �ִ� Team �� �־�� �ǹ̰� ���ݾ�.... UpdateTouch �� �ο������� �־�߸� �Ҹ��°� �ƴϰ� �Ǿ���...
				OccupationStop();
		}
		else	// Hill �� EU �� ������ USSR �� ���� ��� ����
		{
			if ( ActivateVolume[TEAM_EU] == 1 )
			{
				if ( OccupationTeam != TEAM_EU )	OccupationStart(TEAM_EU);
				else if ( bStopOccupationTime )		OccupationRestart();
			}
		}
	}
	else
	{
		if ( USSRTouchCnt > 0 )
		{
			if ( ActivateVolume[TEAM_USSR] == 1 )
			{
				if ( OccupationTeam != TEAM_USSR )	OccupationStart(TEAM_USSR);
				else if ( bStopOccupationTime )		OccupationRestart();
			}
		}
		else
		{
			if ( OccupationTeam >= 0)	// �����ϰ� �ִ� Team �� �־�� �ǹ̰� �ִ�...
				ResetOccupationTeam();	// Hill�� EU �� ���� USSR �� ����.
		}
	}

	if ( ( ActivateVolume[TEAM_EU] == 0 && OccupationTeam == TEAM_EU ) ||
		 ( ActivateVolume[TEAM_USSR] == 0 && OccupationTeam == TEAM_USSR ) )
		 OccupationStop();
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

function ActivateTeam( int nTeam, bool bFlag )
{
	if ( nTeam >= 0 && nTeam <= 1 )
	{
		if ( bFlag == true )
			ActivateVolume[nTeam] = 1;
		else
			ActivateVolume[nTeam] = 0;
	}
	UpdateTouch();
}

defaultproperties
{
	TickGroup=TG_PreAsyncWork
	bColored=true
	BrushColor=(R=255,G=0,B=0,A=255)
	OccupationTeam = -1
	SupportedEvents.Add(class'avaSeqEvent_KOTH')
	bStatic=false
	bSkipActorPropertyReplication=false
	RemoteRole=ROLE_SimulatedProxy
	bStasis=false

	ActivateVolume[0]	= 1
	ActivateVolume[1]	= 1
	IgnoreTeam			= TEAM_Unknown
}