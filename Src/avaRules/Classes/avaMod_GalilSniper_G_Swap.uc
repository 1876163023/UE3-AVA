class avaMod_GalilSniper_G_Swap extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).EquipTime -= 0.5;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13136
	Slot		= WEAPON_SLOT_Grip
}
