class avaSkill_R101 extends avaCharacterModifier;

//51457	����	�⺻ ����	Defence	Low Class Defence
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.Absorb_Stomach += 0.03;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51457
}
