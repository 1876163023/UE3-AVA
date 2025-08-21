class avaSeqAct_ChangeRemainingTime extends SequenceAction;

var() int RemainingTime;

event Activated()
{
	local avaGame	game;
	local int		ResultTime;
	game = avaGame( GetWorldInfo().Game );

	if (InputLinks[0].bHasImpulse )				// Set
	{
		if ( RemainingTime > 1 )
		{
			game.ChangeRemainingTime( RemainingTime );
		}
	}
	else if ( InputLinks[1].bHasImpulse )		// Add
	{
		ResultTime = avaGameReplicationInfo(game.GameReplicationInfo).RemainingTime + RemainingTime;
		if ( ResultTime < 0 )
			ResultTime = 1;
		game.ChangeRemainingTime( ResultTime );
	}
}

defaultproperties
{
	ObjCategory			=	"Game"
	ObjName				=	"RemainingTime"
	bCallHandler		=	false

	InputLinks(0)=(LinkDesc="Set")
	InputLinks(1)=(LinkDesc="Add")

	VariableLinks(0)	=	(ExpectedType=class'SeqVar_Int',	LinkDesc="RemainingTime",		PropertyName=RemainingTime,		MaxVars=1,bWriteable=false )
}