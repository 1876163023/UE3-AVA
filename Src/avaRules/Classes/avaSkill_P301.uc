class avaSkill_P301 extends avaCharacterModifier;

//51207	나이프 숙련	고급 나이프 공격	Knife Mastery	Advanced Knife Move

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_KNIFE].RangeAmp += 0.15;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 51207
}
