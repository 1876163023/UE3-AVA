class avaMod_TPG1_G_Weight extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.Upbase-=3;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.Upbase-=3;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.LateralBase-=0.3;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.Lateralbase-=0.3;
	avaWeap_BaseGun( Weapon ).BaseSpeed-=2;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13135
	Slot		= WEAPON_SLOT_Grip
}
