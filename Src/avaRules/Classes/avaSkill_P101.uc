class avaSkill_P101 extends avaCharacterModifier;

//51201	�̵���	����	Move	Running Basic
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.ChrBaseSpeedPct += 0.03;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 51201
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
