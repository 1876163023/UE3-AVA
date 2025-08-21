class avaSeqAct_GetTeamScore extends SequenceAction;

var() int	ScoreEU;
var() int	ScoreNRF;

event Activated()
{
	local avaTeamGame game;
	game		= avaTeamGame( GetWorldInfo().Game );
	ScoreEU		= game.Teams[0].Score;
	ScoreNRF	= game.Teams[1].Score;

	if ( ScoreEU > ScoreNRF )			OutputLinks[0].bHasImpulse=true;
	else if ( ScoreNRF > ScoreEU )		OutputLinks[1].bHasImpulse=true;	
	else								OutputLinks[2].bHasImpulse=true;
}

defaultproperties
{
	ObjCategory="Level"
	ObjName="Get Team Score"
	bCallHandler=false
	bAutoActivateOutputLinks=false

	OutputLinks(0)=(LinkDesc="Advantage EU")
	OutputLinks(1)=(LinkDesc="Advantage NRF")
	OutputLinks(2)=(LinkDesc="Draw")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Score EU",PropertyName=ScoreEU, MinVars=0, MaxVars=1, bWriteable=true )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Score NRF",PropertyName=ScoreNRF,MinVars=0, MaxVars=1, bWriteable=true )
}