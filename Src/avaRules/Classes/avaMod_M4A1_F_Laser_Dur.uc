class avaMod_M4A1_F_Laser_Dur extends avaMod_Weapon;


static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenMoving.param2-=0.004;
	avaWeap_BaseGun( Weapon ).BaseSpeed-=2;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id = 18062
	CommonAttachedItems[0] = (MeshName="Wp_Laser.Com_Laser.MS_Com_Laser_001",PrimarySocket=Front )
	Slot = WEAPON_SLOT_Front
}