class avaMod_PP2000_M_PKDot_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SetSightInfo( 1, 50, 0.15 );
	avaWeap_BaseSMG( Weapon ).ScopeMeshName = default.ScopeMeshName;
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
	Id			=	18154
	CommonAttachedItems[0] = (MeshName="Wp_Scope.Com_Scope.MS_PK-AS",PrimarySocket=Scope )
	ScopeMeshName		   = "Wp_Scope.Com_Scope.MS_UI-PK-AS"
	Slot = WEAPON_SLOT_Mount
}
