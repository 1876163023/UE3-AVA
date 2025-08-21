class avaMod_Warpack_A_C extends avaCharacterModifier;

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_SMG].AmmoAmp += 2;
	Pawn.WeapTypeAmp[WEAPON_RIFLE].AmmoAmp += 2;
	Pawn.WeapTypeAmp[WEAPON_SNIPER].AmmoAmp += 2;
	Pawn.WeapTypeAmp[WEAPON_SHOTGUN].AmmoAmp += 2;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 8484
}
