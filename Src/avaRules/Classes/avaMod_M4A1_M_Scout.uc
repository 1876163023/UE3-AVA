class avaMod_M4A1_M_Scout extends avaMod_Weapon;

// x4 ACOG

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SetSightInfo( 1, 30, 0.18 );
	avaWeap_BaseRifle( Weapon ).ScopeMeshName = default.ScopeMeshName;
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
	Id = 13100
	CommonAttachedItems[0] = (MeshName="Wp_Scope.Com_Scope.MS_ACOG",PrimarySocket=Scope )
	ScopeMeshName = "Wp_Scope.Com_Scope.MS_UIACOG"
	Slot = WEAPON_SLOT_Mount
}