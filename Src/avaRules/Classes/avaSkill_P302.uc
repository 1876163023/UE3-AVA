class avaSkill_P302 extends avaCharacterModifier;

//51208		샤프 나이프		Sharp Knife
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_KNIFE].DamageAmp += 0.20;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 51208
}
