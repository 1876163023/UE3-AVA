class avaSkill_S102 extends avaCharacterModifier;

//51714		�������� �ͽ���Ʈ		Sniping Expert
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAdd[WEAPON_SNIPER].ZoomSpreadAdd -=0.002;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51714
}
