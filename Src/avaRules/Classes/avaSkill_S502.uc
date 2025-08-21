class avaSkill_S502 extends avaCharacterModifier;

//51723		SR Äü ¸®·Îµå		SR Quick Reload
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_SNIPER].ReloadAmp -=0.1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 51723
}
