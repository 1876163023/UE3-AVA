class avaSkill_S302 extends avaCharacterModifier;

//51719		Pistol Äü ¸®·Îµå		Pistol Quick Reload
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_PISTOL].ReloadAmp -= 0.1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51719
}
