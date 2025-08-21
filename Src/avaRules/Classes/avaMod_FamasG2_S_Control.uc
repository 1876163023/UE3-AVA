class avaMod_FamasG2_S_Control extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Kickback_UpLimit -= 1;
	avaWeap_BaseGun( Weapon ).Kickback_LateralLimit -= 0.5;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13186
	Slot		= WEAPON_SLOT_Stock
}
