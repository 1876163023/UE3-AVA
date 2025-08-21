class avaMod_AK74MRail_B_Upgrade_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).HitDamage += 4;
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] += 0.008;
	avaWeap_BaseGun( Weapon ).KickBack_LateralLimit += 0.5;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	18008
	Slot	= WEAPON_SLOT_Barrel
}