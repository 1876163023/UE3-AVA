class avaSeqAct_GetWinCondition extends SequenceAction;

var		int	WinCondition;
var		int	RemainEU;
var		int	RemainNRF;
var()	int	TargetRemaingScore;

event Activated()
{
	local avaGame		Game;
	local bool			bMatch;
	Game			=	avaGame( GetWorldInfo().Game );
	WinCondition	=	Game.WinCondition;
	if ( Game.bTeamGame )
	{
		RemainEU	=	WinCondition - avaTeamGame( Game ).Teams[0].Score;
		RemainNRF	=	WinCondition - avaTeamGame( Game ).Teams[1].Score;
		if ( TargetRemaingScore > 0 && ( TargetRemaingScore > RemainEU || TargetRemaingScore > RemainNRF ) )
			bMatch = true;
	}
	else
	{
		// @avaDeathMatch Team Match 가 아닌경우에는 어떻게 할까?
	}

	if ( bMatch )	OutputLinks[0].bHasImpulse = true;
	else			OutputLinks[1].bHasImpulse = true;
}

defaultproperties
{
	bCallHandler				=	false
	bAutoActivateOutputLinks	=	false
	ObjCategory					=	"Game"
	ObjName						=	"Get Win Condition"

	OutputLinks(0)	=	(LinkDesc="target match")
	OutputLinks(1)	=	(LinkDesc="target mismatch")

	VariableLinks(0)=(MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="WinCondition",PropertyName=WinCondition,MaxVars=1,bWriteable=true)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Int',LinkDesc="Remaining EU",PropertyName=RemainEU,MaxVars=1,bWriteable=true)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Int',LinkDesc="Remaining NRF",PropertyName=RemainNRF,MaxVars=1,bWriteable=true)
	
}
