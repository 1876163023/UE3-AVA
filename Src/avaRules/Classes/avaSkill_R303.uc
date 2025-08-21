class avaSkill_R303 extends avaCharacterModifier;

//51464		Äü ¾²·Î¿ì		QuickThrow
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.ThrowableWeapReadyAmp -=0.30;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51464
}
