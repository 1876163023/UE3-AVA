class avaMod_TPG1_G_Human extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenMovingA.param1-=0.0015;
	avaWeap_BaseGun( Weapon ).Spread_WhenFallingA.param1-=0.0015;
	avaWeap_BaseGun( Weapon ).Spread_WhenDuckingA.param1-=0.0015;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteadyA.param1-=0.0015;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13132
	Slot		= WEAPON_SLOT_Grip
}
