class avaMod_AUGA3_B_HeavyBarrel_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param1-=0.0018;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param1-=0.0018;
	avaWeap_BaseGun( Weapon ).RangeModifier += 0.02;
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] += 0.008;

	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.LateralModifier += 0.1;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.LateralModifier += 0.1;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.LateralModifier += 0.1;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.LateralModifier += 0.1;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	18196
	Slot	= WEAPON_SLOT_Barrel
}