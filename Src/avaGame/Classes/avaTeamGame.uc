/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaTeamGame extends avaGame;

`include(avaGame/avaGame.uci)

var				avaTeamInfo	Teams[2];
var				class<avaTeamAI> TeamAIType[2];

var				float		FriendlyFireScale;		// scale friendly fire damage by this value - default : 0.3
var				int			bSwapRule;				// true 이면 Team Swap Rule 을 적용한다....
var				int			TestBroadcast;
//var				BYTE		MatchMode;
var	localized	string		RealTeamNames[2];
var				string		TeamSymbolName[2];
var				int			FriendlyFireType;
var				int			AllowThirdPersonCam;
var				int			AllowGhostChat;

function PreBeginPlay()
{
	Super.PreBeginPlay();
	CreateTeam(0);
	CreateTeam(1);
	Teams[0].AI.EnemyTeam = Teams[1];
	Teams[1].AI.EnemyTeam = Teams[0];
}

function class<Pawn> GetDefaultPlayerClass(Controller C)
{
	local avaPlayerReplicationInfo avaPRI;
	avaPRI = avaPlayerReplicationInfo( C.PlayerReplicationInfo );
	if( avaPRI == None )						return DefaultPawnClass;
	if ( avaPRI.Team == None )					return class'avaCharacter';
	if ( !avaPRI.IsValidClassType() )			return DefaultPawnClass;
	if ( avaPRI.Team.TeamIndex == 0 )
	{
		return class<avaPawn>(DynamicLoadObject(EUPawnClassName[avaPRI.PlayerClassID],class'class'));
	}
	else if ( avaPRI.Team.TeamIndex == 1 )
	{
		return class<avaPawn>(DynamicLoadObject(NRFPawnClassName[avaPRI.PlayerClassID],class'class'));
	}
	return DefaultPawnClass;
}

function int CheckWinTeam()
{
	return nWinTeam;
}

function CreateTeam(int TeamIndex )
{
	Teams[TeamIndex]					= spawn( class'avaTeamInfo' );	
	Teams[TeamIndex].TeamIndex			= TeamIndex;	
	Teams[TeamIndex].RealTeamName		= RealTeamNames[TeamIndex];
	GameReplicationInfo.Teams[TeamIndex]= Teams[TeamIndex];	
	Teams[TeamIndex].TeamSymbolName		= TeamSymbolName[TeamIndex];

	// Create TeamAI.
	Teams[TeamIndex].AI = Spawn(TeamAIType[TeamIndex]);
	Teams[TeamIndex].AI.Team = Teams[TeamIndex];
	Teams[TeamIndex].AI.SetObjectiveLists();
}

event InitGame( string Options, out string ErrorMessage )
{
	Super.InitGame(Options, ErrorMessage);
	FriendlyFireType	= GetIntOption( Options, "FFType", 0 );
	AllowThirdPersonCam	= GetIntOption( Options, "TPCam", 0 );
	AllowGhostChat		= GetIntOption( Options, "GhostChat", 1 );
	bSwapRule			= GetIntOption( Options, "SwapRule", 0 );
	TestBroadcast		= GetIntOption( Options, "TestBR", 0 );
	//MatchMode			= GetIntOption( Options, "MatchMode", 0 );

	if ( class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentChannelFlag() == EChannelFlag_Clan )
		class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentRoomsClanNames( TeamSymbolName[0], TeamSymbolName[1] );
}

function InitGameReplicationInfo()
{
	Super.InitGameReplicationInfo();
	avaGameReplicationInfo( GameReplicationInfo ).FriendlyFireType		= FriendlyFireType;
	avaGameReplicationInfo( GameReplicationInfo ).bAllowThirdPersonCam	= bool( AllowThirdPersonCam );
	avaGameReplicationInfo( GameReplicationInfo ).bEnableGhostChat		= bool( AllowGhostChat );
	avaGameReplicationInfo( GameReplicationInfo ).bSwapRule				= bool( bSwapRule );
	avaGameReplicationInfo( GameReplicationInfo ).bTestBroadcast		= bool( TestBroadcast );
	//avaGameReplicationInfo( GameReplicationInfo ).MatchMode				= MatchMode;
}

function NotifyKilled(Controller Killer, Controller KilledPlayer, Pawn KilledPawn)
{	
}

function SetEndGameFocus(PlayerReplicationInfo Winner)
{
	local Controller P;

	if ( Winner != None )
		EndGameFocus = Controller(Winner.Owner).Pawn;
	if ( EndGameFocus != None )
		EndGameFocus.bAlwaysRelevant = true;
	foreach WorldInfo.AllControllers(class'Controller', P)
	{
		P.GameHasEnded( EndGameFocus, (P.PlayerReplicationInfo != None) && (P.PlayerReplicationInfo.Team == GameReplicationInfo.Winner) );
	}
}

function byte PickTeam(byte num, Controller C)
{
	// Team 이 정해지지 않은 경우, 정상적인 경우라면 나올 수 없는 경우이다...
	if ( num < 0 || num >= 2 )
	{
		if ( Teams[0].Size > Teams[1].Size )	return 1;
		else									return 0;
	}
	return num;
}

function bool CanSpectate( PlayerController Viewer, PlayerReplicationInfo ViewTarget )
{
	if ( (ViewTarget == None) || ViewTarget.bOnlySpectator )
		return false;
	return ( Viewer.PlayerReplicationInfo.bOnlySpectator || (ViewTarget.Team == Viewer.PlayerReplicationInfo.Team) );
}

/** ChangeTeam()
* verify whether controller Other is allowed to change team, and if so change his team by calling SetTeam().
* @param Other:  the controller which wants to change teams
* @param num:  the teamindex of the desired team.  If 255, pick the smallest team.
* @param bNewTeam:  if true, broadcast team change notification
*/
function bool ChangeTeam(Controller Other, int num, bool bNewTeam)
{
	local avaTeamInfo NewTeam;

	// don't add spectators to teams
	if ( Other.IsA('PlayerController') && Other.PlayerReplicationInfo.bOnlySpectator )
	{
		Other.PlayerReplicationInfo.Team = None;
		return true;
	}

	NewTeam = (num < 255) ? Teams[PickTeam(num,Other)] : None;

	// check if already on this team
	if ( Other.PlayerReplicationInfo.Team == NewTeam )
		return false;

	// set the new team for Other
	SetTeam(Other, NewTeam, bNewTeam);

	// User 가 게임 중간에 team 을 바꿀수 없기 때문에 Check 하지 않도록 한다...
	// CheckMassacre( num == 1 ? 0 : 1 );
	return true;
}

/** SetTeam()
* Change Other's team to NewTeam.
* @param Other:  the controller which wants to change teams
* @param NewTeam:  the desired team.  
* @param bNewTeam:  if true, broadcast team change notification
*/
function SetTeam(Controller Other, avaTeamInfo NewTeam, bool bNewTeam)
{
	local Actor A;

	`log("avaTeamGame - Bot" @avaBot(Other) @NewTeam @bNewTeam);

	if (Other.PlayerReplicationInfo.Team != None || !ShouldSpawnAtStartSpot(Other))
	{
		// clear the StartSpot, which was a valid start for his old team
		Other.StartSpot = None;
	}

	// remove the controller from his old team
	if ( Other.PlayerReplicationInfo.Team != None )
	{
		Other.PlayerReplicationInfo.Team.RemoveFromTeam(Other);
		Other.PlayerReplicationInfo.Team = none;
	}

	if ( NewTeam==None || (NewTeam!= none && NewTeam.AddToTeam(Other)) )
	{
		// 게임에서 Team 을 변경할 이유가 없기 때문에 Message 를 보내줄 필요가 없다...
		//if ( (NewTeam!=None) && ((WorldInfo.NetMode != NM_Standalone) || (PlayerController(Other) == None) || (PlayerController(Other).Player != None)) )
		//	BroadcastLocalizedMessage( GameMessageClass, 3, Other.PlayerReplicationInfo, None, NewTeam );
		if ( bNewTeam && PlayerController(Other)!=None )
			GameEvent('TeamChange',NewTeam.TeamIndex,Other.PlayerReplicationInfo);
	}

	if ( (PlayerController(Other) != None) && (LocalPlayer(PlayerController(Other).Player) != None) )
	{
		// if local player, notify level actors
		ForEach AllActors(class'Actor', A)
			A.NotifyLocalPlayerTeamReceived();
	}
	avaGameReplicationInfo( GameReplicationInfo ).RecalcHighestUser();
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
	local float BestRating,		NewRating;
	local array<playerstart>	PlayerStarts;
	local int					i, RandStart;
	local byte					Team;
	local avaTeamPlayerStart	TP;

	// use InTeam if player doesn't have a team yet
	Team = ( (Player != None) && (Player.PlayerReplicationInfo != None) && (Player.PlayerReplicationInfo.Team != None) )
			? byte(Player.PlayerReplicationInfo.Team.TeamIndex)
			: InTeam;

	// make array of enabled playerstarts
	foreach WorldInfo.AllNavigationPoints(class'PlayerStart', P)
	{
		// team 기능이 없는 것은 사용하지 않는다.
		TP = avaTeamPlayerStart(P);
		if ( TP == None )					continue;
		if ( !P.bEnabled )					continue;
		if ( Team != TP.TeamNumber )		continue;
		PlayerStarts[PlayerStarts.Length] = P;
	}

	if ( PlayerStarts.Length == 0 )
	{
		`warn( "no active playerstarts for this team "$Team );
		return super.ChoosePlayerStart( Player, InTeam );
	}

	// start at random point to randomize finding "good enough" playerstart
	RandStart = Rand(PlayerStarts.Length);
	for ( i=RandStart; i<PlayerStarts.Length; i++ )
	{
		P = PlayerStarts[i];
		NewRating = RatePlayerStart(P,Team,Player);
		if ( NewRating >= 30 )
		{
			// this PlayerStart is good enough
			return P;
		}
		if ( NewRating > BestRating )
		{
			BestRating = NewRating;
			BestStart = P;
		}
	}
	for ( i=0; i<RandStart; i++ )
	{
		P = PlayerStarts[i];
		NewRating = RatePlayerStart(P,Team,Player);
		if ( NewRating >= 30 )
		{
			// this PlayerStart is good enough
			return P;
		}
		if ( NewRating > BestRating )
		{
			BestRating = NewRating;
			BestStart = P;
		}
	}
	return BestStart;
}

function bool CheckScore(PlayerReplicationInfo Scorer)
{
	return false;
}

function ScoreKill(Controller Killer, Controller Other)
{
	// 다른 편을 죽이면 Score +1
	// 자살할 경우 Score -1
	// 같은 편을 죽이면 Score 0
	local PlayerReplicationInfo		OtherPRI;
	local avaPlayerReplicationInfo	LeaderPRI;
	local int						Score;
	local TeamInfo					Team;

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
	else if ( Killer.PlayerReplicationInfo.Team != Other.PlayerReplicationInfo.Team )				// 다른편을 죽인경우
	{
		Score = 1;
		Killer.PlayerReplicationInfo.AddScore( Score );
		// targetting 된 놈이 죽으면 SquadLeader 의 Score 와 Leader Point 를 욜려준다...
		if ( avaPawn( Other.Pawn ).bTargetted == true )
		{
			LeaderPRI = HasSquadLeader( Killer.PlayerReplicationInfo.Team.TeamIndex );
			if ( LeaderPRI != None && LeaderPRI != Killer.PlayerReplicationInfo )
			{
				LeaderPRI.AddPoint( PointType_Leader, 1 );
			}
		}
		Killer.PlayerReplicationInfo.NetUpdateTime = WorldInfo.TimeSeconds - 1;
	}

	Team = Other.PlayerReplicationInfo.Team;
	if ( Team.TeamIndex == 0 )			CheckMassacre( 0 );
	else if ( Team.TeamIndex == 1 )		CheckMassacre( 1 );
}

function CheckMassacre( int thisTeamFirst )
{
	// 연습채널 모드이면 전멸 Event 를 Check하지 않는다....
	if ( IsPracticeMode() || !IsValidRoundState() )
		return;

	// 죽은 사람이 속한 팀을 먼저 검사,
	if ( IsMassacred( thisTeamFirst ) )
	{
		TriggerMassacreEvent( thisTeamFirst );
	}
	// 상대팀을 다음에 검사.
	else if ( IsMassacred( 1 - thisTeamFirst ) )
	{
		TriggerMassacreEvent( 1 - thisTeamFirst );
	}
}

function bool IsMassacred( int teamNum )
{
	local PlayerController PC;
	local PlayerReplicationInfo PRI;
	local int	TeamCnt;
	assert(  0 <= teamNum && teamNum < 2 );

	// 죽어도 재 생성 가능.
	if ( avaGameReplicationInfo(GameReplicationInfo).bReinforcement )
		return false;

	foreach WorldInfo.AllControllers( class'PlayerController', PC )
	{
		PRI = PC.PlayerReplicationInfo;
		if ( PRI.Team == None )	continue;
		if ( PRI.Team.TeamIndex == teamNum )
		{
			++TeamCnt;
			if ( !PRI.bOutOfLives && !PRI.bOnlySpectator )
				return false;
		}
	}
	`log( "team "$ teamNum $ " is massacred " $ ( TeamCnt > 0 ) );
	return TeamCnt > 0 ? true : false;
}

event function TriggerMassacreEvent( int teamNum ) // [!] 20070516 dEAthcURe|HM 'event'로 변경
{
	local Sequence				GameSeq;
	local array<SequenceObject>	AllSeqEvents;
	local int					i;
	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != none )
	{
		GameSeq.FindSeqObjectsByClass( class'avaSeqEvent_Massacre', true, AllSeqEvents );
		for ( i=0; i<AllSeqEvents.Length; ++i )
			avaSeqEvent_Massacre( AllSeqEvents[i] ).Trigger( WorldInfo, teamNum );
	}
}

function ReduceDamage( out int Damage, pawn injured, Controller instigatedBy, class<DamageType> DamageType )
{
	Super.ReduceDamage( Damage,injured,instigatedBy,DamageType );
	if ( Damage > 0 )
	{
		if ( instigatedBy != None )
		{
			if ( instigatedBy != injured.Controller && !class<avaDamageType>(DamageType).default.bForceTeamDamage )
			{
				if ( Injured.GetTeamNum() == instigatedBy.GetTeamNum() && (Injured.GetTeamNum() != 255) )
				{
					// Map Option 에 따라서 Team 원에게 주는 Damage 를 깍아준다.
					switch ( avaGameReplicationInfo( GameReplicationInfo ).FriendlyFireType )
					{
					case 0:	Damage	=	0;	break;																				// Friendly Fire [None]
					case 1:																										// Friendly Fire [Explosion Only]
							if ( class< avaDmgType_Explosion>(DamageType) != None )	Damage *=	FriendlyFireScale;	
							else													Damage	=	0;
						break;
					case 2:	Damage *=	FriendlyFireScale;	break;																// Friendly Fire [All]
					}
				}
			}
			if ( Damage == 0 )	return;
		}
	}
}

function bool IsWinningTeam( TeamInfo T )
{
	if ( Teams[0].Score > Teams[1].Score )
		return (T == Teams[0]);
	else
		return (T == Teams[1]);
}

function OverridePRI(PlayerController PC, PlayerReplicationInfo OldPRI)
{
}

function Logout(controller Exiting)
{	
	local TeamInfo					TeamInfo;
	local avaPlayerReplicationInfo	LeaderPRI;
	local bool						bEnableGameGoOn;
	
	TeamInfo = Exiting.PlayerReplicationInfo.Team;
	
	// 나간 사람이 분대장이라면 분대장을 다시 설정해야 한다....
	if ( avaPlayerReplicationInfo( Exiting.PlayerReplicationInfo ).bSquadLeader )
	{
		LeaderPRI = GetSquadLeader( Exiting.PlayerReplicationInfo.Team.TeamIndex, Exiting.PlayerReplicationInfo );
		if ( LeaderPRI != None )
			SetSquadLeader( LeaderPRI );
	}

	bEnableGameGoOn = true;

	// WarmUp 중이고 아직 Loading 이 끝나지 않은 사람이 있다면 게임을 끝내지 말자...
	if ( HasUnreadyPlayer() == false || !IsWarmUpRound() )
	{
		// Game 이 계속 될 수 있는 지 Check 한다...
		bEnableGameGoOn = CheckGameGoOn( Exiting );
	}

	// Game 이 계속 될 수 있다면 전멸 Check 라도 하자....
	if ( bEnableGameGoOn == true )
	{
		CheckMassacre( TeamInfo.TeamIndex );
	}

	avaGameReplicationInfo(GameReplicationInfo).PlayerOut( Exiting );	
	Super.Logout(Exiting);
}

function SetSquadLeader( avaPlayerReplicationInfo PRI )
{
	local avaPlayerReplicationInfo	PrvLeader;
	// 이미 분대장임....
	if ( PRI.bSquadLeader == true )	return;
	PrvLeader = avaGame( WorldInfo.Game ).HasSquadLeader( PRI.Team.TeamIndex );
	// 분대장 교체...
	if ( PrvLeader != PRI )
	{
		ResignSquadLeader( PrvLeader );
		avaPlayerController( PRI.Owner ).RaiseAutoMessage( AUTOMESSAGE_ChangeLD, true );
	}
	PRI.bSquadLeader = true;
	avaPlayerController( PRI.Owner ).NotifySquadLeader();
}

function ResignSquadLeader( avaPlayerReplicationInfo PRI )
{
	if ( PRI != None )
		PRI.bSquadLeader = false;
}

function avaPlayerReplicationInfo HasSquadLeader( int nTeam )
{
	local Controller				C;
	local avaPlayerReplicationInfo	avaPRI;
	foreach WorldInfo.AllControllers(class'Controller', C )
	{
		avaPRI = avaPlayerReplicationInfo( C.PlayerReplicationInfo );
		if ( avaPRI == None )	continue;
		if ( avaPRI.Team != None && avaPRI.Team.TeamIndex == nTeam )
		{
			if ( avaPRI.bSquadLeader == true )
				return avaPRI;
		}
	}
	return None;
}

function avaPlayerReplicationInfo GetSquadLeader( int nTeam, optional PlayerReplicationInfo exceptPRI )
{
	local Controller				C;
	local avaPlayerReplicationInfo	Leader;
	local avaPlayerReplicationInfo	avaPRI;
	
	foreach WorldInfo.AllControllers(class'Controller', C)
	{
		avaPRI = avaPlayerReplicationInfo( C.PlayerReplicationInfo );
		if ( avaPRI == None )		continue;
		if ( exceptPRI == avaPRI )	continue;
		if ( avaPRI.Team != None && avaPRI.Team.TeamIndex == nTeam )
		{
			if ( Leader == None || Leader.Level < avaPRI.Level )	// Level 이 높은 사람이 Leader로 임명됨
			{
				Leader = avaPRI;
			}
			else if ( Leader.Level == avaPRI.Level )				// Level 이 같은 경우는 LeaderScore가 높은 사람이 분대장이 됨
			{
				if ( Leader.LeaderScore < avaPRI.LeaderScore )
					Leader = avaPRI;
			}
		}
	}
	return Leader;
}

function SwapTeam()
{
	local avaTeamInfo				TempTeamInfo;
	local avaGameReplicationInfo	avaGRI;

	TempTeamInfo	=	Teams[0];
	Teams[0]		=	Teams[1];
	Teams[1]		=	TempTeamInfo;

	TempTeamInfo	=	avaTeamInfo( GameReplicationInfo.Teams[0] );
	GameReplicationInfo.Teams[0] = GameReplicationInfo.Teams[1];
	GameReplicationInfo.Teams[1] = TempTeamInfo;
	GameReplicationInfo.Teams[0].TeamIndex	=	0;
	GameReplicationInfo.Teams[1].TeamIndex	=	1;

	avaGRI = avaGameReplicationInfo( GameReplicationInfo );
	RoundTimeLimit		  =	InitRoundTimeLimit - avaGRI.RemainingTime;
	CurrentRoundTimeLimit = avaGRI.RemainingTime;
	RemainingTime		  =	avaGRI.RemainingTime;
	avaGRI.SetTargetMissionTime( avaGRI.MissionTime );
	avaGRI.bSwappedTeam	  = true;
	EndRoundEx( None, "" );
	ActivateSwapEvent();

	Super.SwapTeam();
}

function ActivateSwapEvent()
{
	local Sequence				GameSeq;
	local array<SequenceObject>	AllSeqEvents;
	local int					i;
	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != none )
	{
		GameSeq.FindSeqObjectsByClass( class'avaSeqEvent_SwapTeam', true, AllSeqEvents );
		for ( i=0; i<AllSeqEvents.Length; ++i )
			avaSeqEvent_SwapTeam( AllSeqEvents[i] ).Trigger( WorldInfo );
	}
}

event function bool CheckGameGoOn( Controller except )
{
	local avaPlayerReplicationInfo	avaPRI;
	local PlayerController			PC;
	local int						TeamCnt[2];
	if ( IsPracticeMode() )			return true;	// 연습채널 모드이면 AllOut Event 를 Check하지 않는다....
	// 대회용 방일 경우에는 한쪽 진영이 다 나가더라도 게임이 계속 진행된다...
	if ( class'avaNetHandler'.static.GetAvaNetHandler().IsMatchRoom() )	return true;

	// 상대편이 있는지 Check 한다....
	foreach WorldInfo.AllControllers( class'PlayerController', PC )
	{
		if ( PC == except )			continue;
		if ( PC.PlayerReplicationInfo.Team == None )	continue;
		avaPRI = avaPlayerReplicationInfo( PC.PlayerReplicationInfo );
		if ( avaPRI.Team.TeamIndex >= 0 && avaPRI.Team.TeamIndex <= 1 )
			++TeamCnt[avaPRI.Team.TeamIndex];
	}
	if ( TeamCnt[0] == 0 )
	{
		TriggerAllOutEvent( 0 );
		return false;
	}
	else if (TeamCnt[1] == 0 )	
	{
		TriggerAllOutEvent( 1 );
		return false;
	}
	return true;
}

// Team 이 Score 를 획득했음....
function TeamScoreEvent(int Team, float Points, string Desc)
{
	local Controller				C;
	local avaPlayerReplicationInfo	pri;
	
	if ( !avaGameReplicationInfo(GameReplicationInfo).bReinforcement )
		avaGameReplicationInfo(GameReplicationInfo).HmSetRoundEnd(true); // 20070302 dEAthcURe|HM	
	
	foreach WorldInfo.AllControllers( class'Controller', C )
	{
		pri = avaPlayerReplicationInfo(C.PlayerReplicationInfo);
		if ( pri == None ) continue;
		if ( pri.Team == None || pri.Team.TeamIndex < 0 || pri.Team.TeamIndex > 1 )	continue;
		if ( pri.Team.TeamIndex == Team )	pri.RoundWinScore	+= int(Points);
		else								pri.RoundLoseScore	+= int(Points);
	}
	
	// 승패가 결정이 되었기 때문에 서버에 결과를 전송한다....
	if ( avaGameReplicationInfo( WorldInfo.GRI ).nWinTeam != -1 )
		avaGameReplicationInfo( WorldInfo.GRI ).HostReportEndGameToServer();

	GameStats.TeamScoreEvent( Team, Points );	
}

function Reset()
{
	local avaGameReplicationInfo	avaGRI;
	local avaPlayerReplicationInfo	avaPRI;
	local int						i;
	Super.Reset();
	avaGRI = avaGameReplicationInfo(GameReplicationInfo);
	if ( avaGRI.bWarmupRound == false )
	{
		class'avaNetHandler'.static.GetAvaNetHandler().UpdateGameState( avaGameReplicationInfo(GameReplicationInfo).CurrentRound, EGS_RoundBegin );
		if ( !bMigratedHost )
		{
			// Squad Leader 가 있는 지 Check 해서 없으면 Leader 를 선정해 주도록 하자...
			for ( i = 0 ; i <= 1 ; ++ i )
			{
				if ( HasSquadLeader( i ) == None )
				{
					avaPRI = GetSquadLeader( i );
					if ( avaPRI != None )
						avaPRI.bSquadLeader = true;
				}
			}
		}
	}
}

// 증원 처리...
function Reinforce()
{
	local avaGameReplicationInfo	avaGRI;
	local PlayerController			PC;
	local int						i;

	avaGRI = avaGameReplicationInfo( GameReplicationInfo );
	for ( i = 0 ; i < `MAX_TEAM ; ++ i )
	{
		if ( avaGRI.ReinforcementFreq[i] <= 0 )
		{
			continue;
		}
		
		if ( ElapsedTime % avaGRI.ReinforcementFreq[i] == 0 )
		{
			// auto respawn "ready" players
			foreach WorldInfo.AllControllers( class'PlayerController', PC )
			{
				if ( PC.PlayerReplicationInfo.Team != None && i == PC.PlayerReplicationInfo.Team.TeamIndex && avaPlayerController( PC ).CanRestart() )
					RestartPlayer( PC );
			}
		}
	}
}

defaultproperties
{
    bTeamGame				=	True    
    MapPrefix				=	"DM"
 	ScoreBoardType			=	class'avaGame.avaTeamScoreboard'
	FriendlyFireScale		=	0.3
	TeamAIType(0)			=	class'avaGame.avaTeamAI'
	TeamAIType(1)			=	class'avaGame.avaTeamAI'
}
