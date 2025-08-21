class avaMod_M4A1_G_ProtoGrip extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.LateralModifier -= 0.1;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.LateralModifier -= 0.1;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.LateralModifier -= 0.1;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.LateralModifier -= 0.1;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}
defaultproperties
{
	Id = 13201
	Slot = WEAPON_SLOT_Grip
}