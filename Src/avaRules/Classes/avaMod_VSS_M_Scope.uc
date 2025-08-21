class avaMod_VSS_M_Scope extends avaMod_Weapon;

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
	Id = 13249
	CommonAttachedItems[0] = (MeshName="Wp_Scope.Sn_Scope.MS_Sn_Scope_003",PrimarySocket=Scope )
	ScopeMeshName = "avaScopeUI.Distortion.MS_GalilSniper_Scope_Mesh"
	Slot = WEAPON_SLOT_Mount
}