class avaSkill_R103 extends avaCharacterModifier;

//51459		고급 방어술		High Class Defence
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.Absorb_Stomach += 0.01;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51459
}
