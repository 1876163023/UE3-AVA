class avaMod_G36Rail_G_Hard extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).KickBack_WhenMoving.UpBase -= 0.5;
	avaWeap_BaseGun( Weapon ).KickBack_WhenDucking.UpBase -= 0.5;
	avaWeap_BaseGun( Weapon ).KickBack_WhenSteady.UpBase -= 0.5;
	avaWeap_BaseGun( Weapon ).KickBack_WhenMoving.LateralBase -= 0.2;
	avaWeap_BaseGun( Weapon ).KickBack_WhenDucking.LateralBase -= 0.2;
	avaWeap_BaseGun( Weapon ).KickBack_WhenSteady.LateralBase -= 0.2;

	avaWeap_BaseGun( Weapon ).KickBack_WhenMoving.UpModifier += 0.08;
	avaWeap_BaseGun( Weapon ).KickBack_WhenDucking.UpModifier += 0.08;
	avaWeap_BaseGun( Weapon ).KickBack_WhenSteady.UpModifier += 0.08;

	avaWeap_BaseGun( Weapon ).KickBack_WhenMoving.LateralModifier += 0.12;
	avaWeap_BaseGun( Weapon ).KickBack_WhenDucking.LateralModifier += 0.12;
	avaWeap_BaseGun( Weapon ).KickBack_WhenSteady.LateralModifier += 0.12;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13175
	Slot		= WEAPON_SLOT_Grip
}
