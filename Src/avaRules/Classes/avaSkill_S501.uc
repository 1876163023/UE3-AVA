class avaSkill_S501 extends avaCharacterModifier;

//51722	SR АэБо	SR Фќ ЙЋКљ	SR Advanced Matery	SR Quick Move
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_SNIPER].SpeedAmp +=0.02;	
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51722
}
