class avaMod_M82A1_M_HScope extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SetSightInfo( 1, 25, 0.29 );
	avaWeap_BaseGun( Weapon ).SetSightInfo( 2, 8, 0.13 );
	avaWeap_BaseSniperRifle( Weapon ).ScopeMeshName = default.ScopeMeshName;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id = 13264
	CommonAttachedItems[0] = (MeshName="Wp_Scope.Sn_Scope.MS_TPG1_scope",PrimarySocket=Scope )
	ScopeMeshName = "avaScopeUI.Distortion.MS_TPGSniper_Scope_Mesh"
	Slot = WEAPON_SLOT_Mount
}