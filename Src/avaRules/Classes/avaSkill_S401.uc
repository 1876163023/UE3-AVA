class avaSkill_S401 extends avaCharacterModifier;

//51720	SR ผ๗ทร	SR ฤü ทนต๐	SR Mastery	SR Quick Equip
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_SNIPER].EquipAmp -=0.1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51720
}
