class avaMod_GalilSniper_B_SharpShooter extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param1-=0.003;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param1-=0.003;
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] += 0.005;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	13202
	Slot	= WEAPON_SLOT_Barrel
}
