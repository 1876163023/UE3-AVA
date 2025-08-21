class avaMod_SSG550RIS_T_Trigger extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenMoving.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).Spread_WhenFalling.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).AccuracyOffsetA += 0.02;

}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13243
	Slot		= WEAPON_SLOT_Trigger
}
