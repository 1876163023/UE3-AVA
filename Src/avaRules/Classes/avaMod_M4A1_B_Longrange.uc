class avaMod_M4A1_B_Longrange extends avaMod_Weapon;

/*
//static function ApplyToCharacter_Client( avaPawn Pawn );
//static function ApplyToCharacter_Server( avaPawn Pawn );
*/

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).RangeModifier+=0.02;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}
defaultproperties
{
	Id = 13058
	Slot = WEAPON_SLOT_Barrel
}