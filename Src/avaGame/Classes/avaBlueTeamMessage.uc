class avaBlueTeamMessage extends avaLocalMessage
	config(game);


var const Color BlueTeamColor;

static function color GetConsoleColor( PlayerReplicationInfo RelatedPRI_1 )
{
    return default.BlueTeamColor;
}

defaultproperties
{	
	BlueTeamColor=(R=66,G=114,B=210,A=230)
}