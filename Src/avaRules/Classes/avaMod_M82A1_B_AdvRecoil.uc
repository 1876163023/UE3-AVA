class avaMod_M82A1_B_AdvRecoil extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Kickback_WhenMovingA.UpBase -= 3;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFallingA.UpBase -= 3;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.UpBase -= 3;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.UpBase -= 3;

	avaWeap_BaseGun( Weapon ).Kickback_WhenMovingA.LateralBase -= 1.0;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFallingA.LateralBase -= 1.0;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.LateralBase -= 1.0;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.LateralBase -= 1.0;

	avaWeap_BaseGun( Weapon ).FireInterval[(0)] += 0.1;
	avaWeap_BaseGun( Weapon ).FireInterval[(1)] += 0.1;
	avaWeap_BaseGun( Weapon ).FireInterval[(2)] += 0.1;
	avaWeap_BaseGun( Weapon ).SpreadDecayTime += 0.2;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13265
	Slot		= WEAPON_SLOT_Barrel
}
