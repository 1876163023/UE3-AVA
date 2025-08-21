class avaMod_SPAS12_T_Catridge_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).ClipCnt += 3;
	avaWeap_BaseGun( Weapon ).MaxAmmoCount += 2;
	avaWeap_BaseGun( Weapon ).UpdateReloadCnt();
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18162
	Slot		= WEAPON_SLOT_Trigger
}
