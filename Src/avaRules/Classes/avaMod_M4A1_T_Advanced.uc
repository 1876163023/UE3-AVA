class avaMod_M4A1_T_Advanced extends avaMod_Weapon;

/*
//static function ApplyToCharacter_Client( avaPawn Pawn );
//static function ApplyToCharacter_Server( avaPawn Pawn );
*/

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
//	avaWeap_BaseGun( Weapon ).FireInterval[(0)] -=0.00875;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}
defaultproperties
{
	Id = 13068
	Slot = WEAPON_SLOT_Trigger
}