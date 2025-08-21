class avaSkill_P103 extends avaCharacterModifier;

//51203		·¯´× ¸¶½ºÅÍ		Running Master
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.ChrBaseSpeedPct += 0.01;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 51203
}
