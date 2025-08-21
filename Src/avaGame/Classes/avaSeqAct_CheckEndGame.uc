class avaSeqAct_CheckEndGame extends SequenceAction;

var int		nWinTeam;
var float	Team0_Scores;
var float	Team1_Scores;


event Activated()
{
    local avaTeamGame	TeamGame;

	TeamGame = avaTeamGame( GetWorldInfo().Game );

	if ( TeamGame == None )
		return;

	nWinTeam = TeamGame.CheckWinTeam();
	Team0_Scores = TeamGame.Teams[0].Score;
	Team1_Scores = TeamGame.Teams[1].Score;

	if ( nWinTeam >= 0 )	GenerateImpulse( true );
	else					GenerateImpulse( false );
}

function GenerateImpulse( bool bEnd )
{
	if ( bEnd )	OutputLinks[0].bHasImpulse = true;
	else		OutputLinks[1].bHasImpulse = true;
}

defaultproperties
{
    ObjCategory					=	"Game"
    ObjName						=	"Check End Game"
    bCallHandler				=	false
	bAutoActivateOutputLinks	=	false

	OutputLinks(0)	=	(LinkDesc="Yes")
	OutputLinks(1)	=	(LinkDesc="No")

    VariableLinks(0)=(MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="WinTeam",  PropertyName=nWinTeam, MaxVars=1, bWriteable=true )
	VariableLinks(2)=(ExpectedType=class'SeqVar_Float',LinkDesc="Score [0]", PropertyName=Team0_Scores, MaxVars=1, bWriteable=true )
	VariableLinks(3)=(ExpectedType=class'SeqVar_Float',LinkDesc="Score [1]", PropertyName=Team1_Scores, MaxVars=1, bWriteable=true )
}
