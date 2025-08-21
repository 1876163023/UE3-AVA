class avaMod_MiniUzi_G_Silicon_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).AccuracyOffset -= 0.02;
	avaWeap_BaseGun( Weapon ).AccuracyDivisor += 300;
	avaWeap_BaseGun( Weapon ).MaxInAccuracy += 0.5;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18121
	Slot		= WEAPON_SLOT_Grip
}
