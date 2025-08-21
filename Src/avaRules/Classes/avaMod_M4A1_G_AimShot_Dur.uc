class avaMod_M4A1_G_AimShot_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).AccuracyDivisorA  +=  1000;
	avaWeap_BaseGun( Weapon ).AccuracyOffsetA  -=  0.02;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18070
	Slot		= WEAPON_SLOT_Grip
}
