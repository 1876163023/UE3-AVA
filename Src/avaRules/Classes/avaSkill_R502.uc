class avaSkill_R502 extends avaCharacterModifier;

//51468		AR �� ���ε�		AR Quick Reload
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_RIFLE].ReloadAmp -= 0.1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51468
}
