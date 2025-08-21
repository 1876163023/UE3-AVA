class avaMod_MiniUzi_T_Catridge_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).ClipCnt += 7;
	avaWeap_BaseGun( Weapon ).MaxAmmoCount += 21;
	avaWeap_BaseGun( Weapon ).UpdateReloadCnt();

	avaWeap_BaseGun( Weapon ).RangeModifier -= 0.05;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18120
	Slot		= WEAPON_SLOT_Trigger
}
