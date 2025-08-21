class avaMod_MSG90A1_B_Upgrade_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).HitDamage += 8;
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] += 0.02;
	avaWeap_BaseGun( Weapon ).KickBack_LateralLimit += 0.8;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	18174
	Slot	= WEAPON_SLOT_Barrel
}