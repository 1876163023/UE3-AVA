class avaMod_M4A1_G_Grip extends avaMod_Weapon;

/*
//static function ApplyToCharacter_Client( avaPawn Pawn );
//static function ApplyToCharacter_Server( avaPawn Pawn );
*/

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	// Grip �κ� ���͸��� ��ȭ �ʿ�
//	avaWeap_BaseGun( Weapon ).EquipTime		-=0.3;
	//avaWeap_BaseGun( Weapon ).ChagneBodySkin( "" );
	//avaWeap_BaseGun( Weapon ).ChangeStockSkin( "" );
	//avaWeap_BaseGun( Weapon ).ChangeGripSkin( "" );
	// źâ �ϳ� �ø��� ��ũ��Ʈ
	//avaWeap_BaseGun( Weapon ).MaxAmmo += avaWeap_BaseGun( Weapon ).default.ReloadCnt;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}
defaultproperties
{
	Id = 13062
	Slot = WEAPON_SLOT_Grip
}