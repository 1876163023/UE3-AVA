class avaMod_MSG90A1_B_SharpShooter_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenDuckingA.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteadyA.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).FireInterval[(0)] += 0.1;

}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	18084
	Slot	= WEAPON_SLOT_Barrel
}
