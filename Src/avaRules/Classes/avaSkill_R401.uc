class avaSkill_R401 extends avaCharacterModifier;

//51465	���ػ��	�⺻ ���ػ�ݼ�	Aimed Shot	Basic AimedShot
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.ChrAimSpeedPct +=0.1;
	Pawn.ChrCrouchAimSpeedPct +=0.1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 51465
}
