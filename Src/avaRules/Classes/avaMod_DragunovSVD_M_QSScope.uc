class avaMod_DragunovSVD_M_QSScope extends avaMod_Weapon;


static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SetSightInfo( 1, 30, 0.18 );
	avaWeap_BaseSniperRifle( Weapon ).ScopeMeshName = default.ScopeMeshName;
	avaWeap_BaseGun( Weapon ).bHideWeaponInSightMode = true;
	avaWeap_BaseGun( Weapon ).bHideCursorInSightMode = true;
	avaWeap_BaseGun( Weapon ).bReleaseZoomAfterFire = false;
	avaWeap_BaseGun( Weapon ).SightInAnim = 'Zoom_in';
	avaWeap_BaseGun( Weapon ).SightOutAnim = 'Zoom_out';
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id = 13161
	CommonAttachedItems[0] = (MeshName="Wp_Scope.Sn_Scope.MS_Sn_Scope_003",PrimarySocket=Scope )
	ScopeMeshName = "avaScopeUI.Distortion.MS_GalilSniper_Scope_Mesh"
	Slot = WEAPON_SLOT_Mount
}