class avaSkill_S301 extends avaCharacterModifier;

//51718	Pistol ����	Pistol ź�� �߰� ����	Pistol Matery	Pistol Extra Ammo
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_PISTOL].AmmoAmp +=1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51718
}
