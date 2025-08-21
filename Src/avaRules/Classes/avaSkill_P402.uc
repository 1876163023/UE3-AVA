class avaSkill_P402 extends avaCharacterModifier;

//51210		SMG 탄약 추가 보급		SMG Extra Ammo
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_SMG].AmmoAmp += 0.5;
	Pawn.WeapTypeAmp[WEAPON_SHOTGUN].AmmoAmp += 0.5;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51210
}
