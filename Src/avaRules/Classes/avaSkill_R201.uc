class avaSkill_R201 extends avaCharacterModifier;

//51460	Ä¡¸í»ó ¹æ¾î¼ú	Çï¸ä ¹æ¾î	Critical Defence	HardenHelmet
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.HeadDefenceRate += 0.1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51460
}
