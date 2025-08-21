class avaMod_Warpack_C_D extends avaCharacterModifier;

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.HeadDefenceRate += 0.1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 8495
}
