class avaMod_SSG552RIS_S_AimShot extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Kickback_WhenMovingA.UPbase-=0.4;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFallingA.Upbase-=0.4;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.Upbase-=0.4;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.Upbase-=0.4;

	avaWeap_BaseGun( Weapon ).Kickback_WhenMovingA.Lateralbase-=0.25;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFallingA.Lateralbase-=0.25;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.Lateralbase-=0.25;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.Lateralbase-=0.25;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13219
	Slot		= WEAPON_SLOT_Stock
}
