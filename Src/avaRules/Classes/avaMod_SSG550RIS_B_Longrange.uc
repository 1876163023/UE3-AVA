class avaMod_SSG550RIS_B_Longrange extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).RangeModifier += 0.05;
	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.Upbase+=0.2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.Upbase+=0.2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.Upbase+=0.2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.Upbase+=0.2;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	13241
	Slot	= WEAPON_SLOT_Barrel
}
