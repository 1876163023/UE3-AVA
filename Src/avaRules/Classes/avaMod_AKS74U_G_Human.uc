class avaMod_AKS74U_G_Human extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenMoving.param1-=0.0015;
	avaWeap_BaseGun( Weapon ).Spread_WhenFalling.param1-=0.0015;
	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param1-=0.0015;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param1-=0.0015;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13173
	Slot		= WEAPON_SLOT_Grip
}
