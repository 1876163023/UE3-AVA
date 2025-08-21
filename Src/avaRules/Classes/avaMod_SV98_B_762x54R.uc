class avaMod_SV98_B_762x54R extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).HitDamage+=5;	
	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.Upbase+=2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.Upbase+=2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.Upbase+=2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.Upbase+=2;
	avaWeap_BaseGun( Weapon ).SpreadDecayTime+=0.1;

}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13114
	Slot	= WEAPON_SLOT_Barrel
}
