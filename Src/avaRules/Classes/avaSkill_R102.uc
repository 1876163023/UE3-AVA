class avaSkill_R102 extends avaCharacterModifier;

//51458		중급 방어술		Medium Class Defence
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.Absorb_Stomach += 0.01;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51458
}
