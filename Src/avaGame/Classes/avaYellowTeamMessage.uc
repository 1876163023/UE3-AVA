class avaYellowTeamMessage extends avaLocalMessage
	config(game);

var const Color YellowTeamColor;

static function color GetConsoleColor( PlayerReplicationInfo RelatedPRI_1 )
{
    return default.YellowTeamColor;
}


defaultproperties
{	
	YellowTeamColor=(R=255,G=224,B=72,A=220)

}