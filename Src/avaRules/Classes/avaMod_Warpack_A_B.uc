class avaMod_Warpack_A_B extends avaCharacterModifier;

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_SMG].AmmoAmp += 1;
	Pawn.WeapTypeAmp[WEAPON_RIFLE].AmmoAmp += 1;
	Pawn.WeapTypeAmp[WEAPON_SNIPER].AmmoAmp += 1;
	Pawn.WeapTypeAmp[WEAPON_SHOTGUN].AmmoAmp += 1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 8483
}
