class avaMod_SSG550RIS_F_LaserSight_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenMoving.param2-=0.0043;
	avaWeap_BaseGun( Weapon ).BaseSpeed-=2;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	18208
	CommonAttachedItems[0] = (MeshName="Wp_Laser.Com_Laser.MS_Com_Laser_001",PrimarySocket=Front )
	Slot = WEAPON_SLOT_Front
}
