class avaSeqAct_ReportEndGameToServer extends SequenceAction;

event Activated()
{
	local avaGameReplicationInfo	avaGRI;
	avaGRI = avaGameReplicationInfo( GetWorldInfo().Game.GameReplicationInfo );
	if ( avaGRI != None )
	{
		avaGRI.HostReportEndGameToServer();
	}
}

defaultproperties
{
	ObjCategory		=	"Game"
	ObjName			=	"Report End Game To Server"
	bCallHandler	=	false
}