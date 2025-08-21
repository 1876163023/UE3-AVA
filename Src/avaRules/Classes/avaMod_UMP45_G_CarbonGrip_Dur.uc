class avaMod_UMP45_G_CarbonGrip_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SetSightInfo( 0, 90, 0.07 );
	avaWeap_BaseGun( Weapon ).AccuracyDivisorA  +=  800;
	avaWeap_BaseGun( Weapon ).AccuracyOffsetA  -=  0.02;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18169
	Slot		= WEAPON_SLOT_Grip
}
