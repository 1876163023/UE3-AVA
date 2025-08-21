class avaMod_PP2000_B_BurstBarrel_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] -= 0.012;
	avaWeap_BaseGun( Weapon ).AccuracyOffset += 0.03;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	18155
	Slot	= WEAPON_SLOT_Barrel
}
