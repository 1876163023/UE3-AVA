class avaMod_L96A1_B_338LUPUA extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).HitDamage+=10;
	avaWeap_BaseGun( Weapon ).Kickback_WhenMovingA.Upbase+=2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFallingA.Upbase+=2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.Upbase+=2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.Upbase+=2;
	avaWeap_BaseGun( Weapon ).SpreadDecayTime+=0.3;

}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13256
	Slot	= WEAPON_SLOT_Barrel
}
