class avaMod_MSG90A1_T_Catridge extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).ClipCnt += 15;
	avaWeap_BaseGun( Weapon ).MaxAmmoCount += 10;
	avaWeap_BaseGun( Weapon ).UpdateReloadCnt();

	avaWeap_BaseGun( Weapon ).Kickback_WhenFallingA.Lateralbase+=1.5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenMovingA.Lateralbase+=1.5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.Lateralbase+=1.5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.Lateralbase+=1.5;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13158
	Slot		= WEAPON_SLOT_Trigger
}
