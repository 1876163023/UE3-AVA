class avaMod_PP2000_T_7N31AP extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).HitDamage += 6;
	avaWeap_BaseGun( Weapon ).AccuracyOffset += 0.005;

	avaWeap_BaseGun( Weapon ).ClipCnt -= 24;
	avaWeap_BaseGun( Weapon ).MaxAmmoCount -= 28;
	avaWeap_BaseGun( Weapon ).UpdateReloadCnt();
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18129
	Slot		= WEAPON_SLOT_Trigger
}
