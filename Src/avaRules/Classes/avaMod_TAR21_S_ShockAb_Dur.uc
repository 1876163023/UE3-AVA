class avaMod_TAR21_S_ShockAb_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param1-=0.005;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param1-=0.005;
	avaWeap_BaseGun( Weapon ).AccuracyOffset += 0.06;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18207
	Slot		= WEAPON_SLOT_Stock
}
