class avaMod_TPG1_S_Heavy_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).BaseSpeed-=3;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.Upbase-=2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.Upbase-=2;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18108
	Slot		= WEAPON_SLOT_Stock
}
