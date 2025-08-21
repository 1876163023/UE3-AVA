class avaMod_SPAS12_B_Chock_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param1-=0.005;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param1-=0.005;
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] += 0.1;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	18160
	Slot	= WEAPON_SLOT_Barrel
}
