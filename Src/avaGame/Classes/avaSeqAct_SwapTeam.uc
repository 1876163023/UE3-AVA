class avaSeqAct_SwapTeam extends SequenceAction;

event Activated()
{
	// for test....
	if ( GetWorldInfo().NetMode != NM_StandAlone )
	{
		// Request Swap Team To Server...
		class'avaNetHandler'.static.GetAvaNetHandler().SwapTeamInGame();
	}
	else
	{
		avaGame( GetWorldInfo().Game ).SwapTeam();
	}
}

defaultproperties
{
	ObjCategory		=	"Game"
	ObjName			=	"Swap Team"
	bCallHandler	=	false
}
