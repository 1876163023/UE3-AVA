class avaMod_AK74MRail_G_Weight extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.Upbase-=0.5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.Upbase-=0.5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.LateralBase-=0.3;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.Lateralbase-=0.3;
	avaWeap_BaseGun( Weapon ).BaseSpeed-=2;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13133
	Slot		= WEAPON_SLOT_Grip
}
