class avaMod_SSG552RIS_T_AimShot_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).FireIntervalMultiplierA=1.0;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18187
	Slot		= WEAPON_SLOT_Trigger
}
