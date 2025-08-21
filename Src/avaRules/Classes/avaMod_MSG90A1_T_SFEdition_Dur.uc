class avaMod_MSG90A1_T_SFEdition_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).BaseSpeed -= 8;

	avaWeap_BaseGun( Weapon ).AccuracyDivisorA += 600;

	avaWeap_BaseGun( Weapon ).KickBack_WhenMovingA.LateralBase += 0.8;
	avaWeap_BaseGun( Weapon ).KickBack_WhenDuckingA.LateralBase += 0.8;
	avaWeap_BaseGun( Weapon ).KickBack_WhenSteadyA.LateralBase += 0.8;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18092
	Slot		= WEAPON_SLOT_Trigger
}
