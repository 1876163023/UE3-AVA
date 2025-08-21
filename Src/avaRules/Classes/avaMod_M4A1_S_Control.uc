class avaMod_M4A1_S_Control extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Kickback_UpLimit -= 1;
	avaWeap_BaseGun( Weapon ).Kickback_LateralLimit -= 0.5;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.LateralBase-=0.2;
	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.Upbase-=0.2;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13148
	Slot		= WEAPON_SLOT_Stock
}
