class avaSkill_S101 extends avaCharacterModifier;

//51713	��������	�������� ���	Sniping	Sniping Novice
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAdd[WEAPON_SNIPER].ZoomSpreadAdd -=0.006;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51713
}
