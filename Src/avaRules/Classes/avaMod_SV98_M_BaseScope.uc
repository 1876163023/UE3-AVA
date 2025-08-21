class avaMod_SV98_M_BaseScope extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SetSightInfo( 1, 40, 0.20 );
	avaWeap_BaseGun( Weapon ).SetSightInfo( 2, 20, 0.12 );
	avaWeap_BaseSniperRifle( Weapon ).ScopeMeshName = default.ScopeMeshName;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	13074
	CommonAttachedItems[0] = (MeshName="Wp_Scope.Sn_Scope.MS_Sn_Scope_001",PrimarySocket=Scope )
	ScopeMeshName = "avaScopeUI.Distortion.MS_GalilSniper_Scope_Mesh"
	Slot = WEAPON_SLOT_Mount
}
