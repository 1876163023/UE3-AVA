/*=============================================================================
  avaVolume_KOTH
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
  
	2006/04/18 by OZ
		King Of The Hill 을 위한 Volume 이다.

=============================================================================*/
class avaVolume_KOTH extends avaMissionTriggerVolume;

var()	float			OccupationMaxTime;	// 점령하고 있어야 하는 시간이다
var		float			OccupationTime;		// 현재 점령하고 있는 시간이다.
var		bool			bStopOccupationTime;// 시간 Stop
var		bool			bOccupationDone;	// 점령 완료
var		int				OccupationTeam;		// 현재 KOTH Volume 을 소유하고 있는 Team
var		array<Pawn>		EUTouchList;		// 현재 KOTH Volume 안에 있는 Pawn 들
var		array<Pawn>		USSRTouchList;		// 현재 KOTH Volume 안에 있는 Pawn 들
var()	array<vector>	SimpleRegion;		// HUD 에 표시할 간단한 영역
var()	int				ActivateVolume[2];	// 각 진영에 대해서 Activate 되었나...

var()	ETeamType		IgnoreTeam;			// 무시할 Team....

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
	if ( Pawn( Other ) == None )					return;	// Pawn 이 아니다.

	nTeam = Other.GetTeamNum();
	if ( IgnoreTeam != TEAM_Unknown && nTeam == IgnoreTeam )	return;
	if ( EUTouchList.Find( Pawn( Other ) ) >= 0 )	return;	// 이미 들어 있는 Pawn 이다.
	if ( USSRTouchList.Find( Pawn( Other ) ) >= 0 )	return;	// 이미 들어 있는 Pawn 이다.

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
	if ( Pawn( Other ) == None )					return;	// Pawn 이 아니다.
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
	OccupationTeam			=	-1;		// 현재 KOTH Volume 을 소유하고 있는 Team
	EUTouchList.length		=	0;		// 현재 KOTH Volume 안에 있는 Pawn 들
	USSRTouchList.length	=	0;		// 현재 KOTH Volume 안에 있는 Pawn 들
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
			if ( OccupationTeam >= 0 && bStopOccupationTime == false )	// 점령하고 있는 Team 이 있어야 의미가 있잖아.... UpdateTouch 가 인원변동이 있어야만 불리는게 아니게 되었음...
				OccupationStop();
		}
		else	// Hill 에 EU 는 있지만 USSR 은 없는 경우 시작
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
			if ( OccupationTeam >= 0)	// 점령하고 있는 Team 이 있어야 의미가 있다...
				ResetOccupationTeam();	// Hill에 EU 도 없고 USSR 도 없다.
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