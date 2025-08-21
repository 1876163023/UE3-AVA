class avaSkill_S202 extends avaCharacterModifier;

//51717		무빙 스나이핑		Moving Sniping
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAdd[WEAPON_SNIPER].SpreadMovingAdd -=0.001;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 51717
}
