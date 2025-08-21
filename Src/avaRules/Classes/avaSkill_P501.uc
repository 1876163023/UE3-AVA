class avaSkill_P501 extends avaCharacterModifier;

//51211	전장파악	기본 전장파악술	Battle Instint	Basic Battle Instint
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.MiniMapScale -= 0.15;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51211
}
