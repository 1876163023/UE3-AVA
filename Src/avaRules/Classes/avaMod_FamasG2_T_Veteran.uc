class avaMod_FamasG2_T_Veteran extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SpreadDecayTime -= 0.8;
	avaWeap_BaseGun( Weapon ).MaxInaccuracy -=0.2;
	avaWeap_BaseGun( Weapon ).Spread_WhenFalling.param1 +=0.002;
	avaWeap_BaseGun( Weapon ).Spread_WhenMoving.param1 +=0.002;
	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param1 +=0.002;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param1 +=0.002;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13181
	Slot		= WEAPON_SLOT_Trigger
}
