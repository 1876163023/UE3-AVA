class avaMod_AUGA3_T_Process_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.Lateralbase-=0.3;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.Lateralbase-=0.3;
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] += 0.003;

}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18197
	Slot		= WEAPON_SLOT_Trigger
}
