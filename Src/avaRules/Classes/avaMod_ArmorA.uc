class avaMod_ArmorA extends avaCharacterModifier;

//WeapTypeAmp[WEAPON_KNIFE].AmmoAmp
//WeapTypeAmp[WEAPON_PISTOL].ReloadAmp
//WeapTypeAmp[WEAPON_SMG].EquipAmp
//WeapTypeAmp[WEAPON_RIFLE].DamageAmp
//WeapTypeAmp[WEAPON_SNIPER].RangeAmp
//WeapTypeAmp[WEAPON_GRENADE].SpeedAmp
//
//WeapTypeAdd[].ZoomSpreadAdd	
//WeapTypeAdd[].UnZoomSpreadAdd
//WeapTypeAdd[].SpreadFallingAdd
//WeapTypeAdd[].SpreadMovingAdd
//WeapTypeAdd[].SpreadDuckingAdd
//WeapTypeAdd[].SpreadSteadyAdd
//
//ChrBaseSpeedPct				// �⺻�̵��ӵ��������� ��ġ�� Character �ɷ�ġ. Default : 1.0
//ChrAimSpeedPct				// �����̵��ӵ���...
//ChrWalkSpeedPct				// �ȱ��̵��ӵ���...
//ChrCrouchSpeedPct			// �ɾ��̵��ӵ���...
//ChrCrouchAimSpeedPct		// �ɾ������̵��ӵ���...
//ChrSprintSpeedPct			// �뽬�̵��ӵ���...
//ChrCrouchSprintSpeedPct		// �ɾƴ뽬�̵��ӵ���...
//
//FallingDamageAmp
//HeadDefenceRate
//MiniMapScale
//ProjectileVelAmp
//ThrowableWeapReadyAmp


defaultproperties
{
	id = 5889
	Slot = CHAR_SLOT_BD

	NRFRifleExtraMeshes(0) = (MeshName="AVA_NRF_Ammor.MS_NRF_Ammor01")
	NRFPointExtraMeshes(0) = (MeshName="AVA_NRF_Ammor.MS_NRF_Ammor01")
	NRFSniperExtraMeshes(0) = (MeshName="AVA_NRF_Ammor.MS_NRF_Ammor01")

	EURifleExtraMeshes(0) = (MeshName="AVA_EU_Ammor.MS_EU_Ammor01")
	EUPointExtraMeshes(0) = (MeshName="AVA_EU_Ammor.MS_EU_Ammor01")
	EUSniperExtraMeshes(0) = (MeshName="AVA_EU_Ammor.MS_EU_Ammor01")

				
}

// �⺻ �����̹Ƿ� ��� ȿ�� ����
static function ApplyToCharacter_Client( avaPawn Pawn )
{
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}
