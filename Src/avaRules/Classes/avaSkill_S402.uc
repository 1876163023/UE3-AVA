class avaSkill_S402 extends avaCharacterModifier;

//51721		SR ź�� �߰� ����		SR Extra Ammon
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_SNIPER].AmmoAmp +=0.5;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51721
}
