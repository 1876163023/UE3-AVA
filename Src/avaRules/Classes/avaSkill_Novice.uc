class avaSkill_Novice extends avaCharacterModifier;

// SD 1.0���� ���� ��ų
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAmp[WEAPON_RIFLE].DamageAmp += 0.15;
	Pawn.WeapTypeAmp[WEAPON_SMG].DamageAmp += 0.15;
	Pawn.WeapTypeAmp[WEAPON_SNIPER].DamageAmp += 0.08;
	Pawn.Absorb_Stomach += 0.05;
	Pawn.Armor_Stomach += 8;
	Pawn.ArmorMax += 8;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 100
}

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
