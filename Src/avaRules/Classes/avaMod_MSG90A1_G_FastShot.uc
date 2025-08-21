class avaMod_MSG90A1_G_FastShot extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SpreadDecayTime -= 0.3;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id = 13159
	Slot = WEAPON_SLOT_Grip
}