class avaMod_L96A1_T_SniperTrg extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenDuckingA.param1-=0.002;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteadyA.param1-=0.002;
	avaWeap_BaseGun( Weapon ).EquipTime += 0.2;
	avaWeap_BaseGun( Weapon ).SetSightInfo( 0, 90, 0.15 );
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13258
	Slot		= WEAPON_SLOT_Trigger
}
