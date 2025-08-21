class avaMod_M4A1_B_Burst extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
//	avaWeap_BaseGun( Weapon ).MaxInaccuracy-= 0.25;
//	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.UpMax-=0.55;
//	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.LateralMax-=0.25;
//	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.UpMax-=0.7;
//	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.LateralMax-=0.4;
//	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.UpMax-=0.45;
//	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.LateralMax-=0.15;
//	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.UpMax-=0.475;
//	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.LateralMax-=0.175;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}
defaultproperties
{
	Id = 13057
	Slot = WEAPON_SLOT_Barrel
}