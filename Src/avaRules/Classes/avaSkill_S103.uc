class avaSkill_S103 extends avaCharacterModifier;

//51715		스나이핑 마스터		Sniping Master
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAdd[WEAPON_SNIPER].ZoomSpreadAdd -=0.002;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51715
}
