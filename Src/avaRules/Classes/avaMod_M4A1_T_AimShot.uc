class avaMod_M4A1_T_AimShot extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).FireIntervalMultiplierA=1.0;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13196
	Slot		= WEAPON_SLOT_Trigger
}
