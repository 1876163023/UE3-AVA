class avaMod_UMP45_B_9mmBarrel extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).HitDamage -= 3;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.Lateralbase-=0.2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.Lateralbase-=0.2;
	avaWeap_BaseGun( Weapon ).ClipCnt += 5;
	avaWeap_BaseGun( Weapon ).MaxAmmoCount += 15;
	avaWeap_BaseGun( Weapon ).UpdateReloadCnt();
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	13109
	Slot	= WEAPON_SLOT_Barrel
}
