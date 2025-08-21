class avaSeqAct_ScoreTeam extends SequenceAction;

// amount of score;
var() int    ScoreTeam;
var() float  ScoreAmount;
var() string ScoreDesc;
var	  float	 ScoreResult;		// �ջ����� Score
var	  Actor  ScoreActor;	
var() bool	 bAppointWinner;	// �� ���� TRUE ��� �ش� Score �� ȹ���� Team �� Winner �� �����ȴ�...
								// but, �̹� Winner �� ������ �Ǿ� �ִٸ� �� Flag�� ���õȴ�....

event Activated()
{
	//local	avaGame	game;
	//game = avaGame( GetWorldInfo().Game );
	// Warmup Round ��� Score Team �Ұ���....

	// Warmup Round �� Score Team �� �ʿ��ϴ�....
	// Warmup Round �� �ٸ����� �� ������ ��찡 �߻��ϱ� ������...

	//if ( avaGameReplicationInfo( game.GameReplicationInfo ).bWarmupRound == true )
	//	return;
	if (InputLinks[0].bHasImpulse)		// Score Team by Number
	{
		`log( "avaSeqAct_ScoreTeam::Activated (Score Team by Number) : " $ScoreTeam );
	}
	else if (InputLinks[1].bHasImpulse)		// Score Team by Actor
	{
		if ( Pawn( ScoreActor ) != None )				ScoreTeam = Pawn(ScoreActor).GetTeamNum();
		else if ( Controller( ScoreActor ) != None )	ScoreTeam = Controller( ScoreActor ).GetTeamNum();
		else											ScoreTeam = 255;
		`log( "avaSeqAct_ScoreTeam::Activated (Score Team by Acto) : " @ScoreTeam @ScoreActor );
	}
	else if (InputLinks[2].bHasImpulse)	// Opponent Score Team by Team Number
	{
		ScoreTeam = 1 - ScoreTeam;		
		`log( "avaSeqAct_ScoreTeam::Activated (Score Opp-Team by Number) : " $ScoreTeam );
	}
	else if (InputLinks[3].bHasImpulse)	// Opponent Score Team by Actor
	{
		if ( Pawn( ScoreActor ) != None )				ScoreTeam = 1 - Pawn(ScoreActor).GetTeamNum();
		else if ( Controller( ScoreActor ) != None )	ScoreTeam = 1 - Controller( ScoreActor ).GetTeamNum();
		else											ScoreTeam = 255;
		`log( "avaSeqAct_ScoreTeam::Activated (Opponent Score Team by Acto) : " $ScoreTeam );
	}
	else if (InputLinks[4].bHasImpulse)	// Opponent Score Team by Actor
	{
		ScoreTeam = 0;
		`log( "avaSeqAct_ScoreTeam::Activated (Score Team0) " );
	}
	else if (InputLinks[5].bHasImpulse)	// Opponent Score Team by Actor
	{
		ScoreTeam = 1;
		`log( "avaSeqAct_ScoreTeam::Activated (Score Team1) " );
	}
	UpdateScore( ScoreTeam, ScoreAmount, ScoreDesc );
}

function UpdateScore( int teamnum, float amount, string desc )
{
	local avaTeamGame game;
	local avaTeamInfo team;

	`log( "score team "$teamnum $" amount="$amount $" desc="$desc );

	game = avaTeamGame( GetWorldInfo().Game );
	if ( game == none )
	{
		`log( self$" is for avaTeamGame" );
		return;
	}

	if ( teamnum >= 2 )
	{
		`log( self$" team must be 0 or 1. current="$teamnum );
		return;
	}

	// �̹� Winner �� �������� ������ �� �̻� Team Score �� �÷������� �ʴ´�...
	if ( game.nWinTeam != -1 )	return;

	team = game.Teams[teamnum];

	ScoreResult = team.Score += amount;

	if ( bAppointWinner == true || team.Score >= game.WinCondition )
	{
		if ( team.Score >= game.WinCondition )	team.Score	  = game.WinCondition;
		game.nWinTeam = teamnum;
		avaGameReplicationInfo( game.GameReplicationInfo ).nWinTeam = game.nWinTeam;
	}

	game.GameReplicationInfo.NetUpdateTime = GetWorldInfo().TimeSeconds - 1;
	team.NetUpdateTime = GetWorldInfo().TimeSeconds - 1;

	//game.TeamScoreEvent( teamnum, amount, desc );
}

defaultproperties
{
	ObjCategory="Level"
	ObjName="Score Team"
	bCallHandler=false

	//OutputLinks.Empty

	ScoreTeam		= 255
	ScoreAmount		= 1.0
	ScoreDesc		= "kismet"
	bAppointWinner	= false

	InputLinks(0)=(LinkDesc="Score by Number")
	InputLinks(1)=(LinkDesc="Score by Actor")
	InputLinks(2)=(LinkDesc="Opp Score by Number")
	InputLinks(3)=(LinkDesc="Opp Score by Actor")
	InputLinks(4)=(LinkDesc="Score Team0")
	InputLinks(5)=(LinkDesc="Score Team1")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Number",PropertyName=ScoreTeam, MinVars=0, MaxVars=1, bWriteable=true )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Actor",PropertyName=ScoreActor,MinVars=0, MaxVars=1 )
	VariableLinks(2)=(ExpectedType=class'SeqVar_Float',LinkDesc="Amount",PropertyName=ScoreAmount, MinVars=0, MaxVars=1 )

}

