class avaMod_FamasG2_B_SharpShooter_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] += 0.005;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	18145
	Slot	= WEAPON_SLOT_Barrel
}
