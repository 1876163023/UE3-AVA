class avaMod_M4A1_S_Light extends avaMod_Weapon;

/*
//static function ApplyToCharacter_Client( avaPawn Pawn );
//static function ApplyToCharacter_Server( avaPawn Pawn );
*/

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	// Stock 부분 Material 변화 필요
//	Weapon.BaseSpeed	+= 10;	// 기본속도
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}
defaultproperties
{
	Id = 13066
	Slot = WEAPON_SLOT_Stock
}