class avaSeqAct_CheckSwapTeam extends SequenceAction;

event Activated()
{
	local avaGameReplicationInfo	avaGRI;
	local bool						bMustSwapTeam;

	avaGRI	=	avaGameReplicationInfo( GetWorldInfo().Game.GameReplicationInfo );

	bMustSwapTeam = false;
	if ( avaGRI.bSwapRule == true && avaGRI.bSwappedTeam == false )
		bMustSwapTeam = true;

	if ( bMustSwapTeam == true )	OutputLinks[0].bHasImpulse = true;
	else							OutputLinks[1].bHasImpulse = true;
}

defaultproperties
{
    ObjCategory					=	"Game"
    ObjName						=	"Check Swap Team"
    bCallHandler				=	false
	bAutoActivateOutputLinks	=	false

	OutputLinks(0)	=	(LinkDesc="Yes")
	OutputLinks(1)	=	(LinkDesc="No")
}


	