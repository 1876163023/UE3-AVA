class avaMod_DragunovSVD_B_LightBarrel extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).BaseSpeed += 3;
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] -= 0.2;
	avaWeap_BaseGun( Weapon ).RangeModifier -= 0.05;

}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	13191
	Slot	= WEAPON_SLOT_Barrel
}
