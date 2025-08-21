class avaSkill_P401 extends avaCharacterModifier;

//51209	SMG ¼÷·Ã	SMG Äü ¸®·Îµå	SMG Mastery	SMG Qucik Reload
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_SMG].ReloadAmp -= 0.1;
	Pawn.WeapTypeAmp[WEAPON_SHOTGUN].ReloadAmp -= 0.1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 51209
}
