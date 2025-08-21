class avaMod_M4A1_B_Pow extends avaMod_Weapon;

/*
//static function ApplyToCharacter_Client( avaPawn Pawn );
//static function ApplyToCharacter_Server( avaPawn Pawn );
*/

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
/*
	avaWeap_BaseGun( Weapon ).HitDamage+=3;
	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.UpBase+=0.125;
	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.LateralBase+=0.045;
	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.UpModifier+=0.0225;
	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.LateralModifier+=0.005;
	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.UpMax+=0.55;
	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.LateralMax+=0.25;

	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.UpBase+=0.15;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.LateralBase+=0.08;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.UpModifier+=0.04;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.LateralModifier+=0.025;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.UpMax+=0.7;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.LateralMax+=0.4;

	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.UpBase+=0.07;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.LateralBase+=0.032;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.UpModifier+=0.015;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.LateralModifier+=0.025;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.UpMax+=0.45;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.LateralMax+=0.15;

	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.UpBase+=0.08;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.LateralBase+=0.0365;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.UpModifier+=0.075;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.LateralModifier+=0.00375;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.UpMax+=0.475;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.LateralMax+=0.175;
*/
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}
defaultproperties
{
	Id = 13059
	Slot = WEAPON_SLOT_Barrel
}