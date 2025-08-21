class avaMod_MiniUzi_G_Silicon extends avaMod_Weapon;

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
	Id		= 13129
	Slot		= WEAPON_SLOT_Grip
}
