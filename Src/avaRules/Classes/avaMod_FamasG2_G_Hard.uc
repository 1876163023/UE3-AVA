class avaMod_FamasG2_G_Hard extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).KickBack_WhenMoving.UpBase -= 0.5;
	avaWeap_BaseGun( Weapon ).KickBack_WhenDucking.UpBase -= 0.5;
	avaWeap_BaseGun( Weapon ).KickBack_WhenSteady.UpBase -= 0.5;

	avaWeap_BaseGun( Weapon ).KickBack_WhenMoving.UpModifier += 0.05;
	avaWeap_BaseGun( Weapon ).KickBack_WhenDucking.UpModifier += 0.05;
	avaWeap_BaseGun( Weapon ).KickBack_WhenSteady.UpModifier += 0.05;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13184
	Slot		= WEAPON_SLOT_Grip
}
