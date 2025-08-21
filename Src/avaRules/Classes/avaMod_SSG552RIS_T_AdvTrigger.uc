class avaMod_SSG552RIS_T_AdvTrigger extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).RangeModifier -= 0.02;

	avaWeap_BaseGun( Weapon ).SpreadDecayTime -= 0.1;
	avaWeap_BaseGun( Weapon ).Kickback_Laterallimit -= 0.8;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id = 13215
	Slot = WEAPON_SLOT_Trigger
}