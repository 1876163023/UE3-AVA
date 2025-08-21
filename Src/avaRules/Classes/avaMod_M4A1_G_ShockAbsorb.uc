class avaMod_M4A1_G_ShockAbsorb extends avaMod_Weapon;

/*
//static function ApplyToCharacter_Client( avaPawn Pawn );
//static function ApplyToCharacter_Server( avaPawn Pawn );
*/

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
//	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param2-=0.00275;
//	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param2-=0.00275;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}
defaultproperties
{
	Id = 13063
	Slot = WEAPON_SLOT_Grip
}