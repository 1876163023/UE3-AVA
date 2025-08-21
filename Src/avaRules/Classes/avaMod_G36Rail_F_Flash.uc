class avaMod_G36Rail_F_Flash extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	//avaWeap_BaseGun( Weapon ).bEnableFlashLight = true;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	13084
	/*CommonAttachedItems[0] = (MeshName="Wp_Flash.Flash01.MS_Flash01",PrimarySocket=Front )*/
	Slot = WEAPON_SLOT_Front
}
