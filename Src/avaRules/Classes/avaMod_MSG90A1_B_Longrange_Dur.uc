class avaMod_MSG90A1_B_Longrange_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).RangeModifier += 0.03;
	avaWeap_BaseGun( Weapon ).Kickback_WhenMovingA.Upbase+=0.2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFallingA.Upbase+=0.2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.Upbase+=0.2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.Upbase+=0.2;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	18083
	Slot	= WEAPON_SLOT_Barrel
}
