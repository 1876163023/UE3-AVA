class avaMod_L96A1_M_HPScope extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SetSightInfo( 1, 30, 0.18 );
	avaWeap_BaseGun( Weapon ).SetSightInfo( 2, 10, 0.10 );
	avaWeap_BaseSniperRifle( Weapon ).ScopeMeshName = default.ScopeMeshName;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	13255
	CommonAttachedItems[0] = (MeshName="Wp_Scope.Sn_Scope.MS_Sn_Scope_002",PrimarySocket=Scope )
	ScopeMeshName = "avaScopeUI.Distortion.MS_ATN_Scope_Mesh"
	Slot = WEAPON_SLOT_Mount
}
