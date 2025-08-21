class avaMod_M4A1_B_BurstBarrel extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] -= 0.005;
	avaWeap_BaseGun( Weapon ).AccuracyOffset += 0.02;

}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	13108
	Slot	= WEAPON_SLOT_Barrel
}
