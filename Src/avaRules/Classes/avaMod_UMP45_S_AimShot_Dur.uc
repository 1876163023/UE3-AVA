class avaMod_UMP45_S_AimShot_Dur extends avaMod_Weapon;

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
	Id		= 18118
	Slot		= WEAPON_SLOT_Stock
}
