class avaMod_SV98_G_Swap extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).EquipTime -= 0.3;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13137
	Slot		= WEAPON_SLOT_Grip
}
