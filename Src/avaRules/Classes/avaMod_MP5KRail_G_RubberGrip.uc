class avaMod_MP5KRail_G_RubberGrip extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
//	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param2-=0.005;
//	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param2-=0.005;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	13081
	Slot = WEAPON_SLOT_Grip
}
