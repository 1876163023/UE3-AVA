class avaMod_L96A1_S_Heavy extends avaMod_Weapon;

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
	Id		= 13260
	Slot		= WEAPON_SLOT_Stock
}
