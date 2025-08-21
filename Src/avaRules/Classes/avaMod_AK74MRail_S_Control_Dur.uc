class avaMod_AK74MRail_S_Control_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Kickback_UpLimit -= 1;
	avaWeap_BaseGun( Weapon ).Kickback_LateralLimit -= 1.2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.LateralBase-=0.3;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.Upbase-=0.3;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18007
	Slot		= WEAPON_SLOT_Stock
}
