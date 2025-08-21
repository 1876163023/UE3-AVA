class avaSeqAct_PauseTimer extends SequenceAction;

event Activated()
{
	local avaGame	game;
	game = avaGame( GetWorldInfo().Game );
	if ( game == None )	return;

	if (InputLinks[0].bHasImpulse)			// Pause Time
	{
		game.bStopCountDown	=	true;
	}
	else if (InputLinks[1].bHasImpulse)		// Restart Time
	{
		game.bStopCountDown	=	false;
	}
}

defaultproperties
{
	ObjCategory		=	"Game"
	ObjName			=	"Pause Timer"
	bCallHandler	=	false
	//OutputLinks.Empty
	InputLinks(0)=(LinkDesc="Pause")
	InputLinks(1)=(LinkDesc="Start")
}