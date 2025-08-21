class avaMod_G36Rail_B_BurstBarrel_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] -= 0.005;
	avaWeap_BaseGun( Weapon ).AccuracyOffset += 0.05;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	18047
	Slot	= WEAPON_SLOT_Barrel
}
