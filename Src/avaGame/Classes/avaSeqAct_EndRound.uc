//=================================================================================================
// Round를 종료시켜주는 Sequence Action 입니다.
//=================================================================================================
class avaSeqAct_EndRound extends SequenceAction;

var()	string	EndDesc;
var()	int		Winner;
var()	int		WinType;
var		int		RoundCount;
var		float	Team0_Scores;
var		float	Team1_Scores;

event Activated()
{
	local avaGame					Game;
	local avaGameReplicationInfo	avaGRI;
	local Actor						InstigatorActor;
	Game	=	avaGame( GetWorldInfo().Game );
	avaGRI	=	avaGameReplicationInfo( Game.GameReplicationInfo );
	// Warmup Round 라면 End Round 불가능
	if ( Game == NONE )					return;
	if ( avaGRI.bWarmupRound == TRUE )	return;
	if ( VariableLinks.Length >= 1 && VariableLinks[0].LinkedVariables.Length > 0 )
    {
		InstigatorActor = Actor( SeqVar_Object(VariableLinks[0].LinkedVariables[0] ).GetObjectValue());
	}
	RoundCount		=	avaGRI.CurrentRound;
	if ( avaTeamGame( Game ) != NONE )
	{
		Team0_Scores	=	avaTeamGame( Game ).Teams[0].Score;
		Team1_Scores	=	avaTeamGame( Game ).Teams[1].Score;
	}
	game.EndRoundEx( InstigatorActor, EndDesc, Winner, WinType );
}

defaultproperties
{
    ObjCategory			=	"Game"
    ObjName				=	"End Round"
    bCallHandler		=	false
    EndDesc				=	"kismet"
    VariableLinks(0)	=	(MaxVars=1)
    VariableLinks(1)	=	(ExpectedType=class'SeqVar_String',LinkDesc="Desc",  PropertyName=EndDesc, MinVars=0, MaxVars=1 )
	VariableLinks(2)	=	(ExpectedType=class'SeqVar_Int',LinkDesc="Winner",  PropertyName=Winner, MinVars=0, MaxVars=1 )
	VariableLinks(3)	=	(ExpectedType=class'SeqVar_Int',LinkDesc="RoundCount",PropertyName=RoundCount, MaxVars=1, bWriteable=true )
	VariableLinks(4)	=	(ExpectedType=class'SeqVar_Float',LinkDesc="Score [0]", PropertyName=Team0_Scores, MaxVars=1, bWriteable=true )
	VariableLinks(5)	=	(ExpectedType=class'SeqVar_Float',LinkDesc="Score [1]", PropertyName=Team1_Scores, MaxVars=1, bWriteable=true )
	VariableLinks(6)	=	(ExpectedType=class'SeqVar_Int',LinkDesc="WinType", PropertyName=WinType, MinVars=0, MaxVars=1 )
}
