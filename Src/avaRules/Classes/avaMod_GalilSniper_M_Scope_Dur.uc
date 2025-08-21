class avaMod_GalilSniper_M_Scope_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SetSightInfo( 1, 40, 0.22 );
	avaWeap_BaseGun( Weapon ).SetSightInfo( 2, 20, 0.12 );
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id = 18050
	CommonAttachedItems[0] = (MeshName="Wp_Scope.Sn_Scope.MS_Sn_Scope_001",PrimarySocket=Scope )
	Slot = WEAPON_SLOT_Mount
}