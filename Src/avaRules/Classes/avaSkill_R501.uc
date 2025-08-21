class avaSkill_R501 extends avaCharacterModifier;

//51467	AR ¼÷·Ã	AR Åº¾à Ãß°¡ º¸±Þ	AR Mastery	AR Extra Ammo
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_RIFLE].AmmoAmp += 0.5;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51467
}
