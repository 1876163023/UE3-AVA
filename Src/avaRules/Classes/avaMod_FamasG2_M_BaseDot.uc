class avaMod_FamasG2_M_BaseDot extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SetSightInfo( 1, 60, 0.15 );
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
	Id			=	13177

	CommonAttachedItems[0] = (MeshName="Wp_Scope.Com_Scope.MS_Com_Scope_001",PrimarySocket=Scope )
	ScopeMeshName		   = "Wp_Scope.Com_Scope.MS_UIDotsight01";	
	Slot = WEAPON_SLOT_Mount
}
