class avaMod_AK74MRail_T_Mechanism extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] -= 0.005;
	avaWeap_BaseGun( Weapon ).MaxInaccuracy += 0.15;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13122
	Slot		= WEAPON_SLOT_Trigger
}
