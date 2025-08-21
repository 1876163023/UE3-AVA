class avaMod_GalilSniper_S_Light extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).BaseSpeed+=5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFallingA.Lateralbase+=0.5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenMovingA.Lateralbase+=0.5;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13140
	Slot		= WEAPON_SLOT_Stock
}
