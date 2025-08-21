class avaMod_AUGA3_S_Light extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).BaseSpeed+=6;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.Lateralbase+=0.5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.Lateralbase+=0.5;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13229
	Slot		= WEAPON_SLOT_Stock
}
