class avaSkill_R201 extends avaCharacterModifier;

//51460	ġ��� ����	��� ���	Critical Defence	HardenHelmet
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
