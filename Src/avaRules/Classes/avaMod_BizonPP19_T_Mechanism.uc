class avaMod_BizonPP19_T_Mechanism extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] -= 0.007;
	avaWeap_BaseGun( Weapon ).MaxInaccuracy += 0.15;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13120
	Slot		= WEAPON_SLOT_Trigger
}
