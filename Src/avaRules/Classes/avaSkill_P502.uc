class avaSkill_P502 extends avaCharacterModifier;

//51212		고급 전장파악술		Advanced Battle Instint
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.MiniMapScale -= 0.15;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51212
}
