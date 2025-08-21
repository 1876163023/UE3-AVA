class avaMod_L96A1_G_Weight extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.Upbase-=1.5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.Upbase-=1.5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.LateralBase-=0.3;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.Lateralbase-=0.3;
	avaWeap_BaseGun( Weapon ).BaseSpeed-=2;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13259
	Slot		= WEAPON_SLOT_Grip
}
