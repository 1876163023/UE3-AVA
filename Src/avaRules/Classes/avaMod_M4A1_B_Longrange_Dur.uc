class avaMod_M4A1_B_Longrange_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).RangeModifier+=0.02;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id = 18061
	Slot = WEAPON_SLOT_Barrel
}