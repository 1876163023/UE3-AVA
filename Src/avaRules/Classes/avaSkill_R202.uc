class avaSkill_R202 extends avaCharacterModifier;

//51461		��� ��� ���		AdvancedHelmetDefence
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.HeadDefenceRate += 0.1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51461
}
