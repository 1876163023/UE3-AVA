class avaSkill_P101 extends avaCharacterModifier;

//51201	이동술	러닝	Move	Running Basic
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
