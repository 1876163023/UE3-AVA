class avaSeqAct_ScoreRound extends SequenceAction;

// amount of score;
var() int    ScoreTeam;
var() float  ScoreAmount;
var	  Actor  ScoreActor;	
var() string ScoreDesc;

event Activated()
{
	local	avaGame	game;
	game = avaGame( GetWorldInfo().Game );
	if ( avaGameReplicationInfo( game.GameReplicationInfo ).bWarmupRound == true )	return;

	if (InputLinks[0].bHasImpulse)		// Score Team by Number
	{
	}
	else if (InputLinks[1].bHasImpulse)		// Score Team by Actor
	{
		if ( Pawn( ScoreActor ) != None )				ScoreTeam = Pawn(ScoreActor).GetTeamNum();
		else if ( Controller( ScoreActor ) != None )	ScoreTeam = Controller( ScoreActor ).GetTeamNum();
		else											ScoreTeam = 255;
	}
	else if (InputLinks[2].bHasImpulse)	// Opponent Score Team by Team Number
	{
		ScoreTeam = 1 - ScoreTeam;		
	}
	else if (InputLinks[3].bHasImpulse)	// Opponent Score Team by Actor
	{
		if ( Pawn( ScoreActor ) != None )				ScoreTeam = 1 - Pawn(ScoreActor).GetTeamNum();
		else if ( Controller( ScoreActor ) != None )	ScoreTeam = 1 - Controller( ScoreActor ).GetTeamNum();
		else											ScoreTeam = 255;
	}
	else if (InputLinks[4].bHasImpulse)	// Opponent Score Team by Actor
	{
		ScoreTeam = 0;
	}
	else if (InputLinks[5].bHasImpulse)	// Opponent Score Team by Actor
	{
		ScoreTeam = 1;
	}
	UpdateScore( ScoreTeam, ScoreAmount, ScoreDesc );
}

function UpdateScore( int teamnum, float amount, string desc )
{
	local avaTeamGame game;
	game = avaTeamGame( GetWorldInfo().Game );
	if ( game == none )	return;
	if ( teamnum >= 2 )	return;
	game.TeamScoreEvent( teamnum, amount, desc );
}

defaultproperties
{
	ObjCategory="Level"
	ObjName="Score Round"
	bCallHandler=false

	//OutputLinks.Empty

	ScoreTeam		= 255
	ScoreAmount		= 1.0
	ScoreDesc		= "kismet"

	InputLinks(0)=(LinkDesc="Score by Number")
	InputLinks(1)=(LinkDesc="Score by Actor")
	InputLinks(2)=(LinkDesc="Opp Score by Number")
	InputLinks(3)=(LinkDesc="Opp Score by Actor")
	InputLinks(4)=(LinkDesc="Score Team0")
	InputLinks(5)=(LinkDesc="Score Team1")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Number",PropertyName=ScoreTeam, MinVars=0, MaxVars=1, bWriteable=true )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Actor",PropertyName=ScoreActor,MinVars=0, MaxVars=1 )
	VariableLinks(2)=(ExpectedType=class'SeqVar_Float',LinkDesc="Amount",PropertyName=ScoreAmount, MinVars=0, MaxVars=1 )
	VariableLinks(3)=(ExpectedType=class'SeqVar_Float',LinkDesc="Result",PropertyName=ScoreResult, MaxVars=1, bWriteable=true )
}