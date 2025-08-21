class avaMod_SSG552RIS_B_AdvSilencer_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).RangeModifierS += 0.05;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	18185
	Slot	= WEAPON_SLOT_Barrel
}
