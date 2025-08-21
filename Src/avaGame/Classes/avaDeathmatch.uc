/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaDeathmatch extends avaGame;

var bool	bInitPawnClass;
var int		nPawnClass;

// DeathMatch 의 경우 TIme Over 시 그냥 끝내도록 한다...
function TriggerTimeOverEvent()
{
	Super.TriggerTimeOverEvent();
	avaGameReplicationInfo( WorldInfo.GRI ).HostReportEndGameToServer();
}

function CheckPrivateScore( avaPlayerController ScorePlayer, int nScore )
{
	if ( nScore >= avaGameReplicationInfo( WorldInfo.GRI ).WinCondition )
	{
		TriggerAttainScore( ScorePlayer );
		avaGameReplicationInfo( WorldInfo.GRI ).HostReportEndGameToServer();
	}
}

function TriggerAttainScore( avaPlayerController ScorePlayer )
{
	local Sequence					GameSeq;
	local array<SequenceObject>		AllSeqEvents;
	local int						i;
	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != none )
	{
		GameSeq.FindSeqObjectsByClass( class'avaSeqEvent_DMAttainScore', true, AllSeqEvents );
		//	activate them
		for ( i=0; i<AllSeqEvents.Length; ++i )
		{
			avaSeqEvent_DMAttainScore( AllSeqEvents[i] ).Trigger( ScorePlayer );
		}
	}
}

function ScoreKill(Controller Killer, Controller Other)
{
	local PlayerReplicationInfo		OtherPRI;
	local int						Score;
	if ( !IsValidRoundState() )	return;
	OtherPRI = Other.PlayerReplicationInfo;
    if ( OtherPRI != None )
    {
		++OtherPRI.NumLives;
		if ( (MaxLives > 0) && (OtherPRI.NumLives >=MaxLives) )
			OtherPRI.bOutOfLives = true;
    }

	if ( Killer == None || Killer == Other )														// 자살한 경우
	{
		Other.PlayerReplicationInfo.Score -= 1;
		Other.PlayerReplicationInfo.NetUpdateTime = WorldInfo.TimeSeconds - 1;
	}
	else
	{
		Score = 1;
		Killer.PlayerReplicationInfo.AddScore( Score );
	}
}

function class<Pawn> GetDefaultPlayerClass(Controller C)
{
	local avaPlayerReplicationInfo avaPRI;

	if ( bInitPawnClass == false )
	{
		bInitPawnClass	= true;
		nPawnClass		= Rand(2);
	}

	avaPRI = avaPlayerReplicationInfo( C.PlayerReplicationInfo );
	if ( nPawnClass == 0 )
		return class<avaPawn>(DynamicLoadObject(EUPawnClassName[avaPRI.PlayerClassID],class'class'));
	else
		return class<avaPawn>(DynamicLoadObject(NRFPawnClassName[avaPRI.PlayerClassID],class'class'));
}

// 증원 처리...
function Reinforce()
{
	local avaGameReplicationInfo	avaGRI;
	local PlayerController PC;

	avaGRI = avaGameReplicationInfo( GameReplicationInfo );
	if ( avaGRI.ReinforcementFreq[0] <= 0 )
	{
		return;
	}

	foreach WorldInfo.AllControllers( class'PlayerController', PC )
	{
		if ( avaPlayerController( PC ).CanRestart() )
		{
			if ( ( ElapsedTime - avaPlayerReplicationInfo( PC.PlayerReplicationInfo ).LastDeathTime ) % avaGRI.ReinforcementFreq[0] == 0 )
				RestartPlayer( PC );
		}
	}
}

function Logout(controller Exiting)
{	
	// WarmUp 중이고 아직 Loading 이 끝나지 않은 사람이 있다면 게임을 끝내지 말자...
	if ( HasUnreadyPlayer() == false || !IsWarmUpRound() )
	{
		// Game 이 계속 될 수 있는 지 Check 한다...
		CheckGameGoOn( Exiting );
	}
	avaGameReplicationInfo(GameReplicationInfo).PlayerOut( Exiting );	
	Super.Logout(Exiting);
}

event function bool CheckGameGoOn( Controller except )
{
	local PlayerController			PC;
	local int						nCnt;
	if ( IsPracticeMode() )			return true;	// 연습채널 모드이면 AllOut Event 를 Check하지 않는다....
	// 대회용 방일 경우에는 한쪽 진영이 다 나가더라도 게임이 계속 진행된다...
	if ( class'avaNetHandler'.static.GetAvaNetHandler().IsMatchRoom() )	return true;

	// 상대편이 있는지 Check 한다....
	foreach WorldInfo.AllControllers( class'PlayerController', PC )
	{
		if ( PC == except )			continue;
		++nCnt;
	}
	if ( nCnt == 1 )
	{
		TriggerAllOutEvent( 0 );
		return false;
	}
	return true;
}

/** ChoosePlayerStart()
* Return the 'best' player start for this player to start from.  PlayerStarts are rated by RatePlayerStart().
* @param Player is the controller for whom we are choosing a playerstart
* @param InTeam specifies the Player's team (if the player hasn't joined a team yet)
* @returns NavigationPoint chosen as player start (usually a PlayerStart)
 */
function PlayerStart ChoosePlayerStart( Controller Player, optional byte InTeam )
{
	local PlayerStart			P, BestStart;
	local float					BestRating, NewRating;
	local array<playerstart>	PlayerStarts;
	local int					i, RandStart;
	foreach WorldInfo.AllNavigationPoints(class'PlayerStart', P)
	{
		if ( P.bEnabled )
		{
			PlayerStarts[PlayerStarts.Length] = P;
		}
	}

	RandStart = Rand(PlayerStarts.Length);
	BestStart = PlayerStarts[RandStart];
	for ( i = RandStart ; i < PlayerStarts.Length ; i++ )
	{
		P = PlayerStarts[i];
		NewRating = RatePlayerStart(P,255,Player);
		if ( NewRating > BestRating )
		{
			BestRating = NewRating;
			BestStart = P;
		}
	}
	for ( i = 0 ; i < RandStart ; i++ )
	{
		P = PlayerStarts[i];
		NewRating = RatePlayerStart(P,255,Player);
		if ( NewRating > BestRating )
		{
			BestRating = NewRating;
			BestStart = P;
		}
	}
	return BestStart;
}

/** RatePlayerStart()
* Return a score representing how desireable a playerstart is.
* @param P is the playerstart being rated
* @param Team is the team of the player choosing the playerstart
* @param Player is the controller choosing the playerstart
* @returns playerstart score
*/
function float RatePlayerStart(PlayerStart P, byte Team, Controller Player)
{
    local float			NewDist, NearestDist;
    local Controller	OtherPlayer;

	NearestDist	=	-1.0;
	ForEach WorldInfo.AllControllers(class'Controller', OtherPlayer)
    {
		if ( OtherPlayer.bIsPlayer && (OtherPlayer.Pawn != None) )
		{
			NewDist = VSize(OtherPlayer.Pawn.Location - P.Location);
			if ( NewDist < NearestDist || NearestDist < 0.0 )
			{
				NearestDist = NewDist;
			}
		}
    }
    return NearestDist;
}

defaultproperties
{
    MapPrefix="DM"
	DefaultMaxLives	=	1
	SpawnAllowTime	=	1		//1 초로 설정...
	WarmupTime		=	5.0
	bStopCountDown	=	false
	bTeamGame		=	false

	SpawnProtectionTime	=	3.0
}
