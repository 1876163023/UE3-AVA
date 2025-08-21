class avaMod_BizonPP19_G_9mmParam extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).HitDamage += 3;
	avaWeap_BaseGun( Weapon ).ClipCnt -= 11;
	avaWeap_BaseGun( Weapon ).MaxAmmoCount -= 22;
	avaWeap_BaseGun( Weapon ).UpdateReloadCnt();
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	13193
	Slot	= WEAPON_SLOT_Grip
}
