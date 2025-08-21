class avaSkill_P102 extends avaCharacterModifier;

//51202		러닝 익스퍼트		Running Expert
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.ChrBaseSpeedPct += 0.01;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 51202
}
