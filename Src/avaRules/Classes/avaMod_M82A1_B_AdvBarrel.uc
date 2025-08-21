class avaMod_M82A1_B_AdvBarrel extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenSteadyA.param1 -= 0.025;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteadyA.param2 -= 0.025;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13266
	Slot		= WEAPON_SLOT_Barrel
}
