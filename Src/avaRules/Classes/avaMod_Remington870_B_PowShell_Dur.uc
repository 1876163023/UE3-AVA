class avaMod_Remington870_B_PowShell_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).NumFiresPerShot +=1;

}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}
defaultproperties
{
	Id = 18171
	Slot = WEAPON_SLOT_Barrel
}