class avaMod_Warpack_C_A extends avaCharacterModifier;

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.MiniMapScale -= 0.3;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 8492
}
