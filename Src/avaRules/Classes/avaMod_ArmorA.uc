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
//ChrBaseSpeedPct				// 기본이동속도에영향을 미치는 Character 능력치. Default : 1.0
//ChrAimSpeedPct				// 조준이동속도에...
//ChrWalkSpeedPct				// 걷기이동속도에...
//ChrCrouchSpeedPct			// 앉아이동속도에...
//ChrCrouchAimSpeedPct		// 앉아조준이동속도에...
//ChrSprintSpeedPct			// 대쉬이동속도에...
//ChrCrouchSprintSpeedPct		// 앉아대쉬이동속도에...
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

// 기본 군복이므로 방어 효과 없음
static function ApplyToCharacter_Client( avaPawn Pawn )
{
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}
