class avaMod_AN94_G_AimShot extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).AccuracyDivisorA  +=  800;
	avaWeap_BaseGun( Weapon ).AccuracyOffsetA  -=  0.02;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18137
	Slot		= WEAPON_SLOT_Grip
}
