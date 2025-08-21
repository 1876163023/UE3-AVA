class avaMod_AK74MRail_S_Light extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).BaseSpeed+=5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.Lateralbase+=0.2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.Lateralbase+=0.2;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13139
	Slot		= WEAPON_SLOT_Stock
}
