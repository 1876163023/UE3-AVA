class avaMod_Warpack_C_C extends avaCharacterModifier;

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_PISTOL].AmmoAmp += 2;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 8494
}
