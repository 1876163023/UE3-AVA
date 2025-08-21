class avaMod_BizonPP19_T_Cylinder extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).HitDamage += 3;
	avaWeap_BaseGun( Weapon ).AccuracyOffset += 0.02;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13127
	Slot		= WEAPON_SLOT_Trigger
}
