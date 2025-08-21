class avaMod_TPG1_M_Scope_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SetSightInfo( 1, 30, 0.32 );
	avaWeap_BaseGun( Weapon ).SetSightInfo( 2, 20, 0.12 );
	avaWeap_BaseSniperRifle( Weapon ).ScopeMeshName = default.ScopeMeshName;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id = 18102
	CommonAttachedItems[0] = (MeshName="Wp_Scope.Sn_Scope.MS_Sn_Scope_001",PrimarySocket=Scope )
	ScopeMeshName = "avaScopeUI.Distortion.MS_ATN_Scope_Mesh"
	Slot = WEAPON_SLOT_Mount
}