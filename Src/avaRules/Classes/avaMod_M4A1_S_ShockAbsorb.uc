class avaMod_M4A1_S_ShockAbsorb extends avaMod_Weapon;

/*
//static function ApplyToCharacter_Client( avaPawn Pawn );
//static function ApplyToCharacter_Server( avaPawn Pawn );
*/

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	// Stock 부분 Material 변화 필요
//	avaWeap_BaseGun( Weapon ).Spread_WhenFalling.param2-=0.02;
//	avaWeap_BaseGun( Weapon ).Spread_WhenMoving.param2-=0.0035;
//	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param2-=0.001375;
//	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param2-=0.001375;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}
defaultproperties
{
	Id = 13067
	Slot = WEAPON_SLOT_Stock
}