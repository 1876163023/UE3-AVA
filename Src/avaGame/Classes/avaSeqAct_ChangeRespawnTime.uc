class avaSeqAct_ChangeRespawnTime extends SequenceAction;

var() int EURespawnTime;
var() int NRFRespawnTime;

event Activated()
{
	local avaGame game;
	game = avaGame( GetWorldInfo().Game );

	if (InputLinks[0].bHasImpulse )				// Get
	{
		EURespawnTime	= avaGameReplicationInfo(game.GameReplicationInfo).ReinforcementFreq[0];
		NRFRespawnTime	= avaGameReplicationInfo(game.GameReplicationInfo).ReinforcementFreq[1];
	}
	else if ( InputLinks[1].bHasImpulse )		// Set
	{
		if ( EURespawnTime > 0 )	game.ChangeRespawnTime( 0, EURespawnTime  );
		if ( NRFRespawnTime > 0 )	game.ChangeRespawnTime( 1, NRFRespawnTime );
	}
}

defaultproperties
{
	ObjCategory			=	"Game"
	ObjName				=	"Respawn Time"
	bCallHandler		=	false

	InputLinks(0)=(LinkDesc="Get")
	InputLinks(1)=(LinkDesc="Set")

	VariableLinks(0)	=	(ExpectedType=class'SeqVar_Int',	LinkDesc="EU  RespawnTime",		PropertyName=EURespawnTime,		MaxVars=1,bWriteable=true )
	VariableLinks(1)	=	(ExpectedType=class'SeqVar_Int',	LinkDesc="NRF RespawnTime",		PropertyName=NRFRespawnTime,	MaxVars=1,bWriteable=true )
}