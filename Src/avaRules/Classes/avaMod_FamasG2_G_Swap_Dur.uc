class avaMod_FamasG2_G_Swap_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).EquipTime -= 0.2;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18148
	Slot		= WEAPON_SLOT_Grip
}
