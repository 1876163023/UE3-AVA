class avaMod_SPAS12_B_SLUG extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).HitDamage += 50;
	avaWeap_BaseGun( Weapon ).NumFiresPerShot = 1;
	avaWeap_BaseGun( Weapon ).Spread_WhenMoving.param1-=0.03;
	avaWeap_BaseGun( Weapon ).Spread_WhenFalling.param1-=0.03;
	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param1-=0.03;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param1-=0.03;
	avaWeap_BaseGun( Weapon ).Penetration = 3;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	18133
	Slot	= WEAPON_SLOT_Barrel
}
