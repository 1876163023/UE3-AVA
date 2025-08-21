class avaMod_M4A1_T_Hightech extends avaMod_Weapon;

/*
//static function ApplyToCharacter_Client( avaPawn Pawn );
//static function ApplyToCharacter_Server( avaPawn Pawn );
*/

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
//	avaWeap_BaseGun( Weapon ).Spread_WhenFalling.param1-=0.05;
//	avaWeap_BaseGun( Weapon ).Spread_WhenMoving.param1-=0.05;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}
defaultproperties
{
	Id = 13069
	Slot = WEAPON_SLOT_Trigger
}