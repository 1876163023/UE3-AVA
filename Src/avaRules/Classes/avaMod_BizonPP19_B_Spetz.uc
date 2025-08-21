class avaMod_BizonPP19_B_Spetz extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).BaseSpeed-=8;
	avaWeap_BaseGun( Weapon ).AccuracyDivisor +=500;
	avaWeap_BaseGun( Weapon ).MaxInaccuracy -= 1;
	avaWeap_BaseGun( Weapon ).Kickback_LateralLimit -= 1;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	13169
	Slot	= WEAPON_SLOT_Barrel
}
