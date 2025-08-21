class avaSeqAct_ScorePlayer extends SequenceAction;

`include(avaGame/avaGame.uci)

// amount of score;
var(Score) float	ScoreAmount;
var(Score) int		ScoreAttack;
var(Score) int		ScoreDefence;
var(Score) int		ScoreLeader;
var(Score) int		ScoreTactics;
var(Score) bool		ToSameTeamMember;
var()		string	ScoreDesc;

var(Team)  int		ToTeam;

event Activated()
{
	local SeqVar_Object			ObjVar;
	local SeqVar_ObjectList		VarObjList;
	local int					i;
	local avaGame				game;
	game = avaGame( GetWorldInfo().Game );
	
	// WarmUp 중이거나 Server 에 게임결과를 이미 Report 한 이후라면 Score 를 올리지 않는다....
	if ( !game.IsValidRoundState() )
		return;

	if ( game != none )
	{
		foreach LinkedVariables( class'SeqVar_Object', ObjVar, "Target" )
		{
			ScorePlayer( ObjVar.GetObjectValue() );
		}

		foreach LinkedVariables(class'SeqVar_ObjectList',VarObjList,"PlayerList")
		{
			`log( "avaSeqAct_ScorePlayer Activated" @VarObjList );
			for ( i = 0 ; i < VarObjList.ObjList.length ; ++ i )
			{
				`log( "avaSeqAct_ScorePlayer Activated" @VarObjList @VarObjList.ObjList[i] );
				ScorePlayer( VarObjList.ObjList[i] );
			}
		}

		ScorePlayerByTeamNum( ToTeam );
	}
}

function ScorePlayerByTeamNum( int nTeamNum )
{
	local avaPlayerReplicationInfo	avaPRI;
	local avaPlayerController		avaPC;
	Local WorldInfo WorldInfo;
	WorldInfo = GetWorldInfo();
	if ( nTeamNum == 0 || nTeamNum == 1 )
	{
		foreach WorldInfo.AllControllers( class'avaPlayerController', avaPC )
		{
			if ( nTeamNum == avaPC.GetTeamNum() )
			{
				avaPRI = avaPlayerReplicationInfo( avaPC.PlayerReplicationInfo );
				if ( ScoreAttack > 0 )		avaPRI.AddPoint( PointType_Attack,	ScoreAttack );
				if ( ScoreDefence > 0 )		avaPRI.AddPoint( PointType_Defence,	ScoreDefence );
				if ( ScoreLeader > 0 )		avaPRI.AddPoint( PointType_Leader,	ScoreLeader );
				if ( ScoreTactics > 0 )		avaPRI.AddPoint( PointType_Tactics,	ScoreTactics );
			}
		}
	}
}


function ScorePlayer( Object ObjVar )
{
	local Controller	C;
	local Pawn			P;
	C = Controller( ObjVar );
	if ( C == None )
	{
		P = Pawn( ObjVar );
		if ( P != None && P.Controller != None )
			C = P.Controller;
	}
	if ( C != None && C.PlayerReplicationInfo != None )
	{
		UpdateScore( avaPlayerReplicationInfo( C.PlayerReplicationInfo ), ScoreDesc );
	}
}

/// client 에서 실행 방지를 위해 non-simulated function 으로 선언.
function UpdateScore( avaPlayerReplicationInfo pri, string desc )
{
	local avaPlayerReplicationInfo	avaPRI;
	local avaPlayerController		avaPC;
	Local WorldInfo WorldInfo;
	WorldInfo = GetWorldInfo();

	if ( !ToSameTeamMember )
	{
		if ( ScoreAttack > 0 )		pri.AddPoint( PointType_Attack,		ScoreAttack );
		if ( ScoreDefence > 0 )		pri.AddPoint( PointType_Defence,	ScoreDefence );
		if ( ScoreLeader > 0 )		pri.AddPoint( PointType_Leader,		ScoreLeader );
		if ( ScoreTactics > 0 )		pri.AddPoint( PointType_Tactics,	ScoreTactics );
	}
	else
	{
		if ( pri.Team != None )
		{
			foreach WorldInfo.AllControllers( class'avaPlayerController', avaPC )
			{
				if ( avaPC.GetTeamNum() == pri.Team.TeamIndex )
				{
					avaPRI = avaPlayerReplicationInfo( avaPC.PlayerReplicationInfo );
					if ( ScoreAttack > 0 )		avaPRI.AddPoint( PointType_Attack,	ScoreAttack );
					if ( ScoreDefence > 0 )		avaPRI.AddPoint( PointType_Defence,	ScoreDefence );
					if ( ScoreLeader > 0 )		avaPRI.AddPoint( PointType_Leader,	ScoreLeader );
					if ( ScoreTactics > 0 )		avaPRI.AddPoint( PointType_Tactics,	ScoreTactics );
				}
			}
		}
	}
}

defaultproperties
{
	ObjClassVersion=5
	ObjCategory="Level"
	ObjName="Score Player"
	bCallHandler=false

	ScoreAmount  = 1.0
	ScoreAttack	 = 0
	ScoreDefence = 0
	ScoreLeader  = 0
	ScoreTactics = 0
	ScoreDesc    = "kismet"
	ToTeam		 = -1

	InputLinks(0)=(LinkDesc="Score")
	VariableLinks.Add( (ExpectedType=class'SeqVar_ObjectList',LinkDesc="PlayerList") );
}

