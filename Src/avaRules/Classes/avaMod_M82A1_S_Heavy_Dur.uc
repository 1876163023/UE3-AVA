class avaMod_M82A1_S_Heavy_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).BaseSpeed-=5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.Upbase-=1.5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.Upbase-=1.5;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18240
	Slot		= WEAPON_SLOT_Stock
}
